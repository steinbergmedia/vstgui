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

#include "cviewcontainer.h"
#include "coffscreencontext.h"
#include "cbitmap.h"
#include "cframe.h"
#include "ifocusdrawing.h"
#include "controls/ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CCView Implementation
/// @cond ignore
//-----------------------------------------------------------------------------
CCView::CCView (CView* pView)
: pView (pView)
, pNext (0)
, pPrevious (0)
{
	if (pView)
		pView->remember ();
}

//-----------------------------------------------------------------------------
CCView::~CCView ()
{ 
	if (pView)
		pView->forget (); 
}
/// @endcond

const char* kMsgCheckIfViewContainer	= "kMsgCheckIfViewContainer";
const char* kMsgLooseFocus = "LooseFocus";

//-----------------------------------------------------------------------------
// CViewContainer Implementation
//-----------------------------------------------------------------------------
/**
 * CViewContainer constructor.
 * @param rect the size of the container
 * @param pParent the parent CFrame
 * @param pBackground the background bitmap, can be NULL
 */
CViewContainer::CViewContainer (const CRect &rect, CFrame* pParent, CBitmap* pBackground)
: CView (rect)
, pFirstView (0)
, pLastView (0)
, pOffscreenContext (0)
, bDrawInOffscreen (false)
, currentDragView (0)
, mouseDownView (0)
{
	backgroundOffset (0, 0);
	this->pParentFrame = pParent;
	setBackground (pBackground);
	backgroundColor = kBlackCColor;
}

//-----------------------------------------------------------------------------
CViewContainer::CViewContainer (const CViewContainer& v)
: CView (v)
, pFirstView (0)
, pLastView (0)
, pOffscreenContext (0)
, backgroundColor (v.backgroundColor)
, backgroundOffset (v.backgroundOffset)
, bDrawInOffscreen (v.bDrawInOffscreen)
, currentDragView (0)
, mouseDownView (0)
{
	for (long i = 0; i < v.getNbViews (); i++)
	{
		addView ((CView*)v.getView (i)->newCopy ());
	}
}

//-----------------------------------------------------------------------------
CViewContainer::~CViewContainer ()
{
	// remove all views
	removeAll (true);

	 if (pOffscreenContext)
		pOffscreenContext->forget ();
	pOffscreenContext = 0;
}

//-----------------------------------------------------------------------------
void CViewContainer::parentSizeChanged ()
{
	FOREACHSUBVIEW
		pV->parentSizeChanged ();	// notify children that the size of the parent or this container has changed
	ENDFOREACHSUBVIEW
}

//-----------------------------------------------------------------------------
/**
 * @param rect the new size of the container
 * @param invalid the views to dirty
 */
