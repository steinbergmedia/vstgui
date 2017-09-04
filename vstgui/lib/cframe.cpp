// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cframe.h"
#include "coffscreencontext.h"
#include "ctooltipsupport.h"
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
	InvalidRects invalidRects;
	uint32_t lastTicks;
#if VSTGUI_LOG_COLLECT_INVALID_RECTS
	uint32_t numAddedRects;
#endif
};

//------------------------------------------------------------------------
struct CFrame::Impl
{
	using ViewList = std::list<CView*>;
	using FunctionQueue = std::queue<Function>;

	SharedPointer<IPlatformFrame> platformFrame;
	VSTGUIEditorInterface* editor {nullptr};
	IViewAddedRemovedObserver* viewAddedRemovedObserver {nullptr};
	SharedPointer<CTooltipSupport> tooltips;
	SharedPointer<Animation::Animator> animator;
	CView* modalView {nullptr};
	CView* focusView {nullptr};
	CView* activeFocusView {nullptr};
	CollectInvalidRects* collectInvalidRects {nullptr};
	
	ViewList mouseViews;
	DispatchList<CView*> windowActiveStateChangeViews;
	DispatchList<IScaleFactorChangedListener*> scaleFactorChangedListenerList;
	DispatchList<IMouseObserver*> mouseObservers;
	DispatchList<IFocusViewObserver*> focusViewObservers;
	DispatchList<IKeyboardHook*> keyboardHooks;
	FunctionQueue postEventFunctionQueue;

