//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cview.h"
#include "cdrawcontext.h"
#include "cbitmap.h"
#include "cframe.h"
#include "cvstguitimer.h"
#include "idatapackage.h"
#include "animation/animator.h"
#include "../uidescription/icontroller.h"
#include <assert.h>
#if DEBUG
#include <list>
#include <typeinfo>
#endif

namespace VSTGUI {

/// @cond ignore
#define VSTGUI_CHECK_VIEW_RELEASING	DEBUG
#if VSTGUI_CHECK_VIEW_RELEASING
	std::list<CView*>gViewList;
	int32_t gNbCView = 0;

	//-----------------------------------------------------------------------------
	class AllocatedViews
	{
	public:
		AllocatedViews () {}
		~AllocatedViews ()
		{
			if (gNbCView > 0)
			{
				DebugPrint ("Warning: There are %d unreleased CView objects.\n", gNbCView);
				std::list<CView*>::iterator it = gViewList.begin ();
				while (it != gViewList.end ())
				{
					CView* view = (*it);
					DebugPrint ("%s\n", typeid(view).name ());
					it++;
				}
			}
		}
	};

#endif // DEBUG

typedef std::map<CViewAttributeID, CViewAttributeEntry*>::const_iterator CViewAttributeConstIterator;
typedef std::map<CViewAttributeID, CViewAttributeEntry*>::iterator CViewAttributeIterator;
//-----------------------------------------------------------------------------
class CViewAttributeEntry
{
public:
	CViewAttributeEntry (int32_t _size, const void* _data)
	: size (0)
	, data (0)
	{
		updateData (_size, _data);
	}

	~CViewAttributeEntry ()
	{
		if (data)
			free (data);
	}

	int32_t getSize () const { return size; }
	const void* getData () const { return data; }

	void updateData (int32_t _size, const void* _data)
	{
		if (data && size != _size)
		{
			free (data);
			data = 0;
		}
		size = _size;
		if (size)
		{
			if (data == 0)
				data = malloc (_size);
			memcpy (data, _data, size);
		}
	}

#if VSTGUI_RVALUE_REF_SUPPORT
	CViewAttributeEntry (CViewAttributeEntry&& me)
	: size (0)
	, data (0)
	{
		*this = std::move (me);
	}

	CViewAttributeEntry& operator=(CViewAttributeEntry&& me)
	{
		size = me.size;
		data = me.data;
		me.size = 0;
		me.data = nullptr;
		return *this;
	}
#endif

protected:
	int32_t size;
	void* data;
};

//-----------------------------------------------------------------------------
class IdleViewUpdater : public CBaseObject
{
public:
	static void add (CView* view)
	{
		if (gInstance == 0)
			new IdleViewUpdater ();
		gInstance->views.push_back (view);
	}

	static void remove (CView* view)
	{
		if (gInstance)
		{
			gInstance->views.remove (view);
			if (gInstance->views.empty ())
			{
				gInstance->forget ();
			}
		}
	}

protected:
	IdleViewUpdater ()
	{
		gInstance = this;
		timer = new CVSTGUITimer (this, 1000/CView::idleRate);
		timer->start ();
	}

	~IdleViewUpdater ()
	{
		timer->forget ();
		gInstance = 0;
	}

	CMessageResult notify (CBaseObject* sender, IdStringPtr message)
	{
		CBaseObjectGuard guard (this);
		for (std::list<CView*>::const_iterator it = views.begin (); it != views.end ();)
		{
			CView* view = (*it);
			it++;
			view->onIdle ();
		}
		return kMessageNotified;
	}
	CVSTGUITimer* timer;
	std::list<CView*> views;

