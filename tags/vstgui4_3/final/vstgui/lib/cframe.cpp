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
#include "animation/animator.h"
#include "controls/ctextedit.h"
#include <cassert>
#include <vector>
#include <limits>

namespace VSTGUI {

#if MAC_CARBON && MAC_COCOA && VSTGUI_ENABLE_DEPRECATED_METHODS
	static bool createNSViewMode = false;
#endif

IdStringPtr kMsgNewFocusView = "kMsgNewFocusView";
IdStringPtr kMsgOldFocusView = "kMsgOldFocusView";

#define DEBUG_MOUSE_VIEWS	0//DEBUG

//-----------------------------------------------------------------------------
// CFrame Implementation
//-----------------------------------------------------------------------------
/*! @class CFrame
It creates a platform dependend view object. 

On Mac OS X it is a HIView or NSView.\n 
On Windows it's a WS_CHILD Window.

*/
#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
/**
 * @param inSize size of frame
 * @param inSystemWindow parent platform window
 * @param inEditor editor
 */
CFrame::CFrame (const CRect &inSize, void* inSystemWindow, VSTGUIEditorInterface* inEditor)
: CViewContainer (inSize)
, pEditor (inEditor)
, pMouseObservers (0)
, pKeyboardHooks (0)
, pViewAddedRemovedObserver (0)
, pScaleFactorChangedListenerList (0)
, pTooltips (0)
, pAnimator (0)
, pModalView (0)
, pFocusView (0)
, pActiveFocusView (0)
, bActive (true)
, platformFrame (0)
, collectInvalidRects (0)
{
	pParentFrame = this;
	open (inSystemWindow);
}
#endif

//-----------------------------------------------------------------------------
CFrame::CFrame (const CRect& inSize, VSTGUIEditorInterface* inEditor)
: CViewContainer (inSize)
, pEditor (inEditor)
, pMouseObservers (0)
, pKeyboardHooks (0)
, pViewAddedRemovedObserver (0)
, pScaleFactorChangedListenerList (0)
, pTooltips (0)
, pAnimator (0)
, pModalView (0)
, pFocusView (0)
, pActiveFocusView (0)
, bActive (true)
, platformFrame (0)
, collectInvalidRects (0)
{
	pParentFrame = this;
}

//-----------------------------------------------------------------------------
CFrame::~CFrame ()
{
	clearMouseViews (CPoint (0, 0), 0, false);

	setModalView (0);

	setCursor (kCursorDefault);

	pParentFrame = 0;
	removeAll ();

	if (pTooltips)
	{
		pTooltips->forget ();
		pTooltips = 0;
	}
	if (pAnimator)
	{
		pAnimator->forget ();
		pAnimator = 0;
	}

	if (pScaleFactorChangedListenerList)
	{
#if DEBUG
		DebugPrint ("Warning: Scale Factor Changed Listeners are not cleaned up correctly.\n If you register a change listener you must also unregister it !\n");
#endif
		delete pScaleFactorChangedListenerList;
	}
	
	if (pMouseObservers)
	{
	#if DEBUG
		DebugPrint ("Warning: Mouse Observers are not cleaned up correctly.\n If you register a mouse oberver you must also unregister it !\n");
	#endif
		delete pMouseObservers;
	}

	if (pKeyboardHooks)
	{
	#if DEBUG
		DebugPrint ("Warning: Keyboard Hooks are not cleaned up correctly.\n If you register a keyboard hook you must also unregister it !\n");
	#endif
		delete pKeyboardHooks;
	}

	if (platformFrame)
		platformFrame->forget ();

	viewFlags &= ~kIsAttached;
}

#if MAC_COCOA && MAC_CARBON && VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CFrame::setCocoaMode (bool state)
{
	createNSViewMode = state;
}
bool CFrame::getCocoaMode ()
{
	return createNSViewMode;
}
#endif

//-----------------------------------------------------------------------------
void CFrame::close ()
{
	clearMouseViews (CPoint (0, 0), 0, false);

	if (pModalView)
		removeView (pModalView, false);
	setCursor (kCursorDefault);
	pParentFrame = 0;
	removeAll ();
	if (platformFrame)
	{
		platformFrame->forget ();
		platformFrame = 0;
	}
	forget ();
}

//-----------------------------------------------------------------------------
bool CFrame::open (void* systemWin, PlatformType systemWindowType)
{
	if (!systemWin || isAttached ())
		return false;

	platformFrame = IPlatformFrame::createPlatformFrame (this, getViewSize (), systemWin, systemWindowType);
	if (!platformFrame)
		return false;

	attached (this);

	pParentView = 0;

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
		pParentView = 0;

		FOREACHSUBVIEW
			pV->attached (this);
		ENDFOREACHSUBVIEW
		
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
	return result;
}

//-----------------------------------------------------------------------------
void CFrame::enableTooltips (bool state)
{
	if (state)
	{
		if (pTooltips == 0)
			pTooltips = new CTooltipSupport (this);
	}
	else if (pTooltips)
	{
		pTooltips->forget ();
		pTooltips = 0;
	}
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
CDrawContext* CFrame::createDrawContext ()
{
	CDrawContext* pContext = 0;
	
	return pContext;
}
#endif

//-----------------------------------------------------------------------------
void CFrame::draw (CDrawContext* pContext)
{
	return CFrame::drawRect (pContext, getViewSize ());
}

//-----------------------------------------------------------------------------
void CFrame::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (updateRect.getWidth () <= 0 || updateRect.getHeight () <= 0 || pContext == 0)
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
	ViewList::reverse_iterator it = pMouseViews.rbegin ();
	while (it != pMouseViews.rend ())
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
		if (pTooltips)
			pTooltips->onMouseExited ((*it));

		callMouseObserverMouseExited ((*it));

		(*it)->forget ();
		it++;
	}
	pMouseViews.clear ();
}

