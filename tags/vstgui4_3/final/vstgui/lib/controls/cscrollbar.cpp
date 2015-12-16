//
//  cscrollbar.cpp
//  vstgui
//
//  Created by Arne Scheffler on 20/08/14.
//
//

#include "cscrollbar.h"
#include "../cvstguitimer.h"
#include "../animation/animations.h"
#include "../animation/timingfunctions.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../cdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CScrollbar::CScrollbar (const CRect& size, IControlListener* listener, int32_t tag, ScrollbarDirection direction, const CRect& scrollSize)
: CControl (size, listener, tag, 0)
, direction (direction)
, scrollSize (scrollSize)
, scrollerArea (size)
, stepValue (0.1f)
, scrollerLength (0)
, overlayStyle (false)
, mouseIsInside (false)
, drawer (0)
, timer (0)
{
	setTransparency (true);
	setWheelInc (0.05f);
	scrollerArea.inset (2, 2);
	calculateScrollerLength ();
	frameColor (0, 0, 0, 255);
	scrollerColor (0, 0, 255, 255);
	backgroundColor (255, 255, 255, 200);
}

//-----------------------------------------------------------------------------
CScrollbar::CScrollbar (const CScrollbar& v)
: CControl (v)
, direction (v.direction)
, scrollSize (v.scrollSize)
, scrollerArea (v.scrollerArea)
, stepValue (v.stepValue)
, scrollerLength (v.scrollerLength)
, frameColor (v.frameColor)
, scrollerColor (v.scrollerColor)
, backgroundColor (v.backgroundColor)
, overlayStyle (v.overlayStyle)
, mouseIsInside (false)
, drawer (v.drawer)
, timer (0)
{
	calculateScrollerLength ();
}

//-----------------------------------------------------------------------------
CScrollbar::~CScrollbar ()
{
}

//-----------------------------------------------------------------------------
void CScrollbar::setViewSize (const CRect &newSize, bool invalid)
{
	scrollerArea = newSize;
	scrollerArea.inset (2, 2);
	CControl::setViewSize (newSize, invalid);
	calculateScrollerLength ();
}

//-----------------------------------------------------------------------------
void CScrollbar::setScrollSize (const CRect& ssize)
{
	if (scrollSize != ssize)
	{
		scrollSize = ssize;
		calculateScrollerLength ();
		setDirty (true);
	}
}

//-----------------------------------------------------------------------------
void CScrollbar::calculateScrollerLength ()
{
	CCoord newScrollerLength;
	if (direction == kHorizontal)
	{
		double factor = scrollSize.getWidth () > 0. ? getViewSize ().getWidth () / scrollSize.getWidth () : 0.;
		if (factor >= 1.f)
			factor = 0;
		newScrollerLength = (CCoord) (getViewSize ().getWidth () * factor);
	}
	else
	{
		double factor = scrollSize.getHeight () > 0 ? getViewSize ().getHeight () / scrollSize.getHeight () : 0.;
		if (factor >= 1.f)
			factor = 0;
		newScrollerLength = (CCoord) (getViewSize ().getHeight () * factor);
	}
	if (newScrollerLength < 8. && newScrollerLength > 0.)
		newScrollerLength = 8.;
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
	CCoord l = (direction == kHorizontal) ? scrollerArea.getWidth () : scrollerArea.getHeight ();
	CCoord scrollerOffset = (CCoord) (value * (l - scrollerLength));
	if (direction == kHorizontal)
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
		if (!getViewSize ().pointInside (startPoint) || scrollerRect.pointInside (startPoint))
			return;
	}
	bool dir = (direction == kHorizontal && startPoint.x < scrollerRect.left) || (direction == kVertical && startPoint.y < scrollerRect.top);
	float newValue;
	if (direction == kHorizontal)
	{
		if (dir)
			newValue = value - (float)scrollerLength / (float)scrollerArea.getWidth ();
		else
			newValue = value + (float)scrollerLength / (float)scrollerArea.getWidth ();
	}
	else
	{
		if (dir)
			newValue = value - (float)scrollerLength / (float)scrollerArea.getHeight ();
		else
			newValue = value + (float)scrollerLength / (float)scrollerArea.getHeight ();
	}
	if (newValue < 0.f) newValue = 0.f;
	if (newValue > 1.f) newValue = 1.f;
	if (newValue != value)
	{
		value = newValue;
		valueChanged ();
		invalid ();
	}
}