	static IdleViewUpdater* gInstance;
};
IdleViewUpdater* IdleViewUpdater::gInstance = 0;
int32_t CView::idleRate = 30;
/// @endcond

UTF8StringPtr kDegreeSymbol		= "\xC2\xB0";
UTF8StringPtr kInfiniteSymbol		= "\xE2\x88\x9E";
UTF8StringPtr kCopyrightSymbol	= "\xC2\xA9";
UTF8StringPtr kTrademarkSymbol	= "\xE2\x84\xA2";
UTF8StringPtr kRegisteredSymbol	= "\xC2\xAE";
UTF8StringPtr kMicroSymbol		= "\xC2\xB5";
UTF8StringPtr kPerthousandSymbol	= "\xE2\x80\xB0";

//-----------------------------------------------------------------------------
IdStringPtr kMsgViewSizeChanged = "kMsgViewSizeChanged";

bool CView::kDirtyCallAlwaysOnMainThread = false;

//-----------------------------------------------------------------------------
// CView
//-----------------------------------------------------------------------------
CView::CView (const CRect& size)
: size (size)
, mouseableArea (size)
, pParentFrame (0)
, pParentView (0)
, pBackground (0)
, autosizeFlags (kAutosizeNone)
, alphaValue (1.f)
{
	#if VSTGUI_CHECK_VIEW_RELEASING
	static AllocatedViews allocatedViews;
	gNbCView++;
	gViewList.push_back (this);
	#endif

	viewFlags = kMouseEnabled | kVisible;
}

//-----------------------------------------------------------------------------
CView::CView (const CView& v)
: size (v.size)
, mouseableArea (v.mouseableArea)
, pParentFrame (0)
, pParentView (0)
, pBackground (v.pBackground)
, viewFlags (v.viewFlags)
, autosizeFlags (v.autosizeFlags)
, alphaValue (v.alphaValue)
{
	for (CViewAttributeIterator it = attributes.begin (); it != attributes.end (); it++)
		setAttribute (it->first, it->second->getSize (), it->second->getData ());
}

//-----------------------------------------------------------------------------
CView::~CView ()
{
	assert (isAttached () == false);

	IController* controller = 0;
	int32_t size = sizeof (IController*);
	if (getAttribute (kCViewControllerAttribute, sizeof (IController*), &controller, size) == true)
	{
		CBaseObject* obj = dynamic_cast<CBaseObject*> (controller);
		if (obj)
			obj->forget ();
		else
			delete controller;
	}

	for (CViewAttributeIterator it = attributes.begin (); it != attributes.end (); it++)
		delete it->second;

	#if VSTGUI_CHECK_VIEW_RELEASING
	gNbCView--;
	gViewList.remove (this);
	#endif
}

//-----------------------------------------------------------------------------
void CView::setMouseEnabled (bool state)
{
	if (getMouseEnabled () != state)
	{
		if (state)
			viewFlags |= kMouseEnabled;
		else
			viewFlags &= ~kMouseEnabled;

		if (pDisabledBackground)
		{
			setDirty (true);
		}
	}
}

//-----------------------------------------------------------------------------
void CView::setTransparency (bool state)
{
	if (getTransparency() != state)
	{
		if (state)
			viewFlags |= kTransparencyEnabled;
		else
			viewFlags &= ~kTransparencyEnabled;
		setDirty (true);
	}
}

//-----------------------------------------------------------------------------
void CView::setWantsFocus (bool state)
{
	if (state)
		viewFlags |= kWantsFocus;
	else
		viewFlags &= ~kWantsFocus;
}

//-----------------------------------------------------------------------------
void CView::setWantsIdle (bool state)
{
	if (wantsIdle () == state)
		return;
	if (state)
	{
		viewFlags |= kWantsIdle;
		if (isAttached ())
			IdleViewUpdater::add (this);
	}
	else
	{
		viewFlags &= ~kWantsIdle;
		if (isAttached ())
			IdleViewUpdater::remove (this);
	}
}

//-----------------------------------------------------------------------------
void CView::setDirty (bool state)
{
	if (kDirtyCallAlwaysOnMainThread)
	{
		if (state)
			invalidRect (size);
		viewFlags &= ~kDirty;
	}
	else
	{
		if (state)
			viewFlags |= kDirty;
		else
			viewFlags &= ~kDirty;
	}
}

//-----------------------------------------------------------------------------
/**
 * @param parent parent view
 * @return true if view successfully attached to parent
 */
bool CView::attached (CView* parent)
{
	if (isAttached ())
		return false;
	assert (dynamic_cast<CViewContainer*> (parent) != 0);
	pParentView = parent;
	pParentFrame = parent->getFrame ();
	viewFlags |= kIsAttached;
	if (pParentFrame)
		pParentFrame->onViewAdded (this);
	if (wantsIdle ())
		IdleViewUpdater::add (this);
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param parent parent view
 * @return true if view successfully removed from parent
 */
bool CView::removed (CView* parent)
{
	if (!isAttached ())
		return false;
	if (wantsIdle ())
		IdleViewUpdater::remove (this);
	if (pParentFrame)
		pParentFrame->onViewRemoved (this);
	pParentView = 0;
	pParentFrame = 0;
	viewFlags &= ~kIsAttached;
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param where mouse location of mouse down
 * @param buttons button and modifier state
 * @return event result. see #CMouseEventResult
 */
CMouseEventResult CView::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
/**
 * @param where mouse location of mouse up
 * @param buttons button and modifier state
 * @return event result. see #CMouseEventResult
 */
CMouseEventResult CView::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
/**
 * @param where mouse location of mouse move
 * @param buttons button and modifier state
 * @return event result. see #CMouseEventResult
 */
CMouseEventResult CView::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
CMouseEventResult CView::onMouseCancel ()
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
/**
 * @param path the path to use for hit testing. The path will be translated by this views origin, so that the path must not be set again, if the view is moved. Otherwise when the size of the view changes, the path must also be set again.
 */
void CView::setHitTestPath (CGraphicsPath* path)
{
	pHitTestPath = path;
}

//-----------------------------------------------------------------------------
/**
 * @param where location
 * @param buttons button and modifier state
 * @return true if point hits this view
 */
bool CView::hitTest (const CPoint& where, const CButtonState& buttons)
{
	if (pHitTestPath)
	{
		CPoint p (where);
		p.offset (-getViewSize ().left, -getViewSize ().top);
		return pHitTestPath->hitTest (p);
	}
	return where.isInside (mouseableArea);
}

//-----------------------------------------------------------------------------
/**
 * @param point location
 * @return converted point
 */
CPoint& CView::frameToLocal (CPoint& point) const
{
	if (pParentView)
		return pParentView->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
/**
 * @param point location
 * @return converted point
 */
CPoint& CView::localToFrame (CPoint& point) const
{
	if (pParentView)
		return pParentView->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
/**
 * @param rect rect to invalidate
 */
void CView::invalidRect (const CRect& rect)
{
	if (isAttached () && viewFlags & kVisible)
	{
		if (pParentView)
			pParentView->invalidRect (rect);
		else if (pParentFrame)
			pParentFrame->invalidRect (rect);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pContext draw context in which to draw
 */
void CView::draw (CDrawContext* pContext)
{
	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, size);
	}
	setDirty (false);
}

//-----------------------------------------------------------------------------
/**
 * @param where location
 * @param distance wheel distance
 * @param buttons button and modifier state
 * @return true if handled
 */
bool CView::onWheel (const CPoint &where, const float &distance, const CButtonState &buttons)
{
	return false;
}

//------------------------------------------------------------------------
/**
 * @param where location
 * @param axis mouse wheel axis
 * @param distance wheel distance
 * @param buttons button and modifier state
 * @return true if handled
 */
bool CView::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	if (axis == kMouseWheelAxisX)
	{
		#if MAC	// mac os x 10.4.x swaps the axis if the shift modifier is down
		if (!(buttons & kShift))
		#endif
		return onWheel (where, distance*-1, buttons);
	}
	return onWheel (where, distance, buttons);
}

//------------------------------------------------------------------------------
/**
 * @param keyCode key code of pressed key
 * @return -1 if not handled and 1 if handled
 */
int32_t CView::onKeyDown (VstKeyCode& keyCode)
{
	return -1;
}

//------------------------------------------------------------------------------
/**
 * @param keyCode key code of pressed key
 * @return -1 if not handled and 1 if handled
 */
int32_t CView::onKeyUp (VstKeyCode& keyCode)
{
	return -1;
}

//------------------------------------------------------------------------------
/**
 * a drag can only be started from within onMouseDown
 * @param source source drop
 * @param offset bitmap offset
 * @param dragBitmap bitmap to drag
 * @return 0 on failure, negative if source was moved and positive if source was copied
 */
CView::DragResult CView::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	CFrame* frame = getFrame ();
	if (frame)
	{
		return frame->doDrag (source, offset, dragBitmap);
	}
	return kDragError;
}

//------------------------------------------------------------------------------
/**
 * @param sender message sender
 * @param message message text
 * @return message handled or not. See #CMessageResult
 */
CMessageResult CView::notify (CBaseObject* sender, IdStringPtr message)
{
	return kMessageUnknown;
}

//------------------------------------------------------------------------------
void CView::looseFocus ()
{}

//------------------------------------------------------------------------------
void CView::takeFocus ()
{}

//------------------------------------------------------------------------------
/**
 * @param newSize rect of new size of view
 * @param invalid if true set view dirty
 */
void CView::setViewSize (const CRect& newSize, bool invalid)
{
	if (size != newSize)
	{
		size = newSize;
		if (invalid)
			setDirty ();
		if (getParentView ())
			getParentView ()->notify (this, kMsgViewSizeChanged);
	}
}

//------------------------------------------------------------------------------
/**
 * @return visible size of view
 */
CRect CView::getVisibleViewSize () const
{
	if (pParentView)
		return static_cast<CViewContainer*>(pParentView)->getVisibleSize (size);
	else if (pParentFrame)
		return pParentFrame->getVisibleSize (size);
	return CRect (0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
void CView::setVisible (bool state)
{
	if ((viewFlags & kVisible) ? true : false != state)
	{
		if (state)
		{
			viewFlags |= kVisible;
			invalid ();
		}
		else
		{
			invalid ();
			viewFlags &= ~kVisible;
		}
	}
}

//-----------------------------------------------------------------------------
void CView::setAlphaValue (float alpha)
{
	if (alphaValue != alpha)
	{
		alphaValue = alpha;
		// we invalidate the parent to make sure that when alpha == 0 that a redraw occurs
		if (pParentView)
			pParentView->invalidRect (getViewSize ());
	}
}

//-----------------------------------------------------------------------------
VSTGUIEditorInterface* CView::getEditor () const
{
	return pParentFrame ? pParentFrame->getEditor () : 0;
}

//-----------------------------------------------------------------------------
/**
 * @param background new background bitmap
 */
void CView::setBackground (CBitmap* background)
{
	pBackground = background;
	if (getMouseEnabled () == true)
		setDirty (true);
}

//-----------------------------------------------------------------------------
/**
 * @param background new disabled background bitmap
 */
void CView::setDisabledBackground (CBitmap* background)
{
	pDisabledBackground = background;
	if (getMouseEnabled () == false)
		setDirty (true);
}

//-----------------------------------------------------------------------------
const CViewAttributeID kCViewAttributeReferencePointer = 'cvrp';
const CViewAttributeID kCViewTooltipAttribute = 'cvtt';
const CViewAttributeID kCViewControllerAttribute = 'ictr';

//-----------------------------------------------------------------------------
/**
 * @param aId the ID of the Attribute
 * @param outSize on return the size of the attribute
 * @return true if attribute exists. outSize is valid then.
 */
bool CView::getAttributeSize (const CViewAttributeID aId, int32_t& outSize) const
{
	CViewAttributeConstIterator it = attributes.find (aId);
	if (it != attributes.end ())
	{
		outSize = it->second->getSize ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param aId the ID of the Attribute
 * @param inSize the size of the outData pointer
 * @param outData a pointer where to copy the attribute data
 * @param outSize the size in bytes which was copied into outData
 * @return true if attribute exists and outData was big enough. outSize and outData is valid then.
 */
bool CView::getAttribute (const CViewAttributeID aId, const int32_t inSize, void* outData, int32_t& outSize) const
{
	CViewAttributeConstIterator it = attributes.find (aId);
	if (it != attributes.end ())
	{
		if (inSize >= it->second->getSize ())
		{
			outSize = it->second->getSize ();
			if (outSize > 0)
				memcpy (outData, it->second->getData (), outSize);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * copies data into the attribute. If it does not exist, creates a new attribute.
 * @param aId the ID of the Attribute
 * @param inSize the size of the outData pointer
 * @param inData a pointer to the data
 * @return true if attribute was set
 */
bool CView::setAttribute (const CViewAttributeID aId, const int32_t inSize, const void* inData)
{
	if (inData == 0 || inSize <= 0)
		return false;
	CViewAttributeConstIterator it = attributes.find (aId);
	if (it != attributes.end ())
		it->second->updateData (inSize, inData);
	else
		attributes.insert (std::pair<CViewAttributeID, CViewAttributeEntry*> (aId, new CViewAttributeEntry (inSize, inData)));
	return true;
}

//-----------------------------------------------------------------------------
bool CView::removeAttribute (const CViewAttributeID aId)
{
	CViewAttributeConstIterator it = attributes.find (aId);
	if (it != attributes.end ())
	{
		delete it->second;
		attributes.erase (aId);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CView::addAnimation (IdStringPtr name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction, CBaseObject* notificationObject)
{
	if (getFrame ())
	{
		getFrame ()->getAnimator ()->addAnimation (this, name, target, timingFunction, notificationObject);
	}
}

//-----------------------------------------------------------------------------
void CView::removeAnimation (IdStringPtr name)
{
	if (getFrame ())
	{
		getFrame ()->getAnimator ()->removeAnimation (this, name);
	}
}

//-----------------------------------------------------------------------------
void CView::removeAllAnimations ()
{
	if (getFrame ())
	{
		getFrame ()->getAnimator ()->removeAnimations (this);
	}
}

#if DEBUG
//-----------------------------------------------------------------------------
void CView::dumpInfo ()
{
	CRect viewRect = getViewSize (viewRect);
	DebugPrint ("left:%4d, top:%4d, width:%4d, height:%4d ", viewRect.left, viewRect.top, viewRect.getWidth (), viewRect.getHeight ());
	if (getMouseEnabled ())
		DebugPrint ("(Mouse Enabled) ");
	if (getTransparency ())
		DebugPrint ("(Transparent) ");
	CRect mouseRect = getMouseableArea (mouseRect);
	if (mouseRect != viewRect)
		DebugPrint (" (Mouseable Area: left:%4d, top:%4d, width:%4d, height:%4d ", mouseRect.left, mouseRect.top, mouseRect.getWidth (), mouseRect.getHeight ());
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CDragContainerHelper::CDragContainerHelper (IDataPackage* drag)
: drag (drag)
, index (0)
{
	
}

//-----------------------------------------------------------------------------
void* CDragContainerHelper::first (int32_t& outSize, int32_t& outType)
{
	index = 0;
	return next (outSize, outType);
}

//-----------------------------------------------------------------------------
void* CDragContainerHelper::next (int32_t& outSize, int32_t& outType)
{
	IDataPackage::Type type;
	const void* data = 0;
	outSize = drag->getData (index, data, type);
	switch (type)
	{
		case IDataPackage::kFilePath:
		{
			outType = kFile;
			break;
		}
		case IDataPackage::kText:
		{
			outType = kUnicodeText;
			break;
		}
		case IDataPackage::kBinary:
		{
			outType = kUnknown;
			break;
		}
		case IDataPackage::kError:
		{
			outType = kError;
			break;
		}
	}
	index++;
	return const_cast<void*>(data);
}

//-----------------------------------------------------------------------------
int32_t CDragContainerHelper::getType (int32_t idx) const
{
	int32_t outType;
	IDataPackage::Type type = drag->getDataType (idx);
	switch (type)
	{
		case IDataPackage::kFilePath:
		{
			outType = kFile;
			break;
		}
		case IDataPackage::kText:
		{
			outType = kUnicodeText;
			break;
		}
		case IDataPackage::kBinary:
		{
			outType = kUnknown;
			break;
		}
		case IDataPackage::kError:
		{
			outType = kError;
			break;
		}
	}
	return outType;
}

//-----------------------------------------------------------------------------
int32_t CDragContainerHelper::getCount () const
{
	return drag->getCount ();
}

} // namespace