//-----------------------------------------------------------------------------
void CFrame::removeFromMouseViews (CView* view)
{
	bool found = false;
	ViewList::iterator it = pMouseViews.begin ();
	while (it != pMouseViews.end ())
	{
		if (found || (*it) == view)
		{
			if (pTooltips)
				pTooltips->onMouseExited ((*it));

			callMouseObserverMouseExited ((*it));

			(*it)->forget ();
			pMouseViews.erase (it++);
			found = true;
		}
		else
			it++;
	}
}

//-----------------------------------------------------------------------------
void CFrame::checkMouseViews (const CPoint& where, const CButtonState& buttons)
{
	if (mouseDownView)
		return;
	CPoint lp;
	CView* mouseView = getViewAt (where, GetViewOptions (GetViewOptions::kDeep|GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	CView* currentMouseView = pMouseViews.empty () == false ? pMouseViews.back () : 0;
	if (currentMouseView == mouseView)
		return; // no change

	if (pTooltips)
	{
		if (currentMouseView)
			pTooltips->onMouseExited (currentMouseView);
		if (mouseView && mouseView != this)
			pTooltips->onMouseEntered (mouseView);
	}

	if (mouseView == 0 || mouseView == this)
	{
		clearMouseViews (where, buttons);
		return;
	}
	CViewContainer* vc = currentMouseView ? dynamic_cast<CViewContainer*> (currentMouseView) : 0;
	// if the currentMouseView is not a view container, we know that the new mouseView won't be a child of it and that all other
	// views in the list are viewcontainers
	if (vc == 0 && currentMouseView)
	{
		lp = where;
		currentMouseView->frameToLocal (lp);
		currentMouseView->onMouseExited (lp, buttons);
		callMouseObserverMouseExited (currentMouseView);
	#if DEBUG_MOUSE_VIEWS
		DebugPrint ("mouseExited : %p\n", currentMouseView);
	#endif
		currentMouseView->forget ();
		pMouseViews.remove (currentMouseView);
	}
	ViewList::reverse_iterator it = pMouseViews.rbegin ();
	while (it != pMouseViews.rend ())
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
			pMouseViews.erase (--it.base ());
		}
		else
			break;
	}
	vc = pMouseViews.empty () == false ? dynamic_cast<CViewContainer*> (pMouseViews.back ()) : 0;
	if (vc)
	{
		ViewList::iterator it2 = pMouseViews.end ();
		it2--;
		CView* container = mouseView;
		while ((vc = static_cast<CViewContainer*> (container->getParentView ())) != *it2)
		{
			pMouseViews.push_back (vc);
			vc->remember ();
			container = vc;
		}
		pMouseViews.push_back (mouseView);
		mouseView->remember ();
		it2++;
		while (it2 != pMouseViews.end ())
		{
			lp = where;
			(*it2)->frameToLocal (lp);
			(*it2)->onMouseEntered (lp, buttons);
			callMouseObserverMouseEntered ((*it2));
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseEntered : %p\n", (*it2));
		#endif
			it2++;
		}
	}
	else
	{
		// must be pMouseViews.size () == 0
		vstgui_assert (pMouseViews.empty ());
		pMouseViews.push_back (mouseView);
		mouseView->remember ();
		while ((vc = static_cast<CViewContainer*> (mouseView->getParentView ())) != this)
		{
			pMouseViews.push_front (vc);
			vc->remember ();
			mouseView = vc;
		}
		ViewList::iterator it2 = pMouseViews.begin ();
		while (it2 != pMouseViews.end ())
		{
			lp = where;
			(*it2)->frameToLocal (lp);
			(*it2)->onMouseEntered (lp, buttons);
			callMouseObserverMouseEntered ((*it2));
		#if DEBUG_MOUSE_VIEWS
			DebugPrint ("mouseEntered : %p\n", (*it2));
		#endif
			it2++;
		}
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	CPoint where2 (where);
	getTransform ().inverse ().transform (where2);

	// reset views
	mouseDownView = 0;
	if (pFocusView && dynamic_cast<CTextEdit*> (pFocusView))
		setFocusView (0);

	if (pTooltips)
		pTooltips->onMouseDown (where2);

	CMouseEventResult result = callMouseObserverMouseDown (where, buttons);
	if (result != kMouseEventNotHandled)
		return result;

	if (pModalView)
	{
		CBaseObjectGuard rg (pModalView);

		if (pModalView->isVisible () && pModalView->getMouseEnabled () && pModalView->hitTest (where2, buttons))
		{
			CMouseEventResult result = pModalView->onMouseDown (where2, buttons);
			if (result == kMouseEventHandled)
			{
				mouseDownView = pModalView;
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
	
	if (pTooltips)
		pTooltips->onMouseMoved (where2);

	checkMouseViews (where, buttons);

	CMouseEventResult result = callMouseObserverMouseMoved (where, buttons);
	if (result != kMouseEventNotHandled)
		return result;

	if (pModalView)
	{
		CBaseObjectGuard rg (pModalView);
		result = pModalView->onMouseMoved (where2, buttons);
	}
	else
	{
		CPoint p (where);
		result = CViewContainer::onMouseMoved (p, buttons);
		if (result == kMouseEventNotHandled)
		{
			CButtonState buttons2 = (buttons & (kShift | kControl | kAlt | kApple));
			ViewList::const_reverse_iterator it = pMouseViews.rbegin ();
			while (it != pMouseViews.rend ())
			{
				p = where;
				(*it)->getParentView ()->frameToLocal (p);
				result = (*it)->onMouseMoved (p, buttons2);
				if (result == kMouseEventHandled)
					break;
				it++;
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseExited (CPoint &where, const CButtonState& buttons)
{ // this should only get called from the platform implementation

	if (mouseDownView == 0)
	{
		clearMouseViews (where, buttons);
		if (pTooltips)
			pTooltips->hideTooltip ();
	}

	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
int32_t CFrame::onKeyDown (VstKeyCode& keyCode)
{
	int32_t result = -1;

	result = keyboardHooksOnKeyDown (keyCode);

	if (result == -1 && pFocusView)
	{
		CBaseObjectGuard og (pFocusView);
		if (pFocusView->getMouseEnabled ())
			result = pFocusView->onKeyDown (keyCode);
		if (result == -1)
		{
			CView* parent = pFocusView->getParentView ();
			while (parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyDown (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1 && pModalView)
	{
		CBaseObjectGuard og (pModalView);
		result = pModalView->onKeyDown (keyCode);
	}

	if (result == -1 && keyCode.virt == VKEY_TAB)
		result = advanceNextFocusView (pFocusView, (keyCode.modifier & MODIFIER_SHIFT) ? true : false) ? 1 : -1;

	return result;
}

//-----------------------------------------------------------------------------
int32_t CFrame::onKeyUp (VstKeyCode& keyCode)
{
	int32_t result = -1;

	result = keyboardHooksOnKeyUp (keyCode);

	if (result == -1 && pFocusView)
	{
		if (pFocusView->getMouseEnabled ())
			result = pFocusView->onKeyUp (keyCode);
		if (result == -1)
		{
			CView* parent = pFocusView->getParentView ();
			while (parent != this && result == -1)
			{
				if (parent->getMouseEnabled ())
					result = parent->onKeyUp (keyCode);
				parent = parent->getParentView ();
			}
		}
	}

	if (result == -1 && pModalView)
		result = pModalView->onKeyUp (keyCode);

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	bool result = false;

	if (mouseDownView == 0)
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
	if (platformFrame)
		return platformFrame->doDrag (source, offset, dragBitmap);
	return kDragError;
}

//-----------------------------------------------------------------------------
IDataPackage* CFrame::getClipboard ()
{
	if (platformFrame)
		return platformFrame->getClipboard ();
	return 0;
}

//-----------------------------------------------------------------------------
void CFrame::setClipboard (IDataPackage* data)
{
	if (platformFrame)
		platformFrame->setClipboard (data);
}

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	if (CView::kDirtyCallAlwaysOnMainThread)
		return;
	invalidateDirtyViews ();
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CFrame::doIdleStuff ()
{
	if (pEditor)
		pEditor->doIdleStuff ();
}
#endif

//-----------------------------------------------------------------------------
Animation::Animator* CFrame::getAnimator ()
{
	if (pAnimator == 0)
		pAnimator = new Animation::Animator;
	return pAnimator;
}

//-----------------------------------------------------------------------------
/**
 * @return tick count in milliseconds
 */
uint32_t CFrame::getTicks () const
{
	if (platformFrame)
		return platformFrame->getTicks ();
	return std::numeric_limits<uint32_t>::max ();
}

//-----------------------------------------------------------------------------
int32_t CFrame::kDefaultKnobMode = kCircularMode;

//-----------------------------------------------------------------------------
int32_t CFrame::getKnobMode () const
{
	int32_t result = pEditor ? pEditor->getKnobMode () : -1;
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
	if (platformFrame)
	{
		CRect rect (getViewSize ());
		rect.offset (x - size.left, y - size.top);
		if (platformFrame->setSize (rect))
		{
			size = rect;
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
	if (platformFrame)
	{
		CPoint p;
		if (platformFrame->getGlobalPosition (p))
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
	if (platformFrame)
	{
		if (platformFrame->setSize (newSize))
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
	if (platformFrame && pRect)
		return platformFrame->getSize (*pRect);
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
	if (pView == 0 && pModalView == 0)
		return true;

	// If there is a modal view or the view 
	if ((pView && pModalView) || (pView && pView->isAttached ()))
		return false;

	if (pModalView)
	{
		removeView (pModalView, false);
	}
	
	pModalView = pView;

	if (pModalView)
	{
		bool result = addView (pModalView);
		if (result)
		{
			clearMouseViews (CPoint (0, 0), 0, true);
			CViewContainer* container = dynamic_cast<CViewContainer*> (pModalView);
			if (container)
				container->advanceNextFocusView (0, false);
			else
				setFocusView (pModalView->wantsFocus () ? pModalView : 0);
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
void CFrame::beginEdit (int32_t index)
{
	if (pEditor)
		pEditor->beginEdit (index);
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (int32_t index)
{
	if (pEditor)
		pEditor->endEdit (index);
}

//-----------------------------------------------------------------------------
/**
 * @param where location of mouse
 * @return true on success
 */
bool CFrame::getCurrentMouseLocation (CPoint &where) const
{
	if (platformFrame)
	{
		if (platformFrame->getCurrentMousePosition (where))
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

	if (platformFrame)
		platformFrame->getCurrentMouseButtons (buttons);

	return buttons;
}

//-----------------------------------------------------------------------------
/**
 * @param type cursor type see #CCursorType
 */
void CFrame::setCursor (CCursorType type)
{
	if (platformFrame)
		platformFrame->setMouseCursor (type);
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was removed
 */
void CFrame::onViewRemoved (CView* pView)
{
	removeFromMouseViews (pView);

	if (pActiveFocusView == pView)
		pActiveFocusView = 0;
	if (pFocusView == pView)
		setFocusView (0);
	CViewContainer* container = dynamic_cast<CViewContainer*> (pView);
	if (container)
	{
		if (container->isChild (pFocusView, true))
			setFocusView (0);
	}
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewRemoved (this, pView);
	if (pAnimator)
		pAnimator->removeAnimations (pView);
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was added
 */
void CFrame::onViewAdded (CView* pView)
{
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewAdded (this, pView);
}

//-----------------------------------------------------------------------------
/**
 * @param pView new focus view
 */
void CFrame::setFocusView (CView *pView)
{
	static bool recursion = false;
	if (pView == pFocusView || (recursion && pFocusView != 0))
		return;

	if (!bActive)
	{
		pActiveFocusView = pView;
		return;
	}

	recursion = true;

	CView *pOldFocusView = pFocusView;
	if (pView == 0  || (pView && pView->isAttached () == false))
		pFocusView = 0;
	else
		pFocusView = pView;
	if (pFocusView && pFocusView->wantsFocus ())
	{
		pFocusView->invalid ();

		CView* receiver = pFocusView->getParentView ();
		while (receiver != this && receiver != 0)
		{
			receiver->notify (pFocusView, kMsgNewFocusView);
			receiver = receiver->getParentView ();
		}
		notify (pFocusView, kMsgNewFocusView);
	}

	if (pOldFocusView)
	{
		if (pOldFocusView->wantsFocus ())
		{
			pOldFocusView->invalid ();

			CView* receiver = pOldFocusView->getParentView ();
			while (receiver != this && receiver != 0)
			{
				receiver->notify (pOldFocusView, kMsgOldFocusView);
				receiver = receiver->getParentView ();
			}
			notify (pOldFocusView, kMsgOldFocusView);
		}
		pOldFocusView->looseFocus ();
	}
	if (pFocusView && pFocusView->wantsFocus ())
		pFocusView->takeFocus ();
	recursion = false;
}

//-----------------------------------------------------------------------------
bool CFrame::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (pModalView)
	{
		CViewContainer* container = dynamic_cast<CViewContainer*> (pModalView);
		if (container)
		{
			if (oldFocus == 0 || container->isChild (oldFocus, true) == false)
				return container->advanceNextFocusView (0, reverse);
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
					return container->advanceNextFocusView (0, reverse);
				}
			}
		}
		else if (oldFocus != pModalView)
		{
			setFocusView (pModalView);
			return true;
		}
		return false; // currently not supported, but should be done sometime
	}
	if (oldFocus == 0)
	{
		if (pFocusView == 0)
			return CViewContainer::advanceNextFocusView (0, reverse);
		oldFocus = pFocusView;
	}
	if (isChild (oldFocus))
	{
		if (CViewContainer::advanceNextFocusView (oldFocus, reverse))
			return true;
		else
		{
			setFocusView (0);
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
	if (pModalView == pView)
		pModalView = 0;
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (bool withForget)
{
	setModalView (0);
	if (pFocusView)
	{
		pFocusView->looseFocus ();
		pFocusView = 0;
	}
	pActiveFocusView = 0;
	clearMouseViews (CPoint (0, 0), 0, false);
	return CViewContainer::removeAll (withForget);
}

//-----------------------------------------------------------------------------
CView* CFrame::getViewAt (const CPoint& where, const GetViewOptions& options) const
{
	if (pModalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pModalView->getViewSize ().pointInside (where2))
		{
			if (options.deep ())
			{
				CViewContainer* container = dynamic_cast<CViewContainer*> (pModalView);
				if (container)
				{
					return container->getViewAt (where2, options);
				}
			}
			return pModalView;
		}
		return 0;
	}
	return CViewContainer::getViewAt (where, options);
}

//-----------------------------------------------------------------------------
CViewContainer* CFrame::getContainerAt (const CPoint& where, const GetViewOptions& options) const
{
	if (pModalView)
	{
		CPoint where2 (where);
		getTransform ().inverse ().transform (where2);
		if (pModalView->getViewSize ().pointInside (where2))
		{
			CViewContainer* container = dynamic_cast<CViewContainer*> (pModalView);
			if (container)
			{
				if (options.deep ())
					return container->getContainerAt (where2, options);
				return container;
			}
		}
		return 0;
	}
	return CViewContainer::getContainerAt (where, options);
}

//-----------------------------------------------------------------------------
void CFrame::onActivate (bool state)
{
	if (bActive != state)
	{
		if (state)
		{
			bActive = true;
			if (pActiveFocusView)
			{
				setFocusView (pActiveFocusView);
				pActiveFocusView = 0;
			}
			else
				advanceNextFocusView (0, false);
		}
		else
		{
			if (pTooltips)
				pTooltips->hideTooltip ();
			pActiveFocusView = getFocusView ();
			setFocusView (0);
			bActive = false;
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

	if (platformFrame)
	{
		if (platformFrame->scrollRect (src, distance))
			return;
	}
	invalidRect (src);
}

//-----------------------------------------------------------------------------
void CFrame::invalidate (const CRect &rect)
{
	CRect rectView;
	FOREACHSUBVIEW
	if (pV)
	{
		pV->getViewSize (rectView);
		if (rect.rectOverlap (rectView))
			pV->setDirty (true);
	}
	ENDFOREACHSUBVIEW
}

//-----------------------------------------------------------------------------
void CFrame::invalidRect (const CRect& rect)
{
	if (!isVisible () || !platformFrame)
		return;

	CRect _rect (rect);
	getTransform ().transform (_rect);
	if (collectInvalidRects)
		collectInvalidRects->addRect (_rect);
	else
		platformFrame->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
void CFrame::registerKeyboardHook (IKeyboardHook* hook)
{
	if (pKeyboardHooks == 0)
		pKeyboardHooks = new KeyboardHookList ();
	pKeyboardHooks->push_back (hook);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterKeyboardHook (IKeyboardHook* hook)
{
	if (pKeyboardHooks)
	{
		pKeyboardHooks->remove (hook);
		if (pKeyboardHooks->empty ())
		{
			delete pKeyboardHooks;
			pKeyboardHooks = 0;
		}
	}
}

//-----------------------------------------------------------------------------
int32_t CFrame::keyboardHooksOnKeyDown (const VstKeyCode& key)
{
	if (pKeyboardHooks)
	{
		for (KeyboardHookList::reverse_iterator it = pKeyboardHooks->rbegin (); it != pKeyboardHooks->rend ();)
		{
			IKeyboardHook* hook = *it;
			it++;
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
	if (pKeyboardHooks)
	{
		for (KeyboardHookList::reverse_iterator it = pKeyboardHooks->rbegin (); it != pKeyboardHooks->rend ();)
		{
			IKeyboardHook* hook = *it;
			it++;
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
	if (pScaleFactorChangedListenerList == 0)
		pScaleFactorChangedListenerList = new ScaleFactorChangedListenerList ();
	pScaleFactorChangedListenerList->push_back (listener);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterScaleFactorChangedListeneer (IScaleFactorChangedListener* listener)
{
	if (pScaleFactorChangedListenerList)
	{
		pScaleFactorChangedListenerList->remove (listener);
		if (pScaleFactorChangedListenerList->empty ())
		{
			delete pScaleFactorChangedListenerList;
			pScaleFactorChangedListenerList = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void CFrame::registerMouseObserver (IMouseObserver* observer)
{
	if (pMouseObservers == 0)
		pMouseObservers = new MouseObserverList ();
	pMouseObservers->push_back (observer);
}

//-----------------------------------------------------------------------------
void CFrame::unregisterMouseObserver (IMouseObserver* observer)
{
	if (pMouseObservers)
	{
		pMouseObservers->remove (observer);
		if (pMouseObservers->empty ())
		{
			delete pMouseObservers;
			pMouseObservers = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseEntered (CView* view)
{
	if (pMouseObservers)
	{
		for (MouseObserverList::const_iterator it = pMouseObservers->begin (), end = pMouseObservers->end (); it != end; it++)
		{
			(*it)->onMouseEntered (view, this);
		}
	}
}

//-----------------------------------------------------------------------------
void CFrame::callMouseObserverMouseExited (CView* view)
{
	if (pMouseObservers)
	{
		for (MouseObserverList::const_iterator it = pMouseObservers->begin (), end = pMouseObservers->end (); it != end; it++)
		{
			(*it)->onMouseExited (view, this);
		}
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::callMouseObserverMouseDown (const CPoint& _where, const CButtonState& buttons)
{
	CMouseEventResult result = kMouseEventNotHandled;
	if (pMouseObservers)
	{
		CPoint where (_where);
		getTransform ().inverse ().transform (where);
		
		for (MouseObserverList::const_iterator it = pMouseObservers->begin (), end = pMouseObservers->end (); it != end; it++)
		{
			CMouseEventResult result2 = (*it)->onMouseDown (this, where, buttons);
			if (result2 == kMouseEventHandled)
				result = kMouseEventHandled;
			if (pMouseObservers == 0)
				return kMouseEventHandled;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::callMouseObserverMouseMoved (const CPoint& _where, const CButtonState& buttons)
{
	CMouseEventResult result = kMouseEventNotHandled;
	if (pMouseObservers)
	{
		CPoint where (_where);
		getTransform ().inverse ().transform (where);
		
		for (MouseObserverList::const_iterator it = pMouseObservers->begin (), end = pMouseObservers->end (); it != end; it++)
		{
			CMouseEventResult result2 = (*it)->onMouseMoved (this, where, buttons);
			if (result2 == kMouseEventHandled)
				result = kMouseEventHandled;
		}
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
	if (pParentFrame)
	{
		CollectInvalidRects cir (this);
		onActivate (state);
	}
}

//-----------------------------------------------------------------------------
void CFrame::platformScaleFactorChanged ()
{
	if (pScaleFactorChangedListenerList == 0)
		return;
	for (ScaleFactorChangedListenerList::const_iterator it = pScaleFactorChangedListenerList->begin (), end = pScaleFactorChangedListenerList->end (); it != end; it++)
	{
		(*it)->onScaleFactorChanged (this);
	}
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
	if (collectInvalidRects)
	{
		collectInvalidRects->flush ();
		collectInvalidRects = 0;
	}
}

//-----------------------------------------------------------------------------
void CFrame::setCollectInvalidRects (CollectInvalidRects* cir)
{
	if (collectInvalidRects)
		collectInvalidRects->flush ();
	collectInvalidRects = cir;
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
CFrame::CollectInvalidRects::~CollectInvalidRects ()
{
	frame->setCollectInvalidRects (0);
}

//-----------------------------------------------------------------------------
void CFrame::CollectInvalidRects::flush ()
{
	if (!invalidRects.empty ())
	{
		if (frame->isVisible () && frame->platformFrame)
		{
			for (InvalidRects::const_iterator it = invalidRects.begin (), end = invalidRects.end (); it != end; ++it)
				frame->platformFrame->invalidRect (*it);
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

