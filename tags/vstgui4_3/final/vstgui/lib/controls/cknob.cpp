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

#include "cknob.h"
#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include <cmath>

namespace VSTGUI {
#if TARGET_OS_IPHONE
static const float kCKnobRange = 300.f;
#else
static const float kCKnobRange = 200.f;
#endif

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
CKnob::CKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, const CPoint& offset, int32_t drawStyle)
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

	rangeAngle = 1.f;
	setStartAngle ((float)(3.f * kPI / 4.f));
	setRangeAngle ((float)(3.f * kPI / 2.f));
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
	compute ();
}

//------------------------------------------------------------------------
bool CKnob::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getDrawBackground ()->getHeight ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CKnob::drawFocusOnTop ()
{
	if (drawStyle & kCoronaDrawing && wantsFocus ())
	{
		return false;
	}
	return CControl::drawFocusOnTop ();
}

//------------------------------------------------------------------------
bool CKnob::getFocusPath (CGraphicsPath &outPath)
{
	if (drawStyle & kCoronaDrawing && wantsFocus ())
	{
		CRect corona (getViewSize ());
		corona.inset (coronaInset, coronaInset);
		corona.inset (handleLineWidth/2., handleLineWidth/2.);
		outPath.addEllipse (corona);
		return true;
	}
	return CControl::getFocusPath (outPath);
}