	double userScaleFactor {1.};
	double platformScaleFactor {1.};
	bool active {false};
	bool windowActive {false};
	bool inEventHandling {false};

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
CFrame::CFrame (const CRect& inSize, VSTGUIEditorInterface* inEditor)
: CViewContainer (inSize)
{
	pImpl = new Impl;
	pImpl->editor = inEditor;

	setParentFrame (this);
}

//-----------------------------------------------------------------------------
void CFrame::beforeDelete ()
{
	clearMouseViews (CPoint (0, 0), 0, false);

	setModalView (nullptr);

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

	pImpl->platformFrame = nullptr;

	setViewFlag (kIsAttached, false);
	
	delete pImpl;
	pImpl = nullptr;
	
	CViewContainer::beforeDelete ();
}

//-----------------------------------------------------------------------------
void CFrame::close ()
{
	clearMouseViews (CPoint (0, 0), 0, false);

	if (pImpl->modalView)
		removeView (pImpl->modalView, false);
	setCursor (kCursorDefault);
	setParentFrame (nullptr);
	removeAll ();
	pImpl->platformFrame = nullptr;
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

	attached (this);
	
	setParentView (nullptr);

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

//------------------------------------------------------------------------
double CFrame::getScaleFactor () const
{
	return pImpl->platformScaleFactor * pImpl->userScaleFactor;
}

//-----------------------------------------------------------------------------
void CFrame::enableTooltips (bool state)
{
	if (state)
	{
		if (pImpl->tooltips == nullptr)
			pImpl->tooltips = makeOwned<CTooltipSupport> (this);
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

	if (pContext)
		pContext->remember ();

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect newClip (updateRect);
	newClip.bound (oldClip);
	pContext->setClipRect (newClip);

	// draw the background and the children
	CViewContainer::drawRect (pContext, updateRect);

	pContext->setClipRect (oldClip);

	pContext->forget ();
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
			(*it)->frameToLocal (lp);
			(*it)->onMouseExited (lp, buttons);
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseExited : %p\n", (*it));
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
	CView* mouseView = getViewAt (where, GetViewOptions (GetViewOptions::kDeep|GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
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
		currentMouseView->frameToLocal (lp);
		currentMouseView->onMouseExited (lp, buttons);
		callMouseObserverMouseExited (currentMouseView);
	#if DEBUG_MOUSE_VIEWS
		DebugPrint ("mouseExited : %p\n", currentMouseView);
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
			vc->frameToLocal (lp);
			vc->onMouseExited (lp, buttons);
			callMouseObserverMouseExited (vc);
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseExited : %p\n", vc);
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
		while ((vc = static_cast<CViewContainer*> (container->getParentView ())) != *it2)
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
			(*it2)->frameToLocal (lp);
			(*it2)->onMouseEntered (lp, buttons);
			callMouseObserverMouseEntered ((*it2));
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseEntered : %p\n", (*it2));
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
		while ((vc = static_cast<CViewContainer*> (mouseView->getParentView ())) != this)
		{
			pImpl->mouseViews.push_front (vc);
			vc->remember ();
			mouseView = vc;
		}
		auto it2 = pImpl->mouseViews.begin ();
		while (it2 != pImpl->mouseViews.end ())
		{
			lp = where;
			(*it2)->frameToLocal (lp);
			(*it2)->onMouseEntered (lp, buttons);
			callMouseObserverMouseEntered ((*it2));
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseEntered : %p\n", (*it2));
		#endif
			++it2;
		}
	}
}

//------------------------------------------------------------------------
bool CFrame::hitTestSubViews (const CPoint& where, const CButtonState& buttons)
{
	if (pImpl->modalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pImpl->modalView->isVisible () && pImpl->modalView->getMouseEnabled () && pImpl->modalView->hitTest (where2, buttons))
		{
			if (auto viewContainer = pImpl->modalView->asViewContainer ())
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

	// reset views
	setMouseDownView (nullptr);
	if (pImpl->focusView && dynamic_cast<CTextEdit*> (pImpl->focusView))
		setFocusView (nullptr);

	if (pImpl->tooltips)
		pImpl->tooltips->onMouseDown (where2);

	CMouseEventResult result = callMouseObserverMouseDown (where, buttons);
	if (result != kMouseEventNotHandled)
		return result;

	if (pImpl->modalView)
	{
		CBaseObjectGuard rg (pImpl->modalView);

		if (pImpl->modalView->isVisible () && pImpl->modalView->getMouseEnabled () && pImpl->modalView->hitTest (where2, buttons))
		{
			CMouseEventResult result = pImpl->modalView->onMouseDown (where2, buttons);
			if (result == kMouseEventHandled)
			{
				setMouseDownView (pImpl->modalView);
				return kMouseEventHandled;
			}
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

	if (pImpl->modalView)
	{
		CBaseObjectGuard rg (pImpl->modalView);
		result = pImpl->modalView->onMouseMoved (where2, buttons);
	}
	else
	{
		CPoint p (where);
		result = CViewContainer::onMouseMoved (p, buttons);
		if (result == kMouseEventNotHandled)
		{
			CButtonState buttons2 = (buttons & (kShift | kControl | kAlt | kApple));
			auto it = pImpl->mouseViews.rbegin ();
			while (it != pImpl->mouseViews.rend ())
			{
				p = where2;
				(*it)->getParentView ()->frameToLocal (p);
				result = (*it)->onMouseMoved (p, buttons2);
				if (result == kMouseEventHandled)
					break;
				++it;
			}
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
			while (parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyDown (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1 && pImpl->modalView)
	{
		CBaseObjectGuard og (pImpl->modalView);
		result = pImpl->modalView->onKeyDown (keyCode);
	}

	if (result == -1 && keyCode.virt == VKEY_TAB)
		result = advanceNextFocusView (pImpl->focusView, (keyCode.modifier & MODIFIER_SHIFT) ? true : false) ? 1 : -1;

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
			while (parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyUp (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1 && pImpl->modalView)
		result = pImpl->modalView->onKeyUp (keyCode);

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	bool result = false;

	if (getMouseDownView () == nullptr)
	{
		result = CViewContainer::onWheel (where, axis, distance, buttons);
		checkMouseViews (where, buttons);
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const float &distance, const CButtonState &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

//-----------------------------------------------------------------------------
DragResult CFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	if (pImpl->platformFrame)
		return pImpl->platformFrame->doDrag (source, offset, dragBitmap);
	return kDragError;
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

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be set to modal.
 * @return true if view could be set as the modal view. false if there is a already a modal view or the view to be set as modal is already attached.
 */
bool CFrame::setModalView (CView* pView)
{
	if (pView == nullptr && pImpl->modalView == nullptr)
		return true;

	// If there is a modal view or the view 
	if ((pView && pImpl->modalView) || (pView && pView->isAttached ()))
		return false;

	if (pImpl->modalView)
	{
		removeView (pImpl->modalView, false);
	}
	
	pImpl->modalView = pView;

	if (pImpl->modalView)
	{
		bool result = addView (pImpl->modalView);
		if (result)
		{
			clearMouseViews (CPoint (0, 0), 0, true);
			if (auto container = pImpl->modalView->asViewContainer ())
				container->advanceNextFocusView (nullptr, false);
			else
				setFocusView (pImpl->modalView->wantsFocus () ? pImpl->modalView : nullptr);
		}
		return result;
	}
	else
	{
		CPoint where;
		getCurrentMouseLocation (where);
		checkMouseViews (where, getCurrentMouseButtons ());
	}

	return true;
}

//-----------------------------------------------------------------------------
CView* CFrame::getModalView () const { return pImpl->modalView; }

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
	if (pImpl->animator)
		pImpl->animator->removeAnimations (pView);
	if (pView->wantsWindowActiveStateChangeNotification ())
		pImpl->windowActiveStateChangeViews.remove (pView);
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
	if (pImpl->modalView)
	{
		if (auto container = pImpl->modalView->asViewContainer ())
		{
			if (oldFocus == nullptr || container->isChild (oldFocus, true) == false)
				return container->advanceNextFocusView (nullptr, reverse);
			else
			{
				CViewContainer* parentView = static_cast<CViewContainer*> (oldFocus->getParentView ());
				if (parentView)
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
		else if (oldFocus != pImpl->modalView)
		{
			setFocusView (pImpl->modalView);
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
	CViewContainer* parentView = static_cast<CViewContainer*> (oldFocus->getParentView ());
	if (parentView)
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
	if (pImpl->modalView == pView)
		pImpl->modalView = nullptr;
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (bool withForget)
{
	setModalView (nullptr);
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
	if (pImpl->modalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pImpl->modalView->getViewSize ().pointInside (where2))
		{
			if (options.deep ())
			{
				if (auto container = pImpl->modalView->asViewContainer ())
				{
					return container->getViewAt (where2, options);
				}
			}
			return pImpl->modalView;
		}
		return nullptr;
	}
	return CViewContainer::getViewAt (where, options);
}

//-----------------------------------------------------------------------------
CViewContainer* CFrame::getContainerAt (const CPoint& where, const GetViewOptions& options) const
{
	if (pImpl->modalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pImpl->modalView->getViewSize ().pointInside (where2))
		{
			if (auto container = pImpl->modalView->asViewContainer ())
			{
				if (options.deep ())
					return container->getContainerAt (where2, options);
				return container;
			}
		}
		return nullptr;
	}
	return CViewContainer::getContainerAt (where, options);
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
	uint32_t outSize;
	getAttribute ('vfco', sizeof (CColor), &focusColor, outSize);
	return focusColor;
}

//-----------------------------------------------------------------------------
CCoord CFrame::getFocusWidth () const
{
	CCoord focusWidth = 2;
	uint32_t outSize;
	getAttribute ('vfwi', sizeof (CCoord), &focusWidth, outSize);
	return focusWidth;
}

//-----------------------------------------------------------------------------
void CFrame::setFocusDrawingEnabled (bool state)
{
	if (state)
		setAttribute ('vfde', sizeof(bool), &state);
	else
		removeAttribute ('vfde');
}

//-----------------------------------------------------------------------------
void CFrame::setFocusColor (const CColor& color)
{
	setAttribute ('vfco', sizeof (CColor), &color);
}

//-----------------------------------------------------------------------------
void CFrame::setFocusWidth (CCoord width)
{
	setAttribute ('vfwi', sizeof (CCoord), &width);
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
		if (result2 == kMouseEventHandled)
			eventResult = kMouseEventHandled;
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
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseDown (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseMoved (CPoint& where, const CButtonState& buttons)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseMoved (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseUp (CPoint& where, const CButtonState& buttons)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseExited (CPoint& where, const CButtonState& buttons)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onMouseExited (where, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onWheel (where, axis, distance, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnDrop (IDataPackage* drag, const CPoint& where)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onDrop (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragEnter (IDataPackage* drag, const CPoint& where)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onDragEnter (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragLeave (IDataPackage* drag, const CPoint& where)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onDragLeave (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragMove (IDataPackage* drag, const CPoint& where)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onDragMove (drag, where);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyDown (VstKeyCode& keyCode)
{
	Impl::PostEventHandler peh (*pImpl);
	CollectInvalidRects cir (this);
	return onKeyDown (keyCode) == 1;
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyUp (VstKeyCode& keyCode)
{
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
bool CFrame::doAfterEventProcessing (Function&& func)
{
	if (pImpl->inEventHandling)
		pImpl->postEventFunctionQueue.push (std::move (func));
	return pImpl->inEventHandling;
}

//-----------------------------------------------------------------------------
bool CFrame::doAfterEventProcessing (const Function& func)
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
	if (!invalidRects.empty ())
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
	bool add = true;
	for (InvalidRects::iterator it = invalidRects.begin (), end = invalidRects.end (); it != end; ++it)
	{
		CRect r (rect);
		if (r.bound (*it) == rect)
		{
			add = false;
			break;
		}
		r = *it;
		if (r.bound (rect) == *it)
		{
			invalidRects.erase (it);
			break;
		}
	}
	if (add)
		invalidRects.emplace_back (rect);
	uint32_t now = frame->getTicks ();
	if (now - lastTicks > 16)
	{
		flush ();
		lastTicks = now;
	}
}

} // namespace

