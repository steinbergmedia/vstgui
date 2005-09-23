//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// CScrollView written 2004 by Arne Scheffler
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cscrollview__
#include "cscrollview.h"
#endif

#include "cvstguitimer.h"

#define FOREACHSUBVIEW for (CCView *pSv = pFirstView; pSv; pSv = pSv->pNext) {CView *pV = pSv->pView;
#define ENDFOR }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CScrollContainer::CScrollContainer (const CRect &size, const CRect &containerSize, CFrame *pParent, CBitmap *pBackground)
: CViewContainer (size, pParent, pBackground)
, containerSize (containerSize)
, offset (CPoint (0, 0))
{
	setTransparency (true);
	setMode (kOnlyDirtyUpdate);
}

//-----------------------------------------------------------------------------
CScrollContainer::~CScrollContainer ()
{
}


//-----------------------------------------------------------------------------
void CScrollContainer::setContainerSize (const CRect& cs)
{
	containerSize = cs;
	setScrollOffset (offset, false);
}

//-----------------------------------------------------------------------------
void CScrollContainer::setScrollOffset (CPoint newOffset, bool redraw)
{
	if (newOffset.x < containerSize.left - containerSize.width ())
		newOffset.x = containerSize.left - containerSize.width ();
	if (newOffset.x > containerSize.right)
		newOffset.x = containerSize.right;
	if (newOffset.y < containerSize.top)
		newOffset.y = containerSize.top;
	if (newOffset.y > containerSize.bottom)
		newOffset.y = containerSize.bottom;
	CPoint diff (newOffset.x - offset.x, offset.y - newOffset.y);
	if (diff.x == 0 && diff.y == 0)
		return;
	CCView *pV = pFirstView;
	while (pV)
	{
		CRect r;
		pV->pView->getViewSize (r);
		r.offset (diff.x , diff.y);
		pV->pView->setViewSize (r);
		pV->pView->getMouseableArea (r);
		r.offset (diff.x , diff.y);
		pV->pView->setMouseableArea (r);

		pV = pV->pNext;
	}
	offset = newOffset;
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	invalid ();
	#else
	setDirty (true);
	#endif
}

#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
void CScrollContainer::redrawRect (CDrawContext* context, const CRect& rect)
{
	CRect _rect (rect);
	_rect.offset (size.left, size.top);
	_rect.bound (size);
	if (bTransparencyEnabled)
	{
		// as this is transparent, we call the parentview to redraw this area.
		if (pParentView)
			pParentView->redrawRect (context, _rect);
		else if (pParentFrame)
			pParentFrame->drawRect (context, _rect);
	}
	else
		drawRect (context, _rect);
}
#endif

//-----------------------------------------------------------------------------
bool CScrollContainer::isDirty () const
{
	if (bDirty)
		return true;
		
	FOREACHSUBVIEW
		if (pV->isDirty ())
		{
			CRect vs = pV->getViewSize (vs);
			vs.offset (size.left, size.top);
			if (size.rectOverlap (vs))
				return true;
		}
	ENDFOR
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CRect &size, const CRect &containerSize, CFrame* pParent, long style, long scrollbarWidth, CBitmap* pBackground)
: CViewContainer (size, pParent, pBackground)
, sc (0)
, vsb (0)
, hsb (0)
, containerSize (containerSize)
, style (style)
{
	setTransparency (true);
	setMode (kOnlyDirtyUpdate);

	CRect scsize (size);
	scsize.offset (-scsize.left, -scsize.top);
	if (style & kHorizontalScrollbar)
	{
		CRect sbr (size);
		sbr.offset (-size.left, -size.top);
		sbr.top = sbr.bottom - scrollbarWidth;
		if (style & kVerticalScrollbar)
		{
			sbr.right -= (scrollbarWidth - 1);
		}
		hsb = new CScrollbar (sbr, this, kHSBTag, CScrollbar::kHorizontal, containerSize);
		CViewContainer::addView (hsb);
		scsize.bottom = sbr.top;
	}
	if (style & kVerticalScrollbar)
	{
		CRect sbr (size);
		sbr.offset (-size.left, -size.top);
		sbr.left = sbr.right - scrollbarWidth;
		if (style & kHorizontalScrollbar)
		{
			sbr.bottom -= (scrollbarWidth - 1);
		}
		vsb = new CScrollbar (sbr, this, kVSBTag, CScrollbar::kVertical, containerSize);
		CViewContainer::addView (vsb);
		scsize.right = sbr.left;
	}

	sc = new CScrollContainer (scsize, this->containerSize, pParent);
	CViewContainer::addView (sc);
}

