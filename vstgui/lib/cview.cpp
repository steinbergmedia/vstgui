//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
#include "animation/animator.h"
#if DEBUG
#include <list>
#include <typeinfo>
#endif

namespace VSTGUI {

/// @cond ignore
#define VSTGUI_CHECK_VIEW_RELEASING	DEBUG
#if VSTGUI_CHECK_VIEW_RELEASING
	std::list<CView*>gViewList;
	long gNbCView = 0;

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
					DebugPrint ("%s\n", view->getClassName ());
					it++;
				}
			}
		}
	};

#endif // DEBUG

//-----------------------------------------------------------------------------
class CAttributeListEntry
{
public:
	CAttributeListEntry (long size, CViewAttributeID id)
	: nextEntry (0)
	, pointer (0)
	, sizeOfPointer (size)
	, id (id)
	{
		pointer = malloc (size);
	}

	~CAttributeListEntry ()
	{
		if (pointer)
			free (pointer);
	}

	const CViewAttributeID getID () const { return id; }
	const long getSize () const { return sizeOfPointer; }
	void* getPointer () const { return pointer; }
	CAttributeListEntry* getNext () const { return nextEntry; }
	
	void setNext (CAttributeListEntry* entry) { nextEntry = entry; }

	void resize (long newSize)
	{
		if (pointer)
			free (pointer);
		if (newSize > 0)
			pointer = malloc (newSize);
		else
			pointer = 0;
		sizeOfPointer = newSize;
	}

protected:
	CAttributeListEntry () : nextEntry (0), pointer (0), sizeOfPointer (0), id (0) {}

	CAttributeListEntry* nextEntry;
	void* pointer;
	long sizeOfPointer;
	CViewAttributeID id;
};
/// @endcond

const char* kDegreeSymbol		= "\xC2\xB0";
const char* kInfiniteSymbol		= "\xE2\x88\x9E";
const char* kCopyrightSymbol	= "\xC2\xA9";
const char* kTrademarkSymbol	= "\xE2\x84\xA2";
const char* kRegisteredSymbol	= "\xC2\xAE";
const char* kMicroSymbol		= "\xC2\xB5";
const char* kPerthousandSymbol	= "\xE2\x80\xB0";

//-----------------------------------------------------------------------------
const char* kMsgViewSizeChanged = "kMsgViewSizeChanged";
//-----------------------------------------------------------------------------
// CView
//-----------------------------------------------------------------------------
CView::CView (const CRect& size)
: size (size)
, mouseableArea (size)
, pParentFrame (0)
, pParentView (0)
, bDirty (false)
, bMouseEnabled (true)
, bTransparencyEnabled (false)
, bWantsFocus (false)
, bIsAttached (false)
, bVisible (true)
, pBackground (0)
, pAttributeList (0)
, autosizeFlags (kAutosizeNone)
, alphaValue (1.f)
{
	#if VSTGUI_CHECK_VIEW_RELEASING
	static AllocatedViews allocatedViews;
	gNbCView++;
	gViewList.push_back (this);
	#endif
}

//-----------------------------------------------------------------------------
CView::CView (const CView& v)
: size (v.size)
, mouseableArea (v.mouseableArea)
, pParentFrame (0)
, pParentView (0)
, bDirty (false)
, bMouseEnabled (v.bMouseEnabled)
, bTransparencyEnabled (v.bTransparencyEnabled)
, bWantsFocus (v.bWantsFocus)
, bIsAttached (false)
, bVisible (true)
, pBackground (v.pBackground)
, pAttributeList (0)
, autosizeFlags (v.autosizeFlags)
, alphaValue (v.alphaValue)
{
	if (pBackground)
		pBackground->remember ();
	if (v.pAttributeList)
	{
		CAttributeListEntry* entry = v.pAttributeList;
		while (entry)
		{
			setAttribute (entry->getID (), entry->getSize (), entry->getPointer ());
			entry = entry->getNext ();
		}
	}
}

//-----------------------------------------------------------------------------
CView::~CView ()
{
	if (pBackground)
		pBackground->forget ();

	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			CAttributeListEntry* nextEntry = entry->getNext ();
			delete entry;
			entry = nextEntry;
		}
	}
	#if VSTGUI_CHECK_VIEW_RELEASING
	gNbCView--;
	gViewList.remove (this);
	#endif
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
	pParentView = parent;
	pParentFrame = parent->getFrame ();
	bIsAttached = true;
	if (pParentFrame)
		pParentFrame->onViewAdded (this);
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
	if (pParentFrame)
		pParentFrame->onViewRemoved (this);
	pParentView = 0;
	pParentFrame = 0;
	bIsAttached = false;
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param where mouse location of mouse down
 * @param buttons button and modifier state
 * @return event result. see #CMouseEventResult
 */
CMouseEventResult CView::onMouseDown (CPoint &where, const long& buttons)
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
/**
 * @param where mouse location of mouse up
 * @param buttons button and modifier state
 * @return event result. see #CMouseEventResult
 */
CMouseEventResult CView::onMouseUp (CPoint &where, const long& buttons)
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
/**
 * @param where mouse location of mouse move
 * @param buttons button and modifier state
 * @return event result. see #CMouseEventResult
 */
CMouseEventResult CView::onMouseMoved (CPoint &where, const long& buttons)
{
	return kMouseEventNotImplemented;
}

