// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cxypad.h"
#include "../cdrawcontext.h"

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
CMouseEventResult CXYPad::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		invalidMouseWheelEditTimer (this);
		mouseStartValue = getValue ();
		mouseChangeStartPoint = where;
		mouseChangeStartPoint.offset (-getViewSize ().left - getRoundRectRadius () / 2.,
		                              -getViewSize ().top - getRoundRectRadius () / 2.);
		beginEdit ();
		return onMouseMoved (where, buttons);
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CXYPad::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		endEdit ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CXYPad::onMouseCancel ()
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
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CXYPad::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton () && isEditing ())
	{
		if (stopTrackingOnMouseExit)
		{
			if (!hitTest (where, buttons))
			{
				endEdit ();
				return kMouseMoveEventHandledButDontNeedMoreEvents;
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
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool CXYPad::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& _distance,
                      const CButtonState& buttons)
{
	float x, y;
	calculateXY (getValue (), x, y);
	auto distance = _distance * getWheelInc ();
	if (buttons & kMouseWheelInverted)
		distance = -distance;
	if (buttons & kShift)
		distance *= 0.1f;
	if (axis == kMouseWheelAxisX)
		x += distance;
	else
		y += distance;
	boundValues (x, y);
	onMouseWheelEditing (this);
	setValue (calculateValue (x, y));
	if (isDirty ())
	{
		invalid ();
		valueChanged ();
	}
	return true;
}

//------------------------------------------------------------------------
int32_t CXYPad::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_ESCAPE:
		{
			if (isEditing ())
			{
				onMouseCancel ();
				return 1;
			}
			break;
		}
	}
	return -1;
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

} // VSTGUI
