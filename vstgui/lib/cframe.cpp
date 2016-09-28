//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#include "cframe.h"
#include "coffscreencontext.h"
#include "ctooltipsupport.h"
#include "itouchevent.h"
#include "iscalefactorchangedlistener.h"
#include "idatapackage.h"
#include "animation/animator.h"
#include "controls/ctextedit.h"
#include <cassert>
#include <vector>
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
	using KeyboardHookList = std::list<IKeyboardHook*>;
	using MouseObserverList = std::list<IMouseObserver*>;
	using ScaleFactorChangedListenerList = std::list<IScaleFactorChangedListener*>;
	using WindowActiveStateChangeViews = std::vector<CView*>;

	SharedPointer<IPlatformFrame> platformFrame;
	VSTGUIEditorInterface* pEditor {nullptr};
	IViewAddedRemovedObserver* pViewAddedRemovedObserver {nullptr};
	SharedPointer<CTooltipSupport> tooltips;
	SharedPointer<Animation::Animator> animator;
	CView* pModalView {nullptr};
	CView* pFocusView {nullptr};
	CView* pActiveFocusView {nullptr};
	CollectInvalidRects* collectInvalidRects {nullptr};
	KeyboardHookList* pKeyboardHooks {nullptr};
	MouseObserverList* pMouseObservers {nullptr};
	ScaleFactorChangedListenerList* pScaleFactorChangedListenerList {nullptr};
	
	ViewList pMouseViews;
	WindowActiveStateChangeViews windowActiveStateChangeViews;

	double userScaleFactor {1.};
	double platformScaleFactor {1.};
	bool bActive {false};
	bool bWindowActive {false};
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
	pImpl->pEditor = inEditor;

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

	if (pImpl->pScaleFactorChangedListenerList)
	{
#if DEBUG
		DebugPrint ("Warning: Scale Factor Changed Listeners are not cleaned up correctly.\n If you register a change listener you must also unregister it !\n");
#endif
		delete pImpl->pScaleFactorChangedListenerList;
	}
	
	if (pImpl->pMouseObservers)
	{
	#if DEBUG
		DebugPrint ("Warning: Mouse Observers are not cleaned up correctly.\n If you register a mouse oberver you must also unregister it !\n");
	#endif
		delete pImpl->pMouseObservers;
	}

	if (pImpl->pKeyboardHooks)
	{
	#if DEBUG
		DebugPrint ("Warning: Keyboard Hooks are not cleaned up correctly.\n If you register a keyboard hook you must also unregister it !\n");
	#endif
		delete pImpl->pKeyboardHooks;
	}

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

	if (pImpl->pModalView)
		removeView (pImpl->pModalView, false);
	setCursor (kCursorDefault);
	setParentFrame (nullptr);
	removeAll ();
	pImpl->platformFrame = nullptr;
	forget ();
}

//-----------------------------------------------------------------------------
bool CFrame::open (void* systemWin, PlatformType systemWindowType)
{
	if (!systemWin || isAttached ())
		return false;

	pImpl->platformFrame = owned (IPlatformFrame::createPlatformFrame (this, getViewSize (), systemWin, systemWindowType));
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

		for (const auto& pV : children)
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
			pImpl->tooltips = owned (new CTooltipSupport (this));
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
	auto it = pImpl->pMouseViews.rbegin ();
	while (it != pImpl->pMouseViews.rend ())
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
	pImpl->pMouseViews.clear ();
}

//-----------------------------------------------------------------------------
void CFrame::removeFromMouseViews (CView* view)
{
	bool found = false;
	auto it = pImpl->pMouseViews.begin ();
	while (it != pImpl->pMouseViews.end ())
	{
		if (found || (*it) == view)
		{
			if (pImpl->tooltips)
				pImpl->tooltips->onMouseExited ((*it));

			callMouseObserverMouseExited ((*it));

			(*it)->forget ();
			pImpl->pMouseViews.erase (it++);
			found = true;
		}
		else
			++it;
	}
}

