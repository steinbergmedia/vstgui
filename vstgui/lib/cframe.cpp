// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cframe.h"
#include "events.h"
#include "finally.h"
#include "coffscreencontext.h"
#include "ctooltipsupport.h"
#include "cinvalidrectlist.h"
#include "itouchevent.h"
#include "iscalefactorchangedlistener.h"
#include "idatapackage.h"
#include "iviewlayouter.h"
#include "animation/animator.h"
#include "controls/ctextedit.h"
#include "platform/platformfactory.h"
#include "platform/iplatformframe.h"
#include <cassert>
#include <vector>
#include <queue>
#include <stack>
#include <limits>

namespace VSTGUI {

IdStringPtr kMsgNewFocusView = "kMsgNewFocusView";
IdStringPtr kMsgOldFocusView = "kMsgOldFocusView";

#define DEBUG_MOUSE_VIEWS (DEBUG && 0)

//------------------------------------------------------------------------
struct CFrame::CollectInvalidRects
{
	explicit CollectInvalidRects (CFrame* frame);
	~CollectInvalidRects () noexcept;

	void addRect (const CRect& rect);
	void flush ();

private:
	using InvalidRects = std::vector<CRect>;

	SharedPointer<CFrame> frame;
	CInvalidRectList invalidRects;
	uint64_t lastTicks;
#if VSTGUI_LOG_COLLECT_INVALID_RECTS
	uint32_t numAddedRects;
#endif
};

//------------------------------------------------------------------------
struct ModalViewSession
{
	ModalViewSessionID identifier {};
	SharedPointer<CView> view;
};

//------------------------------------------------------------------------
struct CFrame::Impl
{
	using ViewList = std::list<SharedPointer<CView>>;
	using FunctionQueue = std::queue<EventProcessingFunction>;
	using ModalViewSessionStack = std::stack<ModalViewSession>;

	PlatformFramePtr platformFrame;
	VSTGUIEditorInterface* editor {nullptr};
	IViewAddedRemovedObserver* viewAddedRemovedObserver {nullptr};
	SharedPointer<CTooltipSupport> tooltips;
	SharedPointer<Animation::Animator> animator;
	SharedPointer<CView> focusView;
	SharedPointer<CView> activeFocusView;
	CollectInvalidRects* collectInvalidRects {nullptr};
	
	ViewList mouseViews;
	ModalViewSessionStack modalViewSessionStack;
	DispatchList<CView*> windowActiveStateChangeViews;
	DispatchList<IScaleFactorChangedListener*> scaleFactorChangedListenerList;
	DispatchList<IMouseObserver*> mouseObservers;
	DispatchList<IFocusViewObserver*> focusViewObservers;
	DispatchList<IKeyboardHook*> keyboardHooks;
	FunctionQueue postEventFunctionQueue;
	std::optional<ViewLayout> lastCheckSizeConstraintLayout;

	ModalViewSessionID modalViewSessionIDCounter {0};
	double userScaleFactor {1.};
	double platformScaleFactor {1.};
	bool active {false};
	bool windowActive {false};
	bool inEventHandling {false};
	BitmapInterpolationQuality bitmapQuality {BitmapInterpolationQuality::kDefault};

	struct PostEventHandler
	{
		PostEventHandler (Impl& impl) : impl (impl)
		{
			wasInEventHandling = impl.inEventHandling;
			impl.inEventHandling = true;
		}
		~PostEventHandler () noexcept
		{
			vstgui_assert (impl.inEventHandling == true);
			impl.inEventHandling = wasInEventHandling;
			FunctionQueue fl;
			impl.postEventFunctionQueue.swap (fl);
			while (!fl.empty ())
			{
				fl.front () ();
				fl.pop ();
			}
		}