//-----------------------------------------------------------------------------
/**
 * @param point location
 * @return converted point
 */
CPoint& CView::frameToLocal (CPoint& point) const
{
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
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
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
/**
 * @param rect rect to invalidate
 */
void CView::invalidRect (CRect rect)
{
	if (bIsAttached && bVisible)
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
	if (pBackground)
	{
		pBackground->draw (pContext, size);
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
bool CView::onWheel (const CPoint &where, const float &distance, const long &buttons)
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
bool CView::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const long& buttons)
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
long CView::onKeyDown (VstKeyCode& keyCode)
{
	return -1;
}

//------------------------------------------------------------------------------
/**
 * @param keyCode key code of pressed key
 * @return -1 if not handled and 1 if handled
 */
long CView::onKeyUp (VstKeyCode& keyCode)
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
long CView::doDrag (CDropSource* source, const CPoint& offset, CBitmap* dragBitmap)
{
	CFrame* frame = getFrame ();
	if (frame)
	{
		CPoint off (offset);
		localToFrame (off);
		return frame->doDrag (source, off, dragBitmap);
	}
	return 0;
}

//------------------------------------------------------------------------------
/**
 * @param sender message sender
 * @param message message text
 * @return message handled or not. See #CMessageResult
 */
CMessageResult CView::notify (CBaseObject* sender, const char* message)
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
void CView::setViewSize (CRect &newSize, bool invalid)
{
	size = newSize;
	if (invalid)
		setDirty ();
	if (getParentView ())
		getParentView ()->notify (this, kMsgViewSizeChanged);
}

//------------------------------------------------------------------------------
/**
 * @return visible size of view
 */
CRect CView::getVisibleSize () const
{
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return ((CViewContainer*)pParentView)->getVisibleSize (size);
	else if (pParentFrame)
		return pParentFrame->getVisibleSize (size);
	return CRect (0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
void CView::setVisible (bool state)
{
	if (state != bVisible)
	{
		bVisible = state;
		setDirty ();
	}
}

//-----------------------------------------------------------------------------
void CView::setAlphaValue (float alpha)
{
	if (alphaValue != alpha)
	{
		alphaValue = alpha;
		invalid ();
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
	if (pBackground)
		pBackground->forget ();
	pBackground = background;
	if (pBackground)
		pBackground->remember ();
	setDirty (true);
}

//-----------------------------------------------------------------------------
const CViewAttributeID kCViewAttributeReferencePointer = 'cvrp';
const CViewAttributeID kCViewTooltipAttribute = 'cvtt';

//-----------------------------------------------------------------------------
/**
 * @param id the ID of the Attribute
 * @param outSize on return the size of the attribute
 * @return true if attribute exists. outSize is valid then.
 */
bool CView::getAttributeSize (const CViewAttributeID id, long& outSize) const
{
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			entry = entry->getNext ();
		}
		if (entry)
		{
			outSize = entry->getSize ();
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param id the ID of the Attribute
 * @param inSize the size of the outData pointer
 * @param outData a pointer where to copy the attribute data
 * @param outSize the size in bytes which was copied into outData
 * @return true if attribute exists and outData was big enough. outSize and outData is valid then.
 */
bool CView::getAttribute (const CViewAttributeID id, const long inSize, void* outData, long& outSize) const
{
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			entry = entry->getNext ();
		}
		if (entry && inSize >= entry->getSize ())
		{
			outSize = entry->getSize ();
			if (outSize > 0)
				memcpy (outData, entry->getPointer (), outSize);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * copies data into the attribute. If it does not exist, creates a new attribute.
 * @param id the ID of the Attribute
 * @param inSize the size of the outData pointer
 * @param inData a pointer to the data
 * @return true if attribute was set
 */
bool CView::setAttribute (const CViewAttributeID id, const long inSize, const void* inData)
{
	CAttributeListEntry* lastEntry = 0;
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			if (entry->getNext () == 0)
				lastEntry = entry;
			entry = entry->getNext ();
		}
		if (entry)
		{
			if (entry->getSize () < inSize)
				entry->resize (inSize);
			if (entry->getSize () >= inSize)
			{
				if (inSize > 0)
					memcpy (entry->getPointer (), inData, inSize);
				return true;
			}
			else
				return false;
		}
	}
	
	// create a new attribute
	CAttributeListEntry* newEntry = new CAttributeListEntry (inSize, id);
	memcpy (newEntry->getPointer (), inData, inSize);
	if (lastEntry)
		lastEntry->setNext (newEntry);
	else if (!pAttributeList)
		pAttributeList = newEntry;
	else
	{
		delete newEntry;
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool CView::removeAttribute (const CViewAttributeID id)
{
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		CAttributeListEntry* prevEntry = 0;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			prevEntry = entry;
			entry = entry->getNext ();
		}
		if (entry)
		{
			if (prevEntry)
				prevEntry->setNext (entry->getNext ());
			else if (entry == pAttributeList)
				pAttributeList = entry->getNext ();
			delete entry;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void CView::addAnimation (const char* name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction)
{
	if (getFrame ())
	{
		getFrame ()->getAnimator ()->addAnimation (this, name, target, timingFunction);
	}
}

//-----------------------------------------------------------------------------
void CView::removeAnimation (const char* name)
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


} // namespace
