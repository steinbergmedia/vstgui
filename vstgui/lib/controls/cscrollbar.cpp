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
#include "../events.h"
#include "../algorithm.h"
#include "../cbitmap.h"

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

//------------------------------------------------------------------------
void CScrollbar::setDrawer (const SPtr<IScrollbarDrawer>& d) { drawer = d; }

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
		invalid ();
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
	if (newScrollerLength < minScrollerLength && newScrollerLength > 0.)
		newScrollerLength = minScrollerLength;
	if (newScrollerLength != scrollerLength)
	{
		scrollerLength = newScrollerLength;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
CRect CScrollbar::getScrollerRect ()
{
	CRect sr (scrollerArea);
	CCoord l = (direction == kHorizontal) ? scrollerArea.getWidth () : scrollerArea.getHeight ();
	CCoord scrollerOffset = (CCoord)(getValueNormalized () * (l - scrollerLength));
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
	float newValue = getValueNormalized ();
	if (direction == kHorizontal)
	{
		if (dir)
			newValue -= (float)scrollerLength / (float)scrollerArea.getWidth ();
		else
			newValue += (float)scrollerLength / (float)scrollerArea.getWidth ();
	}
	else
	{
		if (dir)
			newValue -= (float)scrollerLength / (float)scrollerArea.getHeight ();
		else
			newValue += (float)scrollerLength / (float)scrollerArea.getHeight ();
	}
	newValue = clampNorm (newValue);
	if (newValue != getValueNormalized ())
	{
		setValueNormalized (newValue);
		valueChanged ();
		invalid ();
	}
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

//------------------------------------------------------------------------
void CScrollbar::setMinScrollerLength (CCoord length)
{
	if (minScrollerLength != length)
	{
		minScrollerLength = length;
		calculateScrollerLength ();
		invalid ();
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseEntered (CPoint& where, const CButtonState& buttons)
{
	if (overlayStyle && scrollerLength != 0)
	{
		addAnimation ("AlphaValueAnimation", makeShared<Animation::AlphaValueAnimation> (1.f),
					  makeShared<Animation::LinearTimingFunction> (100));
	}
	mouseIsInside = true;
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CScrollbar::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	if (overlayStyle && scrollerLength != 0)
	{
		SPtr<Animation::ITimingFunction> timingFunction;
		if (getAlphaValue () == 1.f)
		{
			auto interpolTimingFunction = makeShared<Animation::InterpolationTimingFunction> (400);
			interpolTimingFunction->addPoint (300.f/400.f, 1.f);
			timingFunction = interpolTimingFunction;
		}
		else
			timingFunction = makeShared<Animation::LinearTimingFunction> (100);
		addAnimation ("AlphaValueAnimation", makeShared<Animation::AlphaValueAnimation> (0.001f),
					  timingFunction);
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
		timer = makeShared<CVSTGUITimer> (
			[this] (auto&&) {
				doStepping ();
				timer->setFireTime (80);
			},
			250);
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
			newValue = clampNorm (newValue);
			if (newValue != getValueNormalized ())
			{
				setValueNormalized (newValue);
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
			auto timingFunction = makeShared<Animation::InterpolationTimingFunction> (1100);
			timingFunction->addPoint (1000.f/1100.f, 0);
			addAnimation ("AlphaValueAnimation",
						  makeShared<Animation::AlphaValueAnimation> (0.001f), timingFunction);
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
void CScrollbar::onMouseWheelEvent (MouseWheelEvent& event)
{
	if (scrollerLength == 0 || !getMouseEnabled ())
		return;

	if (!event.modifiers.empty () && !(event.modifiers.has (ModifierKey::Shift) &&
	                                   event.flags & MouseWheelEvent::DirectionInvertedFromDevice))
		return;

	float distance = 0.f;
	if (direction == kHorizontal)
		distance = static_cast<float> (event.deltaX);
	else
		distance = static_cast<float> (event.deltaY);

	if (distance == 0.f)
		return;

	if (event.flags & MouseWheelEvent::DirectionInvertedFromDevice)
		distance *= -1;

	float newValue = getValueNormalized ();
	if (event.modifiers.has (ModifierKey::Shift))
		newValue -= 0.1f * distance * getWheelInc ();
	else
		newValue -= distance * getWheelInc ();
	newValue = clampNorm (newValue);

	if (newValue != getValueNormalized ())
	{
		setValueNormalized (newValue);
		onVisualChange ();
		valueChanged ();
		invalid ();
	}
	event.consumed = true;
}

//-----------------------------------------------------------------------------
void CScrollbar::drawBackground (CDrawContext& context)
{
	CRect r (getViewSize ());
	if (drawer)
		drawer->drawScrollbarBackground (context, r, direction, *this);
	else
	{
		context.setDrawMode (kAliasing);
		context.setLineWidth (1);
		context.setFillColor (backgroundColor);
		context.setFrameColor (frameColor);
		context.setLineStyle (kLineSolid);
		context.drawRect (r, kDrawFilledAndStroked);
	}
}

//-----------------------------------------------------------------------------
void CScrollbar::drawScroller (CDrawContext& context, const CRect& size)
{
	CRect r (size);
	if (drawer)
		drawer->drawScrollbarScroller (context, r, direction, *this);
	else
	{
		context.setLineWidth (1);
		context.setFillColor (scrollerColor);
		context.setFrameColor (frameColor);

		CCoord wideness = (direction == kVertical ? getWidth() : getHeight()) / 2 - 2;
		SPtr<CGraphicsPath> path = (wideness > 2) ? context.createGraphicsPath () : nullptr;
		if (path)
		{
			if (wideness > 4)
				wideness = 4;
			context.setDrawMode (kAntiAliasing | kNonIntegralMode);
			path->addRoundRect (r, wideness);
			context.drawGraphicsPath (path, CDrawContext::kPathFilled);
			context.drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
		else
		{
			context.setDrawMode (kAliasing | kNonIntegralMode);
			context.drawRect (r, kDrawFilledAndStroked);
		}
	}
}

//-----------------------------------------------------------------------------
void CScrollbar::draw (CDrawContext& context)
{
	drawBackground (context);
	if (scrollerLength > 0)
	{
		CRect sr = getScrollerRect ();
		drawScroller (context, sr);
	}
}

}