//-----------------------------------------------------------------------------
CScrollView::~CScrollView ()
{
}

//-----------------------------------------------------------------------------
void CScrollView::setContainerSize (const CRect& cs)
{
	containerSize = cs;
	if (sc)
	{
		sc->setContainerSize (cs);
	}
	if (vsb)
	{
		vsb->setScrollSize (cs);
		valueChanged (vsb);
	}
	if (hsb)
	{
		hsb->setScrollSize (cs);
		valueChanged (hsb);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::addView (CView *pView)
{
	sc->addView (pView);
}

//-----------------------------------------------------------------------------
void CScrollView::valueChanged (CControl *pControl)
{
	if (sc)
	{
		float value = pControl->getValue ();
		long tag = pControl->getTag ();
		CPoint offset;
		CRect vsize = sc->getViewSize (vsize);
		CRect csize = sc->getContainerSize ();
		sc->getScrollOffset (offset);
			
		switch (tag)
		{
			case kHSBTag:
			{
				offset.x = csize.left - (csize.width () - vsize.width ()) * value;
				sc->setScrollOffset (offset, false);
				break;
			}
			case kVSBTag:
			{
				offset.y = csize.top + (csize.height () - vsize.height ()) * value;
				sc->setScrollOffset (offset, false);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CScrollView::drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect)
{
	CRect r (size);
	r.offset (-r.left, -r.top);
	pContext->setFrameColor (kBlackCColor);
	pContext->setLineWidth (1);
	pContext->drawRect (r);
	CViewContainer::drawBackgroundRect (pContext, _updateRect);
}

//-----------------------------------------------------------------------------
bool CScrollView::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	bool result = CViewContainer::onWheel (where, axis, distance, buttons);
	if (!result)
	{
		if (vsb && axis == kMouseWheelAxisY)
			result = vsb->onWheel (where, distance, buttons);
		else if (hsb && axis == kMouseWheelAxisX)
			result = hsb->onWheel (where, distance, buttons);
	}
	return result;
}

//-----------------------------------------------------------------------------
CScrollbar::CScrollbar (const CRect& size, CControlListener* listener, long tag, long style, const CRect& scrollSize)
: CControl (size, listener, tag, 0)
, style (style)
, scrollSize (scrollSize)
, scrollerArea (size)
, stepValue (0.1f)
, scrollerLength (0)
, drawer (0)
, timer (0)
{
	setWheelInc (0.05f);
	scrollerArea.inset (2, 2);
	calculateScrollerLength ();
	frameColor (0, 0, 0, 255);
	scrollerColor (0, 0, 255, 255);
	backgroundColor (255, 255, 255, 200);
}

//-----------------------------------------------------------------------------
CScrollbar::~CScrollbar ()
{
}

//-----------------------------------------------------------------------------
void CScrollbar::setScrollSize (const CRect& ssize)
{
	scrollSize = ssize;
	calculateScrollerLength ();
	setDirty (true);
}

//-----------------------------------------------------------------------------
void CScrollbar::calculateScrollerLength ()
{
	CCoord newScrollerLength = scrollerLength;
	if (style == kHorizontal)
	{
		float factor = (float)size.width () / (float)scrollSize.width ();
		if (factor >= 1.f)
			factor = 0;
		newScrollerLength = size.width () * factor;
	}
	else
	{
		float factor = (float)size.height () / (float)scrollSize.height ();
		if (factor >= 1.f)
			factor = 0;
		newScrollerLength = size.height () * factor;
	}
	if (newScrollerLength != scrollerLength)
	{
		scrollerLength = newScrollerLength;
		setDirty (true);
	}
}

//-----------------------------------------------------------------------------
CRect CScrollbar::getScrollerRect ()
{
	CRect scrollerRect (scrollerArea);
	CCoord l = (style == kHorizontal) ? scrollerArea.width () : scrollerArea.height ();
	CCoord scrollerOffset = value * (l - scrollerLength);
	if (style == kHorizontal)
	{
		scrollerRect.setWidth (scrollerLength);
		scrollerRect.offset (scrollerOffset, 0);
	}
	else
	{
		scrollerRect.setHeight (scrollerLength);
		scrollerRect.offset (0, scrollerOffset);
	}
	return scrollerRect;
}

//-----------------------------------------------------------------------------
void CScrollbar::doStepping ()
{
	CRect scrollerRect = getScrollerRect ();
	if (timer)
	{
		if (!startPoint.isInside (size) || startPoint.isInside (scrollerRect))
			return;
	}
	bool direction = (style == kHorizontal && startPoint.x < scrollerRect.left) || (style == kVertical && startPoint.y < scrollerRect.top);
	float newValue = value;
	if (style == kHorizontal)
	{
		if (direction)
			newValue = value - (float)scrollerLength / (float)scrollerArea.width ();
		else
			newValue = value + (float)scrollerLength / (float)scrollerArea.width ();
	}
	else
	{
		if (direction)
			newValue = value - (float)scrollerLength / (float)scrollerArea.height ();
		else
			newValue = value + (float)scrollerLength / (float)scrollerArea.height ();
	}
	if (newValue < 0.f) newValue = 0.f;
	if (newValue > 1.f) newValue = 1.f;
	if (newValue != value)
	{
		value = newValue;
		if (listener)
			listener->valueChanged (this);
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		invalid ();
		#endif
	}
}

//-----------------------------------------------------------------------------
long CScrollbar::notify (CView* sender, const char* message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		doStepping ();
		timer->setFireTime (80);
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseDown (CPoint &where, const long& buttons)
{
	if (buttons != kLButton || scrollerLength == 0) 
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;

	startPoint = where;
	scrollerRect = getScrollerRect ();
	scrolling = where.isInside (scrollerRect);
	if (scrolling)
	{
		scrollerRect = getScrollerRect ();
		return kMouseEventHandled;
	}
	else if (where.isInside (scrollerArea))
	{
		doStepping ();
		timer = new CVSTGUITimer (this, 250);
		timer->start ();
		return kMouseEventHandled;
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseUp (CPoint &where, const long& buttons)
{
	if (timer)
	{
		delete timer;
		timer = 0;
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseMoved (CPoint &where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (scrolling)
		{
			float newValue = 0.f;
			CPoint newPoint (where);
			newPoint.x -= startPoint.x - scrollerRect.left;
			newPoint.y -= startPoint.y - scrollerRect.top;
			if (style == kHorizontal)
			{
				newValue = (float)(newPoint.x - scrollerArea.left) / ((float)scrollerArea.width () - scrollerRect.width ());
			}
			else
			{
				newValue = (float)(newPoint.y - scrollerArea.top) / ((float)scrollerArea.height () - scrollerRect.height ());
			}
			if (newValue < 0.f) newValue = 0.f;
			if (newValue > 1.f) newValue = 1.f;
			if (newValue != value)
			{
				value = newValue;
				if (listener)
					listener->valueChanged (this);
				#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
				invalid ();
				#endif
			}
		}
		else
		{
			CPoint old (startPoint);
			startPoint = where;
			CRect scollerRect = getScrollerRect ();
			if (where.isInside (size) && old.isInside (scollerRect) && !startPoint.isInside (scrollerRect))
				doStepping ();
		}
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
void CScrollbar::mouse (CDrawContext* pContext, CPoint& where, long buttons)
{
	if (buttons == -1) buttons = pContext->getMouseButtons ();
	if (buttons != kLButton || scrollerLength == 0) return;

	float newValue = 0.f;

	// scrolling
	CRect scrollerRect = getScrollerRect ();
	if (where.isInside (scrollerRect))
	{
		CPoint newPoint (where);
		while (pContext->waitDrag ())
		{
			getMouseLocation (pContext, newPoint);
			newPoint.x -= where.x - scrollerRect.left;
			newPoint.y -= where.y - scrollerRect.top;
			if (style == kHorizontal)
			{
				newValue = (float)(newPoint.x - scrollerArea.left) / ((float)scrollerArea.width () - scrollerRect.width ());
			}
			else
			{
				newValue = (float)(newPoint.y - scrollerArea.top) / ((float)scrollerArea.height () - scrollerRect.height ());
			}
			if (newValue < 0.f) newValue = 0.f;
			if (newValue > 1.f) newValue = 1.f;
			if (newValue != value)
			{
				value = newValue;
				if (listener)
					listener->valueChanged (this);
			}
			doIdleStuff ();
		}
	}
	// stepping
	else if (where.isInside (scrollerArea))
	{
		long kWaitTime = 100;
		long nextUpdateTime = getFrame ()->getTicks ();
		do
		{
			long ticks = getFrame ()->getTicks ();
			if (nextUpdateTime - ticks < 0)
			{
				if (style == kHorizontal)
				{
					if (where.x < scrollerRect.left)
						newValue = value - (float)scrollerLength / (float)scrollerArea.width ();
					else
						newValue = value + (float)scrollerLength / (float)scrollerArea.width ();
				}
				else
				{
					if (where.y < scrollerRect.top)
						newValue = value - (float)scrollerLength / (float)scrollerArea.height ();
					else
						newValue = value + (float)scrollerLength / (float)scrollerArea.height ();
				}
				if (newValue < 0.f) newValue = 0.f;
				if (newValue > 1.f) newValue = 1.f;
				if (newValue != value)
				{
					value = newValue;
					if (listener)
						listener->valueChanged (this);
				}
				scrollerRect = getScrollerRect ();
				if (where.isInside (scrollerRect))
					break;
				nextUpdateTime = ticks + kWaitTime;
			}
			doIdleStuff ();
		} while (pContext->getMouseButtons () == kLButton);
	}
}

//------------------------------------------------------------------------
bool CScrollbar::onWheel (const CPoint &where, const float &_distance, const long &buttons)
{
	if (!bMouseEnabled)
		return false;

	float distance = _distance;
	if (style == kHorizontal)
		distance *= -1;

	if (buttons & kShift)
		value -= 0.1f * distance * wheelInc;
	else
		value -= distance * wheelInc;
	bounceValue ();

	if (isDirty () && listener)
		listener->valueChanged (this);
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	invalid ();
	#endif
	return true;
}

//-----------------------------------------------------------------------------
void CScrollbar::drawBackground (CDrawContext* pContext)
{
	CRect r (size);
	if (drawer)
		drawer->drawScrollbarBackground (pContext, r, style, this);
	else
	{
		pContext->setLineWidth (1);
		pContext->setFillColor (backgroundColor);
		pContext->setFrameColor (frameColor);
		pContext->drawRect (r, kDrawFilledAndStroked);
	}
}

//-----------------------------------------------------------------------------
void CScrollbar::drawScroller (CDrawContext* pContext, const CRect& size)
{
	CRect r (size);
	if (drawer)
		drawer->drawScrollbarScroller (pContext, r, style, this);
	else
	{
		pContext->setLineWidth (1);
		pContext->setFillColor (scrollerColor);
		pContext->setFrameColor (frameColor);
		pContext->drawRect (r, kDrawFilledAndStroked);
	}
}

//-----------------------------------------------------------------------------
void CScrollbar::draw (CDrawContext* pContext)
{
	drawBackground (pContext);
	if (scrollerLength > 0)
	{
		CRect scrollerRect = getScrollerRect ();
		drawScroller (pContext, scrollerRect);
	}
	setDirty (false);
}

