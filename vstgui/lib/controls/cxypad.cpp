// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cxypad.h"
#include "../cdrawcontext.h"
#include "../events.h"

namespace VSTGUI {

//------------------------------------------------------------------------
CXYPad::CXYPad (const CRect& size)
: CParamDisplay (size)
, stopTrackingOnMouseExit (false)
{
	CParamDisplay::setMax (2.f);
}

//------------------------------------------------------------------------
void CXYPad::setHandleBitmap (CBitmap* bitmap)
{
	handle = bitmap;
	invalid ();
}

//------------------------------------------------------------------------
CBitmap* CXYPad::getHandleBitmap () const
{
	return handle;
}

//------------------------------------------------------------------------
void CXYPad::draw (CDrawContext* context)
{
	drawBack (context);

	auto width = getWidth () - getRoundRectRadius ();
	auto height = getHeight () - getRoundRectRadius ();

	float x, y;
	calculateXY (getValue (), x, y);

	CRect r (x*width, y*height, x*width, y*height);
	if (auto bitmap = getHandleBitmap ())
	{
		auto bitmapSize = bitmap->getSize ();
		r.extend (bitmapSize.x / 2., bitmapSize.y / 2.);
		r.offset (getViewSize ().left + getRoundRectRadius () / 2.,
				  getViewSize ().top + getRoundRectRadius () / 2.);
		bitmap->draw (context, r);
	}
	else
	{
		r.extend (getRoundRectRadius () / 2., getRoundRectRadius () / 2.);
		r.offset (getViewSize ().left + getRoundRectRadius () / 2.,
				  getViewSize ().top + getRoundRectRadius () / 2.);
		context->setFillColor (getFontColor ());
		context->setDrawMode (kAntiAliasing);
		context->drawEllipse (r, kDrawFilled);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
void CXYPad::drawBack (CDrawContext* context, CBitmap* newBack)
{
	CParamDisplay::drawBack (context);
}

//------------------------------------------------------------------------
void CXYPad::onMouseDownEvent (MouseDownEvent& event)
{
	if (event.buttonState.isLeft ())
	{
		invalidMouseWheelEditTimer (this);
		mouseStartValue = getValue ();
		mouseChangeStartPoint = event.mousePosition;
		mouseChangeStartPoint.offset (-getViewSize ().left - getRoundRectRadius () / 2.,
		                              -getViewSize ().top - getRoundRectRadius () / 2.);
		beginEdit ();
		onMouseMove (event);
	}
}

//------------------------------------------------------------------------
void CXYPad::onMouseUpEvent (MouseUpEvent& event)
{
	if (isEditing ())
	{
		endEdit ();
		event.consumed = true;
	}
}

//------------------------------------------------------------------------
void CXYPad::onMouseCancelEvent (MouseCancelEvent &event)
{
	if (isEditing ())
	{
		value = mouseStartValue;
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		endEdit ();
		event.consumed = true;
	}
}

//------------------------------------------------------------------------
void CXYPad::onMouseMoveEvent (MouseMoveEvent& event)
{
	if (event.buttonState.isLeft () && isEditing ())
	{
		onMouseMove (event);
	}
}

//------------------------------------------------------------------------
void CXYPad::onMouseMove (MouseDownUpMoveEvent& event)
{
	auto where = event.mousePosition;
	if (stopTrackingOnMouseExit)
	{
		if (!hitTest (where, event))
		{
			endEdit ();
			event.ignoreFollowUpMoveAndUpEvents (true);
			event.consumed = true;
			return;
		}
	}
	float x, y;
	CCoord width = getWidth() - getRoundRectRadius ();
	CCoord height = getHeight() - getRoundRectRadius ();
	where.offset (-getViewSize ().left - getRoundRectRadius () / 2.,
				  -getViewSize ().top - getRoundRectRadius () / 2.);

	x = (float)(where.x / width);
	y = (float)(where.y / height);

	boundValues (x, y);
	setValue (calculateValue (x, y));
	if (isDirty ())
	{
		valueChanged ();
		invalid ();
	}
	lastMouseChangePoint = where;
	event.consumed = true;
}

//------------------------------------------------------------------------
void CXYPad::onMouseWheelEvent (MouseWheelEvent& event)
{
	float x, y;
	calculateXY (getValue (), x, y);

	auto distanceX = static_cast<float> (event.deltaX) * getWheelInc ();
	auto distanceY = static_cast<float> (event.deltaY) * getWheelInc ();
	if (event.flags & MouseWheelEvent::DirectionInvertedFromDevice)
	{
		distanceX *= -1.f;
		distanceY *= -1.f;
	}
	if (event.modifiers.has (ModifierKey::Shift))
	{
		distanceX *= 0.1f;
		distanceY *= 0.1f;
	}
	x += distanceX;
	y += distanceY;
	boundValues (x, y);
	onMouseWheelEditing (this);
	setValue (calculateValue (x, y));
	if (isDirty ())
	{
		invalid ();
		valueChanged ();
	}
	event.consumed = true;
}

//------------------------------------------------------------------------
void CXYPad::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown)
		return;
	if (event.virt == VirtualKey::Escape)
	{
		if (isEditing ())
		{
			onMouseCancel ();
			event.consumed = true;
		}
	}
}

//------------------------------------------------------------------------
void CXYPad::boundValues (float& x, float& y)
{
	if (x < 0.f)
		x = 0.f;
	else if (x > 1.f)
		x = 1.f;
	if (y < 0.f)
		y = 0.f;
	else if (y > 1.f)
		y = 1.f;
}

//------------------------------------------------------------------------
void CXYPad::setDefaultValue (float val)
{
	CControl::setDefaultValue (calculateValue (val, val));
}

//------------------------------------------------------------------------
void CXYPad::setDefaultValues (float x, float y)
{
	CControl::setDefaultValue (calculateValue (x, y));
}

} // VSTGUI
