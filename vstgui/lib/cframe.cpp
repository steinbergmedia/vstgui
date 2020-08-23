// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cframe.h"
#include "coffscreencontext.h"
#include "ctooltipsupport.h"
#include "cinvalidrectlist.h"
#include "itouchevent.h"
#include "iscalefactorchangedlistener.h"
#include "idatapackage.h"
#include "animation/animator.h"
#include "controls/ctextedit.h"
#include "platform/iplatformframe.h"
#include <cassert>
#include <vector>
#include <queue>
#include <limits>

namespace VSTGUI {

IdStringPtr kMsgNewFocusView = "kMsgNewFocusView";
IdStringPtr kMsgOldFocusView = "kMsgOldFocusView";

#define DEBUG_MOUSE_VIEWS	0//DEBUG

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
	uint32_t lastTicks;
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
	using ViewList = std::list<CView*>;
	using FunctionQueue = std::queue<EventProcessingFunction>;
	using ModalViewSessionStack = std::stack<ModalViewSession>;

	SharedPointer<IPlatformFrame> platformFrame;
	VSTGUIEditorInterface* editor {nullptr};
	IViewAddedRemovedObserver* viewAddedRemovedObserver {nullptr};
	SharedPointer<CTooltipSupport> tooltips;
	SharedPointer<Animation::Animator> animator;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	Optional<ModalViewSessionID> legacyModalViewSessionID;
#endif
	CView* focusView {nullptr};
	CView* activeFocusView {nullptr};
	CollectInvalidRects* collectInvalidRects {nullptr};
	
