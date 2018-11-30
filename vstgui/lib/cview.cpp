// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cview.h"
#include "cdrawcontext.h"
#include "cbitmap.h"
#include "cframe.h"
#include "cvstguitimer.h"
#include "cgraphicspath.h"
#include "dispatchlist.h"
#include "idatapackage.h"
#include "iviewlistener.h"
#include "malloc.h"
#include "animation/animator.h"
#include "../uidescription/icontroller.h"
#include "platform/iplatformframe.h"
#include <cassert>
#include <unordered_map>
#if DEBUG
#include <list>
#include <typeinfo>
#endif

namespace VSTGUI {

/// @cond ignore
//------------------------------------------------------------------------
namespace CViewInternal {

#define VSTGUI_CHECK_VIEW_RELEASING	0//DEBUG
#if VSTGUI_CHECK_VIEW_RELEASING

using ViewList = std::list<CView*>;
static ViewList gViewList;
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
			for (const auto& view : gViewList)
				DebugPrint ("%s\n", typeid(view).name ());
		}
	}
};


#endif // VSTGUI_CHECK_VIEW_RELEASING

//-----------------------------------------------------------------------------
class AttributeEntry
{
public:
	AttributeEntry (uint32_t _size, const void* _data)
	{
		updateData (_size, _data);
	}
	
	AttributeEntry (const AttributeEntry& me) = delete;
	AttributeEntry& operator= (const AttributeEntry& me) = delete;
	AttributeEntry (AttributeEntry&& me) noexcept
	{
		*this = std::move (me);
	}
	
	AttributeEntry& operator=(AttributeEntry&& me) noexcept
	{
		data = std::move (me.data);
		return *this;
	}
	
	uint32_t getSize () const { return static_cast<uint32_t> (data.size ()); }
	const void* getData () const { return data.get (); }
	
	void updateData (uint32_t _size, const void* _data)
	{
		data.allocate (_size);
		if (data.size ())
		{
			std::memcpy (data.get (), _data, data.size ());
		}
	}
	
protected:
	Buffer<int8_t> data;
};

//-----------------------------------------------------------------------------
class IdleViewUpdater
{
public:
	static void add (CView* view)
	{
		if (gInstance == nullptr)
			gInstance = std::unique_ptr<IdleViewUpdater> (new IdleViewUpdater ());
		gInstance->views.emplace_back (view);
	}
	
	static void remove (CView* view)
	{
		if (gInstance)
		{
			gInstance->views.remove (view);
			if (!gInstance->inTimer && gInstance->views.empty ())
			{
				gInstance = nullptr;
			}
		}
	}
	
protected:
	using ViewContainer = std::list<CView*>;
	
	IdleViewUpdater ()
	{
		timer = makeOwned<CVSTGUITimer> ([this] (CVSTGUITimer*) { onTimer (); }, 1000/CView::idleRate);
	}
	
	void onTimer ()
	{
		inTimer = true;
		for (ViewContainer::const_iterator it = views.begin (); it != views.end ();)
		{
			CView* view = (*it);
			++it;
			view->onIdle ();
		}
		inTimer = false;
		if (views.empty ())
			gInstance = nullptr;
	}
	SharedPointer<CVSTGUITimer> timer;
	ViewContainer views;
	bool inTimer {false};
	
	static std::unique_ptr<IdleViewUpdater> gInstance;
};
std::unique_ptr<IdleViewUpdater> IdleViewUpdater::gInstance;

} // CViewInternal

uint32_t CView::idleRate = 30;

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
const CViewAttributeID kCViewAttributeReferencePointer = 'cvrp';
const CViewAttributeID kCViewTooltipAttribute = 'cvtt';
const CViewAttributeID kCViewControllerAttribute = 'ictr';
const CViewAttributeID kCViewHitTestPathAttribute = 'cvht';
const CViewAttributeID kCViewCustomDropTarget = 'cvdt';

//-----------------------------------------------------------------------------
// CView
//-----------------------------------------------------------------------------
struct CView::Impl
{
	using ViewAttributes = std::unordered_map<CViewAttributeID, std::unique_ptr<CViewInternal::AttributeEntry>>;
	using ViewListenerDispatcher = DispatchList<IViewListener*>;
	using ViewMouseListenerDispatcher = DispatchList<IViewMouseListener*>;
	
