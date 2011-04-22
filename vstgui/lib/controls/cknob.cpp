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

#include "cknob.h"
#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
// CKnob
//------------------------------------------------------------------------
/*! @class CKnob
Define a knob with a given background and foreground handle.
The handle describes a circle over the background (between -45deg and +225deg).
By clicking alt modifier and left mouse button the default value is used.
By clicking alt modifier and left mouse button the value changes with a vertical move (version 2.1)
*/
//------------------------------------------------------------------------
/**
 * CKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background background bitmap
 * @param handle handle bitmap
 * @param offset offset of background bitmap
 * @param drawStyle draw style
 */
//------------------------------------------------------------------------
CKnob::CKnob (const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, const CPoint& offset, int32_t drawStyle)
: CControl (size, listener, tag, background)
, drawStyle (drawStyle)
, offset (offset)
, pHandle (handle)
, handleLineWidth (1.)
, coronaInset (0)
{
	if (pHandle)
	{
		pHandle->remember ();
		inset = (CCoord)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	else
	{
		inset = 3;
	}
	
	colorShadowHandle = kGreyCColor;
	colorHandle = kWhiteCColor;
	radius = (float)(size.right - size.left) / 2.f;

	rangeAngle = 1.f;
	setStartAngle ((float)(5.f * kPI / 4.f));
	setRangeAngle ((float)(-3.f * kPI / 2.f));
	zoomFactor = 1.5f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CKnob::CKnob (const CKnob& v)
: CControl (v)
, offset (v.offset)
, drawStyle (v.drawStyle)
, colorHandle (v.colorHandle)
, colorShadowHandle (v.colorShadowHandle)
, handleLineWidth (v.handleLineWidth)
, inset (v.inset)
, coronaInset (v.coronaInset)
, startAngle (v.startAngle)
, rangeAngle (v.rangeAngle)
, halfAngle (v.halfAngle)
, aCoef (v.aCoef)
, bCoef (v.bCoef)
, radius (v.radius)
, zoomFactor (v.zoomFactor)
, pHandle (v.pHandle)
{
	if (pHandle)
		pHandle->remember ();
}

//------------------------------------------------------------------------
CKnob::~CKnob ()
{
	if (pHandle)
		pHandle->forget ();
}

//------------------------------------------------------------------------
void CKnob::setViewSize (const CRect &rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	radius = (float)(size.right - size.left) / 2.f;
	compute ();
}

//------------------------------------------------------------------------
bool CKnob::sizeToFit ()
{
	if (pBackground)
	{
		CRect vs (getViewSize ());
		vs.setWidth (pBackground->getWidth ());
		vs.setHeight (pBackground->getHeight ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CKnob::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		pBackground->draw (pContext, size, offset);
	}
	drawHandle (pContext);
	setDirty (false);
}

//------------------------------------------------------------------------
void CKnob::drawHandle (CDrawContext *pContext)
{
	CPoint where;
	valueToPoint (where);

	if (pHandle)
	{
		CCoord width  = pHandle->getWidth ();
		CCoord height = pHandle->getHeight ();
		where.offset (size.left - width / 2, size.top - height / 2);

		where.x = floor (where.x);
		where.y = floor (where.y);

		CRect handleSize (0, 0, width, height);
		handleSize.offset (where.h, where.v);
		pHandle->draw (pContext, handleSize);
	}
	else
	{
		pContext->setDrawMode (kAntiAliasing);
		if (drawStyle == 0)
		{
			CPoint origin (size.width () / 2, size.height () / 2);
			where.offset (size.left - 1, size.top);
			origin.offset (size.left - 1, size.top);
			pContext->setFrameColor (colorShadowHandle);
			pContext->setLineWidth (handleLineWidth);
			pContext->setLineStyle (kLineSolid);
			pContext->moveTo (where);
			pContext->lineTo (origin);
			
			where.offset (1, -1);
			origin.offset (1, -1);
			pContext->setFrameColor (colorHandle);
			pContext->moveTo (where);
			pContext->lineTo (origin);
		}
		if (drawStyle & kCoronaOutline)
		{
			CGraphicsPath* path = pContext->createGraphicsPath ();
			if (path)
			{
				CRect corona (getViewSize ());
				corona.inset (coronaInset, coronaInset);
				double rangeDegree = -(rangeAngle / kPI * 180.);
				double startDegree = (startAngle / kPI * 180.) - 90.;
				path->addArc (corona, startDegree, startDegree + rangeDegree, false);
				pContext->setFrameColor (colorShadowHandle);
				pContext->setLineStyle (kLineSolid);
				pContext->setLineWidth (handleLineWidth+2.);
				pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
				path->forget ();
			}
		}
		if (drawStyle & kCoronaDrawing)
		{
			CGraphicsPath* path = pContext->createGraphicsPath ();
			if (path)
			{
				float coronaValue = getValueNormalized ();
				if (drawStyle & kCoronaInverted)
					coronaValue = 1.f - coronaValue;
				CRect corona (getViewSize ());
				corona.inset (coronaInset, coronaInset);
				double rangeDegree = -(rangeAngle / kPI * 180.);
				if (drawStyle & kCoronaFromCenter)
				{
					double startDegree = 270.;
					rangeDegree = startDegree + (rangeDegree * (coronaValue - 0.5));
					path->addArc (corona, startDegree, rangeDegree, coronaValue > 0.5 ? false : true);
				}
				else
				{
					if (drawStyle & kCoronaInverted)
					{
						double startDegree = (startAngle / kPI * 180.) - 90. + rangeDegree;
						rangeDegree *= coronaValue;
						path->addArc (corona, startDegree, startDegree - rangeDegree, true);
					}
					else
					{
						double startDegree = (startAngle / kPI * 180.) - 90.;
						rangeDegree *= coronaValue;
						path->addArc (corona, startDegree, startDegree + rangeDegree, false);
					}
				}
				pContext->setFrameColor (coronaColor);
				pContext->setLineStyle (drawStyle & kCoronaLineDashDot ? kLineOnOffDash : kLineSolid);
				pContext->setLineWidth (handleLineWidth);
				pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
				path->forget ();
			}
		}
		if (drawStyle & kHandleCircleDrawing)
		{
			where.offset (size.left, size.top);
			CRect r (where.x - 0.5, where.y - 0.5, where.x + 0.5, where.y + 0.5);
			r.inset (-handleLineWidth, -handleLineWidth);
			pContext->setDrawMode (kAntiAliasing);
			pContext->setFrameColor (colorShadowHandle);
			pContext->setFillColor (colorHandle);
			pContext->setLineWidth (0.5);
			pContext->setLineStyle (kLineSolid);
			pContext->drawEllipse (r, kDrawFilledAndStroked);

		}
	}
}

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!buttons.isLeftButton ())
		return kMouseEventNotHandled;

	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	firstPoint = where;
	lastPoint (-1, -1);
	startValue = getOldValue ();

	modeLinear = false;
	fEntryState = value;
	range = 200.f;
	coef = (getMax () - getMin ()) / range;
	oldButton = buttons;

	int32_t mode    = kCircularMode;
	int32_t newMode = getFrame ()->getKnobMode ();
	if (kLinearMode == newMode)
	{
		if (!(buttons & kAlt))
			mode = newMode;
	}
	else if (buttons & kAlt)
	{
		mode = kLinearMode;
	}

	if (mode == kLinearMode)
	{
		if (buttons & kZoomModifier)
			range *= zoomFactor;
		lastPoint = where;
		modeLinear = true;
		coef = (getMax () - getMin ()) / range;
	}
	else
	{
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);
		startValue = valueFromPoint (where2);
	}

	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		float middle = (getMax () - getMin ()) * 0.5f;

		if (where != lastPoint)
		{
			lastPoint = where;
			if (modeLinear)
			{
				CCoord diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
				if (buttons != oldButton)
				{
					range = 200.f;
					if (buttons & kZoomModifier)
						range *= zoomFactor;

					float coef2 = (getMax () - getMin ()) / range;
					fEntryState += (float)(diff * (coef - coef2));
					coef = coef2;
					oldButton = buttons;
				}
				value = (float)(fEntryState + diff * coef);
				bounceValue ();
			}
			else
			{
				where.offset (-size.left, -size.top);
				value = valueFromPoint (where);
				if (startValue - value > middle)
					value = getMax ();
				else if (value - startValue > middle)
					value = getMin ();
				else
					startValue = value;
			}
			if (value != getOldValue ())
				valueChanged ();
			if (isDirty ())
				invalid ();
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CKnob::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	float v = getValueNormalized ();
	if (buttons & kZoomModifier)
		v += 0.1f * distance * wheelInc;
	else
		v += distance * wheelInc;
	setValueNormalized (v);

	if (isDirty () && listener)
	{
		// begin of edit parameter
		beginEdit ();
	
		valueChanged ();
	
		// end of edit parameter
		endEdit ();
	}
	return true;
}

//------------------------------------------------------------------------
int32_t CKnob::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_UP :
		case VKEY_RIGHT :
		case VKEY_DOWN :
		case VKEY_LEFT :
		{
			float distance = 1.f;
			if (keyCode.virt == VKEY_DOWN || keyCode.virt == VKEY_LEFT)
				distance = -distance;

			float v = getValueNormalized ();
			if (mapVstKeyModifier (keyCode.modifier) & kZoomModifier)
				v += 0.1f * distance * wheelInc;
			else
				v += distance * wheelInc;
			setValueNormalized (v);

			if (isDirty () && listener)
			{
				// begin of edit parameter
				beginEdit ();
				
				valueChanged ();
			
				// end of edit parameter
				endEdit ();
			}
		} return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
void CKnob::setStartAngle (float val)
{
	startAngle = val;
	compute ();
}

//------------------------------------------------------------------------
void CKnob::setRangeAngle (float val)
{
	rangeAngle = val;
	compute ();
}

//------------------------------------------------------------------------
void CKnob::compute ()
{
	aCoef = (getMax () - getMin ()) / rangeAngle;
	bCoef = getMin () - aCoef * startAngle;
	halfAngle = ((float)k2PI - fabsf (rangeAngle)) * 0.5f;
	setDirty ();
}

//------------------------------------------------------------------------
void CKnob::valueToPoint (CPoint &point) const
{
	float alpha = (value - bCoef) / aCoef;
	point.h = (CCoord)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.v = (CCoord)(radius - sinf (alpha) * (radius - inset) + 0.5f);
}

//------------------------------------------------------------------------
float CKnob::valueFromPoint (CPoint &point) const
{ // TODO: does not work when startAngle is something different than 135 degree
	float v;
	float alpha = (float)atan2 (radius - point.v, point.h - radius);
	if (alpha < 0.f)
		alpha += (float)k2PI;

	float alpha2 = alpha - startAngle;
	if (rangeAngle < 0)
	{
		alpha2 -= rangeAngle;
		float alpha3 = alpha2;
		if (alpha3 < 0.f)
			alpha3 += (float)k2PI;
		else if (alpha3 > k2PI)
			alpha3 -= (float)k2PI;
		if (alpha3 > halfAngle - rangeAngle)
			v = getMax ();
		else if (alpha3 > -rangeAngle)
			v = getMin ();
		else
		{
			if (alpha2 > halfAngle - rangeAngle)
				alpha2 -= (float)k2PI;
			else if (alpha2 < -halfAngle)
				alpha2 += (float)k2PI;
			v = aCoef * alpha2 + getMax ();
		}
	}
	else
	{
		float alpha3 = alpha2;
		if (alpha3 < 0.f)
			alpha3 += (float)k2PI;
		else if (alpha3 > k2PI)
			alpha3 -= (float)k2PI;
		if (alpha3 > rangeAngle + halfAngle)
			v = getMin ();
		else if (alpha3 > rangeAngle)
			v = getMax ();
		else
		{
			if (alpha2 > rangeAngle + halfAngle)
				alpha2 -= (float)k2PI;
			else if (alpha2 < -halfAngle)
				alpha2 += (float)k2PI;
			v = aCoef * alpha2 + getMin ();
		}
	}

	return v;
}

//------------------------------------------------------------------------
void CKnob::setCoronaInset (CCoord inset)
{
	if (inset != coronaInset)
	{
		coronaInset = inset;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setCoronaColor (CColor color)
{
	if (color != coronaColor)
	{
		coronaColor = color;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setColorShadowHandle (CColor color)
{
	if (color != colorShadowHandle)
	{
		colorShadowHandle = color;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setColorHandle (CColor color)
{
	if (color != colorHandle)
	{
		colorHandle = color;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setHandleLineWidth (CCoord width)
{
	if (width != handleLineWidth)
	{
		handleLineWidth = width;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setDrawStyle (int32_t style)
{
	if (style != drawStyle)
	{
		drawStyle = style;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setHandleBitmap (CBitmap* bitmap)
{
	if (pHandle)
	{
		pHandle->forget ();
		pHandle = 0;
	}

	if (bitmap)
	{
		pHandle = bitmap;
		pHandle->remember ();
		inset = (CCoord)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	setDirty ();
}

//------------------------------------------------------------------------
void CKnob::setMin (float val)
{
	CControl::setMin (val);
	if (getValue () < val)
		setValue (val);
	compute ();
}

//------------------------------------------------------------------------
void CKnob::setMax (float val)
{
	CControl::setMax (val);
	if (getValue () > val)
		setValue (val);
	compute ();
}

//------------------------------------------------------------------------
// CAnimKnob
//------------------------------------------------------------------------
/*! @class CAnimKnob
Such as a CKnob control object, but there is a unique bitmap which contains different views (subbitmaps) of this knob.
According to the value, a specific subbitmap is displayed. The different subbitmaps are stacked in the bitmap object.
*/
//------------------------------------------------------------------------
/**
 * CAnimKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the background bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset)
, bInverseBitmap (false)
{
	heightOfOneImage = size.height ();
	setNumSubPixmaps (background ? (int32_t)(background->getHeight () / heightOfOneImage) : 0);
	inset = 0;
}

//------------------------------------------------------------------------
/**
 * CAnimKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage the height of one sub bitmap
 * @param background the background bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect& size, CControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset)
, bInverseBitmap (false)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CAnimKnob& v)
: CKnob (v)
, bInverseBitmap (v.bInverseBitmap)
{
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CAnimKnob::~CAnimKnob ()
{}

//-----------------------------------------------------------------------------------------------
bool CAnimKnob::sizeToFit ()
{
	if (pBackground)
	{
		CRect vs (getViewSize ());
		vs.setWidth (pBackground->getWidth ());
		vs.setHeight (getHeightOfOneImage ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setHeightOfOneImage (const CCoord& height)
{
	IMultiBitmapControl::setHeightOfOneImage (height);
	if (pBackground && heightOfOneImage)
		setNumSubPixmaps ((int32_t)(pBackground->getHeight () / heightOfOneImage));
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setBackground (CBitmap *background)
{
	CKnob::setBackground (background);
	if (heightOfOneImage == 0)
		heightOfOneImage = size.height ();
	if (background && heightOfOneImage)
		setNumSubPixmaps ((int32_t)(background->getHeight () / heightOfOneImage));
}

//------------------------------------------------------------------------
void CAnimKnob::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		CPoint where (0, 0);
		if (value >= 0.f && heightOfOneImage > 0.) 
		{
			CCoord tmp = heightOfOneImage * (getNumSubPixmaps () - 1);
			if (bInverseBitmap)
				where.v = floor ((1. - value) * tmp);
			else
				where.v = floor (value * tmp);
			where.v -= (int32_t)where.v % (int32_t)heightOfOneImage;
		}

		pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

} // namespace