void CViewContainer::setViewSize (CRect &rect, bool invalid)
{
	if (rect == getViewSize ())
		return;

	CRect oldSize (getViewSize ());
	CView::setViewSize (rect, invalid);

	CCoord widthDelta = rect.getWidth () - oldSize.getWidth ();
	CCoord heightDelta = rect.getHeight () - oldSize.getHeight ();

	if (widthDelta != 0 || heightDelta != 0)
	{
		long numSubviews = getNbViews ();
		long counter = 0;
		bool treatAsColumn = (getAutosizeFlags () & kAutosizeColumn) != 0;
		bool treatAsRow = (getAutosizeFlags () & kAutosizeRow) != 0;
		FOREACHSUBVIEW
			long autosize = pV->getAutosizeFlags ();
			CRect viewSize (pV->getViewSize ());
			CRect mouseSize (pV->getMouseableArea ());
			if (treatAsColumn)
			{
				if (counter)
				{
					viewSize.offset (counter * (widthDelta / (numSubviews)), 0);
					mouseSize.offset (counter * (widthDelta / (numSubviews)), 0);
				}
				viewSize.setWidth (viewSize.getWidth () + (widthDelta / (numSubviews)));
				mouseSize.setWidth (mouseSize.getWidth () + (widthDelta / (numSubviews)));
			}
			else if (widthDelta != 0 && autosize & kAutosizeRight)
			{
				viewSize.right += widthDelta;
				mouseSize.right += widthDelta;
				if (!(autosize & kAutosizeLeft))
				{
					viewSize.left += widthDelta;
					mouseSize.left += widthDelta;
				}
			}
			if (treatAsRow)
			{
				if (counter)
				{
					viewSize.offset (0, counter * (heightDelta / (numSubviews)));
					mouseSize.offset (0, counter * (heightDelta / (numSubviews)));
				}
				viewSize.setHeight (viewSize.getHeight () + (heightDelta / (numSubviews)));
				mouseSize.setHeight (mouseSize.getHeight () + (heightDelta / (numSubviews)));
			}
			else if (heightDelta != 0 && autosize & kAutosizeBottom)
			{
				viewSize.bottom += heightDelta;
				mouseSize.bottom += heightDelta;
				if (!(autosize & kAutosizeTop))
				{
					viewSize.top += heightDelta;
					mouseSize.top += heightDelta;
				}
			}
			if (viewSize != pV->getViewSize ())
			{
				pV->setViewSize (viewSize);
				pV->setMouseableArea (mouseSize);
			}
			counter++;
		ENDFOREACHSUBVIEW
	}
	
	parentSizeChanged ();

	if (pOffscreenContext && bDrawInOffscreen)
	{
		pOffscreenContext->forget ();
		pOffscreenContext = COffscreenContext::create (getFrame (), size.getWidth (), size.getHeight ());
		if (pOffscreenContext)
		{
			pOffscreenContext->setFillColor (kBlackCColor);
			pOffscreenContext->drawRect (CRect (0, 0, size.getWidth (), size.getHeight ()), kDrawFilled);
		}
	}
}

//-----------------------------------------------------------------------------
/**
 * @param rect size to get visible size of
 * @return visible size of rect
 */