	ViewAttributes attributes;
	std::unique_ptr<ViewListenerDispatcher> viewListeners;
	std::unique_ptr<ViewMouseListenerDispatcher> viewMouseListener;
	
	CRect size;
	CRect mouseableArea;
	int32_t viewFlags {0};
	int32_t autosizeFlags {kAutosizeNone};
	float alphaValue {1.f};
	CFrame* parentFrame {nullptr};
	CView* parentView {nullptr};
	
	SharedPointer<CBitmap> background;
	SharedPointer<CBitmap> disabledBackground;
};

//-----------------------------------------------------------------------------
CView::CView (const CRect& size)
{
	pImpl = std::unique_ptr<Impl> (new Impl ());
	pImpl->size = size;
	pImpl->mouseableArea = size;
	
	#if VSTGUI_CHECK_VIEW_RELEASING
	static CViewInternal::AllocatedViews allocatedViews;
	CViewInternal::gNbCView++;
	CViewInternal::gViewList.emplace_back (this);
	#endif

	setViewFlag (kMouseEnabled | kVisible, true);
}

//-----------------------------------------------------------------------------
CView::CView (const CView& v)
{
	pImpl = std::unique_ptr<Impl> (new Impl ());
	pImpl->size = v.pImpl->size;
	pImpl->mouseableArea = v.pImpl->mouseableArea;
	pImpl->viewFlags = v.pImpl->viewFlags;
	pImpl->autosizeFlags = v.pImpl->autosizeFlags;
	pImpl->alphaValue = v.pImpl->alphaValue;
	pImpl->background = v.pImpl->background;
	pImpl->disabledBackground = v.pImpl->disabledBackground;
	setHitTestPath (v.getHitTestPath ());

	for (auto& attribute : v.pImpl->attributes)
		setAttribute (attribute.first, attribute.second->getSize (), attribute.second->getData ());
}

//-----------------------------------------------------------------------------
CView::~CView () noexcept = default;

//-----------------------------------------------------------------------------
void CView::beforeDelete ()
{
	if (pImpl->viewListeners)
	{
		pImpl->viewListeners->forEach ([&] (IViewListener* listener) {
			listener->viewWillDelete (this);
		});
		vstgui_assert (pImpl->viewListeners->empty (), "View listeners not empty");
	}
	if (pImpl->viewMouseListener)
	{
		vstgui_assert (pImpl->viewMouseListener->empty (), "View mouse listeners not empty");
	}

	vstgui_assert (isAttached () == false, "View is still attached");

	setHitTestPath (nullptr);
	setDropTarget (nullptr);
	
	IController* controller = nullptr;
	uint32_t size = sizeof (IController*);
	if (getAttribute (kCViewControllerAttribute, sizeof (IController*), &controller, size) == true)
	{
		auto obj = dynamic_cast<IReference*> (controller);
		if (obj)
			obj->forget ();
		else
			delete controller;
	}
	
	pImpl->attributes.clear ();
	
#if VSTGUI_CHECK_VIEW_RELEASING
	CViewInternal::gNbCView--;
	CViewInternal::gViewList.remove (this);
#endif
	CBaseObject::beforeDelete ();
}

//-----------------------------------------------------------------------------
void CView::setMouseableArea (const CRect& rect)
{
	pImpl->mouseableArea = rect;
}

//-----------------------------------------------------------------------------
CRect& CView::getMouseableArea (CRect& rect) const
{
	rect = getMouseableArea ();
	return rect;
}

//-----------------------------------------------------------------------------
const CRect& CView::getMouseableArea () const
{
	return pImpl->mouseableArea;
}

//-----------------------------------------------------------------------------
/**
 * @param path the path to use for hit testing. The path will be translated by this views origin, so that the path must not be set again, if the view is moved. Otherwise when the size of the view changes, the path must also be set again.
 */
void CView::setHitTestPath (CGraphicsPath* path)
{
	if (auto p = getHitTestPath ())
	{
		p->forget ();
		removeAttribute (kCViewHitTestPathAttribute);
	}
	if (path)
	{
		path->remember ();
		setAttribute (kCViewHitTestPathAttribute, path);
	}
}