	ViewList mouseViews;
	ModalViewSessionStack modalViewSessionStack;
	DispatchList<CView*> windowActiveStateChangeViews;
	DispatchList<IScaleFactorChangedListener*> scaleFactorChangedListenerList;
	DispatchList<IMouseObserver*> mouseObservers;
	DispatchList<IFocusViewObserver*> focusViewObservers;
	DispatchList<IKeyboardHook*> keyboardHooks;
	FunctionQueue postEventFunctionQueue;

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
It creates a platform dependend view object. 

On Mac OS X it is a HIView or NSView.\n 
On Windows it's a WS_CHILD Window.

*/
//-----------------------------------------------------------------------------
CFrame::CFrame (const CRect& inSize, VSTGUIEditorInterface* inEditor) : CViewContainer (inSize)
{
	pImpl = new Impl;
	pImpl->editor = inEditor;

	setParentFrame (this);
}

//-----------------------------------------------------------------------------
void CFrame::beforeDelete ()
{
	clearMouseViews (CPoint (0, 0), 0, false);

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
	
	delete pImpl;
	pImpl = nullptr;
	
	CViewContainer::beforeDelete ();
}

//-----------------------------------------------------------------------------
void CFrame::close ()
{
	clearMouseViews (CPoint (0, 0), 0, false);

	clearModalViewSessions ();

	setCursor (kCursorDefault);
	setParentFrame (nullptr);
	removeAll ();
	if (pImpl->platformFrame)
	{
		pImpl->platformFrame->onFrameClosed ();
		pImpl->platformFrame = nullptr;
	}
	forget ();
}

//-----------------------------------------------------------------------------
bool CFrame::open (void* systemWin, PlatformType systemWindowType, IPlatformFrameConfig* config)
{
	if (!systemWin || isAttached ())
		return false;

	pImpl->platformFrame = owned (IPlatformFrame::createPlatformFrame (this, getViewSize (), systemWin, systemWindowType, config));
	if (!pImpl->platformFrame)
	{
		return false;
	}

	CollectInvalidRects cir (this);

	attached (this);
	
	setParentView (nullptr);

	invalid ();

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::attached (CView* parent)
{
	if (isAttached ())
		return false;
	vstgui_assert (parent == this);
	if (CView::attached (parent))
	{
		setParentView (nullptr);

		for (const auto& pV : getChildren ())
			pV->attached (this);
		
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
	CCoord newWidth = std::round (origWidth * zoomFactor);
	CCoord newHeight = std::round (origHeight * zoomFactor);
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
			pImpl->tooltips = makeOwned<CTooltipSupport> (this, delayTimeInMs);
	}
	else if (pImpl->tooltips)
	{
		pImpl->tooltips = nullptr;
	}
}

//-----------------------------------------------------------------------------
void CFrame::draw (CDrawContext* pContext)
{
	return CFrame::drawRect (pContext, getViewSize ());
}

//-----------------------------------------------------------------------------
void CFrame::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (updateRect.getWidth () <= 0 || updateRect.getHeight () <= 0 || pContext == nullptr)
		return;

	auto lifeGuard = shared (pContext);

	if (pImpl)
		pContext->setBitmapInterpolationQuality (pImpl->bitmapQuality);

	drawClipped (pContext, updateRect, [&] () {
		// draw the background and the children
		CViewContainer::drawRect (pContext, updateRect);
	});
}

//-----------------------------------------------------------------------------
void CFrame::clearMouseViews (const CPoint& where, const CButtonState& buttons, bool callMouseExit)
{
	CPoint lp;
	auto it = pImpl->mouseViews.rbegin ();
	while (it != pImpl->mouseViews.rend ())
	{
		if (callMouseExit)
		{
			lp = where;
			lp = (*it)->translateToLocal (lp);
			(*it)->onMouseExited (lp, buttons);
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseExited : %p[%d,%d]\n", (*it), (int)lp.x, (int)lp.y);
		#endif
		}
		if (pImpl->tooltips)
			pImpl->tooltips->onMouseExited ((*it));

		callMouseObserverMouseExited ((*it));

		(*it)->forget ();
		++it;
	}
	pImpl->mouseViews.clear ();
}

//-----------------------------------------------------------------------------
void CFrame::removeFromMouseViews (CView* view)
{
	bool found = false;
	auto it = pImpl->mouseViews.begin ();
	while (it != pImpl->mouseViews.end ())
	{
		if (found || (*it) == view)
		{
			if (pImpl->tooltips)
				pImpl->tooltips->onMouseExited ((*it));

			callMouseObserverMouseExited ((*it));

			(*it)->forget ();
			pImpl->mouseViews.erase (it++);
			found = true;
		}
		else
			++it;
	}
}

//-----------------------------------------------------------------------------
void CFrame::checkMouseViews (const CPoint& where, const CButtonState& buttons)
{
	if (getMouseDownView ())
		return;
	CPoint lp;
	CView* mouseView = getViewAt (where, GetViewOptions ().deep ().mouseEnabled ().includeViewContainer ());
	CView* currentMouseView = pImpl->mouseViews.empty () == false ? pImpl->mouseViews.back () : nullptr;
	if (currentMouseView == mouseView)
		return; // no change

	if (pImpl->tooltips)
	{
		if (currentMouseView)
			pImpl->tooltips->onMouseExited (currentMouseView);
		if (mouseView && mouseView != this)
			pImpl->tooltips->onMouseEntered (mouseView);
	}

	if (mouseView == nullptr || mouseView == this)
	{
		clearMouseViews (where, buttons);
		return;
	}
	CViewContainer* vc = currentMouseView ? currentMouseView->asViewContainer () : nullptr;
	// if the currentMouseView is not a view container, we know that the new mouseView won't be a child of it and that all other
	// views in the list are viewcontainers
	if (vc == nullptr && currentMouseView)
	{
		lp = where;
		lp = currentMouseView->translateToLocal (lp);
		currentMouseView->onMouseExited (lp, buttons);
		callMouseObserverMouseExited (currentMouseView);
	#if DEBUG_MOUSE_VIEWS
		DebugPrint ("mouseExited : %p[%d,%d]\n", currentMouseView, (int)lp.x, (int)lp.y);
	#endif
		currentMouseView->forget ();
		pImpl->mouseViews.remove (currentMouseView);
	}
	auto it = pImpl->mouseViews.rbegin ();
	while (it != pImpl->mouseViews.rend ())
	{
		vc = static_cast<CViewContainer*> ((*it));
		if (vc == mouseView)
			return;
		if (vc->isChild (mouseView, true) == false)
		{
			lp = where;
			lp = vc->translateToLocal (lp);
			vc->onMouseExited (lp, buttons);
			callMouseObserverMouseExited (vc);
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseExited : %p[%d,%d]\n", vc, (int)lp.x, (int)lp.y);
		#endif
			vc->forget ();
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
		CView* container = mouseView;
		while ((vc = static_cast<CViewContainer*> (container->getParentView ())) != *it2 && vc)
		{
			pImpl->mouseViews.emplace_back (vc);
			vc->remember ();
			container = vc;
		}
		pImpl->mouseViews.emplace_back (mouseView);
		mouseView->remember ();
		++it2;
		while (it2 != pImpl->mouseViews.end ())
		{
			lp = where;
			lp = (*it2)->translateToLocal (lp);
			(*it2)->onMouseEntered (lp, buttons);
			callMouseObserverMouseEntered ((*it2));
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseEntered : %p[%d,%d]\n", (*it2), (int)lp.x, (int)lp.y);
		#endif
			++it2;
		}
	}
	else
	{
		// must be pMouseViews.size () == 0
		vstgui_assert (pImpl->mouseViews.empty ());
		pImpl->mouseViews.emplace_back (mouseView);
		mouseView->remember ();
		while ((vc = static_cast<CViewContainer*> (mouseView->getParentView ())) != this && vc)
		{
			pImpl->mouseViews.push_front (vc);
			vc->remember ();
			mouseView = vc;
		}
		auto it2 = pImpl->mouseViews.begin ();
		while (it2 != pImpl->mouseViews.end ())
		{
			lp = where;
			lp = (*it2)->translateToLocal (lp);
			(*it2)->onMouseEntered (lp, buttons);
			callMouseObserverMouseEntered ((*it2));
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseEntered : %p[%d,%d]\n", (*it2), (int)lp.x, (int)lp.y);
		#endif
			++it2;
		}
	}
}

//------------------------------------------------------------------------
bool CFrame::hitTestSubViews (const CPoint& where, const CButtonState& buttons)
{
	if (auto modalView = getModalView ())
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (modalView->isVisible () && modalView->getMouseEnabled () && modalView->hitTest (where2, buttons))
		{
			if (auto viewContainer = modalView->asViewContainer ())
			{
				return viewContainer->hitTestSubViews (where2, buttons);
			}
			return true;
		}
		return false;
	}
	return CViewContainer::hitTestSubViews (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	CPoint where2 (where);
	getTransform ().inverse ().transform (where2);

	if (pImpl->tooltips)
		pImpl->tooltips->onMouseDown (where2);

	CMouseEventResult result = callMouseObserverMouseDown (where, buttons);
	if (result != kMouseEventNotHandled)
		return result;

	// reset views
	setMouseDownView (nullptr);
	if (pImpl->focusView && dynamic_cast<CTextEdit*> (pImpl->focusView))
		setFocusView (nullptr);

	if (auto modalView = getModalView ())
	{
		CBaseObjectGuard rg (modalView);

		if (modalView->isVisible () && modalView->getMouseEnabled ())
		{
			result = modalView->callMouseListener (MouseListenerCall::MouseDown, where2, buttons);
			if (result == kMouseEventNotHandled || result == kMouseEventNotImplemented)
				result = modalView->onMouseDown (where2, buttons);
			if (result == kMouseEventHandled)
			{
				setMouseDownView (modalView);
				return kMouseEventHandled;
			}
			return result;
		}
	}
	else
		return CViewContainer::onMouseDown (where, buttons);
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	CMouseEventResult result = CViewContainer::onMouseUp (where, buttons);
	CButtonState modifiers = buttons & (kShift | kControl | kAlt | kApple);
	checkMouseViews (where, modifiers);
	return result;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	CPoint where2 (where);
	getTransform ().inverse ().transform (where2);
	
	if (pImpl->tooltips)
		pImpl->tooltips->onMouseMoved (where2);

	checkMouseViews (where, buttons);

	CMouseEventResult result = callMouseObserverMouseMoved (where, buttons);
	if (result != kMouseEventNotHandled)
		return result;

	if (auto modalView = getModalView ())
	{
		CBaseObjectGuard rg (modalView);
		result = modalView->callMouseListener (MouseListenerCall::MouseMoved, where2, buttons);
		if (result == kMouseEventNotHandled || result == kMouseEventNotImplemented)
			result = modalView->onMouseMoved (where2, buttons);
	}
	else
	{
		result = CViewContainer::onMouseMoved (where, buttons);
	}
	if (result == kMouseEventNotHandled)
	{
		CButtonState buttons2 = (buttons & (kShift | kControl | kAlt | kApple));
		auto it = pImpl->mouseViews.rbegin ();
		while (it != pImpl->mouseViews.rend ())
		{
			CPoint p = (*it)->translateToLocal (where2);
			result = (*it)->onMouseMoved (p, buttons2);
			if (result == kMouseEventHandled)
				break;
			++it;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseExited (CPoint &where, const CButtonState& buttons)
{ // this should only get called from the platform implementation

	if (getMouseDownView () == nullptr)
	{
		clearMouseViews (where, buttons);
		if (pImpl->tooltips)
			pImpl->tooltips->hideTooltip ();
	}

	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
int32_t CFrame::onKeyDown (VstKeyCode& keyCode)
{
	int32_t result = keyboardHooksOnKeyDown (keyCode);

	if (result == -1 && pImpl->focusView)
	{
		CBaseObjectGuard og (pImpl->focusView);
		if (pImpl->focusView->getMouseEnabled ())
			result = pImpl->focusView->onKeyDown (keyCode);
		if (result == -1)
		{
			CView* parent = pImpl->focusView->getParentView ();
			while (parent && parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyDown (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1)
	{
		if (auto modalView = getModalView ())
		{
			CBaseObjectGuard og (modalView);
			result = modalView->onKeyDown (keyCode);
		}
	}

	if (result == -1 && keyCode.virt == VKEY_TAB)
	{
		if (keyCode.modifier == 0 || keyCode.modifier == MODIFIER_SHIFT)
			result = advanceNextFocusView (pImpl->focusView, (keyCode.modifier & MODIFIER_SHIFT) ? true : false) ? 1 : -1;
	}

	return result;
}

//-----------------------------------------------------------------------------
int32_t CFrame::onKeyUp (VstKeyCode& keyCode)
{
	int32_t result = keyboardHooksOnKeyUp (keyCode);

	if (result == -1 && pImpl->focusView)
	{
		if (pImpl->focusView->getMouseEnabled ())
			result = pImpl->focusView->onKeyUp (keyCode);
		if (result == -1)
		{
			CView* parent = pImpl->focusView->getParentView ();
			while (parent && parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyUp (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1)
	{
		if (auto modalView = getModalView ())
			result = modalView->onKeyUp (keyCode);
	}

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	if (auto modalView = getModalView ())
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		return modalView->onWheel (where2, axis, distance, buttons);
	}

	bool result = false;
	if (getMouseDownView () == nullptr)
	{
		result = CViewContainer::onWheel (where, axis, distance, buttons);
		checkMouseViews (where, buttons);
	}
	return result;
}

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> CFrame::getClipboard ()
{
	if (pImpl->platformFrame)
		return pImpl->platformFrame->getClipboard ();
	return nullptr;
}

//-----------------------------------------------------------------------------
void CFrame::setClipboard (const SharedPointer<IDataPackage>& data)
{
	if (pImpl->platformFrame)
		pImpl->platformFrame->setClipboard (data);
}

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	if (CView::kDirtyCallAlwaysOnMainThread)
		return;
	invalidateDirtyViews ();
}

//-----------------------------------------------------------------------------
Animation::Animator* CFrame::getAnimator ()
{
	if (pImpl->animator == nullptr)
		pImpl->animator = makeOwned<Animation::Animator> ();
	return pImpl->animator;
}

//-----------------------------------------------------------------------------
/**
 * @return tick count in milliseconds
 */
uint32_t CFrame::getTicks () const
{
	if (pImpl->platformFrame)
		return pImpl->platformFrame->getTicks ();
	return std::numeric_limits<uint32_t>::max ();
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
	CViewContainer::setViewSize (rect, invalid);
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
		return false;

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
			CViewContainer::setViewSize (newSize);
			return true;
		}
		return false;
	}
	CViewContainer::setViewSize (newSize);
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

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be set to modal.
 * @return true if view could be set as the modal view. false if there is a already a modal view or the view to be set as modal is already attached.
 */
bool CFrame::setModalView (CView* pView)
{
	if (pImpl->modalViewSessionStack.empty () && pView == nullptr)
		return true;

	if (pView && !pImpl->modalViewSessionStack.empty ())
		return false;

	if (!pView)
		endLegacyModalViewSession ();
	else
		pImpl->legacyModalViewSessionID = beginModalViewSession (pView);
	
	return true;
}

//-----------------------------------------------------------------------------
void CFrame::endLegacyModalViewSession ()
{
	vstgui_assert (pImpl->legacyModalViewSessionID);
	vstgui_assert (pImpl->modalViewSessionStack.top ().identifier ==
	               *pImpl->legacyModalViewSessionID);
	pImpl->modalViewSessionStack.top ().view->remember ();
	endModalViewSession (*pImpl->legacyModalViewSessionID);
	pImpl->legacyModalViewSessionID = {};
}

#endif

//-----------------------------------------------------------------------------
CView* CFrame::getModalView () const
{
	if (!pImpl->modalViewSessionStack.empty ())
		return pImpl->modalViewSessionStack.top ().view;
	return nullptr;
}

//-----------------------------------------------------------------------------
void CFrame::initModalViewSession (const ModalViewSession& session)
{
	if (auto view = getMouseDownView ())
	{
		onMouseCancel ();
	}
	clearMouseViews (CPoint (0, 0), 0, true);
	if (auto container = session.view->asViewContainer ())
		container->advanceNextFocusView (nullptr, false);
	else
		setFocusView (session.view->wantsFocus () ? session.view : nullptr);

	if (isAttached ())
	{
		CPoint where;
		getCurrentMouseLocation (where);
		checkMouseViews (where, getCurrentMouseButtons ());
	}
}

//-----------------------------------------------------------------------------
void CFrame::clearModalViewSessions ()
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	if (pImpl->legacyModalViewSessionID)
		endLegacyModalViewSession ();
#endif
	while (!pImpl->modalViewSessionStack.empty ())
		endModalViewSession (pImpl->modalViewSessionStack.top ().identifier);
}

//-----------------------------------------------------------------------------
Optional<ModalViewSessionID> CFrame::beginModalViewSession (CView* view)
{
	if (view->isAttached ())
	{
#if DEBUG
		DebugPrint ("the view must not be attached when used for beginModalViewSession");
#endif
		return {};
	}

	if (!addView (view))
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

	removeView (view);

	if (!pImpl->modalViewSessionStack.empty ())
		initModalViewSession (pImpl->modalViewSessionStack.top ());

	return true;
}

//-----------------------------------------------------------------------------
void CFrame::beginEdit (int32_t index)
{
	if (pImpl->editor)
		pImpl->editor->beginEdit (index);
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (int32_t index)
{
	if (pImpl->editor)
		pImpl->editor->endEdit (index);
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
			getTransform().transform (where);
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
void CFrame::onViewRemoved (CView* pView)
{
	removeFromMouseViews (pView);

	if (pImpl->activeFocusView == pView)
		pImpl->activeFocusView = nullptr;
	if (pImpl->focusView == pView)
	{
		if (pImpl->active)
			setFocusView (nullptr);
		else
			pImpl->focusView = nullptr;
	}
	if (auto container = pView->asViewContainer ())
	{
		if (container->isChild (pImpl->focusView, true))
			setFocusView (nullptr);
	}
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewRemoved (this, pView);
	if (pView->wantsWindowActiveStateChangeNotification ())
		pImpl->windowActiveStateChangeViews.remove (pView);
	if (pImpl->animator)
		pImpl->animator->removeAnimations (pView);
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was added
 */
void CFrame::onViewAdded (CView* pView)
{
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewAdded (this, pView);
	if (pView->wantsWindowActiveStateChangeNotification ())
	{
		pImpl->windowActiveStateChangeViews.add (pView);
		pView->onWindowActivate (pImpl->windowActive);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pView new focus view
 */
void CFrame::setFocusView (CView *pView)
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

	CView *pOldFocusView = pImpl->focusView;
	if (pView == nullptr  || (pView && pView->isAttached () == false))
		pImpl->focusView = nullptr;
	else
		pImpl->focusView = pView;
	if (pImpl->focusView && pImpl->focusView->wantsFocus ())
	{
		pImpl->focusView->invalid ();

		CView* receiver = pImpl->focusView->getParentView ();
		while (receiver != this && receiver != nullptr)
		{
			receiver->notify (pImpl->focusView, kMsgNewFocusView);
			receiver = receiver->getParentView ();
		}
		notify (pImpl->focusView, kMsgNewFocusView);
	}

	if (pOldFocusView)
	{
		if (pOldFocusView->wantsFocus ())
		{
			pOldFocusView->invalid ();

			CView* receiver = pOldFocusView->getParentView ();
			while (receiver != this && receiver != nullptr)
			{
				receiver->notify (pOldFocusView, kMsgOldFocusView);
				receiver = receiver->getParentView ();
			}
			notify (pOldFocusView, kMsgOldFocusView);
		}
		pOldFocusView->looseFocus ();
	}
	if (pImpl->focusView && pImpl->focusView->wantsFocus ())
		pImpl->focusView->takeFocus ();
	
	pImpl->focusViewObservers.forEach ([&] (IFocusViewObserver* observer) {
		observer->onFocusViewChanged (this, pImpl->focusView, pOldFocusView);
	});
	
	recursion = false;
}

//-----------------------------------------------------------------------------
CView* CFrame::getFocusView () const
{
	return pImpl->focusView;
}

//-----------------------------------------------------------------------------
bool CFrame::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (auto modalView = getModalView ())
	{
		if (auto container = modalView->asViewContainer ())
		{
			if (oldFocus == nullptr || container->isChild (oldFocus, true) == false)
				return container->advanceNextFocusView (nullptr, reverse);
			else
			{
				if (auto* parentView = static_cast<CViewContainer*> (oldFocus->getParentView ()))
				{
					CView* tempOldFocus = oldFocus;
					while (parentView != container)
					{
						if (parentView->advanceNextFocusView (tempOldFocus, reverse))
							return true;
						else
						{
							tempOldFocus = parentView;
							parentView = static_cast<CViewContainer*> (parentView->getParentView ());
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
	if (auto* parentView = static_cast<CViewContainer*> (oldFocus->getParentView ()))
	{
		CView* tempOldFocus = oldFocus;
		while (parentView)
		{
			if (parentView->advanceNextFocusView (tempOldFocus, reverse))
				return true;
			else
			{
				tempOldFocus = parentView;
				parentView = static_cast<CViewContainer*> (parentView->getParentView ());
			}
		}
	}
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//-----------------------------------------------------------------------------
bool CFrame::removeView (CView* pView, bool withForget)
{
#if DEBUG
	vstgui_assert (getModalView () != pView);
#endif
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (bool withForget)
{
	clearModalViewSessions ();
	if (pImpl->focusView)
	{
		pImpl->focusView->looseFocus ();
		pImpl->focusView = nullptr;
	}
	pImpl->activeFocusView = nullptr;
	clearMouseViews (CPoint (0, 0), 0, false);
	return CViewContainer::removeAll (withForget);
}

//-----------------------------------------------------------------------------
CView* CFrame::getViewAt (const CPoint& where, const GetViewOptions& options) const
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
CViewContainer* CFrame::getContainerAt (const CPoint& where, const GetViewOptions& options) const
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
			pV->setDirty (true);
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
int32_t CFrame::keyboardHooksOnKeyDown (const VstKeyCode& key)
{
	int32_t result = -1;
	pImpl->keyboardHooks.forEachReverse ([&] (IKeyboardHook* hook) {
		if (result <= 0)
		{
			result = hook->onKeyDown (key, this);
		}
	});
	return result;
}

//-----------------------------------------------------------------------------
int32_t CFrame::keyboardHooksOnKeyUp (const VstKeyCode& key)
{
	int32_t result = -1;
	pImpl->keyboardHooks.forEachReverse ([&] (IKeyboardHook* hook) {
		if (result <= 0)
		{
			result = hook->onKeyUp (key, this);
		}
	});
	return result;
}

//-----------------------------------------------------------------------------
void CFrame::registerScaleFactorChangedListeneer (IScaleFactorChangedListener* listener)
{
	pImpl->scaleFactorChangedListenerList.add (listener);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterScaleFactorChangedListeneer (IScaleFactorChangedListener* listener)
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
void CFrame::callMouseObserverMouseEntered (CView* view)
{
	view->callMouseListenerEnteredExited (true);
	pImpl->mouseObservers.forEach ([&] (IMouseObserver* observer) {
		observer->onMouseEntered (view, this);
	});
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseExited (CView* view)
{
	pImpl->mouseObservers.forEach ([&] (IMouseObserver* observer) {
		observer->onMouseExited (view, this);
	});
	view->callMouseListenerEnteredExited (false);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::callMouseObserverMouseDown (const CPoint& _where, const CButtonState& buttons)
{
	CMouseEventResult eventResult = kMouseEventNotHandled;
	if (pImpl->mouseObservers.empty ())
		return eventResult;
	
	CPoint where (_where);
	getTransform ().inverse ().transform (where);

	pImpl->mouseObservers.forEach ([&] (IMouseObserver* observer) {
		CMouseEventResult result2 = observer->onMouseDown (this, where, buttons);
		if (result2 != kMouseEventNotHandled)
			eventResult = result2;
	});

	return eventResult;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::callMouseObserverMouseMoved (const CPoint& _where, const CButtonState& buttons)
{
	CMouseEventResult eventResult = kMouseEventNotHandled;
	if (pImpl->mouseObservers.empty ())
		return eventResult;
	
	CPoint where (_where);
	getTransform ().inverse ().transform (where);
	
	pImpl->mouseObservers.forEach ([&] (IMouseObserver* observer) {
		CMouseEventResult result2 = observer->onMouseMoved (this, where, buttons);
		if (result2 == kMouseEventHandled)
			eventResult = kMouseEventHandled;
	});
	
	return eventResult;
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
IPlatformFrame* CFrame::getPlatformFrame () const
{
	return pImpl->platformFrame;
}

//-----------------------------------------------------------------------------
bool CFrame::platformDrawRect (CDrawContext* context, const CRect& rect)
{
	drawRect (context, rect);
	return true;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!getMouseEnabled ())
		return kMouseEventNotHandled;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseDown (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (!getMouseEnabled ())
		return kMouseEventNotHandled;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseMoved (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (!getMouseEnabled ())
		return kMouseEventNotHandled;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseExited (CPoint& where, const CButtonState& buttons)
{
	if (!getMouseEnabled ())
		return kMouseEventNotHandled;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseExited (where, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onWheel (where, axis, distance, buttons);
}

//-----------------------------------------------------------------------------
DragOperation CFrame::platformOnDragEnter (DragEventData data)
{
	if (!getMouseEnabled ())
		return DragOperation::None;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	data.modifiers = data.modifiers.getModifierState ();
	return getDropTarget ()->onDragEnter (data);
}

//-----------------------------------------------------------------------------
DragOperation CFrame::platformOnDragMove (DragEventData data)
{
	if (!getMouseEnabled ())
		return DragOperation::None;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	data.modifiers = data.modifiers.getModifierState ();
	return getDropTarget ()->onDragMove (data);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragLeave (DragEventData data)
{
	if (!getMouseEnabled ())
		return;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	data.modifiers = data.modifiers.getModifierState ();
	getDropTarget ()->onDragLeave (data);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnDrop (DragEventData data)
{
	if (!getMouseEnabled ())
		return false;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	data.modifiers = data.modifiers.getModifierState ();
	return getDropTarget ()->onDrop (data);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyDown (VstKeyCode& keyCode)
{
	if (!getMouseEnabled ())
		return false;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onKeyDown (keyCode) == 1;
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyUp (VstKeyCode& keyCode)
{
	if (!getMouseEnabled ())
		return false;
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onKeyUp (keyCode) == 1;
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
		listener->onScaleFactorChanged (this, newScaleFactor);
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
				CButtonState buttons (kLButton);
				CPoint where (e.second.location);
				target->frameToLocal (where);
				switch (e.second.state)
				{
					case ITouchEvent::kMoved:
					{
						CMouseEventResult result = target->onMouseMoved (where, buttons);
						if (result == kMouseMoveEventHandledButDontNeedMoreEvents)
						{
							event.unsetTouchTarget(e.first, target);
							if (target->hitTest (where, buttons) == false)
							{
								// when the touch goes out of the target and it tells us to
								const_cast<ITouchEvent::Touch&> (e.second).state = ITouchEvent::kBegan;
								hasBeganTouch = true;
							}
						}
						break;
					}
					case ITouchEvent::kCanceled:
					{
						if (target->onMouseCancel () != kMouseEventHandled)
							target->onMouseUp (where, buttons);
						event.unsetTouchTarget (e.first, target);
						break;
					}
					case ITouchEvent::kEnded:
					{
						target->onMouseUp (where, buttons);
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
				if (std::find (targetDispatched.begin (), targetDispatched.end (), target) == targetDispatched.end ())
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
				setFocusView (0);
		}
		for (const auto& e : event)
		{
			if (e.second.target == 0 && e.second.state == ITouchEvent::kBegan)
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
	uint32_t now = frame->getTicks ();
	if (now - lastTicks > 16)
	{
		flush ();
		lastTicks = now;
	}
}

} // VSTGUI