CRect CViewContainer::getVisibleSize (const CRect rect) const
{
	CRect result (rect);
	result.offset (size.left, size.top);
	result.bound (size);
	if (pParentFrame == this)
	{}
	else if (pParentView && pParentView->isTypeOf("CViewContainer"))
		result = ((CViewContainer*)pParentView)->getVisibleSize (result);
	else if (pParentFrame)
		result = pParentFrame->getVisibleSize (result);
	result.offset (-size.left, -size.top);
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::sizeToFit ()
{
	bool treatAsColumn = (getAutosizeFlags () & kAutosizeColumn) != 0;
	bool treatAsRow = (getAutosizeFlags () & kAutosizeRow) != 0;
	if (treatAsColumn || treatAsRow)
		return false;

	CRect bounds (50000, 50000, -50000, -50000);
	FOREACHSUBVIEW
		CRect vs (pV->getViewSize ());
		if (vs.left < bounds.left)
			bounds.left = vs.left;
		if (vs.right > bounds.right)
			bounds.right = vs.right;
		if (vs.top < bounds.top)
			bounds.top = vs.top;
		if (vs.bottom > bounds.bottom)
			bounds.bottom = vs.bottom;
	ENDFOREACHSUBVIEW
	
	CRect vs (getViewSize ());
	vs.right = vs.left + bounds.right + bounds.left;
	vs.bottom = vs.top + bounds.bottom + bounds.top;
	
	setViewSize (vs);
	setMouseableArea (vs);

	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param color the new background color of the container
 */
void CViewContainer::setBackgroundColor (const CColor& color)
{
	backgroundColor = color;
	setDirty (true);
}

//------------------------------------------------------------------------------
CMessageResult CViewContainer::notify (CBaseObject* sender, const char* message)
{
	if (message == kMsgCheckIfViewContainer)
		return kMessageNotified;
	else if (message == kMsgNewFocusView)
	{
		CView* view = dynamic_cast<CView*> (sender);
		if (view && isChild (view, false))
		{
			IFocusDrawing* focusDrawing = dynamic_cast<IFocusDrawing*> (view);
			if (focusDrawing)
			{
				CGraphicsPath* focusPath = CGraphicsPath::create (getFrame ());
				if (focusPath)
				{
					focusDrawing->getFocusPath (*focusPath);
					CRect r = focusPath->getBoundingBox ();
					invalidRect (r);
					focusPath->forget ();
				}
				
			}
			else
			{
				CCoord width = getFrame ()->getFocusWidth ();
				CRect viewSize (view->getViewSize ());
				viewSize.inset (-width, -width);
				invalidRect (viewSize);
			}
		}
	}
	else if (message == kMsgOldFocusView)
	{
		if (!lastDrawnFocus.isEmpty ())
			invalidRect (lastDrawnFocus);
		lastDrawnFocus = CRect (0, 0, 0, 0);
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @return true on success. false if view was already attached
 */
bool CViewContainer::addView (CView* pView)
{
	if (!pView)
		return false;

	if (pView->isAttached ())
		return false;

	CCView* pSv = new CCView (pView);
	
	CCView* pV = pFirstView;
	if (!pV)
	{
		pLastView = pFirstView = pSv;
	}
	else
	{
		while (pV->pNext)
			pV = pV->pNext;
		pV->pNext = pSv;
		pSv->pPrevious = pV;
		pLastView = pSv;
	}
	if (isAttached ())
	{
		pView->attached (this);
		pView->invalid ();
	}
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @param pBefore the view object
 * @return true on success. false if view was already attached
 */
bool CViewContainer::addView (CView *pView, CView* pBefore)
{
	if (!pView)
		return false;

	if (pView->isAttached ())
		return false;

	CCView* pSv = new CCView (pView);

	CCView* pV = pFirstView;
	if (!pV)
	{
		pLastView = pFirstView = pSv;
	}
	else
	{
		while (pV->pNext && pV->pView != pBefore)
		{
			pV = pV->pNext;
		}
		pSv->pNext = pV;
		if (pV)
		{
			pSv->pPrevious = pV->pPrevious;
			pV->pPrevious = pSv;
			if (pSv->pPrevious == 0)
				pFirstView = pSv;
			else
				pSv->pPrevious->pNext = pSv;
		}
		else
			pLastView = pSv;
	}
	if (isAttached ())
	{
		pView->attached (this);
		pView->invalid ();
	}
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @param mouseableArea the view area in where the view will get mouse events
 * @param mouseEnabled bool to set if view will get mouse events
 * @return true on success. false if view was already attached
 */
bool CViewContainer::addView (CView* pView, CRect &mouseableArea, bool mouseEnabled)
{
	if (!pView)
		return false;

	if (addView (pView))
	{
		pView->setMouseEnabled (mouseEnabled);
		pView->setMouseableArea (mouseableArea);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param withForget bool to indicate if the view's reference counter should be decreased after removed from the container
 * @return true on success
 */
bool CViewContainer::removeAll (const bool &withForget)
{
	if (mouseDownView)
		mouseDownView = 0;
	currentDragView = 0;
	CCView *pV = pFirstView;
	while (pV)
	{
		CCView* pNext = pV->pNext;
		if (pV->pView)
		{
			if (isAttached ())
				pV->pView->removed (this);
			if (withForget)
				pV->pView->forget ();
		}

		delete pV;

		pV = pNext;
	}
	pFirstView = 0;
	pLastView = 0;
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be removed from the container
 * @param withForget bool to indicate if the view's reference counter should be decreased after removed from the container
 * @return true on success
 */
bool CViewContainer::removeView (CView *pView, const bool &withForget)
{
	if (pView == mouseDownView)
		mouseDownView = 0;
	if (pView == currentDragView)
		currentDragView = 0;
	CCView *pV = pFirstView;
	while (pV)
	{
		if (pView == pV->pView)
		{
			CCView* pNext = pV->pNext;
			CCView* pPrevious = pV->pPrevious;
			if (pV->pView)
			{
				pV->pView->invalid ();
				if (isAttached ())
					pV->pView->removed (this);
				if (withForget)
					pV->pView->forget ();
			}
			delete pV;
			if (pPrevious)
			{
				pPrevious->pNext = pNext;
				if (pNext)
					pNext->pPrevious = pPrevious;
				else
					pLastView = pPrevious;
			}
			else
			{
				pFirstView = pNext;
				if (pNext)
					pNext->pPrevious = 0;
				else
					pLastView = 0;	
			}
			return true;
		}
		else
			pV = pV->pNext;
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be checked if it is a child of this container
 * @return true on success
 */
bool CViewContainer::isChild (CView* pView) const
{
	return isChild (pView, false);
}

//-----------------------------------------------------------------------------
bool CViewContainer::isChild (CView *pView, bool deep) const
{
	bool found = false;

	CCView* pV = pFirstView;
	while (pV && !found)
	{
		if (pView == pV->pView)
		{
			found = true;
			break;
		}
		if (deep && pV->pView->isTypeOf ("CViewContainer"))
			found = ((CViewContainer*)pV->pView)->isChild (pView, true);
		pV = pV->pNext;
	}
	return found;
}

//-----------------------------------------------------------------------------
/**
 * @return number of subviews
 */
long CViewContainer::getNbViews () const
{
	long nb = 0;
	for (CCView* pSv = pFirstView; pSv; pSv = pSv->pNext)
		nb++;
	return nb;
}

//-----------------------------------------------------------------------------
/**
 * @param index the index of the view to return
 * @return view at index. NULL if view at index does not exist.
 */
CView* CViewContainer::getView (long index) const
{
	long nb = 0;
	FOREACHSUBVIEW
		if (nb == index)
			return pV;
		nb++;
	ENDFOREACHSUBVIEW
	return 0;
}

//-----------------------------------------------------------------------------
bool CViewContainer::invalidateDirtyViews ()
{
	if (!bVisible)
		return true;
	if (bDirty)
	{
		if (pParentView)
			pParentView->invalidRect (size);
		else if (pParentFrame)
			pParentFrame->invalidRect (size);
		return true;
	}
	FOREACHSUBVIEW
		if (pV->isDirty () && pV->isVisible ())
		{
			if (pV->isTypeOf ("CViewContainer"))
				((CViewContainer*)pV)->invalidateDirtyViews ();
			else
				pV->invalid ();
		}
	ENDFOREACHSUBVIEW
	return true;
}

//-----------------------------------------------------------------------------
void CViewContainer::invalid ()
{
	if (!bVisible)
		return;
	CRect _rect (size);
	if (pParentView)
		pParentView->invalidRect (_rect);
	else if (pParentFrame)
		pParentFrame->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
void CViewContainer::invalidRect (CRect rect)
{
	if (!bVisible)
		return;
	CRect _rect (rect);
	_rect.offset (size.left, size.top);
	_rect.bound (size);
	if (_rect.isEmpty ())
		return;
	if (pParentView)
		pParentView->invalidRect (_rect);
	else if (pParentFrame)
		pParentFrame->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw this container and its subviews
 */
void CViewContainer::draw (CDrawContext* pContext)
{
	CViewContainer::drawRect (pContext, size);
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw the background
 * @param _updateRect the area which to draw
 */
void CViewContainer::drawBackgroundRect (CDrawContext* pContext, CRect& _updateRect)
{
	if (pBackground)
	{
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (_updateRect);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		CRect tr (0, 0, size.getWidth (), size.getHeight ());
		pBackground->draw (pContext, tr, backgroundOffset);
		pContext->setClipRect (oldClip);
	}
	else if ((backgroundColor.alpha != 255 && bTransparencyEnabled) || !bTransparencyEnabled)
	{
		pContext->setDrawMode (kAliasing);
		pContext->setLineWidth (1);
		pContext->setFillColor (backgroundColor);
		pContext->setFrameColor (backgroundColor);
		CRect r (size);
		r.offset (-r.left, -r.top);
		pContext->drawRect (r, kDrawFilledAndStroked);
	}
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CViewContainer::drawBackToFront (CDrawContext* pContext, const CRect& updateRect)
{
	CCoord save[4];
	modifyDrawContext (save, pContext);

	CRect _updateRect (updateRect);
	_updateRect.bound (size);

	CRect clientRect (_updateRect);

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect oldClip2 (oldClip);
	if (bDrawInOffscreen && getFrame () != this)
		oldClip.offset (-oldClip.left, -oldClip.top);
	
	CRect newClip (clientRect);
	newClip.bound (oldClip);
	pContext->setClipRect (newClip);
	
	// draw the background
	drawBackgroundRect (pContext, clientRect);
	
	// draw each view
	FOREACHSUBVIEW
		if (pV->checkUpdate (clientRect))
		{
			CRect viewSize = pV->getViewSize (viewSize);
			viewSize.bound (newClip);
			if (viewSize.getWidth () == 0 || viewSize.getHeight () == 0)
				continue;
			pContext->setClipRect (viewSize);

			pV->drawRect (pContext, clientRect);
		}
	ENDFOREACHSUBVIEW

	pContext->setClipRect (oldClip2);
	restoreDrawContext (pContext, save);
}
#endif

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw
 * @param updateRect the area which to draw
 */
void CViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	CDrawContext* pC;
	CCoord save[4];

	if (!pOffscreenContext && bDrawInOffscreen)
	{
		pOffscreenContext = COffscreenContext::create (getFrame (), size.getWidth (), size.getHeight ());
		if (pOffscreenContext)
		{
			pOffscreenContext->setFillColor (kBlackCColor);
			pOffscreenContext->drawRect (CRect (0, 0, size.getWidth (), size.getHeight ()), kDrawFilled);
		}
	}

	if (bDrawInOffscreen)
		pC = pOffscreenContext;
	else
	{
		pC = pContext;
		modifyDrawContext (save, pContext);
	}

	CRect _updateRect (updateRect);
	_updateRect.bound (size);

	CRect clientRect (_updateRect);
	clientRect.offset (-size.left, -size.top);

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect oldClip2 (oldClip);
	if (bDrawInOffscreen && getFrame () != this)
		oldClip.offset (-oldClip.left, -oldClip.top);
	
	CRect newClip (clientRect);
	newClip.bound (oldClip);
	pC->setClipRect (newClip);
	
	// draw the background
	drawBackgroundRect (pC, clientRect);
	
	CView* _focusView = 0;
	IFocusDrawing* _focusDrawing = 0;
	if (getFrame ()->focusDrawingEnabled () && isChild (getFrame ()->getFocusView (), false) && getFrame ()->getFocusView ()->wantsFocus ())
	{
		_focusView = getFrame ()->getFocusView ();
		_focusDrawing = dynamic_cast<IFocusDrawing*> (_focusView);
	}

	// draw each view
	FOREACHSUBVIEW
		
		if (_focusDrawing && _focusView == pV && !_focusDrawing->drawFocusOnTop ())
		{
			CGraphicsPath* focusPath = CGraphicsPath::create (getFrame ());
			if (focusPath)
			{
				if (_focusDrawing->getFocusPath (*focusPath))
				{
					lastDrawnFocus = focusPath->getBoundingBox ();
					if (!lastDrawnFocus.isEmpty ())
					{
						pC->setClipRect (oldClip2);
						pC->setDrawMode (kAntiAliasing);
						pC->setFillColor (getFrame ()->getFocusColor ());
						focusPath->draw (pC, CGraphicsPath::kFilledEvenOdd);
					}
					_focusDrawing = 0;
					_focusView = 0;
				}
				focusPath->forget ();
			}
		}

		if (checkUpdateRect (pV, clientRect))
		{
			CRect viewSize = pV->getViewSize (viewSize);
			viewSize.bound (newClip);
			if (viewSize.getWidth () == 0 || viewSize.getHeight () == 0)
				continue;
			pC->setClipRect (viewSize);
			float globalContextAlpha = pC->getGlobalAlpha ();
			pC->setGlobalAlpha (globalContextAlpha * pV->getAlphaValue ());
			pV->drawRect (pC, clientRect);
			pC->setGlobalAlpha (globalContextAlpha);
		}
	ENDFOREACHSUBVIEW

	pC->setClipRect (oldClip2);

	if (_focusView)
	{
		CGraphicsPath* focusPath = CGraphicsPath::create (getFrame ());
		if (focusPath)
		{
			if (_focusDrawing)
				_focusDrawing->getFocusPath (*focusPath);
			else
			{
				CCoord focusWidth = getFrame ()->getFocusWidth ();
				CRect r (_focusView->getVisibleSize ());
				if (!r.isEmpty ())
				{
					focusPath->addRect (r);
					r.inset (-focusWidth, -focusWidth);
					focusPath->addRect (r);
				}
			}
			lastDrawnFocus = focusPath->getBoundingBox ();
			if (!lastDrawnFocus.isEmpty ())
			{
				pC->setDrawMode (kAntiAliasing);
				pC->setFillColor (getFrame ()->getFocusColor ());
				focusPath->draw (pC, CGraphicsPath::kFilledEvenOdd);
			}
			focusPath->forget ();
		}
	}
	
	// transfer offscreen
	if (bDrawInOffscreen)
		((COffscreenContext*)pC)->copyFrom (pContext, _updateRect, CPoint (clientRect.left, clientRect.top));
	else
		restoreDrawContext (pContext, save);

	setDirty (false);
}

//-----------------------------------------------------------------------------
/**
 * check if view needs to be updated for rect
 * @param view view to check
 * @param rect update rect
 * @return true if view needs update
 */
bool CViewContainer::checkUpdateRect (CView* view, const CRect& rect)
{
	return view->checkUpdate (rect) && view->isVisible ();
}

//-----------------------------------------------------------------------------
/**
 * @param where point
 * @param buttons mouse button and modifier state
 * @return true if any sub view accepts the hit
 */
bool CViewContainer::hitTestSubViews (const CPoint& where, const long buttons)
{
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
			return true;
	ENDFOREACHSUBVIEW
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param where point
 * @param buttons mouse button and modifier state
 * @return true if container accepts the hit
 */
bool CViewContainer::hitTest (const CPoint& where, const long buttons)
{
	//return hitTestSubViews (where); would change default behavior
	return CView::hitTest (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseDown (CPoint &where, const long& buttons)
{
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			if (pV->isTypeOf("CControl") && ((CControl*)pV)->getListener () && buttons & (kAlt | kShift | kControl | kApple))
			{
				if (((CControl*)pV)->getListener ()->controlModifierClicked ((CControl*)pV, buttons) != 0)
					return kMouseEventHandled;
			}
			CBaseObjectGuard crg (pV);

			if (pV->wantsFocus ())
				getFrame ()->setFocusView (pV);

			CMouseEventResult result = pV->onMouseDown (where2, buttons);
			if (result != kMouseEventNotHandled && pV->getNbReference () > 1)
			{
				if (result == kMouseEventHandled)
					mouseDownView = pV;
			}
			return result;
		}
	ENDFOREACHSUBVIEW
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseUp (CPoint &where, const long& buttons)
{
	if (mouseDownView)
	{
		CBaseObjectGuard crg (mouseDownView);

		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);
		mouseDownView->onMouseUp (where2, buttons);
		mouseDownView = 0;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseMoved (CPoint &where, const long& buttons)
{
	if (mouseDownView)
	{
		CBaseObjectGuard crg (mouseDownView);

		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);
		if (mouseDownView->onMouseMoved (where2, buttons) != kMouseEventHandled)
		{
			mouseDownView = 0;
			return kMouseEventNotHandled;
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
long CViewContainer::onKeyDown (VstKeyCode& keyCode)
{
	long result = -1;

	FOREACHSUBVIEW_REVERSE(true)
		result = pV->onKeyDown (keyCode);
		if (result != -1)
			break;
	ENDFOREACHSUBVIEW

	return result;
}

//-----------------------------------------------------------------------------
long CViewContainer::onKeyUp (VstKeyCode& keyCode)
{
	long result = -1;

	FOREACHSUBVIEW_REVERSE(true)
		result = pV->onKeyUp (keyCode);
		if (result != -1)
			break;
	ENDFOREACHSUBVIEW
	
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	bool result = false;
	CView* view = getViewAt (where);
	if (view)
	{
		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);

		result = view->onWheel (where2, axis, distance, buttons);
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const float &distance, const long &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

//-----------------------------------------------------------------------------
bool CViewContainer::onDrop (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return false;

	bool result = false;

	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	CView* view = getViewAt (where);
	if (view != currentDragView)
	{
		if (currentDragView)
			currentDragView->onDragLeave (drag, where2);
		currentDragView = view;
	}
	if (currentDragView)
	{
		result = currentDragView->onDrop (drag, where2);
		currentDragView->onDragLeave (drag, where2);
	}
	currentDragView = 0;
	
	return result;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragEnter (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return;
	
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	if (currentDragView)
		currentDragView->onDragLeave (drag, where2);
	CView* view = getViewAt (where);
	currentDragView = view;
	if (view)
		view->onDragEnter (drag, where2);
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return;
	
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	if (currentDragView)
		currentDragView->onDragLeave (drag, where2);
	currentDragView = 0;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragMove (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return;
	
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	CView* view = getViewAt (where);
	if (view != currentDragView)
	{
		if (currentDragView)
			currentDragView->onDragLeave (drag, where2);
		if (view)
			view->onDragEnter (drag, where2);
		currentDragView = view;
	}
	else if (currentDragView)
		currentDragView->onDragMove (drag, where2);
}

//-----------------------------------------------------------------------------
void CViewContainer::looseFocus ()
{
	FOREACHSUBVIEW
		pV->looseFocus ();
	ENDFOREACHSUBVIEW
}

//-----------------------------------------------------------------------------
void CViewContainer::takeFocus ()
{
	FOREACHSUBVIEW
		pV->takeFocus ();
	ENDFOREACHSUBVIEW
}

//-----------------------------------------------------------------------------
/**
 * @param oldFocus old focus view
 * @param reverse search order
 * @return true on success
 */
bool CViewContainer::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	bool foundOld = false;
	FOREACHSUBVIEW_REVERSE(reverse)
		if (oldFocus && !foundOld)
		{
			if (oldFocus == pV)
			{
				foundOld = true;
				continue;
			}
		}
		else
		{
			if (pV->wantsFocus ())
			{
				getFrame ()->setFocusView (pV);
				return true;
			}
			else if (pV->isTypeOf ("CViewContainer"))
			{
				if (((CViewContainer*)pV)->advanceNextFocusView (0, reverse))
					return true;
			}
		}
	ENDFOREACHSUBVIEW
	return false;
}

//-----------------------------------------------------------------------------
bool CViewContainer::isDirty () const
{
	if (bDirty)
		return true;
		
	CRect viewSize (size);
	viewSize.offset (-size.left, -size.top);

	FOREACHSUBVIEW
		if (pV->isDirty ())
		{
			CRect r = pV->getViewSize (r);
			r.bound (viewSize);
			if (r.getWidth () > 0 && r.getHeight () > 0)
				return true;
		}
	ENDFOREACHSUBVIEW
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param p location
 * @param deep search deep
 * @return view at position p
 */
CView* CViewContainer::getViewAt (const CPoint& p, bool deep) const
{
	if (!pParentFrame || !bVisible)
		return 0;

	CPoint where (p);

	// convert to relativ pos
	where.offset (-size.left, -size.top);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && where.isInside (pV->getMouseableArea ()))
		{
			if (deep)
			{
				if (pV->isTypeOf ("CViewContainer"))
					return ((CViewContainer*)pV)->getViewAt (where, deep);
			}
			return pV;
		}
	ENDFOREACHSUBVIEW

	return 0;
}

//-----------------------------------------------------------------------------
/**
 * @param p location
 * @param deep search deep
 * @return view container at position p
 */
CViewContainer* CViewContainer::getContainerAt (const CPoint& p, bool deep) const
{
	if (!pParentFrame)
		return 0;

	CPoint where (p);

	// convert to relativ pos
	where.offset (-size.left, -size.top);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && where.isInside (pV->getMouseableArea ()))
		{
			if (deep && pV->isTypeOf ("CViewContainer"))
				return ((CViewContainer*)pV)->getContainerAt (where, deep);
			break;
		}
	ENDFOREACHSUBVIEW

	return const_cast<CViewContainer*>(this);
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::frameToLocal (CPoint& point) const
{
	point.offset (-size.left, -size.top);
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::localToFrame (CPoint& point) const
{
	point.offset (size.left, size.top);
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
bool CViewContainer::removed (CView* parent)
{
	 if (pOffscreenContext)
		pOffscreenContext->forget ();
	pOffscreenContext = 0;

	FOREACHSUBVIEW
		pV->removed (this);
	ENDFOREACHSUBVIEW
	
	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
bool CViewContainer::attached (CView* parent)
{
	pParentFrame = parent->getFrame ();

	// create offscreen bitmap
	if (!pOffscreenContext && bDrawInOffscreen)
	{
		pOffscreenContext = COffscreenContext::create (getFrame (), size.getWidth (), size.getHeight ());
		if (pOffscreenContext)
		{
			pOffscreenContext->setFillColor (kBlackCColor);
			pOffscreenContext->drawRect (CRect (0, 0, size.getWidth (), size.getHeight ()), kDrawFilled);
		}
	}

	FOREACHSUBVIEW
		pV->attached (this);
	ENDFOREACHSUBVIEW

	return CView::attached (parent);
}

//-----------------------------------------------------------------------------
void CViewContainer::useOffscreen (bool b)
{
	bDrawInOffscreen = b;
	
	if (!bDrawInOffscreen && pOffscreenContext)
	{
		pOffscreenContext->forget ();
		pOffscreenContext = 0;
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::modifyDrawContext (CCoord save[4], CDrawContext* pContext)
{
	// store
	CPoint offset = pContext->getOffset ();
	save[0] = offset.x;
	save[1] = offset.y;
	offset.x += size.left;
	offset.y += size.top;
	pContext->setOffset (offset);
}

//-----------------------------------------------------------------------------
void CViewContainer::restoreDrawContext (CDrawContext* pContext, CCoord save[4])
{
	// restore
	CPoint offset (save[0], save[1]);
	pContext->setOffset (offset);
}

#if DEBUG
static long _debugDumpLevel = 0;
//-----------------------------------------------------------------------------
void CViewContainer::dumpInfo ()
{
	DebugPrint ("CViewContainer: Offscreen:%s ", bDrawInOffscreen ? "Yes" : "No");
	CView::dumpInfo ();
}

//-----------------------------------------------------------------------------
void CViewContainer::dumpHierarchy ()
{
	_debugDumpLevel++;
	FOREACHSUBVIEW
		for (long i = 0; i < _debugDumpLevel; i++)
			DebugPrint ("\t");
		pV->dumpInfo ();
		DebugPrint ("\n");
		if (pV->isTypeOf ("CViewContainer"))
			((CViewContainer*)pV)->dumpHierarchy ();
	ENDFOREACHSUBVIEW
	_debugDumpLevel--;
}

#endif

} // namespace