//-----------------------------------------------------------------------------
CGraphicsPath* CView::getHitTestPath () const
{
	CGraphicsPath* path = nullptr;
	if (getAttribute (kCViewHitTestPathAttribute, path))
		return path;
	return nullptr;
}

//-----------------------------------------------------------------------------
bool CView::hasViewFlag (int32_t bit) const
{
	return hasBit (pImpl->viewFlags, bit);
}

//-----------------------------------------------------------------------------
void CView::setViewFlag (int32_t bit, bool state)
{
	setBit (pImpl->viewFlags, bit, state);
}

//-----------------------------------------------------------------------------
void CView::setMouseEnabled (bool state)
{
	if (getMouseEnabled () != state)
	{
		setViewFlag (kMouseEnabled, state);

		if (pImpl->disabledBackground)
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
		setViewFlag (kTransparencyEnabled, state);
		setDirty (true);
	}
}

//-----------------------------------------------------------------------------
void CView::setWantsFocus (bool state)
{
	setViewFlag (kWantsFocus, state);
}

//-----------------------------------------------------------------------------
void CView::setWantsIdle (bool state)
{
	if (wantsIdle () == state)
		return;
	setViewFlag (kWantsIdle, state);
	if (isAttached ())
		state ? CViewInternal::IdleViewUpdater::add (this) : CViewInternal::IdleViewUpdater::remove (this);
}

//-----------------------------------------------------------------------------
void CView::setDirty (bool state)
{
	if (kDirtyCallAlwaysOnMainThread)
	{
		if (state)
			invalidRect (getViewSize ());
		setViewFlag (kDirty, false);
	}
	else
	{
		setViewFlag (kDirty, state);
	}
}