//-----------------------------------------------------------------------------
void CFrame::checkMouseViews (const CPoint& where, const CButtonState& buttons)
{
	if (mouseDownView)
		return;
	CPoint lp;
	CView* mouseView = getViewAt (where, GetViewOptions (GetViewOptions::kDeep|GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	CView* currentMouseView = pImpl->pMouseViews.empty () == false ? pImpl->pMouseViews.back () : nullptr;
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
		pImpl->pMouseViews.remove (currentMouseView);
	}
	auto it = pImpl->pMouseViews.rbegin ();
	while (it != pImpl->pMouseViews.rend ())
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
			pImpl->pMouseViews.erase (--it.base ());
		}
		else
			break;
	}
	vc = pImpl->pMouseViews.empty () == false ? pImpl->pMouseViews.back ()->asViewContainer () : nullptr;
	if (vc)
	{
		auto it2 = pImpl->pMouseViews.end ();
		--it2;
		CView* container = mouseView;
		while ((vc = static_cast<CViewContainer*> (container->getParentView ())) != *it2)
		{
			pImpl->pMouseViews.push_back (vc);
			vc->remember ();
			container = vc;
		}
		pImpl->pMouseViews.push_back (mouseView);
		mouseView->remember ();
		++it2;
		while (it2 != pImpl->pMouseViews.end ())
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
		vstgui_assert (pImpl->pMouseViews.empty ());
		pImpl->pMouseViews.push_back (mouseView);
		mouseView->remember ();
		while ((vc = static_cast<CViewContainer*> (mouseView->getParentView ())) != this)
		{
			pImpl->pMouseViews.push_front (vc);
			vc->remember ();
			mouseView = vc;
		}
		auto it2 = pImpl->pMouseViews.begin ();
		while (it2 != pImpl->pMouseViews.end ())
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
	if (pImpl->pModalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pImpl->pModalView->isVisible () && pImpl->pModalView->getMouseEnabled () && pImpl->pModalView->hitTest (where2, buttons))
		{
			if (auto viewContainer = pImpl->pModalView->asViewContainer ())
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
	mouseDownView = nullptr;
	if (pImpl->pFocusView && dynamic_cast<CTextEdit*> (pImpl->pFocusView))
		setFocusView (nullptr);

	if (pImpl->tooltips)
		pImpl->tooltips->onMouseDown (where2);

	CMouseEventResult result = callMouseObserverMouseDown (where, buttons);
	if (result != kMouseEventNotHandled)
		return result;

	if (pImpl->pModalView)
	{
		CBaseObjectGuard rg (pImpl->pModalView);

		if (pImpl->pModalView->isVisible () && pImpl->pModalView->getMouseEnabled () && pImpl->pModalView->hitTest (where2, buttons))
		{
			CMouseEventResult result = pImpl->pModalView->onMouseDown (where2, buttons);
			if (result == kMouseEventHandled)
			{
				mouseDownView = pImpl->pModalView;
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

	if (pImpl->pModalView)
	{
		CBaseObjectGuard rg (pImpl->pModalView);
		result = pImpl->pModalView->onMouseMoved (where2, buttons);
	}
	else
	{
		CPoint p (where);
		result = CViewContainer::onMouseMoved (p, buttons);
		if (result == kMouseEventNotHandled)
		{
			CButtonState buttons2 = (buttons & (kShift | kControl | kAlt | kApple));
			auto it = pImpl->pMouseViews.rbegin ();
			while (it != pImpl->pMouseViews.rend ())
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

	if (mouseDownView == nullptr)
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

	if (result == -1 && pImpl->pFocusView)
	{
		CBaseObjectGuard og (pImpl->pFocusView);
		if (pImpl->pFocusView->getMouseEnabled ())
			result = pImpl->pFocusView->onKeyDown (keyCode);
		if (result == -1)
		{
			CView* parent = pImpl->pFocusView->getParentView ();
			while (parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyDown (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1 && pImpl->pModalView)
	{
		CBaseObjectGuard og (pImpl->pModalView);
		result = pImpl->pModalView->onKeyDown (keyCode);
	}

	if (result == -1 && keyCode.virt == VKEY_TAB)
		result = advanceNextFocusView (pImpl->pFocusView, (keyCode.modifier & MODIFIER_SHIFT) ? true : false) ? 1 : -1;

	return result;
}

//-----------------------------------------------------------------------------
int32_t CFrame::onKeyUp (VstKeyCode& keyCode)
{
	int32_t result = keyboardHooksOnKeyUp (keyCode);

	if (result == -1 && pImpl->pFocusView)
	{
		if (pImpl->pFocusView->getMouseEnabled ())
			result = pImpl->pFocusView->onKeyUp (keyCode);
		if (result == -1)
		{
			CView* parent = pImpl->pFocusView->getParentView ();
			while (parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyUp (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1 && pImpl->pModalView)
		result = pImpl->pModalView->onKeyUp (keyCode);

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	bool result = false;

	if (mouseDownView == nullptr)
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
		pImpl->animator = owned (new Animation::Animator);
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
	int32_t result = pImpl->pEditor ? pImpl->pEditor->getKnobMode () : -1;
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
	if (pView == nullptr && pImpl->pModalView == nullptr)
		return true;

	// If there is a modal view or the view 
	if ((pView && pImpl->pModalView) || (pView && pView->isAttached ()))
		return false;

	if (pImpl->pModalView)
	{
		removeView (pImpl->pModalView, false);
	}
	
	pImpl->pModalView = pView;

	if (pImpl->pModalView)
	{
		bool result = addView (pImpl->pModalView);
		if (result)
		{
			clearMouseViews (CPoint (0, 0), 0, true);
			if (auto container = pImpl->pModalView->asViewContainer ())
				container->advanceNextFocusView (nullptr, false);
			else
				setFocusView (pImpl->pModalView->wantsFocus () ? pImpl->pModalView : nullptr);
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
CView* CFrame::getModalView () const { return pImpl->pModalView; }

//-----------------------------------------------------------------------------
void CFrame::beginEdit (int32_t index)
{
	if (pImpl->pEditor)
		pImpl->pEditor->beginEdit (index);
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (int32_t index)
{
	if (pImpl->pEditor)
		pImpl->pEditor->endEdit (index);
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

	if (pImpl->pActiveFocusView == pView)
		pImpl->pActiveFocusView = nullptr;
	if (pImpl->pFocusView == pView)
		setFocusView (nullptr);
	if (auto container = pView->asViewContainer ())
	{
		if (container->isChild (pImpl->pFocusView, true))
			setFocusView (nullptr);
	}
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewRemoved (this, pView);
	if (pImpl->animator)
		pImpl->animator->removeAnimations (pView);
	if (pView->wantsWindowActiveStateChangeNotification ())
	{
		pImpl->windowActiveStateChangeViews.erase (
		    std::find (pImpl->windowActiveStateChangeViews.begin (), pImpl->windowActiveStateChangeViews.end (), pView));
	}
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
		pImpl->windowActiveStateChangeViews.push_back (pView);
		pView->onWindowActivate (pImpl->bWindowActive);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pView new focus view
 */
void CFrame::setFocusView (CView *pView)
{
	static bool recursion = false;
	if (pView == pImpl->pFocusView || (recursion && pImpl->pFocusView != nullptr))
		return;

	if (!pImpl->bActive)
	{
		pImpl->pActiveFocusView = pView;
		return;
	}

	recursion = true;

	CView *pOldFocusView = pImpl->pFocusView;
	if (pView == nullptr  || (pView && pView->isAttached () == false))
		pImpl->pFocusView = nullptr;
	else
		pImpl->pFocusView = pView;
	if (pImpl->pFocusView && pImpl->pFocusView->wantsFocus ())
	{
		pImpl->pFocusView->invalid ();

		CView* receiver = pImpl->pFocusView->getParentView ();
		while (receiver != this && receiver != nullptr)
		{
			receiver->notify (pImpl->pFocusView, kMsgNewFocusView);
			receiver = receiver->getParentView ();
		}
		notify (pImpl->pFocusView, kMsgNewFocusView);
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
	if (pImpl->pFocusView && pImpl->pFocusView->wantsFocus ())
		pImpl->pFocusView->takeFocus ();
	recursion = false;
}

//-----------------------------------------------------------------------------
CView* CFrame::getFocusView () const
{
	return pImpl->pFocusView;
}

//-----------------------------------------------------------------------------
bool CFrame::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (pImpl->pModalView)
	{
		if (auto container = pImpl->pModalView->asViewContainer ())
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
		else if (oldFocus != pImpl->pModalView)
		{
			setFocusView (pImpl->pModalView);
			return true;
		}
		return false; // currently not supported, but should be done sometime
	}
	if (oldFocus == nullptr)
	{
		if (pImpl->pFocusView == nullptr)
			return CViewContainer::advanceNextFocusView (nullptr, reverse);
		oldFocus = pImpl->pFocusView;
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
	if (pImpl->pModalView == pView)
		pImpl->pModalView = nullptr;
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (bool withForget)
{
	setModalView (nullptr);
	if (pImpl->pFocusView)
	{
		pImpl->pFocusView->looseFocus ();
		pImpl->pFocusView = nullptr;
	}
	pImpl->pActiveFocusView = nullptr;
	clearMouseViews (CPoint (0, 0), 0, false);
	return CViewContainer::removeAll (withForget);
}

//-----------------------------------------------------------------------------
CView* CFrame::getViewAt (const CPoint& where, const GetViewOptions& options) const
{
	if (pImpl->pModalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pImpl->pModalView->getViewSize ().pointInside (where2))
		{
			if (options.deep ())
			{
				if (auto container = pImpl->pModalView->asViewContainer ())
				{
					return container->getViewAt (where2, options);
				}
			}
			return pImpl->pModalView;
		}
		return nullptr;
	}
	return CViewContainer::getViewAt (where, options);
}

//-----------------------------------------------------------------------------
CViewContainer* CFrame::getContainerAt (const CPoint& where, const GetViewOptions& options) const
{
	if (pImpl->pModalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pImpl->pModalView->getViewSize ().pointInside (where2))
		{
			if (auto container = pImpl->pModalView->asViewContainer ())
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
	if (pImpl->bActive != state)
	{
		if (state)
		{
			pImpl->bActive = true;
			if (pImpl->pActiveFocusView)
			{
				setFocusView (pImpl->pActiveFocusView);
				pImpl->pActiveFocusView = nullptr;
			}
			else
				advanceNextFocusView (nullptr, false);
		}
		else
		{
			if (pImpl->tooltips)
				pImpl->tooltips->hideTooltip ();
			pImpl->pActiveFocusView = getFocusView ();
			setFocusView (nullptr);
			pImpl->bActive = false;
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
	CRect rectView;
	for (const auto& pV : children)
	{
		pV->getViewSize (rectView);
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
	return pImpl->pViewAddedRemovedObserver;
}

//-----------------------------------------------------------------------------
void CFrame::setViewAddedRemovedObserver (IViewAddedRemovedObserver* observer)
{
	pImpl->pViewAddedRemovedObserver = observer;
}

//-----------------------------------------------------------------------------
void CFrame::registerKeyboardHook (IKeyboardHook* hook)
{
	if (pImpl->pKeyboardHooks == nullptr)
		pImpl->pKeyboardHooks = new Impl::KeyboardHookList ();
	pImpl->pKeyboardHooks->push_back (hook);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterKeyboardHook (IKeyboardHook* hook)
{
	if (pImpl->pKeyboardHooks)
	{
		pImpl->pKeyboardHooks->remove (hook);
		if (pImpl->pKeyboardHooks->empty ())
		{
			delete pImpl->pKeyboardHooks;
			pImpl->pKeyboardHooks = nullptr;
		}
	}
}

//-----------------------------------------------------------------------------
int32_t CFrame::keyboardHooksOnKeyDown (const VstKeyCode& key)
{
	if (pImpl->pKeyboardHooks)
	{
		for (auto it = pImpl->pKeyboardHooks->rbegin (); it != pImpl->pKeyboardHooks->rend ();)
		{
			IKeyboardHook* hook = *it;
			++it;
			int32_t result = hook->onKeyDown (key, this);
			if (result > 0)
				return result;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
int32_t CFrame::keyboardHooksOnKeyUp (const VstKeyCode& key)
{
	if (pImpl->pKeyboardHooks)
	{
		for (auto it = pImpl->pKeyboardHooks->rbegin (); it != pImpl->pKeyboardHooks->rend ();)
		{
			IKeyboardHook* hook = *it;
			++it;
			int32_t result = hook->onKeyUp (key, this);
			if (result > 0)
				return result;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
void CFrame::registerScaleFactorChangedListeneer (IScaleFactorChangedListener* listener)
{
	if (pImpl->pScaleFactorChangedListenerList == nullptr)
		pImpl->pScaleFactorChangedListenerList = new Impl::ScaleFactorChangedListenerList ();
	pImpl->pScaleFactorChangedListenerList->push_back (listener);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterScaleFactorChangedListeneer (IScaleFactorChangedListener* listener)
{
	if (pImpl->pScaleFactorChangedListenerList)
	{
		pImpl->pScaleFactorChangedListenerList->remove (listener);
		if (pImpl->pScaleFactorChangedListenerList->empty ())
		{
			delete pImpl->pScaleFactorChangedListenerList;
			pImpl->pScaleFactorChangedListenerList = nullptr;
		}
	}
}

//-----------------------------------------------------------------------------
void CFrame::registerMouseObserver (IMouseObserver* observer)
{
	if (pImpl->pMouseObservers == nullptr)
		pImpl->pMouseObservers = new Impl::MouseObserverList ();
	pImpl->pMouseObservers->push_back (observer);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterMouseObserver (IMouseObserver* observer)
{
	if (pImpl->pMouseObservers)
	{
		pImpl->pMouseObservers->remove (observer);
		if (pImpl->pMouseObservers->empty ())
		{
			delete pImpl->pMouseObservers;
			pImpl->pMouseObservers = nullptr;
		}
	}
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseEntered (CView* view)
{
	if (pImpl->pMouseObservers == nullptr)
		return;
	for (auto& observer : *pImpl->pMouseObservers)
		observer->onMouseEntered (view, this);
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseExited (CView* view)
{
	if (pImpl->pMouseObservers == nullptr)
		return;
	for (auto& observer : *pImpl->pMouseObservers)
		observer->onMouseExited (view, this);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::callMouseObserverMouseDown (const CPoint& _where, const CButtonState& buttons)
{
	if (pImpl->pMouseObservers == nullptr)
		return kMouseEventNotHandled;

	CMouseEventResult result = kMouseEventNotHandled;
	CPoint where (_where);
	getTransform ().inverse ().transform (where);
	
	for (auto& observer : *pImpl->pMouseObservers)
	{
		CMouseEventResult result2 = observer->onMouseDown (this, where, buttons);
		if (result2 == kMouseEventHandled)
			result = kMouseEventHandled;
		if (pImpl->pMouseObservers == nullptr)
			return kMouseEventHandled;
	}
	return result;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::callMouseObserverMouseMoved (const CPoint& _where, const CButtonState& buttons)
{
	if (pImpl->pMouseObservers == nullptr)
		return kMouseEventNotHandled;

	CMouseEventResult result = kMouseEventNotHandled;
	CPoint where (_where);
	getTransform ().inverse ().transform (where);
	
	for (auto& observer : *pImpl->pMouseObservers)
	{
		CMouseEventResult result2 = observer->onMouseMoved (this, where, buttons);
		if (result2 == kMouseEventHandled)
			result = kMouseEventHandled;
	}
	return result;
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
	return pImpl->pEditor;
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
	CollectInvalidRects cir (this);
	return onMouseDown (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseMoved (CPoint& where, const CButtonState& buttons)
{
	CollectInvalidRects cir (this);
	return onMouseMoved (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseUp (CPoint& where, const CButtonState& buttons)
{
	CollectInvalidRects cir (this);
	return onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseExited (CPoint& where, const CButtonState& buttons)
{
	CollectInvalidRects cir (this);
	return onMouseExited (where, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	CollectInvalidRects cir (this);
	return onWheel (where, axis, distance, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnDrop (IDataPackage* drag, const CPoint& where)
{
	CollectInvalidRects cir (this);
	return onDrop (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragEnter (IDataPackage* drag, const CPoint& where)
{
	CollectInvalidRects cir (this);
	return onDragEnter (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragLeave (IDataPackage* drag, const CPoint& where)
{
	CollectInvalidRects cir (this);
	return onDragLeave (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragMove (IDataPackage* drag, const CPoint& where)
{
	CollectInvalidRects cir (this);
	return onDragMove (drag, where);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyDown (VstKeyCode& keyCode)
{
	CollectInvalidRects cir (this);
	return onKeyDown (keyCode) == 1;
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyUp (VstKeyCode& keyCode)
{
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
	if (pImpl->bWindowActive == state)
		return;
	pImpl->bWindowActive = state;
	CollectInvalidRects cir (this);
	for (auto& view : pImpl->windowActiveStateChangeViews)
		view->onWindowActivate (state);
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
	if (pImpl->pScaleFactorChangedListenerList == nullptr)
		return;
	for (auto& listener : *pImpl->pScaleFactorChangedListenerList)
		listener->onScaleFactorChanged (this, newScaleFactor);
}

#if VSTGUI_TOUCH_EVENT_HANDLING
//-----------------------------------------------------------------------------
void CFrame::platformOnTouchEvent (ITouchEvent& event)
{
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
					targetDispatched.push_back (target);
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
		invalidRects.push_back (rect);
	uint32_t now = frame->getTicks ();
	if (now - lastTicks > 16)
	{
		flush ();
		lastTicks = now;
	}
}

} // namespace