	private:
		Impl& impl;
		bool wasInEventHandling;
	};
};

//-----------------------------------------------------------------------------
// CFrame Implementation
//-----------------------------------------------------------------------------
/*! @class CFrame
It creates a platform dependent view object. 

On Mac OS X it is a HIView or NSView.\n 
On Windows it's a WS_CHILD Window.

*/
//-----------------------------------------------------------------------------
CFrame::CFrame (const CRect& inSize, VSTGUIEditorInterface* inEditor) : CViewContainer (inSize)
{
	pImpl = new Impl;
	pImpl->editor = inEditor;

	setParentFrame (shared (this));
}

//-----------------------------------------------------------------------------
void CFrame::beforeDelete ()
{
	clearMouseViews (CPoint (0, 0), Modifiers (), false);

	clearModalViewSessions ();

	setCursor (kCursorDefault);

	setParentFrame (nullptr);
	removeAll ();

	pImpl->tooltips = nullptr;
	pImpl->animator = nullptr;

#if DEBUG
	if (!pImpl->scaleFactorChangedListenerList.empty ())
	{
		DebugPrint ("Warning: Scale Factor Changed Listeners are not cleaned up correctly.\n If you register a change listener you must also unregister it !\n");
	}
	
	if (!pImpl->mouseObservers.empty ())
	{
		DebugPrint ("Warning: Mouse Observers are not cleaned up correctly.\n If you register a mouse oberver you must also unregister it !\n");
	}

	if (!pImpl->keyboardHooks.empty ())
	{
		DebugPrint ("Warning: Keyboard Hooks are not cleaned up correctly.\n If you register a keyboard hook you must also unregister it !\n");
	}
#endif

	if (pImpl->platformFrame)
	{
		pImpl->platformFrame->onFrameClosed ();
		pImpl->platformFrame = nullptr;
	}

	setViewFlag (kIsAttached, false);

	CViewContainer::beforeDelete ();

	delete pImpl;
	pImpl = nullptr;
}

//-----------------------------------------------------------------------------
void CFrame::close ()
{
	clearMouseViews (CPoint (0, 0), Modifiers (), false);

	clearModalViewSessions ();

	setCursor (kCursorDefault);
	setParentFrame (nullptr);
	removeAll ();
	if (pImpl->platformFrame)
	{
		pImpl->platformFrame->onFrameClosed ();
		pImpl->platformFrame = nullptr;
	}
}

//-----------------------------------------------------------------------------
bool CFrame::open (void* systemWin, PlatformType systemWindowType, IPlatformFrameConfig* config)
{
	if (!systemWin || isAttached ())
		return false;

	pImpl->platformFrame = getPlatformFactory ().createFrame (this, getViewSize (), systemWin,
	                                                          systemWindowType, config);
	if (!pImpl->platformFrame)
	{
		return false;
	}

	CollectInvalidRects cir (this);

	attached (shared (this));

	setParentView (nullptr);

	invalid ();

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::attached (const SharedPointer<CViewContainer>& parent)
{
	if (isAttached ())
		return false;
	vstgui_assert (parent.get () == this);
	if (CView::attached (parent))
	{
		setParentView (nullptr);

		for (const auto& pV : getChildren ())
			pV->attached (shared (this));

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CFrame::setZoom (double zoomFactor)
{
	if (zoomFactor == 0.)
		return false;

	bool result = true;
	CGraphicsTransform currentTransform = getTransform ();
	CCoord origWidth = getWidth () / currentTransform.m11;
	CCoord origHeight = getHeight () / currentTransform.m22;
	CCoord newWidth = origWidth * zoomFactor;
	CCoord newHeight = origHeight * zoomFactor;
	setAutosizingEnabled (false);
	setTransform (CGraphicsTransform ().scale (zoomFactor, zoomFactor));
	if (!setSize (newWidth, newHeight))
	{
		setTransform (currentTransform);
		setSize (origWidth * currentTransform.m11, origHeight * currentTransform.m22);
		result = false;
	}
	invalid ();
	setAutosizingEnabled (true);
	if (result)
	{
		pImpl->userScaleFactor = zoomFactor;
		dispatchNewScaleFactor (getScaleFactor ());
	}
	return result;
}

//-----------------------------------------------------------------------------
double CFrame::getZoom () const
{
	return pImpl->userScaleFactor;
}

//-----------------------------------------------------------------------------
void CFrame::setBitmapInterpolationQuality (BitmapInterpolationQuality quality)
{
	if (pImpl && pImpl->bitmapQuality != quality)
	{
		pImpl->bitmapQuality = quality;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
BitmapInterpolationQuality CFrame::getBitmapInterpolationQuality () const
{
	if (pImpl)
		return pImpl->bitmapQuality;
	return BitmapInterpolationQuality::kDefault;
}

//-----------------------------------------------------------------------------
double CFrame::getScaleFactor () const
{
	return pImpl->platformScaleFactor * pImpl->userScaleFactor;
}

//-----------------------------------------------------------------------------
void CFrame::enableTooltips (bool state, uint32_t delayTimeInMs)
{
	if (state)
	{
		if (pImpl->tooltips == nullptr)
			pImpl->tooltips = makeShared<CTooltipSupport> (*this, delayTimeInMs);
	}
	else if (pImpl->tooltips)
	{
		pImpl->tooltips = nullptr;
	}
}

//-----------------------------------------------------------------------------
void CFrame::draw (CDrawContext& context) { return CFrame::drawRect (context, getViewSize ()); }

//-----------------------------------------------------------------------------
void CFrame::drawRect (CDrawContext& context, const CRect& updateRect)
{
	if (updateRect.getWidth () <= 0 || updateRect.getHeight () <= 0)
		return;

	if (pImpl)
		context.setBitmapInterpolationQuality (pImpl->bitmapQuality);

	drawClipped (context, updateRect, [&] () {
		// draw the background and the children
		CViewContainer::drawRect (context, updateRect);
	});
}

//-----------------------------------------------------------------------------
void CFrame::clearMouseViews (const CPoint& where, Modifiers modifiers, bool callMouseExit)
{
	auto it = pImpl->mouseViews.rbegin ();
	while (it != pImpl->mouseViews.rend ())
	{
		if (callMouseExit)
		{
			MouseExitEvent exitEvent;
			exitEvent.modifiers = modifiers;
			exitEvent.mousePosition = (*it)->translateToLocal (where, true);
			dispatchEvent ((*it), exitEvent);
#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseExited  : %p[%d,%d]\n", (*it), (int)exitEvent.mousePosition.x,
						(int)exitEvent.mousePosition.y);
#endif
		}
		if (pImpl->tooltips)
			pImpl->tooltips->onMouseExited ((*it));

		callMouseObserverMouseExited (*(*it).get ());

		++it;
	}
	pImpl->mouseViews.clear ();
}

//-----------------------------------------------------------------------------
void CFrame::removeFromMouseViews (const SharedPointer<CView>& view)
{
	bool found = false;
	auto it = pImpl->mouseViews.begin ();
	while (it != pImpl->mouseViews.end ())
	{
		if (found || (*it) == view)
		{
			if (pImpl->tooltips)
				pImpl->tooltips->onMouseExited ((*it));

			callMouseObserverMouseExited (*(*it).get ());

			pImpl->mouseViews.erase (it++);
			found = true;
		}
		else
			++it;
	}
}

//-----------------------------------------------------------------------------
void CFrame::checkMouseViews (const MouseEvent& event)
{
	if (getMouseDownView ())
		return;
	auto mouseView = getViewAt (event.mousePosition,
								GetViewOptions ().deep ().mouseEnabled ().includeViewContainer ());
	auto currentMouseView =
		pImpl->mouseViews.empty () == false ? pImpl->mouseViews.back () : nullptr;
	if (currentMouseView == mouseView)
		return; // no change

	if (pImpl->tooltips)
	{
		if (currentMouseView)
			pImpl->tooltips->onMouseExited (currentMouseView);
		if (mouseView && mouseView.get () != this)
			pImpl->tooltips->onMouseEntered (mouseView);
	}

	if (mouseView == nullptr || mouseView.get () == this)
	{
		clearMouseViews (event.mousePosition, event.modifiers);
		return;
	}

	auto callMouseExitForView = [this, &event] (auto view) {
		MouseExitEvent exitEvent (event);
		exitEvent.mousePosition = view->translateToLocal (exitEvent.mousePosition, true);
		dispatchEvent (view, exitEvent);
		callMouseObserverMouseExited (*view.get ());
#if DEBUG_MOUSE_VIEWS
		DebugPrint ("mouseExited  : %p[%d,%d]\n", view, (int)exitEvent.mousePosition.x,
					(int)exitEvent.mousePosition.y);
#endif
	};

	auto callMouseEnterForView = [this, &event] (auto view) {
		MouseEnterEvent enterEvent (event);
		enterEvent.mousePosition = view->translateToLocal (enterEvent.mousePosition, true);
		dispatchEvent (view, enterEvent);
		callMouseObserverMouseEntered (*view.get ());
#if DEBUG_MOUSE_VIEWS
		DebugPrint ("mouseEntered : %p[%d,%d]\n", view, (int)enterEvent.mousePosition.x,
					(int)enterEvent.mousePosition.y);
#endif
	};

	auto vc = currentMouseView ? currentMouseView->asViewContainer () : nullptr;
	// if the currentMouseView is not a view container, we know that the new mouseView won't be a child of it and that all other
	// views in the list are viewcontainers
	if (vc == nullptr && currentMouseView)
	{
		callMouseExitForView (currentMouseView);
		pImpl->mouseViews.remove (currentMouseView);
	}
	auto it = pImpl->mouseViews.rbegin ();
	while (it != pImpl->mouseViews.rend ())
	{
		vc = (*it).cast<CViewContainer> ();
		if (vc == mouseView.cast<CViewContainer> ())
			return;
		if (vc->isChild (mouseView, true) == false)
		{
			callMouseExitForView (vc);
			pImpl->mouseViews.erase (--it.base ());
		}
		else
			break;
	}
	vc = pImpl->mouseViews.empty () == false ? pImpl->mouseViews.back ()->asViewContainer () : nullptr;
	if (vc)
	{
		auto it2 = pImpl->mouseViews.end ();
		--it2;
		auto container = mouseView;
		while ((vc = container->getParentView ()).get () != (*it2).get () && vc)
		{
			pImpl->mouseViews.emplace_back (vc);
			container = vc;
		}
		pImpl->mouseViews.emplace_back (mouseView);
		++it2;
		while (it2 != pImpl->mouseViews.end ())
		{
			callMouseEnterForView (*it2);
			++it2;
		}
	}
	else
	{
		// must be pMouseViews.size () == 0
		vstgui_assert (pImpl->mouseViews.empty ());
		pImpl->mouseViews.emplace_back (mouseView);
		while ((vc = mouseView->getParentView ()).get () != this && vc)
		{
			pImpl->mouseViews.push_front (vc);
			mouseView = vc.cast<CView> ();
		}
		auto it2 = pImpl->mouseViews.begin ();
		while (it2 != pImpl->mouseViews.end ())
		{
			callMouseEnterForView (*it2);
			++it2;
		}
	}
}

//------------------------------------------------------------------------
bool CFrame::hitTestSubViews (const CPoint& where, const Event& event)
{
	if (auto modalView = getModalView ())
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (modalView->isVisible () && modalView->getMouseEnabled () && modalView->hitTest (where2, event))
		{
			if (auto viewContainer = modalView->asViewContainer ())
			{
				return viewContainer->hitTestSubViews (where2, event);
			}
			return true;
		}
		return false;
	}
	return CViewContainer::hitTestSubViews (where, event);
}

//-----------------------------------------------------------------------------
void CFrame::dispatchEvent (const SharedPointer<CView>& view, Event& event)
{
	view->dispatchEvent (event);
}

//-----------------------------------------------------------------------------
void CFrame::dispatchEventToChildren (Event& event)
{
	CView::dispatchEvent (event);
}

//-----------------------------------------------------------------------------
void CFrame::dispatchKeyboardEvent (KeyboardEvent& event)
{
	if (static_cast<uint32_t> (event.virt) > static_cast<uint32_t> (VirtualKey::Equals))
		event.virt = VirtualKey::None;
	dispatchKeyboardEventToHooks (event);
	if (event.consumed)
		return;

	if (pImpl->focusView)
	{
		if (pImpl->focusView->getMouseEnabled ())
			dispatchEvent (pImpl->focusView, event);
		if (event.consumed)
			return;
		auto parent = pImpl->focusView->getParentView ();
		while (parent && parent.get () != this)
		{
			if (parent->getMouseEnabled ())
			{
				dispatchEvent (parent, event);
				if (event.consumed)
					return;
			}
			parent = parent->getParentView ();
		}
	}
	if (auto modalView = getModalView ())
	{
		dispatchEvent (modalView, event);
		if (event.consumed)
			return;
	}
	if (event.type != EventType::KeyUp && event.virt == VirtualKey::Tab)
	{
		if (event.modifiers.empty () || event.modifiers.is (ModifierKey::Shift))
		{
			if (advanceNextFocusView (pImpl->focusView, event.modifiers.is (ModifierKey::Shift)))
				event.consumed = true;
		}
	}
}

//------------------------------------------------------------------------
void CFrame::dispatchMouseDownEvent (MouseDownEvent& event)
{
	auto originMousePosition = event.mousePosition;
	auto transformedMousePosition = event.mousePosition;
	getTransform ().inverse ().transform (transformedMousePosition);
	if (auto tooltips = pImpl->tooltips)
		tooltips->onMouseDown (transformedMousePosition);

	event.mousePosition = transformedMousePosition;
	callMouseObserverOtherMouseEvent (event);
	if (event.consumed)
		return;
	event.mousePosition = originMousePosition;

	setMouseDownView (nullptr);
	if (pImpl->focusView && pImpl->focusView.cast<CTextEdit> ())
		setFocusView (nullptr);

	if (auto modalView = getModalView ())
	{
		if (modalView->isVisible () && modalView->getMouseEnabled ())
		{
			event.mousePosition = transformedMousePosition;
			dispatchEvent (modalView, event);
			if (event.consumed)
				setMouseDownView (modalView);
		}
		return;
	}
	dispatchEventToChildren (event);
}

//------------------------------------------------------------------------
void CFrame::dispatchMouseMoveEvent (MouseMoveEvent& event)
{
	auto originMousePosition = event.mousePosition;
	auto transformedMousePosition = event.mousePosition;
	getTransform ().inverse ().transform (transformedMousePosition);

	if (auto tooltips = pImpl->tooltips)
		tooltips->onMouseMoved (transformedMousePosition);

	checkMouseViews (event);

	event.mousePosition = transformedMousePosition;
	callMouseObserverOtherMouseEvent (event);
	if (event.consumed)
		return;
	event.mousePosition = originMousePosition;

	if (auto modalView = getModalView ())
	{
		if (modalView->isVisible () && modalView->getMouseEnabled ())
		{
			event.mousePosition = transformedMousePosition;
			dispatchEvent (modalView, event);
		}
	}
	else
		dispatchEventToChildren (event);
	if (event.consumed == false)
	{
		event.buttonState.clear ();
		auto it = pImpl->mouseViews.rbegin ();
		while (it != pImpl->mouseViews.rend ())
		{
			CPoint p (transformedMousePosition);
			auto view = *it;
			if (view->asViewContainer ())
			{
				if (auto parent = view->getParentView ())
				{
					if (parent.get () != this)
					{
						p.offsetInverse (parent->getViewSize ().getTopLeft ());
						parent->translateToLocal (p, true);
					}
				}
			}
			else
				view->translateToLocal (p, true);
			event.mousePosition = p;
			dispatchEvent (view, event);
			if (event.consumed)
				break;
			++it;
		}
	}
}

//------------------------------------------------------------------------
void CFrame::dispatchMouseUpEvent (MouseUpEvent& event)
{
	auto originMousePosition = event.mousePosition;
	auto transformedMousePosition = event.mousePosition;
	getTransform ().inverse ().transform (transformedMousePosition);
	
	auto f = finally ([this] () { setMouseDownView (nullptr); });

	event.mousePosition = transformedMousePosition;
	callMouseObserverOtherMouseEvent (event);
	if (event.consumed)
		return;
	event.mousePosition = originMousePosition;

	if (auto modalView = getModalView ())
	{
		if (modalView->isVisible () && modalView->getMouseEnabled ())
		{
			event.mousePosition = transformedMousePosition;
			dispatchEvent (modalView, event);
		}
		return;
	}
	dispatchEventToChildren (event);
}

//------------------------------------------------------------------------
void CFrame::dispatchMouseEvent (MouseEvent& event)
{
	if (event.type == EventType::MouseMove)
		dispatchMouseMoveEvent (castMouseMoveEvent (event));
	else if (event.type == EventType::MouseDown)
		dispatchMouseDownEvent (castMouseDownEvent (event));
	else if (event.type == EventType::MouseUp)
		dispatchMouseUpEvent (castMouseUpEvent (event));
	else if (event.type == EventType::MouseEnter)
	{}
	else if (event.type == EventType::MouseExit)
	{
		if (getMouseDownView () == nullptr)
		{
			clearMouseViews (event.mousePosition, event.modifiers);
			if (pImpl->tooltips)
				pImpl->tooltips->hideTooltip ();
		}
		event.consumed = true;
	}
	else
	{
		vstgui_assert (false);
	}
}

//-----------------------------------------------------------------------------
void CFrame::dispatchEvent (Event& event)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);

	if (auto mouseEvent = asMouseEvent (event))
	{
		dispatchMouseEvent (*mouseEvent);
		return;
	}
	if (auto keyEvent = asKeyboardEvent (event))
	{
		dispatchKeyboardEvent (*keyEvent);
		return;
	}

	auto mousePosEvent = asMousePositionEvent (event);
	CPoint mousePosition;
	if (mousePosEvent)
		mousePosition = mousePosEvent->mousePosition;

	auto modalView = getModalView ();
	if (modalView && mousePosEvent)
		getTransform ().inverse ().transform (mousePosEvent->mousePosition);

	if (modalView)
		dispatchEvent (modalView, event);
	else
		dispatchEventToChildren (event);

	if (mousePosEvent)
	{
		MouseEvent mouseEvent;
		mouseEvent.mousePosition = mousePosEvent->mousePosition;
		mouseEvent.modifiers = mousePosEvent->modifiers;
		checkMouseViews (mouseEvent);
	}
}

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> CFrame::getClipboard ()
{
	return getPlatformFactory ().getClipboard ();
}

//-----------------------------------------------------------------------------
void CFrame::setClipboard (const SharedPointer<IDataPackage>& data)
{
	getPlatformFactory ().setClipboard (data);
}

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	// TODO: remove method
}

//-----------------------------------------------------------------------------
SharedPointer<Animation::Animator> CFrame::getAnimator ()
{
	if (pImpl->animator == nullptr)
		pImpl->animator = makeShared<Animation::Animator> ();
	return pImpl->animator;
}

//-----------------------------------------------------------------------------
/**
 * @return tick count in milliseconds
 */
uint64_t CFrame::getTicks () const
{
	return getPlatformFactory ().getTicks ();
}

//-----------------------------------------------------------------------------
int32_t CFrame::kDefaultKnobMode = kCircularMode;

//-----------------------------------------------------------------------------
int32_t CFrame::getKnobMode () const
{
	int32_t result = pImpl->editor ? pImpl->editor->getKnobMode () : -1;
	if (result == -1)
		result = kDefaultKnobMode;
	return result;
}

//-----------------------------------------------------------------------------
/**
 * repositions the frame
 * @param x x coordinate
 * @param y y coordinate
 * @return true on success
 */
bool CFrame::setPosition (CCoord x, CCoord y)
{
	if (pImpl->platformFrame)
	{
		CRect rect (getViewSize ());
		rect.offset (x - getViewSize ().left, y - getViewSize ().top);
		if (pImpl->platformFrame->setSize (rect))
		{
			setViewSize (rect, false);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * get global position of frame
 * @param x x coordinate
 * @param y y coordinate
 * @return true on success
 */
bool CFrame::getPosition (CCoord &x, CCoord &y) const
{
	if (pImpl->platformFrame)
	{
		CPoint p;
		if (pImpl->platformFrame->getGlobalPosition (p))
		{
			x = p.x;
			y = p.y;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void CFrame::setViewSize (const CRect& rect, bool invalid)
{
	if (pImpl->lastCheckSizeConstraintLayout)
	{
		auto layout = std::move (*pImpl->lastCheckSizeConstraintLayout);
		pImpl->lastCheckSizeConstraintLayout = {};
		if (layout.size == rect)
		{
			if (applyViewLayout (layout))
				return;
		}
	}
	CViewContainer::setViewSize (rect, invalid);
}

//-----------------------------------------------------------------------------
CPoint CFrame::checkSizeConstraint (const CPoint& newSize) const
{
	pImpl->lastCheckSizeConstraintLayout = calculateViewLayout ({0., 0., newSize.x, newSize.y});
	if (pImpl->lastCheckSizeConstraintLayout)
		return pImpl->lastCheckSizeConstraintLayout->size.getSize ();
	return getViewSize ().getSize ();
}

//-----------------------------------------------------------------------------
/**
 * set size of frame (and the platform representation)
 * @param width new width
 * @param height new height
 * @return true on success
 */
bool CFrame::setSize (CCoord width, CCoord height)
{
	if ((width == getViewSize ().getWidth ()) && (height == getViewSize ().getHeight ()))
		return true;

	CRect newSize (getViewSize ());
	newSize.setWidth (width);
	newSize.setHeight (height);

	if (getEditor ())
	{
		if (getEditor ()->beforeSizeChange (newSize, getViewSize ()) == false)
			return false;
	}
	if (pImpl->platformFrame)
	{
		if (pImpl->platformFrame->setSize (newSize))
		{
			setViewSize (newSize);
			return true;
		}
		return false;
	}
	setViewSize (newSize);
	return true;
}

//-----------------------------------------------------------------------------
/**
 * get size relative to parent
 * @param pRect size
 * @return true on success
 */
bool CFrame::getSize (CRect* pRect) const
{
	if (pImpl->platformFrame && pRect)
		return pImpl->platformFrame->getSize (*pRect);
	return false;
}

//-----------------------------------------------------------------------------
bool CFrame::getSize (CRect& outSize) const
{
	return getSize (&outSize);
}

//-----------------------------------------------------------------------------
SharedPointer<CView> CFrame::getModalView () const
{
	if (!pImpl->modalViewSessionStack.empty ())
		return pImpl->modalViewSessionStack.top ().view;
	return nullptr;
}

//-----------------------------------------------------------------------------
void CFrame::initModalViewSession (const ModalViewSession& session)
{
	if (getMouseDownView ())
	{
		onMouseCancel ();
	}
	clearMouseViews (CPoint (0, 0), Modifiers (), true);
	if (auto container = session.view->asViewContainer ())
		container->advanceNextFocusView (nullptr, false);
	else
		setFocusView (session.view->wantsFocus () ? session.view : nullptr);

	if (isAttached ())
	{
		CPoint where;
		getCurrentMouseLocation (where);
		MouseEvent mouseEvent;
		mouseEvent.mousePosition = where;
		// TODO: modifiers and mouse button state
		checkMouseViews (mouseEvent);
	}
}

//-----------------------------------------------------------------------------
void CFrame::clearModalViewSessions ()
{
	while (!pImpl->modalViewSessionStack.empty ())
		endModalViewSession (pImpl->modalViewSessionStack.top ().identifier);
}

//-----------------------------------------------------------------------------
Optional<ModalViewSessionID> CFrame::beginModalViewSession (const SharedPointer<CView>& view)
{
	if (view->isAttached ())
	{
#if DEBUG
		DebugPrint ("the view must not be attached when used for beginModalViewSession");
#endif
		return {};
	}

	if (!addSubview (view))
	{
		return {};
	}

	ModalViewSession session;
	session.identifier = ++pImpl->modalViewSessionIDCounter;
	session.view = view;
	pImpl->modalViewSessionStack.push (session);

	initModalViewSession (session);

	return makeOptional (session.identifier);
}

//-----------------------------------------------------------------------------
bool CFrame::endModalViewSession (ModalViewSessionID sessionID)
{
	if (pImpl->modalViewSessionStack.empty ())
		return false;
	if (pImpl->modalViewSessionStack.top ().identifier != sessionID)
		return false;

	auto view = pImpl->modalViewSessionStack.top ().view;
	pImpl->modalViewSessionStack.pop ();

	removeSubview (view);

	if (!pImpl->modalViewSessionStack.empty ())
		initModalViewSession (pImpl->modalViewSessionStack.top ());

	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param where location of mouse
 * @return true on success
 */
bool CFrame::getCurrentMouseLocation (CPoint &where) const
{
	if (pImpl->platformFrame)
	{
		if (pImpl->platformFrame->getCurrentMousePosition (where))
		{
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @return mouse and modifier state
 */
CButtonState CFrame::getCurrentMouseButtons () const
{
	CButtonState buttons = 0;

	if (pImpl->platformFrame)
		pImpl->platformFrame->getCurrentMouseButtons (buttons);

	return buttons;
}

//-----------------------------------------------------------------------------
/**
 * @param type cursor type see #CCursorType
 */
void CFrame::setCursor (CCursorType type)
{
	if (pImpl->platformFrame)
		pImpl->platformFrame->setMouseCursor (type);
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was removed
 */
void CFrame::onViewRemoved (CView& view)
{
	auto pView = &view;
	removeFromMouseViews (shared (pView));

	if (pImpl->activeFocusView.get () == pView)
		pImpl->activeFocusView.reset ();
	if (pImpl->focusView.get () == pView)
	{
		if (pImpl->active)
			setFocusView (nullptr);
		else
			pImpl->focusView = nullptr;
	}
	if (auto container = view.asViewContainer ())
	{
		if (container->isChild (pImpl->focusView, true))
			setFocusView (nullptr);
	}
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewRemoved (*this, view);
	if (view.wantsWindowActiveStateChangeNotification ())
		pImpl->windowActiveStateChangeViews.remove (pView);
	if (pImpl->animator)
		pImpl->animator->removeAnimations (shared (pView));
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was added
 */
void CFrame::onViewAdded (CView& view)
{
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewAdded (*this, view);
	if (view.wantsWindowActiveStateChangeNotification ())
	{
		pImpl->windowActiveStateChangeViews.add (&view);
		view.onWindowActivate (pImpl->windowActive);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pView new focus view
 */
void CFrame::setFocusView (const SharedPointer<CView>& pView)
{
	static bool recursion = false;
	if (pView == pImpl->focusView || (recursion && pImpl->focusView != nullptr))
		return;

	if (pView && !pImpl->modalViewSessionStack.empty ())
	{
		if (auto modalContainer = pImpl->modalViewSessionStack.top ().view->asViewContainer ())
		{
			if (!modalContainer->isChild (pView, true))
			{
#if DEBUG
				DebugPrint (
				    "Could not set the focus view " \
				     "as it is not a child of the currently displayed modal view\n");
#endif
				return;
			}
		}
	}

	if (!pImpl->active)
	{
		pImpl->activeFocusView = pView;
		return;
	}

	recursion = true;

	auto pOldFocusView = pImpl->focusView;
	if (pView == nullptr  || (pView && pView->isAttached () == false))
		pImpl->focusView = nullptr;
	else
		pImpl->focusView = pView;
	if (pImpl->focusView && pImpl->focusView->wantsFocus ())
	{
		pImpl->focusView->invalid ();

		auto receiver = pImpl->focusView->getParentView ();
		while (receiver.get () != this && receiver != nullptr)
		{
			receiver->notify (pImpl->focusView.get (), kMsgNewFocusView);
			receiver = receiver->getParentView ();
		}
		notify (pImpl->focusView.get (), kMsgNewFocusView);
	}

	if (pOldFocusView)
	{
		if (pOldFocusView->wantsFocus ())
		{
			pOldFocusView->invalid ();

			auto receiver = pOldFocusView->getParentView ();
			while (receiver.get () != this && receiver != nullptr)
			{
				receiver->notify (pOldFocusView.get (), kMsgOldFocusView);
				receiver = receiver->getParentView ();
			}
			notify (pOldFocusView.get (), kMsgOldFocusView);
		}
		pOldFocusView->looseFocus ();
	}
	if (pImpl->focusView && pImpl->focusView->wantsFocus ())
		pImpl->focusView->takeFocus ();

	pImpl->focusViewObservers.forEach ([&] (IFocusViewObserver* observer) {
		observer->onFocusViewChanged (*this, pImpl->focusView, pOldFocusView);
	});

	recursion = false;
}

//-----------------------------------------------------------------------------
SharedPointer<CView> CFrame::getFocusView () const { return pImpl->focusView; }

//-----------------------------------------------------------------------------
bool CFrame::advanceNextFocusView (const SharedPointer<CView>& _oldFocus, bool reverse)
{
	auto oldFocus = _oldFocus;
	if (auto modalView = getModalView ())
	{
		if (auto container = modalView->asViewContainer ())
		{
			if (oldFocus == nullptr || container->isChild (oldFocus, true) == false)
				return container->advanceNextFocusView (nullptr, reverse);
			else
			{
				if (auto parentView = oldFocus->getParentView ())
				{
					auto tempOldFocus = oldFocus;
					while (parentView != container)
					{
						if (parentView->advanceNextFocusView (tempOldFocus, reverse))
							return true;
						else
						{
							tempOldFocus = parentView;
							parentView = parentView->getParentView ();
						}
					}
					if (container->advanceNextFocusView (tempOldFocus, reverse))
						return true;
					return container->advanceNextFocusView (nullptr, reverse);
				}
			}
		}
		else if (oldFocus != modalView)
		{
			setFocusView (modalView);
			return true;
		}
		return false; // currently not supported, but should be done sometime
	}
	if (oldFocus == nullptr)
	{
		if (pImpl->focusView == nullptr)
			return CViewContainer::advanceNextFocusView (nullptr, reverse);
		oldFocus = pImpl->focusView;
	}
	if (isChild (oldFocus))
	{
		if (CViewContainer::advanceNextFocusView (oldFocus, reverse))
			return true;
		else
		{
			setFocusView (nullptr);
			return false;
		}
	}
	if (auto parentView = oldFocus->getParentView ())
	{
		auto tempOldFocus = oldFocus;
		while (parentView)
		{
			if (parentView->advanceNextFocusView (tempOldFocus, reverse))
				return true;
			else
			{
				tempOldFocus = parentView;
				parentView = parentView->getParentView ();
			}
		}
	}
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//-----------------------------------------------------------------------------
bool CFrame::removeSubview (const SharedPointer<CView>& view)
{
#if DEBUG
	vstgui_assert (getModalView () != view);
#endif
	return CViewContainer::removeSubview (view);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll ()
{
	clearModalViewSessions ();
	if (pImpl->focusView)
	{
		pImpl->focusView->looseFocus ();
		pImpl->focusView = nullptr;
	}
	pImpl->activeFocusView = nullptr;
	clearMouseViews (CPoint (0, 0), Modifiers (), false);
	return CViewContainer::removeAll ();
}

//-----------------------------------------------------------------------------
SharedPointer<CView> CFrame::getViewAt (const CPoint& where, const GetViewOptions& options) const
{
	if (auto modalView = getModalView ())
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (modalView->getViewSize ().pointInside (where2))
		{
			if (options.getDeep ())
			{
				if (auto container = modalView->asViewContainer ())
				{
					return container->getViewAt (where2, options);
				}
			}
			return modalView;
		}
		return nullptr;
	}
	return CViewContainer::getViewAt (where, options);
}

//-----------------------------------------------------------------------------
SharedPointer<CViewContainer> CFrame::getContainerAt (const CPoint& where,
													  const GetViewOptions& options) const
{
	if (auto modalView = getModalView ())
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (modalView->getViewSize ().pointInside (where2))
		{
			if (auto container = modalView->asViewContainer ())
			{
				if (options.getDeep ())
					return container->getContainerAt (where2, options);
				return container;
			}
		}
		return nullptr;
	}
	return CViewContainer::getContainerAt (where, options);
}

//------------------------------------------------------------------------
bool CFrame::getViewsAt (const CPoint& where, ViewList& views, const GetViewOptions& options) const
{
	if (auto modalView = getModalView ())
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (modalView->getViewSize ().pointInside (where2))
		{
			if (options.getDeep ())
			{
				if (auto container = modalView->asViewContainer ())
				{
					container->getViewsAt (where2, views, options);
				}
			}
			if (!options.getIncludeViewContainer () && modalView->asViewContainer ())
				return true;
			if (options.getMouseEnabled () && modalView->getMouseEnabled () == false)
				return true;
			if (!options.getIncludeInvisible () && !modalView->isVisible ())
				return true;
			views.emplace_back (modalView);
			return true;
		}
		return false;
	}
	return CViewContainer::getViewsAt (where, views, options);
}

//-----------------------------------------------------------------------------
void CFrame::onActivate (bool state)
{
	if (pImpl->active != state)
	{
		if (state)
		{
			pImpl->active = true;
			if (pImpl->activeFocusView)
			{
				setFocusView (pImpl->activeFocusView);
				pImpl->activeFocusView = nullptr;
			}
			else
				advanceNextFocusView (nullptr, false);
		}
		else
		{
			if (pImpl->tooltips)
				pImpl->tooltips->hideTooltip ();
			pImpl->activeFocusView = getFocusView ();
			setFocusView (nullptr);
			pImpl->active = false;
		}
	}
}

//-----------------------------------------------------------------------------
bool CFrame::focusDrawingEnabled () const
{
	uint32_t attrSize;
	if (getAttributeSize ('vfde', attrSize))
		return true;
	return false;
}

//-----------------------------------------------------------------------------
CColor CFrame::getFocusColor () const
{
	CColor focusColor (kRedCColor);
	getAttribute ('vfco', focusColor);
	return focusColor;
}

//-----------------------------------------------------------------------------
CCoord CFrame::getFocusWidth () const
{
	CCoord focusWidth = 2;
	getAttribute ('vfwi', focusWidth);
	return focusWidth;
}

//-----------------------------------------------------------------------------
void CFrame::setFocusDrawingEnabled (bool state)
{
	if (state)
		setAttribute ('vfde', state);
	else
		removeAttribute ('vfde');
}

//-----------------------------------------------------------------------------
void CFrame::setFocusColor (const CColor& color)
{
	setAttribute ('vfco', color);
}

//-----------------------------------------------------------------------------
void CFrame::setFocusWidth (CCoord width)
{
	setAttribute ('vfwi', width);
}

//-----------------------------------------------------------------------------
/**
 * @param src rect which to scroll
 * @param distance point of distance
 */
void CFrame::scrollRect (const CRect& src, const CPoint& distance)
{
	CRect rect (src);
	rect.offset (getViewSize ().left, getViewSize ().top);

	if (pImpl->platformFrame)
	{
		if (pImpl->platformFrame->scrollRect (src, distance))
			return;
	}
	invalidRect (src);
}

//-----------------------------------------------------------------------------
void CFrame::invalidate (const CRect &rect)
{
	for (const auto& pV : getChildren ())
	{
		CRect rectView = pV->getViewSize ();
		if (rect.rectOverlap (rectView))
			pV->invalid ();
	}
}

//-----------------------------------------------------------------------------
void CFrame::invalidRect (const CRect& rect)
{
	if (!isVisible () || !pImpl->platformFrame)
		return;

	CRect _rect (rect);
	getTransform ().transform (_rect);
	_rect.makeIntegral ();
	if (pImpl->collectInvalidRects)
		pImpl->collectInvalidRects->addRect (_rect);
	else
		pImpl->platformFrame->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
IViewAddedRemovedObserver* CFrame::getViewAddedRemovedObserver () const
{
	return pImpl->viewAddedRemovedObserver;
}

//-----------------------------------------------------------------------------
void CFrame::setViewAddedRemovedObserver (IViewAddedRemovedObserver* observer)
{
	pImpl->viewAddedRemovedObserver = observer;
}

//-----------------------------------------------------------------------------
void CFrame::registerKeyboardHook (IKeyboardHook* hook)
{
	pImpl->keyboardHooks.add (hook);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterKeyboardHook (IKeyboardHook* hook)
{
	pImpl->keyboardHooks.remove (hook);
}

//-----------------------------------------------------------------------------
void CFrame::dispatchKeyboardEventToHooks (KeyboardEvent& event)
{
	pImpl->keyboardHooks.forEachReverse (
		[&] (IKeyboardHook* hook) {
			hook->onKeyboardEvent (event, *this);
			return event.consumed;
		},
		[] (bool consumed) { return consumed; });
}

//-----------------------------------------------------------------------------
void CFrame::registerScaleFactorChangedListener (IScaleFactorChangedListener* listener)
{
	pImpl->scaleFactorChangedListenerList.add (listener);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterScaleFactorChangedListener (IScaleFactorChangedListener* listener)
{
	pImpl->scaleFactorChangedListenerList.remove (listener);
}

//-----------------------------------------------------------------------------
void CFrame::registerMouseObserver (IMouseObserver* observer)
{
	pImpl->mouseObservers.add (observer);
}

//-----------------------------------------------------------------------------
void CFrame::registerFocusViewObserver (IFocusViewObserver* observer)
{
	pImpl->focusViewObservers.add (observer);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterFocusViewObserver (IFocusViewObserver* observer)
{
	pImpl->focusViewObservers.remove (observer);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterMouseObserver (IMouseObserver* observer)
{
	pImpl->mouseObservers.remove (observer);
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseEntered (CView& view)
{
	pImpl->mouseObservers.forEach (
		[&] (IMouseObserver* observer) { observer->onMouseEntered (view, *this); });
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseExited (CView& view)
{
	pImpl->mouseObservers.forEach (
		[&] (IMouseObserver* observer) { observer->onMouseExited (view, *this); });
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverOtherMouseEvent (MouseEvent& event)
{
	pImpl->mouseObservers.forEach (
		[&] (IMouseObserver* observer) { observer->onMouseEvent (event, *this); });
}

//------------------------------------------------------------------------
bool CFrame::performDrag (const DragDescription& desc, const SharedPointer<IDragCallback>& callback)
{
	if (auto platformFrame = getPlatformFrame ())
	{
		if (platformFrame->doDrag (desc, callback))
		{
			setMouseDownView (nullptr);
			return true;
		}
	}
	return false;
}

#if DEBUG
//-----------------------------------------------------------------------------
void CFrame::dumpHierarchy ()
{
	dumpInfo ();
	DebugPrint ("\n");
	CViewContainer::dumpHierarchy ();
}
#endif

//-----------------------------------------------------------------------------
VSTGUIEditorInterface* CFrame::getEditor () const
{
	return pImpl->editor;
}

//-----------------------------------------------------------------------------
PlatformFramePtr CFrame::getPlatformFrame () const { return pImpl->platformFrame; }

//-----------------------------------------------------------------------------
void CFrame::platformDrawRects (const PlatformGraphicsDeviceContextPtr& context, double scaleFactor,
								const std::vector<CRect>& rects)
{
	CDrawContext drawContext (context, getViewSize (), scaleFactor);
	for (auto rect : rects)
		drawRect (drawContext, rect);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnEvent (Event& event)
{
	dispatchEvent (event);
}

//-----------------------------------------------------------------------------
DragOperation CFrame::platformOnDragEnter (DragEventData data)
{
	if (!getMouseEnabled ())
		return DragOperation::None;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return getDropTarget ()->onDragEnter (data);
}

//-----------------------------------------------------------------------------
DragOperation CFrame::platformOnDragMove (DragEventData data)
{
	if (!getMouseEnabled ())
		return DragOperation::None;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return getDropTarget ()->onDragMove (data);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragLeave (DragEventData data)
{
	if (!getMouseEnabled ())
		return;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	getDropTarget ()->onDragLeave (data);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnDrop (DragEventData data)
{
	if (!getMouseEnabled ())
		return false;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return getDropTarget ()->onDrop (data);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnActivate (bool state)
{
	if (getFrame ())
	{
		CollectInvalidRects cir (this);
		onActivate (state);
	}
}

//------------------------------------------------------------------------
void CFrame::platformOnWindowActivate (bool state)
{
	if (pImpl->windowActive == state)
		return;
	pImpl->windowActive = state;
	CollectInvalidRects cir (this);
	pImpl->windowActiveStateChangeViews.forEach ([&] (CView* view) {
		view->onWindowActivate (state);
	});
}

//-----------------------------------------------------------------------------
void CFrame::platformScaleFactorChanged (double newScaleFactor)
{
	if (pImpl->platformScaleFactor == newScaleFactor)
		return;
	pImpl->platformScaleFactor = newScaleFactor;
	dispatchNewScaleFactor (getScaleFactor ());
}

//-----------------------------------------------------------------------------
void CFrame::dispatchNewScaleFactor (double newScaleFactor)
{
	pImpl->scaleFactorChangedListenerList.forEach ([&] (IScaleFactorChangedListener* listener) {
		listener->onScaleFactorChanged (*this, newScaleFactor);
	});
}

#if VSTGUI_TOUCH_EVENT_HANDLING
//-----------------------------------------------------------------------------
void CFrame::platformOnTouchEvent (ITouchEvent& event)
{
	Impl::PostEventHandler peh (*pImpl);
	std::vector<CView*> targetDispatched;
	bool hasBeganTouch = false;
	for (const auto& e : event)
	{
		CView* target = e.second.target;
		if (target)
		{
			if (e.second.targetIsSingleTouch)
			{
				CPoint where (e.second.location);
				target->frameToLocal (where);
				switch (e.second.state)
				{
					case ITouchEvent::kMoved:
					{
						MouseMoveEvent moveEvent (where, MouseButton::Left);
						dispatchEvent (target, moveEvent);
						if (moveEvent.ignoreFollowUpMoveAndUpEvents ())
						{
							event.unsetTouchTarget (e.first, target);
							MouseMoveEvent mouseMoveEvent (where, MouseButton::Left);
							if (target->hitTest (where, mouseMoveEvent) == false)
							{
								// when the touch goes out of the target and it tells us to
								const_cast<ITouchEvent::Touch&> (e.second).state =
								    ITouchEvent::kBegan;
								hasBeganTouch = true;
							}
						}
						break;
					}
					case ITouchEvent::kCanceled:
					{
						MouseCancelEvent cancelEvent;
						dispatchEvent (target, cancelEvent);
						if (cancelEvent.consumed == false)
						{
							MouseUpEvent upEvent (where, MouseButton::Left);
							dispatchEvent (target, upEvent);
						}
						event.unsetTouchTarget (e.first, target);
						break;
					}
					case ITouchEvent::kEnded:
					{
						MouseUpEvent upEvent (where, MouseButton::Left);
						dispatchEvent (target, upEvent);
						event.unsetTouchTarget (e.first, target);
						break;
					}
					default:
					{
						// do nothing
						break;
					}
				}
			}
			else
			{
				if (std::find (targetDispatched.begin (), targetDispatched.end (), target) ==
				    targetDispatched.end ())
				{
					target->onTouchEvent (event);
					targetDispatched.emplace_back (target);
				}
			}
		}
		else if (e.second.state == ITouchEvent::kBegan)
		{
			hasBeganTouch = true;
		}
	}
	if (hasBeganTouch)
	{
		if (CView* focusView = getFocusView ())
		{
			if (dynamic_cast<CTextEdit*> (focusView))
				setFocusView (nullptr);
		}
		for (const auto& e : event)
		{
			if (e.second.target == nullptr && e.second.state == ITouchEvent::kBegan)
			{
				findSingleTouchEventTarget (const_cast<ITouchEvent::Touch&> (e.second));
			}
		}
		onTouchEvent (event);
	}
}

#endif

//-----------------------------------------------------------------------------
bool CFrame::doAfterEventProcessing (EventProcessingFunction&& func)
{
	if (pImpl->inEventHandling)
		pImpl->postEventFunctionQueue.push (std::move (func));
	return pImpl->inEventHandling;
}

//-----------------------------------------------------------------------------
bool CFrame::doAfterEventProcessing (const EventProcessingFunction& func)
{
	if (pImpl->inEventHandling)
		pImpl->postEventFunctionQueue.push (func);
	return pImpl->inEventHandling;
}

//-----------------------------------------------------------------------------
bool CFrame::inEventProcessing () const
{
	return pImpl->inEventHandling;
}

//-----------------------------------------------------------------------------
void CFrame::onStartLocalEventLoop ()
{
	if (pImpl->collectInvalidRects)
	{
		pImpl->collectInvalidRects->flush ();
		pImpl->collectInvalidRects = nullptr;
	}
}

//-----------------------------------------------------------------------------
void CFrame::setCollectInvalidRects (CollectInvalidRects* cir)
{
	if (pImpl->collectInvalidRects)
		pImpl->collectInvalidRects->flush ();
	pImpl->collectInvalidRects = cir;
}

//-----------------------------------------------------------------------------
CFrame::CollectInvalidRects::CollectInvalidRects (CFrame* frame)
: frame (frame)
, lastTicks (frame->getTicks ())
{
#if VSTGUI_LOG_COLLECT_INVALID_RECTS
	numAddedRects = 0;
#endif
	frame->setCollectInvalidRects (this);
}

//-----------------------------------------------------------------------------
CFrame::CollectInvalidRects::~CollectInvalidRects () noexcept
{
	frame->setCollectInvalidRects (nullptr);
}

//-----------------------------------------------------------------------------
void CFrame::CollectInvalidRects::flush ()
{
	if (!invalidRects.data ().empty ())
	{
		if (frame->isVisible () && frame->pImpl->platformFrame)
		{
			for (auto& rect : invalidRects)
				frame->pImpl->platformFrame->invalidRect (rect);
		#if VSTGUI_LOG_COLLECT_INVALID_RECTS
			DebugPrint ("%d -> %d\n", numAddedRects, invalidRects.size ());
			numAddedRects = 0;
		#endif
		}
		invalidRects.clear ();
	}
}

//-----------------------------------------------------------------------------
void CFrame::CollectInvalidRects::addRect (const CRect& rect)
{
#if VSTGUI_LOG_COLLECT_INVALID_RECTS
	numAddedRects++;
#endif
	invalidRects.add (rect);
	auto now = frame->getTicks ();
	if (now - lastTicks > 16)
	{
		flush ();
		lastTicks = now;
	}
}

} // VSTGUI