//-----------------------------------------------------------------------------
void CView::setSubviewState (bool state)
{
	vstgui_assert (isSubview () != state, "");
	setViewFlag (kIsSubview, state);
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
	vstgui_assert (parent->asViewContainer ());
	pImpl->parentView = parent;
	pImpl->parentFrame = parent->getFrame ();
	setViewFlag (kIsAttached, true);
	if (pImpl->parentFrame)
		pImpl->parentFrame->onViewAdded (this);
	if (wantsIdle ())
		CViewInternal::IdleViewUpdater::add (this);
	if (pImpl->viewListeners)
	{
		pImpl->viewListeners->forEach (
		    [&] (IViewListener* listener) { listener->viewAttached (this); });
	}
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
		CViewInternal::IdleViewUpdater::remove (this);
	if (pImpl->viewListeners)
	{
		pImpl->viewListeners->forEach (
		    [&] (IViewListener* listener) { listener->viewRemoved (this); });
	}
	if (pImpl->parentFrame)
		pImpl->parentFrame->onViewRemoved (this);
	pImpl->parentView = nullptr;
	pImpl->parentFrame = nullptr;
	setViewFlag (kIsAttached, false);
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
 * @param where location
 * @param buttons button and modifier state
 * @return true if point hits this view
 */
bool CView::hitTest (const CPoint& where, const CButtonState& buttons)
{
	if (auto path = getHitTestPath ())
	{
		CPoint p (where);
		p.offset (-getViewSize ().left, -getViewSize ().top);
		return path->hitTest (p);
	}
	return getMouseableArea ().pointInside (where);
}

//-----------------------------------------------------------------------------
/**
 * @param point location
 * @return converted point
 */
CPoint& CView::frameToLocal (CPoint& point) const
{
	if (pImpl->parentView)
		return pImpl->parentView->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
/**
 * @param point location
 * @return converted point
 */
CPoint& CView::localToFrame (CPoint& point) const
{
	if (pImpl->parentView)
		return pImpl->parentView->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
CGraphicsTransform CView::getGlobalTransform () const
{
	using ParentViews = std::list<CViewContainer*>;

	CGraphicsTransform transform;
	ParentViews parents;
	
	CViewContainer* parent = getParentView () ? getParentView ()->asViewContainer () : nullptr;
	while (parent)
	{
		parents.push_front (parent);
		parent = parent->getParentView () ? parent->getParentView ()->asViewContainer () : nullptr;
	}
	for (const auto& parent : parents)
	{
		CGraphicsTransform t = parent->getTransform ();
		t.translate (parent->getViewSize ().getTopLeft ());
		transform = transform * t;
	}

	if (auto This = this->asViewContainer ())
		transform = transform * This->getTransform ();
	return transform;
}

//-----------------------------------------------------------------------------
/**
 * @param rect rect to invalidate
 */
void CView::invalidRect (const CRect& rect)
{
	if (isAttached () && hasViewFlag (kVisible))
	{
		vstgui_assert (pImpl->parentView);
		pImpl->parentView->invalidRect (rect);
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
		getDrawBackground ()->draw (pContext, getViewSize ());
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

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------------
/**
 * a drag can only be started from within onMouseDown
 * @param source source drop
 * @param offset bitmap offset
 * @param dragBitmap bitmap to drag
 * @return see DragResult
 */
DragResult CView::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	if (auto frame = getFrame ())
	{
		if (auto platformFrame = frame->getPlatformFrame ())
		{
			return platformFrame->doDrag (source, offset, dragBitmap);
		}
	}
	return kDragError;
}
#endif

//------------------------------------------------------------------------------
/**
 * A drag can only be started from within onMouseDown or onMouseMove.
 * This method may return immediately before the drop occurs, if you want to be notified about the
 * result you have to provide a callback object.
 *
 * @param dragDescription drag description
 * @param callback callback
 * @return true if the drag was started, otherwise false
 */
bool CView::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback)
{
	if (auto frame = getFrame ())
	{
		if (auto platformFrame = frame->getPlatformFrame ())
		{
			return platformFrame->doDrag (dragDescription, callback);
		}
	}
	return false;
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
{
	if (!pImpl->viewListeners)
		return;
	pImpl->viewListeners->forEach (
	    [&] (IViewListener* listener) { listener->viewLostFocus (this); });
}

//------------------------------------------------------------------------------
void CView::takeFocus ()
{
	if (!pImpl->viewListeners)
		return;
	pImpl->viewListeners->forEach (
	    [&] (IViewListener* listener) { listener->viewTookFocus (this); });
}

//------------------------------------------------------------------------------
/**
 * @param newSize rect of new size of view
 * @param doInvalid if true set view dirty
 */
void CView::setViewSize (const CRect& newSize, bool doInvalid)
{
	if (getViewSize () != newSize)
	{
		if (doInvalid && kDirtyCallAlwaysOnMainThread)
			invalid ();
		CRect oldSize = getViewSize ();
		pImpl->size = newSize;
		if (doInvalid)
			setDirty ();
		if (getParentView ())
			getParentView ()->notify (this, kMsgViewSizeChanged);
		if (pImpl->viewListeners)
		{
			pImpl->viewListeners->forEach (
			    [&] (IViewListener* listener) { listener->viewSizeChanged (this, oldSize); });
		}
	}
}

//------------------------------------------------------------------------------
const CRect& CView::getViewSize () const
{
	return pImpl->size;
}

//------------------------------------------------------------------------------
/**
 * @return visible size of view
 */
CRect CView::getVisibleViewSize () const
{
	if (pImpl->parentView)
		return static_cast<CViewContainer*>(pImpl->parentView)->getVisibleSize (getViewSize ());
	return CRect (0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
void CView::setVisible (bool state)
{
	if (hasViewFlag (kVisible) != state)
	{
		if (state)
		{
			setViewFlag (kVisible, true);
			invalid ();
		}
		else
		{
			invalid ();
			setViewFlag (kVisible, false);
		}
	}
}

//-----------------------------------------------------------------------------
void CView::setAlphaValueNoInvalidate (float value)
{
	pImpl->alphaValue = value;
}

//-----------------------------------------------------------------------------
void CView::setAlphaValue (float alpha)
{
	if (pImpl->alphaValue != alpha)
	{
		pImpl->alphaValue = alpha;
		// we invalidate the parent to make sure that when alpha == 0 that a redraw occurs
		if (pImpl->parentView)
			pImpl->parentView->invalidRect (getViewSize ());
	}
}

//-----------------------------------------------------------------------------
float CView::getAlphaValue () const
{
	return pImpl->alphaValue;
}

//-----------------------------------------------------------------------------
void CView::setAutosizeFlags (int32_t flags)
{
	pImpl->autosizeFlags = flags;
}

//-----------------------------------------------------------------------------
int32_t CView::getAutosizeFlags () const
{
	return pImpl->autosizeFlags;
}

//-----------------------------------------------------------------------------
void CView::setParentFrame (CFrame* frame)
{
	pImpl->parentFrame = frame;
}

//-----------------------------------------------------------------------------
void CView::setParentView (CView* parent)
{
	pImpl->parentView = parent;
}

//-----------------------------------------------------------------------------
CView* CView::getParentView () const
{
	return pImpl->parentView;
}

//-----------------------------------------------------------------------------
CFrame* CView::getFrame () const
{
	return pImpl->parentFrame;
}

//-----------------------------------------------------------------------------
VSTGUIEditorInterface* CView::getEditor () const
{
	return pImpl->parentFrame ? pImpl->parentFrame->getEditor () : nullptr;
}

//-----------------------------------------------------------------------------
/**
 * @param background new background bitmap
 */
void CView::setBackground (CBitmap* background)
{
	pImpl->background = background;
	if (getMouseEnabled () == true)
		setDirty (true);
}

//-----------------------------------------------------------------------------
CBitmap* CView::getBackground () const
{
	return pImpl->background;
}

//-----------------------------------------------------------------------------
CBitmap* CView::getDisabledBackground () const
{
	return pImpl->disabledBackground;
}

//-----------------------------------------------------------------------------
CBitmap* CView::getDrawBackground () const
{
	return (pImpl->disabledBackground ? (getMouseEnabled () ? pImpl->background : pImpl->disabledBackground): pImpl->background);
}

//-----------------------------------------------------------------------------
/**
 * @param background new disabled background bitmap
 */
void CView::setDisabledBackground (CBitmap* background)
{
	pImpl->disabledBackground = background;
	if (getMouseEnabled () == false)
		setDirty (true);
}

//-----------------------------------------------------------------------------
/**
 * @param aId the ID of the Attribute
 * @param outSize on return the size of the attribute
 * @return true if attribute exists. outSize is valid then.
 */
bool CView::getAttributeSize (const CViewAttributeID aId, uint32_t& outSize) const
{
	auto it = pImpl->attributes.find (aId);
	if (it != pImpl->attributes.end ())
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
bool CView::getAttribute (const CViewAttributeID aId, const uint32_t inSize, void* outData, uint32_t& outSize) const
{
	auto it = pImpl->attributes.find (aId);
	if (it != pImpl->attributes.end ())
	{
		if (inSize >= it->second->getSize ())
		{
			outSize = it->second->getSize ();
			if (outSize > 0)
				std::memcpy (outData, it->second->getData (), static_cast<size_t> (outSize));
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
bool CView::setAttribute (const CViewAttributeID aId, const uint32_t inSize, const void* inData)
{
	if (inData == nullptr || inSize <= 0)
		return false;
	auto it = pImpl->attributes.find (aId);
	if (it != pImpl->attributes.end ())
		it->second->updateData (inSize, inData);
	else
		pImpl->attributes.emplace (aId, std::unique_ptr<CViewInternal::AttributeEntry> (new CViewInternal::AttributeEntry (inSize, inData)));
	return true;
}

//-----------------------------------------------------------------------------
bool CView::removeAttribute (const CViewAttributeID aId)
{
	auto it = pImpl->attributes.find (aId);
	if (it != pImpl->attributes.end ())
	{
		pImpl->attributes.erase (aId);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CView::addAnimation (IdStringPtr name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction, CBaseObject* notificationObject)
{
	vstgui_assert (isAttached (), "to start an animation, the view needs to be attached");
	if (auto frame = getFrame ())
	{
		frame->getAnimator ()->addAnimation (this, name, target, timingFunction, notificationObject);
	}
}

//-----------------------------------------------------------------------------
void CView::addAnimation (IdStringPtr name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction, const Animation::DoneFunction& doneFunc)
{
	vstgui_assert (isAttached (), "to start an animation, the view needs to be attached");
	if (auto frame = getFrame ())
	{
		frame->getAnimator ()->addAnimation (this, name, target, timingFunction, doneFunc);
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
	CRect viewRect = getViewSize ();
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
void CView::registerViewListener (IViewListener* listener)
{
	if (!pImpl->viewListeners)
		pImpl->viewListeners =
		    std::unique_ptr<Impl::ViewListenerDispatcher> (new Impl::ViewListenerDispatcher);
	pImpl->viewListeners->add (listener);
}

//-----------------------------------------------------------------------------
void CView::unregisterViewListener (IViewListener* listener)
{
	if (!pImpl->viewListeners)
		return;
	pImpl->viewListeners->remove (listener);
}

//------------------------------------------------------------------------
void CView::registerViewMouseListener (IViewMouseListener* listener)
{
	if (!pImpl->viewMouseListener)
		pImpl->viewMouseListener = std::unique_ptr<Impl::ViewMouseListenerDispatcher> (
		    new Impl::ViewMouseListenerDispatcher);
	pImpl->viewMouseListener->add (listener);
}

//------------------------------------------------------------------------
void CView::unregisterViewMouseListener (IViewMouseListener* listener)
{
	if (!pImpl->viewMouseListener)
		return;
	pImpl->viewMouseListener->remove (listener);
}

//-----------------------------------------------------------------------------
CMouseEventResult CView::callMouseListener (MouseListenerCall type, CPoint pos, CButtonState buttons)
{
	CMouseEventResult result = kMouseEventNotHandled;
	if (!pImpl->viewMouseListener)
		return result;
	pImpl->viewMouseListener->forEachReverse (
	    [&] (IViewMouseListener* l) {
		    switch (type)
		    {
			    case MouseListenerCall::MouseDown: return l->viewOnMouseDown (this, pos, buttons);
			    case MouseListenerCall::MouseUp: return l->viewOnMouseUp (this, pos, buttons);
			    case MouseListenerCall::MouseMoved: return l->viewOnMouseMoved (this, pos, buttons);
			    case MouseListenerCall::MouseCancel: return l->viewOnMouseCancel (this);
		    }
			return kMouseEventNotHandled;
	    },
	    [&] (CMouseEventResult res) {
		    if (res != kMouseEventNotHandled && res != kMouseEventNotImplemented)
		    {
			    result = res;
			    return true;
		    }
		    return false;
	    });
	return result;
}

//-----------------------------------------------------------------------------
void CView::callMouseListenerEnteredExited (bool mouseEntered)
{
	if (!pImpl->viewMouseListener)
		return;
	pImpl->viewMouseListener->forEachReverse ([&] (IViewMouseListener* l) {
		if (mouseEntered)
			l->viewOnMouseEntered (this);
		else
			l->viewOnMouseExited (this);
	});
}

//-----------------------------------------------------------------------------
SharedPointer<IDropTarget> CView::getDropTarget ()
{
	IDropTarget* dropTarget = nullptr;
	if (getAttribute (kCViewCustomDropTarget, dropTarget))
		return dropTarget;
	return nullptr;
}

//-----------------------------------------------------------------------------
void CView::setDropTarget (const SharedPointer<IDropTarget>& dt)
{
	IDropTarget* dropTarget = nullptr;
	if (getAttribute (kCViewCustomDropTarget, dropTarget))
		dropTarget->forget ();
	dropTarget = dt;
	if (dropTarget)
	{
		setAttribute (kCViewCustomDropTarget, dropTarget);
		dropTarget->remember ();
	}
	else
	{
		removeAttribute (kCViewCustomDropTarget);
	}
}

//-----------------------------------------------------------------------------
void CView::setTooltipText (UTF8StringPtr text)
{
	if (text)
		setAttribute (kCViewTooltipAttribute, static_cast<uint32_t> (strlen (text) + 1), text);
	else
		removeAttribute (kCViewTooltipAttribute);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CDragContainerHelper::CDragContainerHelper (IDataPackage* drag)
: drag (drag)
{
	vstgui_assert (drag, "drag cannot be nullptr");
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
	const void* data = nullptr;
	outSize = static_cast<int32_t> (drag->getData (static_cast<uint32_t> (index), data, type));
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
	IDataPackage::Type type = drag->getDataType (static_cast<uint32_t> (idx));
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
	return static_cast<int32_t> (drag->getCount ());
}

} // namespace
