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
