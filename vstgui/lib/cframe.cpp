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

#include "cframe.h"
#include "coffscreencontext.h"

namespace VSTGUI {

#if MAC_CARBON && MAC_COCOA
	static bool createNSViewMode = false;
#endif

const char* kMsgNewFocusView = "kMsgNewFocusView";
const char* kMsgOldFocusView = "kMsgOldFocusView";

//-----------------------------------------------------------------------------
// CFrame Implementation
//-----------------------------------------------------------------------------
/*! @class CFrame
It creates a platform dependend view object. 

On Mac OS X it is a HIView or NSView.\n 
On Windows it's a WS_CHILD Window.

*/
//-----------------------------------------------------------------------------
/**
 * @param inSize size of frame
 * @param inSystemWindow parent platform window
 * @param inEditor editor
 */
CFrame::CFrame (const CRect &inSize, void* inSystemWindow, VSTGUIEditorInterface* inEditor)
: CViewContainer (inSize, 0, 0)
, pEditor (inEditor)
, pMouseObserver (0)
, pKeyboardHook (0)
, pViewAddedRemovedObserver (0)
, pModalView (0)
, pFocusView (0)
, pActiveFocusView (0)
, pMouseOverView (0)
, bActive (true)
{
	bIsAttached = true;
	
	pParentFrame = this;

	initFrame (inSystemWindow);

}

//-----------------------------------------------------------------------------
CFrame::~CFrame ()
{
	if (pModalView)
		removeView (pModalView, false);

	setCursor (kCursorDefault);

	pParentFrame = 0;
	removeAll ();

	if (platformFrame)
		platformFrame->forget ();
}

#if MAC_COCOA && MAC_CARBON
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
bool CFrame::initFrame (void* systemWin)
{
	if (!systemWin)
		return false;

	platformFrame = IPlatformFrame::createPlatformFrame (this, size, systemWin);
	if (!platformFrame)
		return false;

	return true;
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
	return CFrame::drawRect (pContext, size);
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
CMouseEventResult CFrame::onMouseDown (CPoint &where, const long& buttons)
{
	// reset views
	mouseDownView = 0;
	if (pFocusView)
		setFocusView (NULL);
	if (pMouseOverView)
	{
		CBaseObjectGuard rg (pMouseOverView);

		pMouseOverView->onMouseExited (where, buttons);
		if (getMouseObserver ())
			getMouseObserver ()->onMouseExited (pMouseOverView, this);
		pMouseOverView = 0;
	}

	if (getMouseObserver ())
		getMouseObserver ()->onMouseDown (this, where);

	if (pModalView)
	{
		CBaseObjectGuard rg (pModalView);

		if (pModalView->hitTest (where, buttons))
		{
			CMouseEventResult result = pModalView->onMouseDown (where, buttons);
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
CMouseEventResult CFrame::onMouseUp (CPoint &where, const long& buttons)
{
	CMouseEventResult result = CViewContainer::onMouseUp (where, buttons);
	long modifiers = buttons & (kShift | kControl | kAlt | kApple);
	onMouseMoved (where, modifiers);
	return result;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseMoved (CPoint &where, const long& buttons)
{
	if (getMouseObserver ())
		getMouseObserver ()->onMouseMoved (this, where);
	if (pModalView)
		return pModalView->onMouseMoved (where, buttons);
	else
	{
		CMouseEventResult result = CViewContainer::onMouseMoved (where, buttons);
		if (result == kMouseEventNotHandled)
		{
			CView* v = getViewAt (where, true);
			if (v != pMouseOverView)
			{
				if (pMouseOverView)
				{
					CBaseObjectGuard rg (pMouseOverView);

					CPoint lr (where);
					pMouseOverView->frameToLocal (lr);
					pMouseOverView->onMouseExited (lr, buttons);
					if (getMouseObserver ())
						getMouseObserver ()->onMouseExited (pMouseOverView, this);
				}
				pMouseOverView = 0;
				if (v)
				{
					CBaseObjectGuard rg (v);

					CPoint lr (where);
					v->frameToLocal (lr);
					v->onMouseEntered (lr, buttons);
					pMouseOverView = v;
					if (getMouseObserver ())
						getMouseObserver ()->onMouseEntered (pMouseOverView, this);
					v->onMouseMoved (lr, buttons);
				}
				return kMouseEventHandled;
			}
			else if (pMouseOverView)
			{
				CPoint lr (where);
				pMouseOverView->frameToLocal (lr);
				return pMouseOverView->onMouseMoved (lr, mouseDownView ? buttons : 0);
			}
		}
		return result;
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseExited (CPoint &where, const long& buttons)
{ // this should only get called from the platform implementation
	if (pMouseOverView)
	{
		CBaseObjectGuard rg (pMouseOverView);

		CPoint lr (where);
		pMouseOverView->frameToLocal (lr);
		pMouseOverView->onMouseExited (lr, buttons);
		if (getMouseObserver ())
			getMouseObserver ()->onMouseExited (pMouseOverView, this);
	}
	pMouseOverView = 0;

	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
long CFrame::onKeyDown (VstKeyCode& keyCode)
{
	long result = -1;

	if (getKeyboardHook ())
		result = getKeyboardHook ()->onKeyDown (keyCode, this);

	if (result == -1 && pFocusView)
	{
		CBaseObjectGuard og (pFocusView);
		result = pFocusView->onKeyDown (keyCode);
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
long CFrame::onKeyUp (VstKeyCode& keyCode)
{
	long result = -1;

	if (getKeyboardHook ())
		result = getKeyboardHook ()->onKeyUp (keyCode, this);

	if (result == -1 && pFocusView)
		result = pFocusView->onKeyUp (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyUp (keyCode);

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	bool result = false;

	if (mouseDownView == 0)
	{
		CView* view = pModalView ? pModalView : getViewAt (where);
		if (view)
		{
			result = view->onWheel (where, axis, distance, buttons);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const float &distance, const long &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	invalidateDirtyViews ();
}

//-----------------------------------------------------------------------------
void CFrame::doIdleStuff ()
{
	if (pEditor)
		pEditor->doIdleStuff ();
}

//-----------------------------------------------------------------------------
/**
 * @return tick count in milliseconds
 */
unsigned long CFrame::getTicks () const
{
	if (platformFrame)
		return platformFrame->getTicks ();
	return -1;
}

//-----------------------------------------------------------------------------
long CFrame::getKnobMode () const
{
	if (pEditor)
		return pEditor->getKnobMode ();
	return kCircularMode;
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
		CRect rect (size);
		size.offset (x - size.left, y - size.top);
		return platformFrame->setSize (rect);
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
void CFrame::setViewSize (CRect& rect, bool invalid)
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
	if ((width == size.width ()) && (height == size.height ()))
		return false;

	CRect newSize (size);
	newSize.setWidth (width);
	newSize.setHeight (height);

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
	// If there is a modal view or the view 
	if ((pView && pModalView) || (pView && pView->isAttached ()))
		return false;

	if (pModalView)
		removeView (pModalView, false);
	
	pModalView = pView;
	if (pModalView)
		return addView (pModalView);

	return true;
}

//-----------------------------------------------------------------------------
void CFrame::beginEdit (long index)
{
	if (pEditor)
		pEditor->beginEdit (index);
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (long index)
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
		return platformFrame->getCurrentMousePosition (where);
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @return mouse and modifier state
 */
long CFrame::getCurrentMouseButtons () const
{
	long buttons = 0;

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
	if (pMouseOverView == pView)
		pMouseOverView = 0;
	if (pActiveFocusView == pView)
		pActiveFocusView = 0;
	if (pFocusView == pView)
		setFocusView (0);
	if (pView->isTypeOf ("CViewContainer"))
	{
		CViewContainer* container = (CViewContainer*)pView;
		if (container->isChild (pMouseOverView, true))
			pMouseOverView = 0;
		if (container->isChild (pFocusView, true))
			setFocusView (0);
	}
	if (getViewAddedRemovedObserver ())
		getViewAddedRemovedObserver ()->onViewRemoved (this, pView);
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
		if (pModalView->isTypeOf("CViewContainer"))
		{
			return ((CViewContainer*)pModalView)->advanceNextFocusView (oldFocus, reverse);
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
			setFocusView (NULL);
			return false;
		}
	}
	CView* parentView = oldFocus->getParentView ();
	if (parentView && parentView->isTypeOf ("CViewContainer"))
	{
		CView* tempOldFocus = oldFocus;
		CViewContainer* vc = (CViewContainer*)parentView;
		while (vc)
		{
			if (vc->advanceNextFocusView (tempOldFocus, reverse))
				return true;
			else
			{
				tempOldFocus = vc;
				if (vc->getParentView () && vc->getParentView ()->isTypeOf ("CViewContainer"))
					vc = (CViewContainer*)vc->getParentView ();
				else
					vc = 0;
			}
		}
	}
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//-----------------------------------------------------------------------------
bool CFrame::removeView (CView* pView, const bool &withForget)
{
	if (pModalView == pView)
		pModalView = 0;
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (const bool &withForget)
{
	pModalView = 0;
	pFocusView = 0;
	pActiveFocusView = 0;
	pMouseOverView = 0;
	return CViewContainer::removeAll (withForget);
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
			pActiveFocusView = getFocusView ();
			setFocusView (0);
			bActive = false;
		}
	}
}

//-----------------------------------------------------------------------------
bool CFrame::focusDrawingEnabled () const
{
	long attrSize;
	if (getAttributeSize ('vfde', attrSize))
		return true;
	return false;
}

//-----------------------------------------------------------------------------
CColor CFrame::getFocusColor () const
{
	CColor focusColor (kRedCColor);
	long outSize;
	getAttribute ('vfco', sizeof (CColor), &focusColor, outSize);
	return focusColor;
}

//-----------------------------------------------------------------------------
CCoord CFrame::getFocusWidth () const
{
	CCoord focusWidth = 2;
	long outSize;
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
	rect.offset (size.left, size.top);

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
void CFrame::invalidRect (CRect rect)
{
	if (!bVisible)
		return;
	if (platformFrame)
		platformFrame->invalidRect (rect);
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
CMouseEventResult CFrame::platformOnMouseDown (CPoint& where, const long& buttons)
{
	return onMouseDown (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseMoved (CPoint& where, const long& buttons)
{
	return onMouseMoved (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseUp (CPoint& where, const long& buttons)
{
	return onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::platformOnMouseExited (CPoint& where, const long& buttons)
{
	return onMouseExited (where, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	return onWheel (where, axis, distance, buttons);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnDrop (CDragContainer* drag, const CPoint& where)
{
	return onDrop (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragEnter (CDragContainer* drag, const CPoint& where)
{
	return onDragEnter (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragLeave (CDragContainer* drag, const CPoint& where)
{
	return onDragLeave (drag, where);
}

//-----------------------------------------------------------------------------
void CFrame::platformOnDragMove (CDragContainer* drag, const CPoint& where)
{
	return onDragMove (drag, where);
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyDown (VstKeyCode& keyCode)
{
	return onKeyDown (keyCode) == 1;
}

//-----------------------------------------------------------------------------
bool CFrame::platformOnKeyUp (VstKeyCode& keyCode)
{
	return onKeyUp (keyCode) == 1;
}

//-----------------------------------------------------------------------------
void CFrame::platformOnActivate (bool state)
{
	onActivate (state);
}

} // namespace

