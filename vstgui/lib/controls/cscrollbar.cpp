// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
: CControl (size, listener, tag, nullptr)
, direction (direction)
, scrollSize (scrollSize)
, scrollerArea (size)
, stepValue (0.1f)
, scrollerLength (0)
, overlayStyle (false)
, mouseIsInside (false)
, drawer (nullptr)
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
{
	calculateScrollerLength ();
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
	CRect sr (scrollerArea);
	CCoord l = (direction == kHorizontal) ? scrollerArea.getWidth () : scrollerArea.getHeight ();
	CCoord scrollerOffset = (CCoord) (value * (l - scrollerLength));
	if (direction == kHorizontal)
	{
		sr.setWidth (scrollerLength);
		sr.offset (scrollerOffset, 0);
	}
	else
	{
		sr.setHeight (scrollerLength);
		sr.offset (0, scrollerOffset);
	}
	return sr;
}

//-----------------------------------------------------------------------------
void CScrollbar::doStepping ()
{
	CRect sr = getScrollerRect ();
	if (timer)
	{
		if (!getViewSize ().pointInside (startPoint) || sr.pointInside (startPoint))
			return;
	}
	bool dir = (direction == kHorizontal && startPoint.x < sr.left) || (direction == kVertical && startPoint.y < sr.top);
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
	if (overlayStyle && scrollerLength != 0)
	{
		addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::LinearTimingFunction (100));
	}
	mouseIsInside = true;
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	if (overlayStyle && scrollerLength != 0)
	{
		Animation::ITimingFunction* timingFunction = nullptr;
		if (getAlphaValue () == 1.f)
		{
			auto* interpolTimingFunction = new Animation::InterpolationTimingFunction (400);
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
		timer = makeOwned<CVSTGUITimer> (this, 250, true);
		return kMouseEventHandled;
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	timer = nullptr;
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
CMouseEventResult CScrollbar::onMouseCancel ()
{
	timer = nullptr;
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void CScrollbar::onVisualChange ()
{
	if (isAttached () && overlayStyle && !mouseIsInside)
	{
		if (scrollerLength != 0)
		{
			auto timingFunction = new Animation::InterpolationTimingFunction (1100);
			timingFunction->addPoint (1000.f/1100.f, 0);
			addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (0.001f), timingFunction);
			setAlphaValue (1.f);
		}
		else
		{
			removeAnimation ("AlphaValueAnimation");
			setAlphaValue (0.f);
		}
	}
}

//------------------------------------------------------------------------
bool CScrollbar::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &_distance, const CButtonState &buttons)
{
	if (scrollerLength == 0 || !getMouseEnabled ())
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
		value -= 0.1f * distance * getWheelInc ();
	else
		value -= distance * getWheelInc ();
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
		auto path = (wideness > 2) ? owned (pContext->createGraphicsPath ()) : nullptr;
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
		CRect sr = getScrollerRect ();
		drawScroller (pContext, sr);
	}
	setDirty (false);
}

}
