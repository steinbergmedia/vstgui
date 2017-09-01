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
void CXYPad::draw (CDrawContext* context)
{
	CParamDisplay::drawBack (context);

	float x, y;
	calculateXY (getValue (), x, y);
	CCoord width = getWidth() - getRoundRectRadius ();
	CCoord height = getHeight() - getRoundRectRadius ();
	CRect r (x*width, y*height, x*width, y*height);
	r.extend (getRoundRectRadius () / 2., getRoundRectRadius () / 2.);
	r.offset (getViewSize ().left + getRoundRectRadius() / 2., getViewSize ().top + getRoundRectRadius() / 2.);
	context->setFillColor (getFontColor ());
	context->setDrawMode (kAntiAliasing);
	context->drawEllipse (r, kDrawFilled);
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CXYPad::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		mouseChangeStartPoint = where;
		mouseChangeStartPoint.offset (-getViewSize ().left - getRoundRectRadius() / 2., -getViewSize ().top - getRoundRectRadius() / 2.);
		beginEdit ();
		return onMouseMoved (where, buttons);
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CXYPad::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		endEdit ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CXYPad::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
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
		where.offset (-getViewSize ().left - getRoundRectRadius() / 2., -getViewSize ().top - getRoundRectRadius() / 2.);

		x = (float)(where.x / width);
		y = (float)(where.y / height);

		boundValues (x, y);
		setValue (calculateValue (x, y));
		if (listener && isDirty ())
			listener->valueChanged (this);
		invalid ();
		lastMouseChangePoint = where;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
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

} // namespace