//-----------------------------------------------------------------------------
CMessageResult CScrollbar::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer && timer)
	{
		doStepping ();
		timer->setFireTime (80);
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
void CScrollbar::setOverlayStyle (bool state)
{
	if (overlayStyle != state)
	{
		overlayStyle = state;
		setAlphaValue (overlayStyle ? 0.001f : 1.f);
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseEntered (CPoint& where, const CButtonState& buttons)
{
	if (overlayStyle)
	{
		addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::LinearTimingFunction (100));
	}
	mouseIsInside = true;
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	if (overlayStyle)
	{
		Animation::ITimingFunction* timingFunction = 0;
		if (getAlphaValue () == 1.f)
		{
			Animation::InterpolationTimingFunction* interpolTimingFunction = new Animation::InterpolationTimingFunction (400);
			interpolTimingFunction->addPoint (300.f/400.f, 1.f);
			timingFunction = interpolTimingFunction;
		}
		else
			timingFunction = new Animation::LinearTimingFunction (100);
		addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (0.001f), timingFunction);
	}
	mouseIsInside = false;
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	if (buttons != kLButton || scrollerLength == 0)
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;

	startPoint = where;
	scrollerRect = getScrollerRect ();
	scrolling = scrollerRect.pointInside (where);
	if (scrolling)
	{
		scrollerRect = getScrollerRect ();
		return kMouseEventHandled;
	}
	else if (scrollerArea.pointInside (where))
	{
		doStepping ();
		timer = new CVSTGUITimer (this, 250);
		timer->start ();
		return kMouseEventHandled;
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	if (timer)
	{
		timer->forget ();
		timer = 0;
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		if (scrolling)
		{
			float newValue = 0.f;
			CPoint newPoint (where);
			newPoint.x -= startPoint.x - scrollerRect.left;
			newPoint.y -= startPoint.y - scrollerRect.top;
			if (direction == kHorizontal)
			{
				newValue = (float)((float)(newPoint.x - scrollerArea.left) / ((float)scrollerArea.getWidth () - scrollerRect.getWidth ()));
			}
			else
			{
				newValue = (float)((float)(newPoint.y - scrollerArea.top) / ((float)scrollerArea.getHeight () - scrollerRect.getHeight ()));
			}
			if (newValue < 0.f) newValue = 0.f;
			if (newValue > 1.f) newValue = 1.f;
			if (newValue != value)
			{
				value = newValue;
				valueChanged ();
				invalid ();
			}
		}
		else
		{
			CPoint old (startPoint);
			startPoint = where;
			CRect scollerRect = getScrollerRect ();
			if (getViewSize ().pointInside (where) && scollerRect.pointInside (old) && !scrollerRect.pointInside (startPoint))
				doStepping ();
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CScrollbar::onVisualChange ()
{
	if (overlayStyle && !mouseIsInside)
	{
		Animation::InterpolationTimingFunction* timingFunction = new Animation::InterpolationTimingFunction (1100);
		timingFunction->addPoint (1000.f/1100.f, 0);
		addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (0.001f), timingFunction);
		setAlphaValue (1.f);
	}
}

//------------------------------------------------------------------------
bool CScrollbar::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &_distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	if (buttons != 0 && !(buttons & (kShift|kMouseWheelInverted)))
		return false;

	if (direction == kHorizontal && axis == kMouseWheelAxisY)
		return false;

	if (direction == kVertical && axis == kMouseWheelAxisX)
		return false;

	float distance = _distance;
	if (direction == kHorizontal && axis == kMouseWheelAxisY)
		distance *= -1;
	if (buttons & kMouseWheelInverted)
		distance *= -1;

	if (buttons & kShift)
		value -= 0.1f * distance * wheelInc;
	else
		value -= distance * wheelInc;
	bounceValue ();

	if (isDirty ())
	{
		onVisualChange ();
		valueChanged ();
		invalid ();
	}
	return true;
}

//-----------------------------------------------------------------------------
void CScrollbar::drawBackground (CDrawContext* pContext)
{
	CRect r (getViewSize ());
	if (drawer)
		drawer->drawScrollbarBackground (pContext, r, direction, this);
	else
	{
		pContext->setDrawMode (kAliasing);
		pContext->setLineWidth (1);
		pContext->setFillColor (backgroundColor);
		pContext->setFrameColor (frameColor);
		pContext->setLineStyle (kLineSolid);
		pContext->drawRect (r, kDrawFilledAndStroked);
	}
}

//-----------------------------------------------------------------------------
void CScrollbar::drawScroller (CDrawContext* pContext, const CRect& size)
{
	CRect r (size);
	if (drawer)
		drawer->drawScrollbarScroller (pContext, r, direction, this);
	else
	{
		pContext->setLineWidth (1);
		pContext->setFillColor (scrollerColor);
		pContext->setFrameColor (frameColor);

		CCoord wideness = (direction == kVertical ? getWidth() : getHeight()) / 2 - 2;
		OwningPointer<CGraphicsPath> path = wideness > 2 ? pContext->createGraphicsPath () : 0;
		if (path)
		{
			if (wideness > 4)
				wideness = 4;
			pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
			path->addRoundRect (r, wideness);
			pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
			pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
		else
		{
			pContext->setDrawMode (kAliasing|kNonIntegralMode);
			pContext->drawRect (r, kDrawFilledAndStroked);
		}
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

}