//------------------------------------------------------------------------
void CKnob::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), offset);
	}
	if (pHandle)
		drawHandle (pContext);
	else
	{
		if (drawStyle & kCoronaOutline)
			drawCoronaOutline (pContext);
		if (drawStyle & kCoronaDrawing)
			drawCorona (pContext);
		if (drawStyle & kHandleCircleDrawing)
			drawHandleAsCircle (pContext);
		else
			drawHandleAsLine (pContext);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
void CKnob::addArc (CGraphicsPath* path, const CRect& r, double startAngle, double sweepAngle) const
{
	CCoord w = r.getWidth ();
	CCoord h = r.getHeight ();
	double endAngle = startAngle + sweepAngle;
	if (w != h)
	{
		startAngle = atan2 (sin (startAngle) * h, cos (startAngle) * w);
		endAngle = atan2 (sin (endAngle) * h, cos (endAngle) * w);
	}
	path->addArc (r, startAngle / kPI * 180, endAngle / kPI * 180, sweepAngle >= 0);
}

//------------------------------------------------------------------------
void CKnob::drawCoronaOutline (CDrawContext* pContext) const
{
	OwningPointer<CGraphicsPath> path = pContext->createGraphicsPath ();
	if (path == 0)
		return;
	CRect corona (getViewSize ());
	corona.inset (coronaInset, coronaInset);
	addArc (path, corona, startAngle, rangeAngle);
	pContext->setFrameColor (colorShadowHandle);
	CLineStyle lineStyle (kLineSolid);
	lineStyle.setLineCap (CLineStyle::kLineCapRound);
	pContext->setLineStyle (lineStyle);
	pContext->setLineWidth (handleLineWidth+2.);
	pContext->setDrawMode (kAntiAliasing);
	pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//------------------------------------------------------------------------
void CKnob::drawCorona (CDrawContext* pContext) const
{
	OwningPointer<CGraphicsPath> path = pContext->createGraphicsPath ();
	if (path == 0)
		return;
	float coronaValue = getValueNormalized ();
	if (drawStyle & kCoronaInverted)
		coronaValue = 1.f - coronaValue;
	CRect corona (getViewSize ());
	corona.inset (coronaInset, coronaInset);
	if (drawStyle & kCoronaFromCenter)
		addArc (path, corona, 1.5 * kPI, rangeAngle * (coronaValue - 0.5));
	else
	{
		if (drawStyle & kCoronaInverted)
			addArc (path, corona, startAngle + rangeAngle, -rangeAngle * coronaValue);
		else
			addArc (path, corona, startAngle, rangeAngle * coronaValue);
	}
	pContext->setFrameColor (coronaColor);
	CLineStyle lineStyle (drawStyle & kCoronaLineDashDot ? kLineOnOffDash : kLineSolid);
	lineStyle.setLineCap (CLineStyle::kLineCapRound);
	if (drawStyle & kCoronaLineDashDot)
		lineStyle.getDashLengths ()[1] = 2;
	pContext->setLineStyle (lineStyle);
	pContext->setLineWidth (handleLineWidth);
	pContext->setDrawMode (kAntiAliasing);
	pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//------------------------------------------------------------------------
void CKnob::drawHandleAsCircle (CDrawContext* pContext) const
{
	CPoint where;
	valueToPoint (where);

	where.offset (getViewSize ().left, getViewSize ().top);
	CRect r (where.x - 0.5, where.y - 0.5, where.x + 0.5, where.y + 0.5);
	r.extend (handleLineWidth, handleLineWidth);
	pContext->setDrawMode (kAntiAliasing);
	pContext->setFrameColor (colorShadowHandle);
	pContext->setFillColor (colorHandle);
	pContext->setLineWidth (0.5);
	pContext->setLineStyle (kLineSolid);
	pContext->setDrawMode (kAntiAliasing);
	pContext->drawEllipse (r, kDrawFilledAndStroked);
}

//------------------------------------------------------------------------
void CKnob::drawHandleAsLine (CDrawContext* pContext) const
{
	CPoint where;
	valueToPoint (where);

	CPoint origin (getViewSize ().getWidth () / 2, getViewSize ().getHeight () / 2);
	where.offset (getViewSize ().left - 1, getViewSize ().top);
	origin.offset (getViewSize ().left - 1, getViewSize ().top);
	pContext->setFrameColor (colorShadowHandle);
	pContext->setLineWidth (handleLineWidth);
	pContext->setLineStyle (CLineStyle (CLineStyle::kLineCapRound));
	pContext->setDrawMode (kAntiAliasing);
	pContext->drawLine (where, origin);
	
	where.offset (1, -1);
	origin.offset (1, -1);
	pContext->setFrameColor (colorHandle);
	pContext->drawLine (where, origin);
}

//------------------------------------------------------------------------
void CKnob::drawHandle (CDrawContext *pContext)
{
	CPoint where;
	valueToPoint (where);

	CCoord width  = pHandle->getWidth ();
	CCoord height = pHandle->getHeight ();
	where.offset (getViewSize ().left - width / 2, getViewSize ().top - height / 2);

	where.x = floor (where.x);
	where.y = floor (where.y);

	CRect handleSize (0, 0, width, height);
	handleSize.offset (where.x, where.y);
	pHandle->draw (pContext, handleSize);
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
	range = kCKnobRange;
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
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		startValue = valueFromPoint (where2);
		lastPoint = where;
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
	if (isEditing ())
	{
		float middle = (getMax () - getMin ()) * 0.5f;

		if (where != lastPoint)
		{
			lastPoint = where;
			if (modeLinear)
			{
				CCoord diff = (firstPoint.y - where.y) + (where.x - firstPoint.x);
				if (buttons != oldButton)
				{
					range = kCKnobRange;
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
				where.offset (-getViewSize ().left, -getViewSize ().top);
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
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
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

	if (isDirty ())
	{
		invalid ();

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

			if (isDirty ())
			{
				invalid ();

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
	setDirty ();
}

//------------------------------------------------------------------------
void CKnob::valueToPoint (CPoint &point) const
{
	float alpha = (value - getMin()) / (getMax() - getMin());
	alpha = startAngle + alpha*rangeAngle;

	CPoint c(size.getWidth () / 2., size.getHeight () / 2.);
	double xradius = c.x - inset;
	double yradius = c.y - inset;

	point.x = (CCoord)(c.x + cosf (alpha) * xradius + 0.5f);
	point.y = (CCoord)(c.y + sinf (alpha) * yradius + 0.5f);
}

//------------------------------------------------------------------------
float CKnob::valueFromPoint (CPoint &point) const
{
	float v;
	double d = rangeAngle * 0.5;
	double a = startAngle + d;

	CPoint c (size.getWidth () / 2., size.getHeight () / 2.);
	double xradius = c.x - inset;
	double yradius = c.y - inset;

	double dx = (point.x - c.x) / xradius;
	double dy = (point.y - c.y) / yradius;

	double alpha = atan2 (dy, dx) - a;
	while (alpha >= kPI)
		alpha -= k2PI;
	while (alpha < -kPI)
		alpha += k2PI;

	if (d < 0.0)
		alpha = -alpha;

	if (alpha > d)
		v = getMax ();
	else if (alpha < -d)
		v = getMin ();
	else
		v = float (0.5 + alpha / rangeAngle);

	v *= (getMax () - getMin ());

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
CAnimKnob::CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset)
, bInverseBitmap (false)
{
	heightOfOneImage = size.getHeight ();
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
CAnimKnob::CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
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
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
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
	if (getDrawBackground () && heightOfOneImage > 0)
		setNumSubPixmaps ((int32_t)(getDrawBackground ()->getHeight () / heightOfOneImage));
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setBackground (CBitmap *background)
{
	CKnob::setBackground (background);
	if (heightOfOneImage == 0)
		heightOfOneImage = getViewSize ().getHeight ();
	if (background && heightOfOneImage > 0)
		setNumSubPixmaps ((int32_t)(background->getHeight () / heightOfOneImage));
}

//------------------------------------------------------------------------
void CAnimKnob::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		CPoint where (0, 0);
		float val = getValueNormalized ();
		if (val >= 0.f && heightOfOneImage > 0.)
		{
			CCoord tmp = heightOfOneImage * (getNumSubPixmaps () - 1);
			if (bInverseBitmap)
				where.y = floor ((1. - val) * tmp);
			else
				where.y = floor (val * tmp);
			where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
		}

		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

} // namespace
