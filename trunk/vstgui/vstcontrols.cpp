//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
// Standard Control Objects
//
// Version 3.5       $Date: 2007-10-16 20:41:34 $
//
// Added new objects        : Michael Schmidt          08.97
// Added new objects        : Yvan Grabit              01.98
// Added BeOS version       : Georges-Edouard Berenger 05.99
// Added MacOSX version     : Arne Scheffler           02.03
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef __vstcontrols__
#include "vstcontrols.h"
#endif

#include "vstkeycode.h"

#ifdef check
#undef check
#endif

#if VSTGUI_BUILD_COCOA
#include "cocoasupport.h"
#endif

BEGIN_NAMESPACE_VSTGUI

#if defined(__MACH__) && defined (__MWERKS__)
 #ifndef cosf
 #define cosf (float)cos
 #endif

 #ifndef sinf
 #define sinf (float)sin
 #endif
#endif

// some external variables (vstgui.cpp)
extern long gStandardFontSize [];
extern const char *gStandardFontName [];

//------------------------------------------------------------------------
/*! @defgroup views Views
 *	@ingroup viewsandcontrols
 */
//------------------------------------------------------------------------
/*! @defgroup controls Controls
 *	@ingroup views
 *	@brief Controls are views the user can interact with
 */
//------------------------------------------------------------------------
/*! @defgroup containerviews Container Views
 *	@ingroup views
 */
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// CControl
//------------------------------------------------------------------------
/*! @class CControl
This object manages the tag identification and the value of a control object.

Note:
Since version 2.1, when an object uses the transparency for its background and draws on it (tranparency area)
or the transparency area changes during different draws (CMovieBitmap ,...), the background will be false (not updated),
you have to rewrite the draw function in order to redraw the background and then call the draw of the object.
*/
CControl::CControl (const CRect& size, CControlListener* listener, long tag, CBitmap *pBackground)
: CView (size)
, listener (listener)
, tag (tag)
, oldValue (1)
, defaultValue (0.5f)
, value (0)
, vmin (0)
, vmax (1.f)
, wheelInc (0.1f)
, lastTicks (-1)
{
	#if WINDOWS
		delta = GetDoubleClickTime ();
	#elif MAC
		delta = GetDblTime ();
	#else
		delta = 500;
	#endif

	if (delta < 250)
		delta = 250;

	setTransparency (false);
	setMouseEnabled (true);
	backOffset (0 ,0);

	setBackground (pBackground);
}

//------------------------------------------------------------------------
CControl::CControl (const CControl& c)
: CView (c)
, listener (c.listener)
, tag (c.tag)
, oldValue (c.oldValue)
, defaultValue (c.defaultValue)
, value (c.value)
, vmin (c.vmin)
, vmax (c.vmax)
, wheelInc (c.wheelInc)
, lastTicks (c.lastTicks)
{
}

//------------------------------------------------------------------------
CControl::~CControl ()
{
}

//------------------------------------------------------------------------
void CControl::beginEdit ()
{
	// begin of edit parameter
	if (listener)
		listener->controlBeginEdit (this);
	if (getFrame ())
	{
		getFrame ()->setFocusView (this);
		getFrame ()->beginEdit (tag);
	}
}

//------------------------------------------------------------------------
void CControl::endEdit ()
{
	// end of edit parameter
	if (getFrame ())
		getFrame ()->endEdit (tag);
	if (listener)
		listener->controlEndEdit (this);
}

//------------------------------------------------------------------------
bool CControl::isDirty () const
{
	if (oldValue != value || CView::isDirty ())
		return true;
	return false;
}

//------------------------------------------------------------------------
void CControl::setDirty (const bool val)
{
	CView::setDirty (val);
	if (val)
	{
		if (value != -1.f)
			oldValue = -1.f;
		else
			oldValue = 0.f;
	}
	else
		oldValue = value;
}

//------------------------------------------------------------------------
void CControl::setBackOffset (CPoint &offset)
{
	backOffset = offset;
}

//-----------------------------------------------------------------------------
void CControl::copyBackOffset ()
{
	backOffset (size.left, size.top);
}

//------------------------------------------------------------------------
void CControl::bounceValue ()
{
	if (value > vmax)
		value = vmax;
	else if (value < vmin)
		value = vmin;
}

//-----------------------------------------------------------------------------
bool CControl::checkDefaultValue (long button)
{
	if (button == (kControl|kLButton))
	{
		// begin of edit parameter
		beginEdit ();
	
		value = getDefaultValue ();
		if (isDirty () && listener)
			listener->valueChanged (this);

		// end of edit parameter
		endEdit ();
		return true;
	}
	return false;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
bool CControl::isDoubleClick ()
{
	long ticks = getFrame ()->getTicks ();
	if (lastTicks <= 0)
	{
		lastTicks = ticks;
		return false;
	}

	if (lastTicks + delta > ticks)
		lastTicks = 0;
	else
	{
		lastTicks = ticks;
		return false;
	}
	return true;
}
#endif

//------------------------------------------------------------------------
// COnOffButton
//------------------------------------------------------------------------
/*! @class COnOffButton
Define a button with 2 positions.
The bitmap includes the 2 subbitmaps (i.e the rectangle used for the display of this button is half-height of the bitmap).
When its value changes, the listener is called.
*/
//------------------------------------------------------------------------
/**
 * COnOffButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap of the on/off button
 * @param style style (kPostListenerUpdate or kPreListenerUpdate)
 */
//------------------------------------------------------------------------
COnOffButton::COnOffButton (const CRect& size, CControlListener* listener, long tag, CBitmap* background, long style)
: CControl (size, listener, tag, background)
, style (style)
{}

//------------------------------------------------------------------------
COnOffButton::COnOffButton (const COnOffButton& v)
: CControl (v)
, style (v.style)
{
}

//------------------------------------------------------------------------
COnOffButton::~COnOffButton ()
{}

//------------------------------------------------------------------------
void COnOffButton::draw (CDrawContext *pContext)
{
	CCoord off;

	if (value && pBackground)
		off = pBackground->getHeight () / 2;
	else
		off = 0;

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, CPoint (0, off));
		else
			pBackground->draw (pContext, size, CPoint (0, off));
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void COnOffButton::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

 	if (button == -1) button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}
	value = ((long)value) ? 0.f : 1.f;
	
	if (listener && style == kPostListenerUpdate)
	{
		// begin of edit parameter
		beginEdit ();
	
		listener->valueChanged (this);
	
		// end of edit parameter
		endEdit ();
	}
	
	doIdleStuff ();
	
	if (listener && style == kPreListenerUpdate)
	{
		// begin of edit parameter
		beginEdit ();
	
		listener->valueChanged (this);
	
		// end of edit parameter
		endEdit ();
	}
}
#endif

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	value = ((long)value) ? 0.f : 1.f;

	invalid ();

	if (listener && style == kPostListenerUpdate)
	{
		// begin of edit parameter
		beginEdit ();
	
		listener->valueChanged (this);
	
		// end of edit parameter
		endEdit ();
	}
	
	doIdleStuff ();
	
	if (listener && style == kPreListenerUpdate)
	{
		// begin of edit parameter
		beginEdit ();
	
		listener->valueChanged (this);
	
		// end of edit parameter
		endEdit ();
	}

	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

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
 */
//------------------------------------------------------------------------
CKnob::CKnob (const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* handle, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
, pHandle (handle)
{
	if (pHandle)
	{
		pHandle->remember ();
		inset = (long)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	else
		inset = 3;
	
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
, colorHandle (v.colorHandle)
, colorShadowHandle (v.colorShadowHandle)
, inset (v.inset)
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
void CKnob::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, offset);
		else
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

		CRect handleSize (0, 0, width, height);
		handleSize.offset (where.h, where.v);
		pHandle->drawTransparent (pContext, handleSize);
	}
	else
	{
		CPoint origin (size.width () / 2, size.height () / 2);
		
		where.offset (size.left - 1, size.top);
		origin.offset (size.left - 1, size.top);
		pContext->setFrameColor (colorShadowHandle);
		pContext->moveTo (where);
		pContext->lineTo (origin);
		
		where.offset (1, -1);
		origin.offset (1, -1);
		pContext->setFrameColor (colorHandle);
		pContext->moveTo (where);
		pContext->lineTo (origin);
	}
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CKnob::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	// check if default value wanted
	if (checkDefaultValue (button))
		return;

	float old = oldValue;
	CPoint firstPoint;
	bool  modeLinear = false;
	float fEntryState = value;
	float middle = (vmax - vmin) * 0.5f;
	float range = 200.f;
	float coef = (vmax - vmin) / range;
	long  oldButton = button;

	long mode    = kCircularMode;
	long newMode = getFrame ()->getKnobMode ();
	if (kLinearMode == newMode)
	{
		if (!(button & kAlt))
			mode = newMode;
	}
	else if (button & kAlt) 
		mode = kLinearMode;

	if (mode == kLinearMode && (button & kLButton))
	{
		if (button & kShift)
			range *= zoomFactor;
		firstPoint = where;
		modeLinear = true;
		coef = (vmax - vmin) / range;
	}
	else
	{
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);
		old = valueFromPoint (where2);
	}

	CPoint oldWhere (-1, -1);

	// begin of edit parameter
	beginEdit ();
	do
	{
		button = pContext->getMouseButtons ();
		if (where != oldWhere)
		{
			oldWhere = where;
			if (modeLinear)
			{
				CCoord diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
				if (button != oldButton)
				{
					range = 200.f;
					if (button & kShift)
						range *= zoomFactor;
	
					float coef2 = (vmax - vmin) / range;
					fEntryState += diff * (coef - coef2);
					coef = coef2;
					oldButton = button;
				}
				value = fEntryState + diff * coef;
				bounceValue ();
			}
			else
			{
				where.offset (-size.left, -size.top);
				value = valueFromPoint (where);
				if (old - value > middle)
					value = vmax;
				else if (value - old > middle)
					value = vmin;
				else
					old = value;
			}
			if (isDirty () && listener)
				listener->valueChanged (this);
		}
		getMouseLocation (pContext, where);
		doIdleStuff ();

	} while (button & kLButton);

	// end of edit parameter
	endEdit ();
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseDown (CPoint& where, const long& buttons)
{
	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	firstPoint = where;
	lastPoint (-1, -1);
	startValue = oldValue;

	modeLinear = false;
	fEntryState = value;
	range = 200.f;
	coef = (vmax - vmin) / range;
	oldButton = buttons;

	long mode    = kCircularMode;
	long newMode = getFrame ()->getKnobMode ();
	if (kLinearMode == newMode)
	{
		if (!(buttons & kAlt))
			mode = newMode;
	}
	else if (buttons & kAlt) 
		mode = kLinearMode;

	if (mode == kLinearMode && (buttons & kLButton))
	{
		if (buttons & kShift)
			range *= zoomFactor;
		lastPoint = where;
		modeLinear = true;
		coef = (vmax - vmin) / range;
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
CMouseEventResult CKnob::onMouseUp (CPoint& where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		float middle = (vmax - vmin) * 0.5f;

		if (where != lastPoint)
		{
			lastPoint = where;
			if (modeLinear)
			{
				CCoord diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
				if (buttons != oldButton)
				{
					range = 200.f;
					if (buttons & kShift)
						range *= zoomFactor;

					float coef2 = (vmax - vmin) / range;
					fEntryState += diff * (coef - coef2);
					coef = coef2;
					oldButton = buttons;
				}
				value = fEntryState + diff * coef;
				bounceValue ();
			}
			else
			{
				where.offset (-size.left, -size.top);
				value = valueFromPoint (where);
				if (startValue - value > middle)
					value = vmax;
				else if (value - startValue > middle)
					value = vmin;
				else
					startValue = value;
			}
			if (value != oldValue && listener)
				listener->valueChanged (this);
			if (isDirty ())
				invalid ();
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CKnob::onWheel (const CPoint& where, const float &distance, const long &buttons)
{
	if (!bMouseEnabled)
		return false;

	if (buttons & kShift)
		value += 0.1f * distance * wheelInc;
	else
		value += distance * wheelInc;
	bounceValue ();

	if (isDirty () && listener)
	{
		// begin of edit parameter
		beginEdit ();
	
		listener->valueChanged (this);
	
		// end of edit parameter
		endEdit ();
	}
	return true;
}

//------------------------------------------------------------------------
long CKnob::onKeyDown (VstKeyCode& keyCode)
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

			if (keyCode.modifier & MODIFIER_SHIFT)
				value += 0.1f * distance * wheelInc;
			else
				value += distance * wheelInc;
			bounceValue ();

			if (isDirty () && listener)
			{
				// begin of edit parameter
				beginEdit ();
				
				listener->valueChanged (this);
			
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
	aCoef = (vmax - vmin) / rangeAngle;
	bCoef = vmin - aCoef * startAngle;
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
{
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
			v = vmax;
		else if (alpha3 > -rangeAngle)
			v = vmin;
		else
		{
			if (alpha2 > halfAngle - rangeAngle)
				alpha2 -= (float)k2PI;
			else if (alpha2 < -halfAngle)
				alpha2 += (float)k2PI;
			v = aCoef * alpha2 + vmax;
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
			v = vmin;
		else if (alpha3 > rangeAngle)
			v = vmax;
		else
		{
			if (alpha2 > rangeAngle + halfAngle)
				alpha2 -= (float)k2PI;
			else if (alpha2 < -halfAngle)
				alpha2 += (float)k2PI;
			v = aCoef * alpha2 + vmin;
		}
	}

	return v;
}

//------------------------------------------------------------------------
void CKnob::setColorShadowHandle (CColor color)
{
	colorShadowHandle = color;
	setDirty ();
}

//------------------------------------------------------------------------
void CKnob::setColorHandle (CColor color)
{
	colorHandle = color;
	setDirty ();
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
		inset = (long)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
}


//------------------------------------------------------------------------
// CParamDisplay
//------------------------------------------------------------------------
/*! @class CParamDisplay
Define a rectangle view where a text-value can be displayed with a given font and color.
The user can specify its convert function (from float to char) by default the string format is "%2.2f".
The text-value is centered in the given rect.
*/
CParamDisplay::CParamDisplay (const CRect& size, CBitmap* background, const long style)
: CControl (size, 0, 0, background)
, stringConvert (0)
, stringConvert2 (0)
, string2FloatConvert (0)
, horiTxtAlign (kCenterText)
, style (style)
, bTextTransparencyEnabled (true)
, bAntialias (true)
{
	backOffset (0, 0);

	fontID      = kNormalFont; fontID->remember ();
	txtFace     = kNormalFace;
	fontColor   = kWhiteCColor;
	backColor   = kBlackCColor;
	frameColor  = kBlackCColor;
	shadowColor = kRedCColor;
	userData    = 0;
	if (style & kNoDrawStyle)
		setDirty (false);
}

//------------------------------------------------------------------------
CParamDisplay::CParamDisplay (const CParamDisplay& v)
: CControl (v)
, stringConvert (v.stringConvert)
, stringConvert2 (v.stringConvert2)
, string2FloatConvert (v.string2FloatConvert)
, userData (v.userData)
, horiTxtAlign (v.horiTxtAlign)
, style (v.style)
, fontID (v.fontID)
, txtFace (v.txtFace)
, fontColor (v.fontColor)
, backColor (v.backColor)
, frameColor (v.frameColor)
, shadowColor (v.shadowColor)
, bTextTransparencyEnabled (v.bTextTransparencyEnabled)
, bAntialias (v.bAntialias)
{
	fontID->remember ();
}

//------------------------------------------------------------------------
CParamDisplay::~CParamDisplay ()
{
	if (fontID)
		fontID->forget ();
}

//------------------------------------------------------------------------
void CParamDisplay::setStyle (long val)
{
	if (style != val)
	{
		style = val;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::draw (CDrawContext *pContext)
{
	char string[256];
	string[0] = 0;

	if (stringConvert2)
		stringConvert2 (value, string, userData);
	else if (stringConvert)
		stringConvert (value, string);
	else
		sprintf (string, "%2.2f", value);

	drawText (pContext, string);
}

//------------------------------------------------------------------------
void CParamDisplay::drawText (CDrawContext *pContext, char *string, CBitmap *newBack)
{
	setDirty (false);

	if (style & kNoDrawStyle)
		return;

	// draw the background
	if (newBack)
	{
		if (bTransparencyEnabled)
			newBack->drawTransparent (pContext, size, backOffset);
		else
			newBack->draw (pContext, size, backOffset);
	}
	else if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, backOffset);
		else
			pBackground->draw (pContext, size, backOffset);
	}
	else
	{
		if (!bTransparencyEnabled)
		{
			pContext->setFillColor (backColor);
			pContext->drawRect (size, kDrawFilled);
	
			if (!(style & (k3DIn|k3DOut|kNoFrame))) 
			{
				pContext->setLineWidth (1);
				pContext->setFrameColor (frameColor);
				pContext->drawRect (size);
			}
		}
	}
	// draw the frame for the 3D effect
	if (style & (k3DIn|k3DOut)) 
	{
		CRect r (size);
		r.right--; r.top++;
		pContext->setLineWidth (1);
		if (style & k3DIn)
			pContext->setFrameColor (backColor);
		else
			pContext->setFrameColor (frameColor);
		CPoint p;
		pContext->moveTo (p (r.left, r.bottom));
		pContext->lineTo (p (r.left, r.top));
		pContext->lineTo (p (r.right, r.top));

		if (style & k3DIn)
			pContext->setFrameColor (frameColor);
		else
			pContext->setFrameColor (backColor);
		pContext->moveTo (p (r.right, r.top));
		pContext->lineTo (p (r.right, r.bottom));
		pContext->lineTo (p (r.left, r.bottom));
	}

	if (!(style & kNoTextStyle) && string && strlen (string))
	{
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (size);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		pContext->setFont (fontID, 0, txtFace);
	
		// draw darker text (as shadow)
		if (style & kShadowText) 
		{
			CRect newSize (size);
			newSize.offset (1, 1);
			pContext->setFontColor (shadowColor);
			#if VSTGUI_USES_UTF8
			pContext->drawStringUTF8 (string, newSize, horiTxtAlign, bAntialias);
			#else
			pContext->drawString (string, newSize, !bTextTransparencyEnabled, horiTxtAlign);
			#endif
		}
		pContext->setFontColor (fontColor);
		#if VSTGUI_USES_UTF8
		pContext->drawStringUTF8 (string, size, horiTxtAlign, bAntialias);
		#else
		pContext->drawString (string, size, !bTextTransparencyEnabled, horiTxtAlign);
		#endif
		pContext->setClipRect (oldClip);
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFont (CFontRef fontID)
{
	setDirty ();
	if (this->fontID)
		this->fontID->forget ();
	this->fontID = fontID;
	fontID->remember ();
}

//------------------------------------------------------------------------
void CParamDisplay::setTxtFace (CTxtFace txtFace)
{
	// to force the redraw
	if (this->txtFace != txtFace)
		setDirty ();
	this->txtFace = txtFace;
}

//------------------------------------------------------------------------
void CParamDisplay::setFontColor (CColor color)
{
	// to force the redraw
	if (fontColor != color)
		setDirty ();
	fontColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setBackColor (CColor color)
{
	// to force the redraw
	if (backColor != color)
		setDirty ();
	backColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setFrameColor (CColor color)
{
	// to force the redraw
	if (frameColor != color)
		setDirty ();
	frameColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setShadowColor (CColor color)
{
	// to force the redraw
	if (shadowColor != color)
		setDirty ();
	shadowColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setHoriAlign (CHoriTxtAlign hAlign)
{
	// to force the redraw
	if (horiTxtAlign != hAlign)
		setDirty ();
	horiTxtAlign = hAlign;
}

//------------------------------------------------------------------------
void CParamDisplay::setStringConvert (void (*convert) (float value, char* string))
{
	stringConvert = convert;
}

//------------------------------------------------------------------------
void CParamDisplay::setStringConvert (void (*convert) (float value, char* string,
									  void* userDta), void* userData)
{
	stringConvert2 = convert;
	this->userData = userData;
}

//------------------------------------------------------------------------
void CParamDisplay::setString2FloatConvert (void (*convert) (char *string, float &output))
{
	string2FloatConvert = convert;
}

//------------------------------------------------------------------------
// CTextLabel
//------------------------------------------------------------------------
/*! @class CTextLabel
*/
//------------------------------------------------------------------------
/**
 * CTextLabel constructor.
 * @param size the size of this view
 * @param txt the initial text as c string (can be UTF-8 encoded if VSTGUI_USES_UTF8 is set)
 * @param background the background bitmap
 * @param style the display style (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CRect& size, const char* txt, CBitmap* background, const long style)
: CParamDisplay (size, background, style)
, text (0)
{
	setText (txt);
}

//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CTextLabel& v)
: CParamDisplay (v)
, text (0)
{
	setText (v.getText ());
}

//------------------------------------------------------------------------
CTextLabel::~CTextLabel ()
{
	freeText ();
}

//------------------------------------------------------------------------
void CTextLabel::freeText ()
{
	if (text)
		free (text);
	text = 0;
}

//------------------------------------------------------------------------
void CTextLabel::setText (const char* txt)
{
	if (!text && !txt || (text && txt && strcmp (text, txt) == 0))
		return;
	freeText ();
	if (txt)
	{
		text = (char*)malloc (strlen (txt)+1);
		strcpy (text, txt);
	}
	setDirty (true);
}

//------------------------------------------------------------------------
const char* CTextLabel::getText () const
{
	return text;
}

//------------------------------------------------------------------------
void CTextLabel::draw (CDrawContext *pContext)
{
	drawText (pContext, text);
	setDirty (false);
}


//------------------------------------------------------------------------
// CTextEdit
//------------------------------------------------------------------------
/*! @class CTextEdit
Define a rectangle view where a text-value can be displayed and edited with a given font and color.
The user can specify its convert function (from char to char). The text-value is centered in the given rect.
A bitmap can be used as background.
*/
//------------------------------------------------------------------------
/**
 * CTextEdit constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param txt the initial text as c string (can be UTF-8 encoded if VSTGUI_USES_UTF8 is set)
 * @param background the background bitmap
 * @param style the display style (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
CTextEdit::CTextEdit (const CRect& size, CControlListener* listener, long tag, const char *txt, CBitmap* background, const long style)
: CParamDisplay (size, background, style)
, platformFontColor (0)
, platformControl (0)
, platformFont (0)
, editConvert (0)
, editConvert2 (0)
{
	this->listener = listener;
	this->tag = tag;

	if (txt)
		strcpy (text, txt);
	else
		strcpy (text, "");
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::CTextEdit (const CTextEdit& v)
: CParamDisplay (v)
, platformFontColor (0)
, platformControl (0)
, platformFont (0)
, editConvert (v.editConvert)
, editConvert2 (v.editConvert2)
{
	setText ((char*)v.text);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::~CTextEdit ()
{
	listener = 0;
	if (platformControl)
		looseFocus ();
}

//------------------------------------------------------------------------
void CTextEdit::setText (const char *txt)
{
	if (txt)
	{
		if (strcmp (text, txt))
		{
			strncpy (text, txt, 255);
			text[255] = 0;	// make sure we end with a 0 byte

			// to force the redraw
			setDirty ();
		}
	}
	else
	{
		if (strcmp (text, ""))
		{
			strcpy (text, "");

			// to force the redraw
			setDirty ();
		}
	}
}

//------------------------------------------------------------------------
void CTextEdit::getText (char *txt) const
{
	if (txt)
		strcpy (txt, text);
}

//------------------------------------------------------------------------
void CTextEdit::draw (CDrawContext *pContext)
{
	if (platformControl)
	{
		#if MAC
		HIViewSetNeedsDisplay ((HIViewRef)platformControl, true);
		#endif
		setDirty (false);
		return;
	}

	char string[256];
	string[0] = 0;

	if (editConvert2)
		editConvert2 (text, string, userData);
	else if (editConvert)
		editConvert (text, string);
	// Allow to display strings through the stringConvert
	// callbacks inherited from CParamDisplay
	else if (stringConvert2)
	{
		string[0] = 0;
		stringConvert2 (value, string, userData);
		strcpy(text, string);
	}
	else if (stringConvert)
	{
		string[0] = 0;
		stringConvert (value, string);
		strcpy(text, string);
	}
	else
		sprintf (string, "%s", text);

	drawText (pContext, string);
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CTextEdit::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (button & kLButton)
	{
		if (getFrame ()->getFocusView () != this)
		{
			if (style & kDoubleClickStyle)
				if (!isDoubleClick ())
					return;
		
			beginEdit();
			takeFocus ();
		}
	}
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CTextEdit::onMouseDown (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (getFrame ()->getFocusView () != this)
		{
			if (style & kDoubleClickStyle)
			{
				if (!(buttons & kDoubleClick))
					return kMouseEventNotHandled;
			}
		
			beginEdit();
			takeFocus ();
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
#if WINDOWS
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

END_NAMESPACE_VSTGUI

extern void* hInstance;
inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }

BEGIN_NAMESPACE_VSTGUI

extern long standardFontSize [];
extern const char *standardFontName [];

#ifdef STRICT
#define WINDOWSPROC WNDPROC
#else
#define WINDOWSPROC FARPROC
#endif

static WINDOWSPROC oldWndProcEdit;
LONG_PTR WINAPI WindowProcEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LONG_PTR WINAPI WindowProcEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch (message)
	{
		case WM_GETDLGCODE :
		{
			long flags = DLGC_WANTALLKEYS;
			return flags;
		}

		case WM_KEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				CTextEdit *textEdit = (CTextEdit*) GetWindowLongPtr (hwnd, GWLP_USERDATA);
				if (textEdit)
				{
					textEdit->bWasReturnPressed = true;
					if (textEdit->getFrame ())
						textEdit->getFrame ()->setFocusView (0);
				}
			}
		} break;

		case WM_KILLFOCUS:
		{
			CTextEdit *textEdit = (CTextEdit*) GetWindowLongPtr (hwnd, GWLP_USERDATA);
			if (textEdit)
				textEdit->looseFocus ();
		} break;
	}

	return CallWindowProc (oldWndProcEdit, hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
#endif

#if MAC
static EventHandlerRef gTextEditEventHandler = 0;
static bool gTextEditCanceled = false;
pascal OSStatus CarbonEventsTextControlProc (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
pascal OSStatus CarbonEventsTextControlProc (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	CTextEdit* textEdit = (CTextEdit*)inUserData;

	switch (eventClass)
	{
		case kEventClassKeyboard:
		{
			switch (eventKind)
			{
				case kEventRawKeyDown:
				case kEventRawKeyRepeat:
				{
					char macCharCode;
					UInt32 keyCode;
					UInt32 modifiers;
					GetEventParameter (inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (char), NULL, &macCharCode);
					GetEventParameter (inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
					GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
					if (macCharCode == 13 || macCharCode == 3 || macCharCode == 27)
					{
						if (macCharCode == 27)
							gTextEditCanceled = true;
						else
							textEdit->bWasReturnPressed = true;

						textEdit->looseFocus ();

						result = noErr;
					}
					break;
				}
			}
			break;
		}
		#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
		case kEventClassControl:
		{
			switch (eventKind)
			{
				case kEventControlDraw:
				{
					CGContextRef cgContext;
					if (GetEventParameter (inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (cgContext), NULL, &cgContext) == noErr)
					{
						#if MAC_OS_X_VERSION_MIN_ALLOWED < MAC_OS_X_VERSION_10_4
						if (HIRectConvert)
						#endif
						{
							CRect viewSize = textEdit->getViewSize (viewSize);
							viewSize.inset (-10, -10);
							CViewContainer* container = (CViewContainer*)textEdit->getParentView ();
							while (!container->isTypeOf ("CScrollContainer"))
							{
								CRect containerSize = container->getViewSize (containerSize);
								viewSize.offset (containerSize.left, containerSize.top);
								if (container == container->getParentView () || container->getParentView () == 0)
									break;
								container = ((CViewContainer*)container->getParentView ());
							}
							viewSize = container->getVisibleSize (viewSize);
							CPoint cp (viewSize.left, viewSize.top);
							container->localToFrame (cp);
							viewSize.offset (-viewSize.left, -viewSize.top);
							viewSize.offset (cp.x, cp.y);
							CGRect cgViewSize = CGRectMake (viewSize.left, viewSize.top, viewSize.getWidth (), viewSize.getHeight ());
							HIRectConvert (&cgViewSize, kHICoordSpaceView, (HIViewRef)textEdit->getFrame ()->getPlatformControl (), kHICoordSpaceView, textEdit->platformControl);
							CGContextClipToRect (cgContext, cgViewSize);
							CGAffineTransform ctm = CGContextGetCTM (cgContext);
						}
						result = CallNextEventHandler (inHandlerCallRef, inEvent);
					}
					break;
				}
			}
			break;
		}
		#endif
		case kEventClassWindow:
		{
			WindowRef window;
			if (GetEventParameter (inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof (WindowRef), NULL, &window) != noErr)
				break;
			switch (eventKind)
			{
				case kEventWindowDeactivated:
				{
//					result = CallNextEventHandler (inHandlerCallRef, inEvent);
//					ClearKeyboardFocus (window);

					textEdit->looseFocus ();

					break;
				}
			}
			break;
		}
	}
	return result;
}
#endif

//------------------------------------------------------------------------
void CTextEdit::parentSizeChanged ()
{
	#if MAC_COCOA
	if (getFrame ()->getNSView ())
	{
		moveNSTextField (platformControl, this);
		return;
	}
	#endif
	
	#if MAC
	if (platformControl)
	{
		CRect rect (size);
		if (rect.getHeight () > fontID->getSize ())
		{
			rect.top = (short)(rect.top + rect.getHeight () / 2 - fontID->getSize () / 2 + 1);
			rect.bottom = (short)(rect.top + fontID->getSize ());
		}
		CPoint p (rect.left, rect.top);
		localToFrame (p);
		HIRect hiRect;
		HIViewGetFrame ((HIViewRef)platformControl, &hiRect);
		hiRect.origin.x = p.x;
		hiRect.origin.y = p.y;
		HIViewSetFrame ((HIViewRef)platformControl, &hiRect);
	}
	#endif
}

//------------------------------------------------------------------------
void CTextEdit::setViewSize (CRect& newSize, bool invalid)
{
	#if MAC_COCOA
	if (getFrame ()->getNSView ())
	{
		CView::setViewSize (newSize, invalid);
		moveNSTextField (platformControl, this);
		return;
	}
	#endif
	
	#if MAC
	if (platformControl)
	{
		HIViewMoveBy ((HIViewRef)platformControl, newSize.left - size.left, newSize.top - size.top);
		if (newSize.getWidth () != size.getWidth () || newSize.getHeight () != size.getHeight ())
		{
			HIRect r;
			HIViewGetFrame ((HIViewRef)platformControl, &r);
			r.size.width = newSize.getWidth ();
			r.size.height = newSize.getHeight ();
			HIViewSetFrame ((HIViewRef)platformControl, &r);
		}
	}
	#endif
	CView::setViewSize (newSize, invalid);
}

//------------------------------------------------------------------------
void CTextEdit::takeFocus ()
{
	if (platformControl)
		return;

	bWasReturnPressed = false;

#if WINDOWS || MAC
	// calculate offset for CViewContainers
	CRect rect (size);
	CView* parent = getParentView ();
	CRect vSize;
	while (parent)
	{
		if (parent->notify (this, kMsgCheckIfViewContainer) == kMessageNotified)
		{
			parent->getViewSize (vSize);
			rect.offset (vSize.left, vSize.top);
		}
		parent = parent->getParentView ();
	}
	if (getFrame ())
	{
		CDrawContext* pContext = getFrame ()->createDrawContext ();
		rect.offset (pContext->offset.h, pContext->offset.v);
		pContext->forget ();
	}
#endif

#if WINDOWS
	int wstyle = 0;
	if (horiTxtAlign == kLeftText)
		wstyle |= ES_LEFT;
	else if (horiTxtAlign == kRightText)
		wstyle |= ES_RIGHT;
	else
		wstyle |= ES_CENTER;

	UTF8StringHelper stringHelper (text);

	wstyle |= WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	platformControl = (void*)CreateWindowEx (0,
		TEXT("EDIT"), stringHelper, wstyle,
		(int)rect.left, (int)rect.top, (int)rect.width () + 1, (int)rect.height () + 1,
		(HWND)getFrame ()->getSystemWindow (), NULL, GetInstance (), 0);

	// get/set the current font
	LOGFONT logfont = {0};

	CCoord fontH = fontID->getSize ();
	if (fontH > rect.height () - 2)
		fontH = rect.height () - 2;

	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = (LONG)-fontH;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	UTF8StringHelper fontNameHelper (fontID->getName ());
	VSTGUI_STRCPY (logfont.lfFaceName, fontNameHelper);

	logfont.lfClipPrecision	 = CLIP_STROKE_PRECIS;
	logfont.lfOutPrecision	 = OUT_STRING_PRECIS;
	logfont.lfQuality 	     = DEFAULT_QUALITY;
	logfont.lfCharSet        = ANSI_CHARSET;
  
	platformFont = (HANDLE)CreateFontIndirect (&logfont);
	platformFontColor = 0;

	SetWindowLongPtr ((HWND)platformControl, GWLP_USERDATA, (LONG_PTR)this);
	SendMessage ((HWND)platformControl, WM_SETFONT, (WPARAM)platformFont, true);
	SendMessage ((HWND)platformControl, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG (0, 0));
	SendMessage ((HWND)platformControl, EM_SETSEL, 0, -1);
	SendMessage ((HWND)platformControl, EM_LIMITTEXT, 255, 0);
	SetFocus ((HWND)platformControl);

	oldWndProcEdit = (WINDOWSPROC)SetWindowLongPtr ((HWND)platformControl, GWLP_WNDPROC, (LONG_PTR)WindowProcEdit);
#endif // WINDOWS

#if MAC_COCOA
	if (getFrame ()->getNSView ())
	{
		platformControl = addNSTextField (getFrame (), this);
		return;
	}
#endif // MAC_COCOA

#if MAC
	extern bool hiToolboxAllowFocusChange;
	bool oldState = hiToolboxAllowFocusChange;
	hiToolboxAllowFocusChange = false;
	
	WindowRef window = (WindowRef)getFrame ()->getSystemWindow ();

	extern bool isWindowComposited (WindowRef window);
	if (!isWindowComposited (window))
	{
		HIRect hiRect;
		HIViewGetFrame ((HIViewRef)getFrame ()->getPlatformControl (), &hiRect);
		rect.offset ((CCoord)hiRect.origin.x, (CCoord)hiRect.origin.y);
	}
	Rect r;
	r.left   = (short)rect.left;// + 2;
	r.right  = (short)rect.right;// - 4;
	r.top    = (short)rect.top;// + 2;
	r.bottom = (short)rect.bottom;// - 4;
	if (rect.getHeight () > fontID->getSize ())
	{
		r.top = (short)(rect.top + rect.getHeight () / 2 - fontID->getSize () / 2 + 1);
		r.bottom = (short)(r.top + fontID->getSize ());
	}
	HIViewRef textControl = 0;
	if (CreateEditUnicodeTextControl (NULL, &r, NULL, false, NULL, &textControl) == noErr)
	{
		HIViewAddSubview ((HIViewRef)getFrame ()->getPlatformControl (), textControl);
		EventTypeSpec eventTypes[] = { { kEventClassWindow, kEventWindowDeactivated }, { kEventClassKeyboard, kEventRawKeyDown }, { kEventClassKeyboard, kEventRawKeyRepeat }, { kEventClassControl, kEventControlDraw } };
		InstallControlEventHandler (textControl, CarbonEventsTextControlProc, GetEventTypeCount (eventTypes), eventTypes, this, &gTextEditEventHandler);
		platformControl = textControl;
		if (strlen (text) > 0)
		{
			CFStringRef textString = CFStringCreateWithCString (NULL, text, kCFStringEncodingUTF8);
			if (textString)
			{
				SetControlData (textControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &textString);
				CFRelease (textString);
			}
			ControlEditTextSelectionRec selection;
			selection.selStart = 0;
			selection.selEnd = strlen (text);
			SetControlData (textControl, kControlEditTextPart, kControlEditTextSelectionTag, sizeof (ControlEditTextSelectionRec), &selection);
		}
		Boolean singleLineStyle = true;
		SetControlData (textControl, kControlEditTextPart, kControlEditTextSingleLineTag, sizeof (Boolean), &singleLineStyle);
		ControlFontStyleRec fontStyle;
		memset (&fontStyle, 0, sizeof (fontStyle));
		fontStyle.flags = kControlUseJustMask | kControlUseSizeMask | kControlUseFontMask;
		switch (horiTxtAlign)
		{
			case kLeftText: fontStyle.just = teFlushLeft; break;
			case kRightText: fontStyle.just = teFlushRight; break;
			default: fontStyle.just = teCenter; break;
		}
		fontStyle.size = fontID->getSize ();
		Str255 fontName;
		CopyCStringToPascal (fontID->getName (), fontName); 
		GetFNum (fontName, &fontStyle.font);
		SetControlData (textControl, kControlEditTextPart, kControlFontStyleTag, sizeof (fontStyle), &fontStyle);
		HIViewSetVisible (textControl, true);
		HIViewAdvanceFocus (textControl, 0);
		SetKeyboardFocus ((WindowRef)getFrame ()->getSystemWindow (), textControl, kControlEditTextPart);
		SetUserFocusWindow ((WindowRef)getFrame ()->getSystemWindow ());
	}
	hiToolboxAllowFocusChange = oldState;

#endif // MAC
}

//------------------------------------------------------------------------
void CTextEdit::looseFocus ()
{
	// Call this yet to avoid recursive call
	if (getFrame () && getFrame ()->getFocusView () == this)
		getFrame ()->setFocusView (0);

	if (platformControl == 0)
		return;

	endEdit();

	char oldText[256];
	strcpy (oldText, text);
	
#if WINDOWS
	TCHAR newText[255];
	GetWindowText ((HWND)platformControl, newText, 255);
	UTF8StringHelper windowText (newText);
	strcpy (text, windowText);

	HWND _control = (HWND)platformControl;
	platformControl = 0;	// DestroyWindow will also trigger a looseFocus call, so make sure we didn't get here again.
	DestroyWindow (_control);
	if (platformFont)
	{
		DeleteObject ((HGDIOBJ)platformFont);
		platformFont = 0;
	}
	if (platformFontColor)
	{
		DeleteObject (platformFontColor);
		platformFontColor = 0;
	}
#endif // WINDOWS

#if MAC_COCOA
	if (getFrame ()->getNSView ())
	{
		getNSTextFieldText(platformControl, text, 255);
		removeNSTextField (platformControl);
	}
	#if MAC
	else
	{
	#endif
#endif

#if MAC

	if (platformControl == 0)
		return;

	if (gTextEditEventHandler)
		RemoveEventHandler (gTextEditEventHandler);
	gTextEditEventHandler = 0;

	if (platformControl)
	{
		CFStringRef cfstr;
		if (!gTextEditCanceled && GetControlData ((HIViewRef)platformControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof cfstr, (void*)&cfstr, NULL) == noErr)
		{
			CFStringGetCString (cfstr, text, 255, kCFStringEncodingUTF8);
			CFRelease (cfstr);
		}
		HIViewSetVisible ((HIViewRef)platformControl, false);
		HIViewRemoveFromSuperview ((HIViewRef)platformControl);
		if (pParentFrame)
			pParentFrame->setCursor (kCursorDefault);
		SetUserFocusWindow (kUserFocusAuto);
	}
	#if MAC_COCOA
	}
	#endif
#endif // MAC

	platformControl = 0;

	// update dependency
	bool change = false;
	if (strcmp (oldText, text))
	{
		change = true;
		if (listener)
			listener->valueChanged (this);
	}

	// if you want to destroy the text edit do it with the loose focus message
	CView* receiver = pParentView ? pParentView : pParentFrame;
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
}

//------------------------------------------------------------------------
void CTextEdit::setTextEditConvert (void (*convert) (char *input, char *string))
{
	editConvert = convert;
}

//------------------------------------------------------------------------
void CTextEdit::setTextEditConvert (void (*convert) (char *input, char *string,
									  void *userDta), void *userData)
{
	editConvert2 = convert;
	this->userData = userData;
}

//------------------------------------------------------------------------
// COptionMenuScheme
//------------------------------------------------------------------------
/*! @class COptionMenuScheme
Used to define the appearance (font color, background color...) of a popup-menu.
To define the scheme of a menu, use the appropriate setScheme method (see COptionMenu).
@section coptionmenuscheme_new_in_3_0 New since 3.0
You can also use the global variable gOptionMenuScheme to use one scheme on all menus.
@section coptionmenuscheme_note Note
If you want to use it on Mac OS X, you must set the macro MAC_ENABLE_MENU_SCHEME (needs Mac OS X 10.3 or higher)
*/
COptionMenuScheme* gOptionMenuScheme = 0;

//------------------------------------------------------------------------
COptionMenuScheme::COptionMenuScheme ()
{
#if WINDOWS
	COLORREF c = GetSysColor (COLOR_MENU);
	backgroundColor (GetRValue (c), GetGValue (c), GetBValue (c), 0);
	c = GetSysColor (COLOR_HIGHLIGHT);
	selectionColor (GetRValue (c), GetGValue (c), GetBValue (c), 0);
	c = GetSysColor (COLOR_MENUTEXT);
	textColor (GetRValue (c), GetGValue (c), GetBValue (c), 0);
	c = GetSysColor (COLOR_HIGHLIGHTTEXT);
	hiliteTextColor (GetRValue (c), GetGValue (c), GetBValue (c), 0);
	c = GetSysColor (COLOR_GRAYTEXT);
	disableTextColor (GetRValue (c), GetGValue (c), GetBValue (c), 0);
#else
	backgroundColor = kGreyCColor;
	selectionColor = kBlueCColor;
	textColor = kBlackCColor;
	hiliteTextColor = kWhiteCColor;
	disableTextColor = kWhiteCColor;
#endif
	font = kNormalFontSmall; font->remember ();
	#if MAC_ENABLE_MENU_SCHEME
	registerWithToolbox ();
	#endif
}

//------------------------------------------------------------------------
COptionMenuScheme::~COptionMenuScheme ()
{
	if (font)
		font->forget ();
	#if MAC_ENABLE_MENU_SCHEME
	unregisterWithToolbox ();
	#endif
}

//------------------------------------------------------------------------
void COptionMenuScheme::getItemSize (const char* text, CDrawContext* pContext, CPoint& size)
{
	if (!strcmp (text, kMenuSeparator)) // separator
	{
		#if MAC
		size.h = 6;
		size.v = 9;
		#else
		// was: size.h = size.v = 6;
		size.h = 6;
		size.v = 18;
		// separators must have same height, otherwise we have problems
		// in multi-column menus :(
		#endif
	}
	else
	{
		pContext->setFont (font);
		size.h = pContext->getStringWidth (text) + 18;
		size.v = 18;
		#if MAC
		size.h += 18;
		#endif
	}
}

//------------------------------------------------------------------------
void COptionMenuScheme::drawItemBack (CDrawContext* pContext, const CRect& rect, bool hilite)
{
	if (hilite)
		pContext->setFillColor (selectionColor);
	else
		pContext->setFillColor (backgroundColor);
	pContext->drawRect (rect, kDrawFilled);
}

//------------------------------------------------------------------------
void COptionMenuScheme::drawItem (const char* text, long itemId, long state, CDrawContext* pContext, const CRect& rect)
{	
	bool hilite = (state & kSelected) != 0;
	
	drawItemBack (pContext, rect, hilite);

	if (!strcmp (text, kMenuSeparator))
	{
		CCoord y = rect.top + rect.height () / 2;

		const CColor bc = { 0, 0, 0, 150};
		const CColor wc = { 255, 255, 255, 150};
		
		pContext->setFrameColor (bc);
		pContext->moveTo (CPoint (rect.left + 2, y - 1));
		pContext->lineTo (CPoint (rect.right - 2, y - 1));
		pContext->setFrameColor (wc);
		pContext->moveTo (CPoint (rect.left + 2, y));
		pContext->lineTo (CPoint (rect.right - 2, y));
		return;
	}

	CRect r;
	if (state & kChecked)
	{		
		r (6, 4, 14, 12);
		r.offset (rect.left, rect.top);
		if (hilite)
			pContext->setFillColor (hiliteTextColor);
		else
			pContext->setFillColor (textColor);
		pContext->drawEllipse (r, kDrawFilled);
	}

	r = rect;
	r.left += 18;
	pContext->setFont (font);
	if (state & kDisabled)
		pContext->setFontColor (disableTextColor);
	else
	{
		if (hilite)
			pContext->setFontColor (hiliteTextColor);
		else
			pContext->setFontColor (textColor);
	}

	// this needs to be done right, without changing the text pointer in anyway ;-)
	const char *ptr = strstr (text, "\t");
	if (ptr)
	{
		char* tmpText = (char*)malloc ((text - ptr) + 1);
		strncpy (tmpText, text, (text - ptr));
		pContext->drawString (tmpText, r, false, kLeftText);
		free (tmpText);

		char modifier[32];
		strcpy (modifier, ptr + 1);

		r.left = r.right - 50;
		pContext->drawString (modifier, r, false, kLeftText);
	}
	else
		pContext->drawString (text, r, false, kLeftText);
}

#if MAC_ENABLE_MENU_SCHEME
#define kHIMenuViewClassID              CFSTR("com.apple.HIMenuView")

struct HIMenuScheme
{
	HIViewRef hiView;
	COptionMenuScheme* scheme;
	COptionMenu* menu;
	COffscreenContext* offscreenContext;
	float maxWidth;
	HIPoint origin;
};

const EventParamName kEventParamCOptionMenuScheme = 'COMS';
const EventParamName kEventParamCOptionMenu = 'COM ';

#define kItemHeight		18

static CFStringRef gOptionMenuSchemeClassID = 0;
static HIObjectClassRef gMenuClassRef = 0;
static CFIndex gOptionMenuSchemeClassIDRefCount = 0;

//------------------------------------------------------------------------
void COptionMenuScheme::registerWithToolbox ()
{
	if (gOptionMenuSchemeClassID == 0)
	{
		gOptionMenuSchemeClassID = CFStringCreateWithFormat (NULL, NULL, CFSTR("net.sourceforge.vstgui.COptionMenuSchemeClassID.%d"), this);

		static const EventTypeSpec events[] =
		{
			{ kEventClassHIObject, kEventHIObjectConstruct } ,
			{ kEventClassHIObject, kEventHIObjectInitialize },
			{ kEventClassHIObject, kEventHIObjectDestruct },

			{ kEventClassControl, kEventControlHitTest },
			{ kEventClassControl, kEventControlGetPartRegion },
			{ kEventClassControl, kEventControlDraw },
			{ kEventClassControl, kEventControlGetOptimalBounds },

			{ kEventClassMenu, kEventMenuCreateFrameView },

			{ kEventClassScrollable, kEventScrollableGetInfo },
			{ kEventClassScrollable, kEventScrollableScrollTo },
		};

		HIObjectRegisterSubclass (	gOptionMenuSchemeClassID,
									kHIMenuViewClassID,
									kNilOptions,
									COptionMenuScheme::eventHandler,
									GetEventTypeCount (events),
									events,
									NULL,
									&gMenuClassRef);
		gOptionMenuSchemeClassIDRefCount = CFGetRetainCount (gOptionMenuSchemeClassID);
	}
	else
		CFRetain (gOptionMenuSchemeClassID);
}

//------------------------------------------------------------------------
void COptionMenuScheme::unregisterWithToolbox ()
{
	if (gOptionMenuSchemeClassID)
	{
		if (CFGetRetainCount (gOptionMenuSchemeClassID) == gOptionMenuSchemeClassIDRefCount)
		{
			HIObjectUnregisterClass (gMenuClassRef);
			gMenuClassRef = 0;
		}
		CFRelease (gOptionMenuSchemeClassID);
	}
}

//------------------------------------------------------------------------
pascal OSStatus COptionMenuScheme::eventHandler (EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus err = eventNotHandledErr;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	HIMenuScheme* scheme = (HIMenuScheme*)inUserData;
	
	switch (eventClass)
	{
		case kEventClassHIObject:
		{
			switch (eventKind)
			{
				case kEventHIObjectConstruct:
				{
					scheme = (HIMenuScheme*)calloc (1, sizeof (HIMenuScheme));
					GetEventParameter (inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof (HIObjectRef), NULL, &scheme->hiView);
					SetEventParameter (inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof (HIMenuScheme*), &scheme);
					scheme->maxWidth = 100;
					scheme->origin.x = 0;
					scheme->origin.y = 0;
					err = noErr;
					break;
				}
				case kEventHIObjectInitialize:
				{
					err = CallNextEventHandler (inCallRef, inEvent);
					if (err == noErr)
					{
						GetEventParameter (inEvent, kEventParamCOptionMenuScheme, typeVoidPtr, NULL, sizeof (COptionMenuScheme*), NULL, &scheme->scheme);
						GetEventParameter (inEvent, kEventParamCOptionMenu, typeVoidPtr, NULL, sizeof (COptionMenu*), NULL, &scheme->menu);
						scheme->scheme->remember ();
						scheme->menu->remember ();
						scheme->offscreenContext = new COffscreenContext (scheme->menu->getFrame (), 600, 100);
					}
					break;
				}
				case kEventHIObjectDestruct:
				{
					delete scheme->offscreenContext;
					scheme->scheme->forget ();
					scheme->menu->forget ();
					free (scheme);
					err = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassControl:
		{
			HIViewRef control;
			GetEventParameter (inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof (HIViewRef), NULL, &control);
			switch (eventKind)
			{
				case kEventControlInitialize:
				{
					err = CallNextEventHandler (inCallRef, inEvent);
					break;
				}
				case kEventControlHitTest:
				{
					HIPoint mouseLoc;
					GetEventParameter (inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof (mouseLoc), NULL, &mouseLoc);
					ControlPartCode partHit = 0;//(ControlPartCode)mouseLoc.y / kItemHeight + 1;
					char temp[1024];
					CPoint size;
					CCoord yPos = -scheme->origin.y;
					for (long i = 0; i < scheme->menu->getNbEntries (); i++)
					{
						scheme->menu->getEntry (i, temp);
						scheme->scheme->getItemSize (temp, scheme->offscreenContext, size);
						yPos += size.y;
						if (yPos >= mouseLoc.y)
						{
							partHit = i + 1;
							break;
						}
					}
					SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (partHit), &partHit);
					err = noErr;
					break;
				}
				case kEventControlGetOptimalBounds:
				{
					HIRect r = { {0, 0}, { 0, 0 }};
					r.size.width = scheme->maxWidth;
					char temp[1024];
					CPoint size;
					for (long i = 0; i < scheme->menu->getNbEntries (); i++)
					{
						scheme->menu->getEntry (i, temp);
						scheme->scheme->getItemSize (temp, scheme->offscreenContext, size);
						if (!strncmp (temp, kMenuSubMenu, 2))
							size.x += 16;
						r.size.height += size.y;
						if (r.size.width < size.x)
							r.size.width = size.x;
					}
					//r.size.height += scheme->menu->getNbEntries ();
					scheme->maxWidth = r.size.width;
					SetEventParameter (inEvent, kEventParamControlOptimalBounds, typeHIRect, sizeof (HIRect), &r);
					err = noErr;
					break;
				}
				case kEventControlGetPartRegion:
				{
					HIRect r = { {-scheme->origin.x, -scheme->origin.y}, { 0, 0 }};
					ControlPartCode whichItem;
					RgnHandle outRegion = NULL;
					GetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof (whichItem), NULL, &whichItem);
					GetEventParameter (inEvent, kEventParamControlRegion, typeQDRgnHandle, NULL, sizeof(outRegion), NULL, &outRegion);
					if (whichItem <= 0)
					{
						r.origin.x = r.origin.y = 0.f;
						r.size.width = scheme->maxWidth;
						char temp[1024];
						CPoint size;
						for (long i = 0; i < scheme->menu->getNbEntries (); i++)
						{
							scheme->menu->getEntry (i, temp);
							scheme->scheme->getItemSize (temp, scheme->offscreenContext, size);
							if (!strncmp (temp, kMenuSubMenu, 2))
								size.x += 16;
							r.size.height += size.y;
							if (r.size.width < size.x)
								r.size.width = size.x;
						}
						scheme->maxWidth = r.size.width;
						r.size.height += scheme->menu->getNbEntries ();
					}
					else
					{
						char temp[1024];
						CPoint size;
						for (long i = 0; i < whichItem; i++)
						{
							r.origin.y += size.y;
							scheme->menu->getEntry (i, temp);
							scheme->scheme->getItemSize (temp, scheme->offscreenContext, size);
							r.size.height += size.y;
						}
						r.size.width = scheme->maxWidth;
					}
					SetRectRgn (outRegion, (short) r.origin.x, (short) r.origin.y,(short) (r.origin.x + r.size.width), (short) (r.origin.y+ r.size.height + 1));
					err = noErr;
					break;
				}
				case kEventControlDraw:
				{
					CGContextRef cgContext;
					GetEventParameter (inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (cgContext), NULL, &cgContext);
					HIRect r;
					HIViewGetBounds (control, &r);
					CGContextClearRect (cgContext, r);
					if (control != scheme->hiView)
					{
						err = noErr;
						break;
					}
					ControlPartCode focusPart;
					HIViewGetFocusPart (scheme->hiView, &focusPart);
					focusPart--;
					WindowRef window = HIViewGetWindow (scheme->hiView);
					CDrawContext context (NULL, cgContext, window);
					char entryText[1024];
					CPoint size;
					CRect drawingRect (r.origin.x, r.origin.y, r.origin.x + r.size.width, r.origin.y + r.size.height); 
					CRect rect (-scheme->origin.x, -scheme->origin.y);
					rect.setHeight (kItemHeight);
					rect.setWidth (scheme->maxWidth);
					for (int i = 0; i < scheme->menu->getNbEntries (); i++)
					{
						if (scheme->menu->getEntry (i, entryText))
						{
							scheme->scheme->getItemSize (entryText, &context, size);
							long state = scheme->menu->isCheckEntry (i) ? kChecked : 0;
							if (focusPart >= 0 && focusPart == i)
								state |= kSelected;
							long offset = 0;
							if (!strncmp (entryText, kMenuSubMenu, 2))
							{
								state |= kSubMenu;
								offset = 2;
							}
							else if (!strncmp (entryText, kMenuTitle, 2))
							{
								state |= kTitle;
								offset = 2;
							}
							else if (!strncmp (entryText, kMenuDisable, 2))
							{
								state |= kDisabled;
								offset = 2;
							}
							rect.setHeight (size.y+1);
							CRect clip (rect);
							clip.bound (drawingRect);
							context.setClipRect (clip);
							scheme->scheme->drawItem (entryText+offset, i, state, &context, rect);
							rect.offset (0, size.y);
						}
					}
					err = noErr;
					break;
				}
				case kEventControlGetFrameMetrics:
				{
					#if 1
					HIViewRef contentView = 0;
					HIViewFindByID (scheme->hiView, kHIViewWindowContentID, &contentView);
					if (contentView)
						err = SendEventToEventTargetWithOptions (inEvent, GetControlEventTarget (contentView), kEventTargetDontPropagate);
					#else
					err = CallNextEventHandler (inCallRef, inEvent);
					HIViewFrameMetrics	metrics;
					GetEventParameter (inEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, NULL, sizeof (metrics), NULL, &metrics);
					metrics.top = metrics.bottom = metrics.left = metrics.right = 0;
					SetEventParameter (inEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, sizeof (metrics), &metrics);
					#endif
					break;
				}
				case kEventControlOwningWindowChanged:
				{
					err = CallNextEventHandler (inCallRef, inEvent);
					WindowRef newWindow = GetControlOwner (control);
					HIWindowChangeFeatures (newWindow, 0, kWindowIsOpaque);
					HIViewRef root = HIViewGetRoot (newWindow);
					if (root)
					{
						HIRect bounds, frame;
						HIViewGetBounds (root, &bounds);
						HIViewGetFrame (root, &frame);
					}
					break;
				}
				case kEventControlAddedSubControl:
				{
					HIViewRef subControl = 0;
					GetEventParameter (inEvent, kEventParamControlSubControl, typeControlRef, NULL, sizeof (subControl), NULL, &subControl);
					if (subControl && subControl != scheme->hiView)
					{
						HIViewRemoveFromSuperview (subControl);
						CFRelease (subControl);
					}
					break;
				}
			}
			break;
		}
		case kEventClassMenu:
		{
			switch (eventKind)
			{
				case kEventMenuCreateFrameView:
				{
					err = CallNextEventHandler (inCallRef, inEvent);
					HIViewRef frameView;
					GetEventParameter (inEvent, kEventParamMenuFrameView, typeControlRef, NULL, sizeof (ControlRef), NULL, &frameView);
					HIViewFindByID (frameView, kHIViewWindowContentID, &frameView);
					if (frameView)
					{
						EventTypeSpec events [] = { 
							{ kEventClassControl, kEventControlDraw }, 
							{ kEventClassControl, kEventControlOwningWindowChanged }, 
							{ kEventClassControl, kEventControlGetFrameMetrics },
							{ kEventClassControl, kEventControlAddedSubControl }
						};
						InstallControlEventHandler (frameView, COptionMenuScheme::eventHandler, GetEventTypeCount (events), events, scheme, NULL);
					}
					break;
				}
			}
			break;
		}
		case kEventClassScrollable:
		{
			switch (eventKind)
			{
				case kEventScrollableScrollTo:
				{
					err = CallNextEventHandler (inCallRef, inEvent);
					GetEventParameter (inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof (scheme->origin), NULL, &scheme->origin);
					if (scheme->origin.x < 0.f)
						scheme->origin.x = 0.f;
					if (scheme->origin.y < 0.f)
						scheme->origin.y = 0.f;
					printf ("x:%d, y:%d\n", (int)scheme->origin.x, (int)scheme->origin.y);
					break;
				}
				case kEventScrollableGetInfo:
				{
					err = CallNextEventHandler (inCallRef, inEvent);

					HIPoint origin = { 0, 0 };
					HIRect r = { {0, 0}, { 0, 0 }};
					r.size.width = scheme->maxWidth;
					char temp[1024];
					CCoord maxHeight = 0;
					CPoint size;
					for (long i = 0; i < scheme->menu->getNbEntries (); i++)
					{
						scheme->menu->getEntry (i, temp);
						scheme->scheme->getItemSize (temp, scheme->offscreenContext, size);
						if (!strncmp (temp, kMenuSubMenu, 2))
							size.x += 16;
						r.size.height += size.y;
						if (r.size.width < size.x)
							r.size.width = size.x;
						if (maxHeight < size.y)
							maxHeight = size.y;
					}
					scheme->maxWidth = r.size.width;

					SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof( size ), &r.size );
					HIViewGetBounds (scheme->hiView, &r);
					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof( size ), &r.size );
					SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof( origin ), &scheme->origin );

					r.size.height = maxHeight;
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof( size ), &r.size );

					err = noErr;
					break;
				}
			}
			break;
		}
	}
	
	return err;
}
#endif

//------------------------------------------------------------------------
// COptionMenu
//------------------------------------------------------------------------
/*! @class COptionMenu
Define a rectangle view where a text-value can be displayed with a given font and color.
The text-value is centered in the given rect.
A bitmap can be used as background, a second bitmap can be used when the option menu is popuped.
There are 2 styles with or without a shadowed text. When a mouse click occurs, a popup menu is displayed.
*/
//------------------------------------------------------------------------
/**
 * COptionMenu constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the background bitmap
 * @param bgWhenClick the background bitmap if the option menu is displayed
 * @param style the style of the display (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
COptionMenu::COptionMenu (const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* bgWhenClick, const long style)
: CParamDisplay (size, background, style)
, bgWhenClick (bgWhenClick)
, nbItemsPerColumn (-1)
, prefixNumbers (0)
, scheme (0)
{
	this->listener = listener;
	this->tag = tag;

	nbEntries = 0;
	nbSubMenus = 0;
	currentIndex = -1;
	lastButton = kRButton;
	platformControl = 0;
	lastResult = -1;
	lastMenu = 0;

	#if MAC
	menuID = 0;
	#endif
	
	if (bgWhenClick)
		bgWhenClick->remember ();

	nbSubMenuAllocated = nbAllocated = 0;
	
	check = 0;
	entry = 0;
	submenuEntry = 0;
}

//------------------------------------------------------------------------
COptionMenu::COptionMenu (const COptionMenu& v)
: CParamDisplay (v)
, platformControl (0)
, entry (0)
, submenuEntry (0)
, check (0)
, nbEntries (0)
, nbSubMenus (0)
, currentIndex (-1)
, bgWhenClick (v.bgWhenClick)
, lastButton (kRButton)
, nbItemsPerColumn (v.nbItemsPerColumn)
, nbAllocated (0)
, nbSubMenuAllocated (0)
, lastResult (-1)
, prefixNumbers (0)
, lastMenu (0)
, scheme (v.scheme)
{
	if (bgWhenClick)
		bgWhenClick->remember ();

	// todo copy items and submenues
}

//------------------------------------------------------------------------
COptionMenu::~COptionMenu ()
{
	removeAllEntry ();

	if (bgWhenClick)
		bgWhenClick->forget ();
}

//------------------------------------------------------------------------
void COptionMenu::setPrefixNumbers (long preCount)
{
	prefixNumbers = preCount;
}

//-----------------------------------------------------------------------------
bool COptionMenu::allocateSubMenu (long nb)
{
	long newAllocated = nbSubMenuAllocated + nb;

	if (submenuEntry)
		submenuEntry = (COptionMenu**)realloc (submenuEntry, newAllocated * sizeof (COptionMenu*));
	else
		submenuEntry = (COptionMenu**)malloc (newAllocated * sizeof (COptionMenu*));

	long i;
	for (i = nbSubMenuAllocated; i < newAllocated; i++)
		submenuEntry[i] = 0;

	nbSubMenuAllocated = newAllocated;

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::allocateMenu (long nb)
{
	long newAllocated = nbAllocated + nb;

	if (check)
		check = (bool*)realloc (check, newAllocated * sizeof (bool));
	else
		check = (bool*)malloc (newAllocated * sizeof (bool));
	if (!check)
		return false;

	if (entry)
		entry = (char**)realloc (entry, newAllocated * sizeof (char*));
	else
		entry = (char**)malloc (newAllocated * sizeof (char*));
	if (!entry)
	{
		free (check);
		return false;
	}

	long i;
	for (i = nbAllocated; i < newAllocated; i++)
	{
		check[i] = false;
		entry[i] = 0;
	}

	nbAllocated = newAllocated;

	return true;
}

//------------------------------------------------------------------------
COptionMenu* COptionMenu::getSubMenu (long idx) const
{
	if (submenuEntry && idx < nbSubMenus)
		return submenuEntry[idx];
	return 0;
}

//------------------------------------------------------------------------
bool COptionMenu::addEntry (COptionMenu *subMenu, const char *txt)
{
	if (nbEntries >= MAX_ENTRY || !subMenu || !txt)
		return false;

	if (nbEntries >= nbAllocated)
		if (!allocateMenu (32))
			return false;

	entry[nbEntries] = (char*)malloc (256);
	switch (prefixNumbers)
	{
		case 2:
			sprintf (entry[nbEntries], "-M%1d %s", (int)(nbEntries + 1), txt);
			break;

		case 3:
			sprintf (entry[nbEntries], "-M%02d %s", (int)(nbEntries + 1), txt);
			break;

		case 4:
			sprintf (entry[nbEntries], "-M%03d %s", (int)(nbEntries + 1), txt);
			break;

		default:
			sprintf (entry[nbEntries], "-M%s", txt);
	}


	if (nbSubMenus >= nbSubMenuAllocated)
		if (!allocateSubMenu (10))
			return false;

	submenuEntry[nbSubMenus++] = subMenu;
	subMenu->remember ();

	nbEntries++;

	if (currentIndex < 0)
		currentIndex = 0;
	
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::addEntry (const char *txt, long index)
{
	if (nbEntries >= MAX_ENTRY)
		return false;
	
	if (nbEntries >= nbAllocated)
		if (!allocateMenu (32))
			return false;
		
	entry[nbEntries] = (char*)malloc (256);

	long pos = nbEntries;

	// switch the entries for the insert
	if (index >= 0)
	{
		for (long i = nbEntries; i > index; i--)
			strcpy (entry[i], entry[i - 1]);
		if (index >= nbEntries)
			pos = nbEntries;
		else
			pos = index;	
		if (currentIndex >= index)
			currentIndex++;	
	}

	*entry[pos] = 0;
	if (txt)
	{
		switch (prefixNumbers)
		{
			case 2:
				sprintf (entry[pos], "%1d %s", (int)(index + 1), txt);
				break;

			case 3:
				sprintf (entry[pos], "%02d %s", (int)(index + 1), txt);
				break;

			case 4:
				sprintf (entry[pos], "%03d %s", (int)(index + 1), txt);
				break;

			default:
				strncpy (entry[pos], txt, 256);
		}
	}
	
	nbEntries++;
	
	if (currentIndex < 0)
		currentIndex = 0;
	
	return true;
}

//------------------------------------------------------------------------
long COptionMenu::getCurrent (char *txt, bool countSeparator) const
{
	if (currentIndex < 0)
		return -1;

	long result = 0;
	
	if (countSeparator)
	{
		if (txt)
			strcpy (txt, entry[currentIndex]);
		result = currentIndex;
	}
	else
	{
		for (long i = 0; i < currentIndex; i++)
		{
			if (strcmp (entry[i], kMenuSeparator) && strncmp (entry[i], kMenuTitle, 2))
				result++;
		}
		if (txt)
			strcpy (txt, entry[currentIndex]);
	}
	return result;
}

//------------------------------------------------------------------------
bool COptionMenu::setCurrent (long index, bool countSeparator)
{
	if (index < 0 || index >= nbEntries)
		return false;

	if (countSeparator)
	{
		if (!strcmp (entry[index], kMenuSeparator) && strncmp (entry[index], kMenuTitle, 2))
			return false;

		currentIndex = index;
	}
	else
	{
		long newCurrent = 0;
		long i = 0;
		while (i <= index && newCurrent < nbEntries)
		{
			if (strcmp (entry[newCurrent], kMenuSeparator) && strncmp (entry[newCurrent], kMenuTitle, 2))
				i++;
			newCurrent++;
		}
		currentIndex = newCurrent - 1;
	}
	if (style & (kMultipleCheckStyle & ~kCheckStyle))
		check[currentIndex] = !check[currentIndex];

	// to force the redraw
	setDirty ();

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::getEntry (long index, char *txt) const
{
	if (index < 0 || index >= nbEntries)
		return false;

	if (txt)
		strcpy (txt, entry[index]);
	return true;
}

//------------------------------------------------------------------------
const char* COptionMenu::getEntry (long index) const
{
	if (index < 0 || index >= nbEntries)
		return 0;
	return entry[index];
}

//------------------------------------------------------------------------
bool COptionMenu::setEntry (long index, char *txt)
{
	if (index < 0 || index >= nbEntries)
		return false;

	if (txt)
		strcpy (entry[index], txt);
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::removeEntry (long index)
{
	if (index < 0 || index >= nbEntries)
		return false;
	
	nbEntries--;

	// switch the entries
	for (long i = index; i < nbEntries; i++)
	{
		strcpy (entry[i], entry[i + 1]);
		check[i] = check [i + 1];
	}

	if (currentIndex >= index)
		currentIndex--;

	// delete the last one
	free (entry[nbEntries]);
	entry[nbEntries] = 0;
	check[nbEntries] = false;

	if (nbEntries == 0)
		currentIndex = -1;
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::removeAllEntry ()
{
	long i;
	for (i = 0; i < nbEntries; i++)
	{
		free (entry[i]);
		entry[i] = 0;
		check[i] = false;
	}

	nbEntries = 0;
	currentIndex = -1;

	for (i = 0; i < nbSubMenus; i++)
	{
		submenuEntry[i]->forget ();
		submenuEntry[i] = 0;
	}
	nbSubMenus = 0;

	if (check)
		free (check);
	check = 0;
	if (entry)
		free (entry);
	entry = 0;
	if (submenuEntry)
		free (submenuEntry);
	submenuEntry = 0;
	nbAllocated = 0;
	nbSubMenuAllocated = 0;

	return true;
}

//------------------------------------------------------------------------
long COptionMenu::getIndex (char *txt) const
{
	if (!txt)
		return -1;

	// search entries
	for (long i = 0; i < nbEntries; i++)
		if (!strcmp (entry[i], txt))
			return i;

	// not found
	return -1;
}

//------------------------------------------------------------------------
bool COptionMenu::checkEntry (long index, bool state)
{
	if (index < 0 || index >= nbEntries)
		return false;

	check[index] = state;

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::checkEntryAlone (long index)
{
	if (index < 0 || index >= nbEntries)
		return false;
	for (long i = 0; i < nbEntries; i++)
		check[i] = false;
	check[index] = true;

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::isCheckEntry (long index) const
{
	if (index < 0 || index >= nbEntries)
		return false;

	return check[index];
}

//------------------------------------------------------------------------
void COptionMenu::draw (CDrawContext *pContext)
{
	if (currentIndex >= 0 && nbEntries) 
		drawText (pContext, entry[currentIndex] + prefixNumbers, getFrame ()->getFocusView () == this ? bgWhenClick : 0);
	else
		drawText (pContext, NULL, getFrame ()->getFocusView () == this ? bgWhenClick : 0);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void COptionMenu::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled || !getFrame () || !pContext)
		return;

	lastButton = (button != -1) ? button : pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (lastButton & (kLButton|kRButton|kApple))
	{
		if (bgWhenClick)
		{
			char string[256];
			if (currentIndex >= 0)
				sprintf (string, "%s", entry[currentIndex]);
			else
				string[0] = 0;
		
			drawText (pContext, string, bgWhenClick);
		}

		beginEdit();
		takeFocus ();
	}
}
#endif

//------------------------------------------------------------------------
CMouseEventResult COptionMenu::onMouseDown (CPoint& where, const long& buttons)
{
	lastButton = buttons;
	if (lastButton & (kLButton|kRButton|kApple))
	{
		beginEdit ();
		if (bgWhenClick)
			invalid ();
		takeFocus ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
COptionMenu *COptionMenu::getLastItemMenu (long &idxInMenu) const
{
	idxInMenu = lastMenu ? (long)lastMenu->getValue (): -1;
	return lastMenu;
}

//------------------------------------------------------------------------
COptionMenu *COptionMenu::getItemMenu (long idx, long &idxInMenu, long &offsetIdx)
{
#if WINDOWS
	long oldIDx = offsetIdx;
	offsetIdx += nbEntries;

	if (idx < offsetIdx)
	{
		idxInMenu = idx - oldIDx;
		return this;
	}
	
#elif MAC
	if (menuID == offsetIdx)
	{
		idxInMenu = idx;
		return this;
	}
#endif
	COptionMenu *menu = 0;
	for (long i = 0; i < nbSubMenus; i++)
	{
		menu = submenuEntry[i]->getItemMenu (idx, idxInMenu, offsetIdx);
		if (menu)
			break;
	}
	return menu;
}

//------------------------------------------------------------------------
void COptionMenu::removeItems ()
{
	for (long i = 0; i < nbSubMenus; i++)
		submenuEntry[i]->removeItems ();
	
#if WINDOWS
	// destroy the menu
	if (platformControl)
		DestroyMenu ((HMENU)platformControl);
	platformControl = 0;

#elif MAC
	// destroy the menu
	if (menuID)
		DeleteMenu (menuID);
	if (platformControl)
		DisposeMenu ((MenuHandle)platformControl);
	platformControl = 0;

#endif
}

//------------------------------------------------------------------------
void *COptionMenu::appendItems (long &offsetIdx)
{
	bool multipleCheck = style & (kMultipleCheckStyle & ~kCheckStyle);

#if WINDOWS
	void *menu = (void*)CreatePopupMenu ();
	
	bool ownerDraw = (scheme != 0) || (gOptionMenuScheme != 0);

	int flags = 0;
	long idxSubmenu = 0;
	long offset = offsetIdx;
	offsetIdx += nbEntries;
	long inc = 0;
	for (long i = 0; i < nbEntries; i++)
	{
		UTF8StringHelper entryText (entry[i]);
		//---Separator-----
		if (!strcmp (entry[i], kMenuSeparator))
		{
			if (ownerDraw)
				AppendMenu ((HMENU)menu, MF_OWNERDRAW|MF_SEPARATOR, 0, entryText);
			else
				AppendMenu ((HMENU)menu, MF_SEPARATOR, 0, entryText);
		}
		else
		{
			flags = ownerDraw ? MF_OWNERDRAW : MF_STRING;
			if (nbEntries < 160 && nbItemsPerColumn > 0 && inc && !(inc % nbItemsPerColumn))
				flags |= MF_MENUBARBREAK;

			if (!strncmp (entry[i], kMenuSubMenu, 2))
			{
				if (idxSubmenu < nbSubMenus)
				{
					void *submenu = submenuEntry[idxSubmenu]->appendItems (offsetIdx);
					if (submenu)
					{
						idxSubmenu++;
						AppendMenu ((HMENU)menu, flags|MF_POPUP|MF_ENABLED, (UINT_PTR)submenu, (const TCHAR*)entryText + 2);
					}
					else
						continue;
				}
				else
					continue;
			}
			//---Disable/Gray entry-----------
			else if (!strncmp (entry[i], kMenuDisable, 2))
			{
				AppendMenu ((HMENU)menu, flags|MF_GRAYED, offset + inc, (const TCHAR*)entryText + 2);
			}
			//---Disable entry--------
			else if (!strncmp (entry[i], kMenuTitle, 2))
			{
				AppendMenu ((HMENU)menu, flags|MF_DISABLED, offset + inc, (const TCHAR*)entryText + 2);
			}
			//---Multiple Checked entry---
			else if (multipleCheck)
			{
				AppendMenu ((HMENU)menu, flags|MF_ENABLED | 
					(check[i] ? MF_CHECKED : MF_UNCHECKED), offset + inc, entryText);
			}
			//---Checked Entry---------
			else if (style & kCheckStyle)
			{
				AppendMenu ((HMENU)menu, flags|MF_ENABLED | 
					((i == currentIndex) ? MF_CHECKED : MF_UNCHECKED), offset + inc, entryText);
			}
			else
				AppendMenu ((HMENU)menu, flags|MF_ENABLED, offset + inc, entryText);
		}
		inc++;
	}
	platformControl = menu;
	return menu;

#elif MAC
	//---Get an non-existing ID for the menu:
	menuID = UniqueID ('MENU');
		
	MenuHandle theMenu = 0;
	//---Create the menu
	#if MAC_ENABLE_MENU_SCHEME
	extern long pSystemVersion;
	if ((scheme || gOptionMenuScheme) && pSystemVersion >= 0x1030)
	{
		COptionMenuScheme* s = gOptionMenuScheme ? gOptionMenuScheme : scheme;
		EventRef initEvent = NULL;
		if (CreateEvent (NULL, kEventClassHIObject, kEventHIObjectInitialize, 0, 0, &initEvent) == noErr)
		{
			MenuDefSpec customMenuDef;
			COptionMenu* optMenu = this;
			SetEventParameter (initEvent, kEventParamCOptionMenu, typeVoidPtr, sizeof(COptionMenu*), &optMenu);
			customMenuDef.defType = kMenuDefClassID;
			customMenuDef.u.view.classID = gOptionMenuSchemeClassID;
			customMenuDef.u.view.initEvent = initEvent;
			SetEventParameter (initEvent, kEventParamCOptionMenuScheme, typeVoidPtr, sizeof(COptionMenuScheme*), &s);
			CreateCustomMenu (&customMenuDef, menuID, 0, &theMenu);
			ReleaseEvent (initEvent);
			if (theMenu == NULL)
				return NULL;
		}
	}
	else
	#endif
		//theMenu = NewMenu (menuID, "\pPopUp");
		CreateNewMenu (menuID, kMenuAttrCondenseSeparators, &theMenu);
		
	char text2[256];
	long keyChar;
	long idxSubmenu = 0;
	offsetIdx += nbEntries;
	short keyModifiers;
	bool useGlyph;
	for (long i = 0; i < nbEntries; i++)
	{
		keyChar = 0;
		keyModifiers = kMenuNoModifiers;
		useGlyph = false;
		
		strcpy (text2, entry[i]);
		char *ptr = strstr (text2, "\t");
		// this does not work correctly in all cases for UTF-8
		if (ptr)
		{
			*ptr = '\0';
			ptr++;
			if (strlen (ptr) > 0)
			{
				if (!strstr (ptr, "Ctrl"))
				   	keyModifiers |= kMenuNoCommandModifier;
				if (strstr (ptr, "Alt"))
					keyModifiers |= kMenuOptionModifier;
				if (strstr (ptr, "Shift"))
					keyModifiers |= kMenuShiftModifier;
				if (strstr (ptr, "Apple"))
					keyModifiers |= kMenuControlModifier;
				
				if (!strncmp (ptr, "Del", 3))
					keyChar = 0x17;
				else if (!strncmp (ptr, "Left", 4))
				{
					keyChar = 0x64;
					useGlyph = true;
				}
				else if (!strncmp (ptr, "Right", 5))
				{
					keyChar = 0x65;
					useGlyph = true;
				}
				else if (!strncmp (ptr, "Up", 2))
				{
					keyChar = 0x68;
					useGlyph = true;
				}
				else if (!strncmp (ptr, "Down", 4))
				{
					keyChar = 0x6a;
					useGlyph = true;
				}
				else
					keyChar = (long)ptr[strlen (ptr) - 1];
			}
			else 
			{
				keyModifiers = kMenuNoCommandModifier;
				keyChar = (long)*ptr;	
			}
		}

		if (!strcmp (entry[i], kMenuSeparator))
		{
			AppendMenuItemTextWithCFString (theMenu, CFSTR(""), kMenuItemAttrSeparator, 0, NULL);
		}
		else
		{
			CFStringRef itemString = 0;
			MenuItemAttributes itemAttribs = kMenuItemAttrIgnoreMeta;
			// Submenu
			if (!strncmp (entry[i], kMenuSubMenu, 2))
			{
				if (idxSubmenu < nbSubMenus)
				{
					itemString = CFStringCreateWithCString (NULL, entry[i] + 2, kCFStringEncodingUTF8);
					InsertMenuItemTextWithCFString (theMenu, itemString, i+1, itemAttribs, 0);
					CFRelease (itemString);
					void *submenu = submenuEntry[idxSubmenu]->appendItems (offsetIdx);
					if (submenu)
					{
						SetMenuItemHierarchicalID (theMenu, i + 1, submenuEntry[idxSubmenu]->getMenuID ());
						idxSubmenu++;
						continue;
					}
					else
						continue;
				}
				else
					continue;
			}
			//---Disable/Gray entry-----------
			else if (!strncmp (entry[i], kMenuDisable, 2))
			{
				itemString = CFStringCreateWithCString (NULL, entry[i] + 2, kCFStringEncodingUTF8);
				itemAttribs |= kMenuItemAttrDisabled;
			}
			//---Disable entry--------
			else if (!strncmp (entry[i], kMenuTitle, 2))
			{
				itemString = CFStringCreateWithCString (NULL, entry[i] + 2, kCFStringEncodingUTF8);
				itemAttribs |= kMenuItemAttrSectionHeader;
			}
			else
				itemString = CFStringCreateWithCString (NULL, entry[i], kCFStringEncodingUTF8);

			InsertMenuItemTextWithCFString (theMenu, itemString, i+1, itemAttribs, 0);
			CFRelease (itemString);
		}

		//---Set the shortcut
		if (keyChar != 0)
		{
			SetItemCmd (theMenu, i + 1, keyChar);
			if (useGlyph)
				SetMenuItemKeyGlyph (theMenu, i + 1, keyChar);
			SetMenuItemModifiers (theMenu, i + 1, keyModifiers);
		}
		
		if (multipleCheck && check[i])
			CheckMenuItem (theMenu, i + 1, true);

	}
	
	// set the check
	if (style & kCheckStyle && !multipleCheck)
		CheckMenuItem (theMenu, currentIndex + 1, true);

	InsertMenu ((MenuHandle)theMenu, -1);
	
	platformControl = (void*)theMenu;
	return platformControl;
#elif MAC_COCOA
	return 0; // not used

#endif
}

//------------------------------------------------------------------------
void COptionMenu::setValue (float val)
{
	if ((long)val < 0 || (long)val >= nbEntries)
		return;
	
	currentIndex = (long)val;
	if (style & (kMultipleCheckStyle & ~kCheckStyle))
		check[currentIndex] = !check[currentIndex];
	CParamDisplay::setValue (val);
	
	// to force the redraw
	setDirty ();
}

//------------------------------------------------------------------------
void COptionMenu::takeFocus ()
{
	if (!getFrame ())
		return;

	lastResult = -1;
	lastMenu = 0;

#if MAC || WINDOWS
	// calculate Screen Position
	#if WINDOWS
	HWND hwnd = (HWND)getFrame ()->getSystemWindow ();

	#endif

	CRect rect;
	#if WINDOWS
	RECT rctWinParent;
	GetWindowRect (hwnd, &rctWinParent);
	rect.left = (CCoord)rctWinParent.left;
	rect.top  = (CCoord)rctWinParent.top;
	#endif
	CView* parent = getParentView ();
	while (parent)
	{
		if (parent->notify (this, kMsgCheckIfViewContainer) == kMessageNotified)
		{
			CRect vSize;
			parent->getViewSize (vSize);
			rect.offset (vSize.left, vSize.top);
		}
		parent = parent->getParentView ();
	}
#endif
	
#if WINDOWS
	MSG msg;
	long result = -1;

	//---Create the popup menu---
	long offIdx = 0;
	appendItems (offIdx);
	
	//---Popup the menu---
	long offset;
	if (style & kPopupStyle)
		offset = (long)(rect.top + size.top);
	else
		offset = (long)(rect.top + size.bottom);

	int flags = TPM_LEFTALIGN;
	if (lastButton & kRButton)
		flags |= TPM_RIGHTBUTTON;

	if (TrackPopupMenu ((HMENU)platformControl, flags, 
			 (int)(rect.left + size.left), offset, 0, hwnd, 0))
	{
		if (PeekMessage (&msg, hwnd, WM_COMMAND, WM_COMMAND, PM_REMOVE))
		{
			if (HIWORD (msg.wParam) == 0)
			{
				result = LOWORD (msg.wParam);
				lastResult = result;
			}
		}
	}

	//---Destroy the menu----
	removeItems ();
	
	//---Update the dependencies
	if (result != -1 || bgWhenClick)
	{
		// to force the redraw
		if (bgWhenClick)
			setDirty ();

		if (result != -1)
		{
			long idx = 0;
			offIdx = 0;
			COptionMenu *menu = getItemMenu (result, idx, offIdx);
			if (menu)
			{
				lastMenu = menu;
				menu->setValue ((float)idx);
			
				// update dependency
				if (listener)
					listener->valueChanged (menu);
			}
		}
	}
#endif // WINDOWS

#if MAC_COCOA
	if (getFrame ()->getNSView ())
	{
		COptionMenu* usedMenu = 0;
		long index = showNSContextMenu (this, &usedMenu);
		if (index >= 0 && usedMenu)
		{
			lastMenu = usedMenu;
			lastResult = index;
			usedMenu->setValue (index);
			if (listener)
				listener->valueChanged (usedMenu);
		}
		getFrame ()->setFocusView (0);
		endEdit();
		return;
	}
#endif // MAC_COCOA
	
#if MAC
	// no entries, no menu
	if (nbEntries == 0)
	{
		endEdit();
		getFrame ()->setFocusView (0);
		return;
	}
		
	//---Transform local coordinates to global coordinates
	long offset;

	if (style & kPopupStyle)
		offset = (long)size.top;
	else
		offset = (long)size.bottom;

	CCoord gx = 0, gy = 0;
	Point LToG;
	getFrame()->getPosition(gx, gy);
	LToG.v = (short)(gy + rect.top + offset);
	LToG.h = (short)(gx + rect.left + size.left);
		
	//---Create the popup menu---
	long offIdx = 0;
	MenuHandle theMenu = (MenuHandle)appendItems (offIdx);

	// Calculate the menu size (height and width)
	CalcMenuSize (theMenu);
	
	// Get a handle to the screen
	RgnHandle rgn = GetGrayRgn ();

	Rect bounds;
	GetRegionBounds (rgn, &bounds);
	int bottom      = bounds.bottom;
	long menuHeight = GetMenuHeight (theMenu);

	// Calculate the size of one menu item (round to the next int)
	int menuItemSize = (menuHeight + nbEntries - 1) / nbEntries;

	setDirty (false);	

	//---Popup the Menu
	long popUpItem = style & kPopupStyle ? (value + 1) : 1;
	long PopUpMenuItem = 0;

	if (LToG.v + menuHeight >= bottom - menuItemSize / 2)
	{
		if (nbEntries * menuItemSize >= bottom)
		{
			popUpItem = currentIndex + 2;
			if (popUpItem > nbEntries)
				popUpItem = nbEntries;
		}
		if (nbEntries * menuItemSize >= bottom)
			PopUpMenuItem = PopUpMenuSelect (theMenu, LToG.v, LToG.h, popUpItem);
		else
			PopUpMenuItem = PopUpMenuSelect (theMenu, bottom - menuHeight - menuItemSize, LToG.h, popUpItem);
	}
	else
		PopUpMenuItem = PopUpMenuSelect (theMenu, LToG.v, LToG.h, popUpItem);

	//---Destroy the menu----
	removeItems ();
	
	// HiWord indicates MenuID, LoWord indicates the item index
	short result = LoWord (PopUpMenuItem) - 1;	
	lastResult = result;
	short menuIDResult = HiWord (PopUpMenuItem);
	if (menuIDResult != 0 || bgWhenClick) 
	{
		// to force the redraw
		if (bgWhenClick)
			setDirty ();

		if (menuIDResult != 0)
		{
			long idx = 0;
			offIdx = menuIDResult;
			COptionMenu *menu = getItemMenu (result, idx, offIdx);
			if (menu)
			{
				lastMenu = menu;
				menu->setValue (result);
				if (listener)
					listener->valueChanged (menu);
			}
		}
	}

#endif

	getFrame ()->setFocusView (0);
	endEdit();
}

//------------------------------------------------------------------------
void COptionMenu::looseFocus ()
{	
	if (platformControl == 0) 
		return;

	platformControl = 0;

	CView* receiver = pParentView ? pParentView : pParentFrame;
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
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
CAnimKnob::CAnimKnob (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset)
, bInverseBitmap (false)
{
	heightOfOneImage = size.height ();
	subPixmaps = (short)(background->getHeight () / heightOfOneImage);
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
CAnimKnob::CAnimKnob (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset)
, subPixmaps (subPixmaps)
, bInverseBitmap (false)
{
	setHeightOfOneImage (heightOfOneImage);
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CAnimKnob& v)
: CKnob (v)
, subPixmaps (v.subPixmaps)
, bInverseBitmap (v.bInverseBitmap)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CAnimKnob::~CAnimKnob ()
{}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setHeightOfOneImage (const CCoord& height)
{
	IMultiBitmapControl::setHeightOfOneImage (height);
	if (pBackground && heightOfOneImage)
		subPixmaps = (short)(pBackground->getHeight () / heightOfOneImage);
}

//-----------------------------------------------------------------------------------------------
bool CAnimKnob::isDirty () const
{
	if (!bDirty)
	{
		CPoint p;
		valueToPoint (p);
		if (p == lastDrawnPoint)
			return false;
	}
	return CKnob::isDirty ();
}

//------------------------------------------------------------------------
void CAnimKnob::draw (CDrawContext *pContext)
{
	CPoint where (0, 0);
	if (value >= 0.f) 
	{
		CCoord tmp = heightOfOneImage * (subPixmaps - 1);
		if (bInverseBitmap)
			where.v = (CCoord)((1 - value) * (float)tmp);
		else
			where.v = (CCoord)(value * (float)tmp);
		for (CCoord realY = 0; realY <= tmp; realY += heightOfOneImage) 
		{
			if (where.v < realY) 
			{
				where.v = realY - heightOfOneImage;
				if (where.v < 0)
					where.v = 0;
				break;
			}
		}
	}

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	valueToPoint (lastDrawnPoint);
	setDirty (false);
}

//------------------------------------------------------------------------
// CVerticalSwitch
//------------------------------------------------------------------------
/*! @class CVerticalSwitch
Define a switch with a given number of positions, the current position is defined by the position
of the last click on this object (the object is divided in its height by the number of position).
Each position has its subbitmap, each subbitmap is stacked in the given handle bitmap.
By clicking Alt+Left Mouse the default value is used.
*/
//------------------------------------------------------------------------
/**
 * CVerticalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the switch bitmap
 * @param offset unused 
 */
//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.height ();
	subPixmaps = (long)(background->getHeight () / heightOfOneImage);
	iMaxPositions = subPixmaps;

	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
/**
 * CVerticalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param iMaxPositions TODO
 * @param background the switch bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, long iMaxPositions, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
, subPixmaps (subPixmaps)
, iMaxPositions (iMaxPositions)
{
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CVerticalSwitch& v)
: CControl (v)
, offset (v.offset)
, subPixmaps (v.subPixmaps)
, iMaxPositions (v.iMaxPositions)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CVerticalSwitch::~CVerticalSwitch ()
{}

//------------------------------------------------------------------------
void CVerticalSwitch::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		// source position in bitmap
		CPoint where (0, heightOfOneImage * ((long)(value * (iMaxPositions - 1) + 0.5f)));

		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CVerticalSwitch::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;
	
	if (button == -1) button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	// check if default value wanted
	if (checkDefaultValue (button))
		return;

	double coef = (double)heightOfOneImage / (double)iMaxPositions;

	// begin of edit parameter
	beginEdit ();
	do
	{
		value = (long)((where.v - size.top) / coef) / (float)(iMaxPositions - 1);
		if (value > 1.f)
			value = 1.f;
		else if (value < 0.f)
			value = 0.f;

		if (isDirty () && listener)
			listener->valueChanged (this);
		
		getMouseLocation (pContext, where);
		
		doIdleStuff ();
	}
	while (pContext->getMouseButtons () == button);

	// end of edit parameter
	endEdit ();
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = (double)heightOfOneImage / (double)iMaxPositions;

	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseUp (CPoint& where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		value = (long)((where.v - size.top) / coef) / (float)(iMaxPositions - 1);
		if (value > 1.f)
			value = 1.f;
		else if (value < 0.f)
			value = 0.f;

		if (isDirty () && listener)
			listener->valueChanged (this);
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CHorizontalSwitch
//------------------------------------------------------------------------
/*! @class CHorizontalSwitch
Same as the CVerticalSwitch but horizontal.
*/
//------------------------------------------------------------------------
/**
 * CHorizontalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap of the switch
 * @param offset unused
 */
//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.width ();
	subPixmaps = (long)(background->getWidth () / heightOfOneImage);
	iMaxPositions = subPixmaps;

	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
/**
 * CHorizontalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param iMaxPositions TODO
 * @param background the switch bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, long iMaxPositions, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
, subPixmaps (subPixmaps)
, iMaxPositions (iMaxPositions)
{
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CHorizontalSwitch& v)
: CControl (v)
, offset (v.offset)
, subPixmaps (v.subPixmaps)
, iMaxPositions (v.iMaxPositions)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CHorizontalSwitch::~CHorizontalSwitch ()
{}

//------------------------------------------------------------------------
void CHorizontalSwitch::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		// source position in bitmap
		CPoint where (0, heightOfOneImage * ((long)(value * (iMaxPositions - 1) + 0.5f)));

		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CHorizontalSwitch::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (!(button & kLButton))
		return;
	
	// check if default value wanted
	if (checkDefaultValue (button))
		return;

	double coef = (double)pBackground->getWidth () / (double)iMaxPositions;

	// begin of edit parameter
	beginEdit ();
	do
	{
		value = (long)((where.h - size.left) / coef) / (float)(iMaxPositions - 1);
		if (value > 1.f)
			value = 1.f;
		else if (value < 0.f)
			value = 0.f;

		if (isDirty () && listener)
			listener->valueChanged (this);
		
		getMouseLocation (pContext, where);
		
		doIdleStuff ();
	}
	while (pContext->getMouseButtons () == button);

	// end of edit parameter
	endEdit ();
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = (double)pBackground->getWidth () / (double)iMaxPositions;

	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseUp (CPoint& where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		value = (long)((where.h - size.left) / coef) / (float)(iMaxPositions - 1);
		if (value > 1.f)
			value = 1.f;
		else if (value < 0.f)
			value = 0.f;

		if (isDirty () && listener)
			listener->valueChanged (this);
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CRockerSwitch
//------------------------------------------------------------------------
/*! @class CRockerSwitch
Define a rocker switch with 3 states using 3 subbitmaps.
One click on its leftside, then the first subbitmap is displayed.
One click on its rightside, then the third subbitmap is displayed.
When the mouse button is relaxed, the second subbitmap is framed. */
//------------------------------------------------------------------------
/**
 * CRockerSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap with 3 stacked images of the rocker switch
 * @param offset
 * @param style
 */
//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset, const long style)
: CControl (size, listener, tag, background)
, offset (offset)
, style (style)
{
	heightOfOneImage = size.height ();
}

//------------------------------------------------------------------------
/**
 * CRockerSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param heightOfOneImage height of one image in pixel
 * @param background bitmap with 3 stacked images of the rocker switch
 * @param offset
 * @param style
 */
//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRect& size, CControlListener* listener, long tag, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset, const long style)
: CControl (size, listener, tag, background)
, offset (offset)
, style (style)
{
	setHeightOfOneImage (heightOfOneImage);
}

//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRockerSwitch& v)
: CControl (v)
, offset (v.offset)
, style (v.style)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CRockerSwitch::~CRockerSwitch ()
{}

//------------------------------------------------------------------------
void CRockerSwitch::draw (CDrawContext *pContext)
{
	CPoint where (offset.h, offset.v);

	if (value == 1.f)
		where.v += 2 * heightOfOneImage;
	else if (value == 0.f)
		where.v += heightOfOneImage;

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CRockerSwitch::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (!(button & kLButton))
		return;

	float fEntryState = value;

	CCoord  width_2  = size.width () / 2;
	CCoord  height_2 = size.height () / 2;
	
	// begin of edit parameter
	beginEdit ();
	
	if (button)
	{
		do
		{
			if (style & kHorizontal) 
			{
				if (where.h >= size.left && where.v >= size.top  &&
					where.h <= (size.left + width_2) && where.v <= size.bottom)
					value = -1.0f;
				else if (where.h >= (size.left + width_2) && where.v >= size.top  &&
					where.h <= size.right && where.v <= size.bottom)
					value = 1.0f;
				else
					value = fEntryState;
			}
			else
			{
				if (where.h >= size.left && where.v >= size.top  &&
					where.h <= size.right && where.v <= (size.top + height_2))
					value = -1.0f;
				else if (where.h >= size.left && where.v >= (size.top + height_2) &&
					where.h <= size.right && where.v <= size.bottom)
					value = 1.0f;
				else
					value = fEntryState;
			}

			if (isDirty () && listener)
				listener->valueChanged (this);

			getMouseLocation (pContext, where);

			doIdleStuff ();
		}
		while (pContext->getMouseButtons ());
	}
	else
	{
		if (where.h >= size.left && where.v >= size.top  &&
				where.h <= (size.left + width_2) && where.v <= size.bottom)
			value = -1.0f;
		else if (where.h >= (size.left + width_2) && where.v >= size.top  &&
				where.h <= size.right && where.v <= size.bottom)
			value = 1.0f;

		if (listener)
			listener->valueChanged (this);
	}

	value = 0.f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);

	// end of edit parameter
	endEdit ();
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseUp (CPoint& where, const long& buttons)
{
	value = 0.f;
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		CCoord  width_2  = size.width () / 2;
		CCoord  height_2 = size.height () / 2;

		if (style & kHorizontal) 
		{
			if (where.h >= size.left && where.v >= size.top  &&
				where.h <= (size.left + width_2) && where.v <= size.bottom)
				value = -1.0f;
			else if (where.h >= (size.left + width_2) && where.v >= size.top  &&
				where.h <= size.right && where.v <= size.bottom)
				value = 1.0f;
			else
				value = fEntryState;
		}
		else
		{
			if (where.h >= size.left && where.v >= size.top  &&
				where.h <= size.right && where.v <= (size.top + height_2))
				value = -1.0f;
			else if (where.h >= size.left && where.v >= (size.top + height_2) &&
				where.h <= size.right && where.v <= size.bottom)
				value = 1.0f;
			else
				value = fEntryState;
		}

		if (isDirty () && listener)
			listener->valueChanged (this);
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CRockerSwitch::onWheel (const CPoint& where, const float &distance, const long &buttons)
{
	if (!bMouseEnabled)
		return false;

	if (distance > 0)
		value = -1.0f;
	else
		value = 1.0f;

	// begin of edit parameter
	beginEdit ();

	if (isDirty () && listener)
		listener->valueChanged (this);

	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);
	
	// end of edit parameter
	endEdit ();

	return true;
}


//------------------------------------------------------------------------
// CMovieBitmap
//------------------------------------------------------------------------
/**
 * CMovieBitmap constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
, subPixmaps (subPixmaps)
{
	setHeightOfOneImage (size.getHeight ());
	subPixmaps = (long)(background->getHeight () / heightOfOneImage);
}

//------------------------------------------------------------------------
/**
 * CMovieBitmap constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of subPixmaps
 * @param heightOfOneImage height of one image in pixel
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
, subPixmaps (subPixmaps)
{
	setHeightOfOneImage (heightOfOneImage);
}

//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CMovieBitmap& v)
: CControl (v)
, offset (v.offset)
, subPixmaps (v.subPixmaps)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CMovieBitmap::~CMovieBitmap ()
{}

//------------------------------------------------------------------------
void CMovieBitmap::draw (CDrawContext *pContext)
{
	CPoint where (offset.h, offset.v);

	if (value > 1.0f)
		value = 1.0f;

 	if (value > 0.0f)
		where.v += heightOfOneImage * (int)(value * (subPixmaps - 1) + 0.5);

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}


//------------------------------------------------------------------------
// CMovieButton
//------------------------------------------------------------------------
/**
 * CMovieButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background), offset (offset), buttonState (value)
{
	heightOfOneImage = size.height ();
}

//------------------------------------------------------------------------
/**
 * CMovieButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param heightOfOneImage height of one image in pixel
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CRect& size, CControlListener* listener, long tag, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
, buttonState (value)
{
	setHeightOfOneImage (heightOfOneImage);
}

//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CMovieButton& v)
: CControl (v)
, offset (v.offset)
, buttonState (v.buttonState)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CMovieButton::~CMovieButton ()
{}

//------------------------------------------------------------------------
void CMovieButton::draw (CDrawContext *pContext)
{
	CPoint where;

	where.h = 0;

	bounceValue ();

	if (value)
		where.v = heightOfOneImage;
	else
		where.v = 0;

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	buttonState = value;

	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CMovieButton::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (!(button & kLButton))
		return;

	// this simulates a real windows button
	float fEntryState = value;

	// begin of edit parameter
	beginEdit ();
		
	if (pContext->getMouseButtons ())
	{
		do
		{
			if (where.h >= size.left &&
					where.v >= size.top  &&
					where.h <= size.right &&
					where.v <= size.bottom)
				value = !fEntryState;
			else
				value = fEntryState;
	    
			if (isDirty () && listener)
				listener->valueChanged (this);
	    
			getMouseLocation (pContext, where);

			doIdleStuff ();
		}
		while (pContext->getMouseButtons () == button);
	}
	else
	{
		value = !value;
		if (listener)
			listener->valueChanged (this);
	}

	// end of edit parameter
	endEdit ();

	buttonState = value;
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseUp (CPoint& where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (where.h >= size.left &&
				where.v >= size.top  &&
				where.h <= size.right &&
				where.v <= size.bottom)
			value = !fEntryState;
		else
			value = fEntryState;
	
		if (isDirty () && listener)
			listener->valueChanged (this);
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CAutoAnimation
//------------------------------------------------------------------------
/*! @class CAutoAnimation
An auto-animation control contains a given number of subbitmap which can be displayed in loop.
Two functions allows to get the previous or the next subbitmap (these functions increase or decrease the current value of this control).
*/
// displays bitmaps within a (child-) window
//------------------------------------------------------------------------
/**
 * CAutoAnimation constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
, bWindowOpened (false)
{
	heightOfOneImage = size.height ();
	subPixmaps = (long)(background->getHeight () / heightOfOneImage);

	totalHeightOfBitmap = heightOfOneImage * subPixmaps;
}

//------------------------------------------------------------------------
/**
 * CAutoAnimation constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
, subPixmaps (subPixmaps)
, bWindowOpened (false)
{
	setHeightOfOneImage (heightOfOneImage);
	totalHeightOfBitmap = heightOfOneImage * subPixmaps;
}

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CAutoAnimation& v)
: CControl (v)
, offset (v.offset)
, subPixmaps (v.subPixmaps)
, totalHeightOfBitmap (v.totalHeightOfBitmap)
, bWindowOpened (v.bWindowOpened)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CAutoAnimation::~CAutoAnimation ()
{}

//------------------------------------------------------------------------
void CAutoAnimation::draw (CDrawContext *pContext)
{
	if (isWindowOpened ())
	{	
		CPoint where;
		where.v = (long)value + offset.v;
		where.h = offset.h;
		
		if (pBackground)
		{
			if (bTransparencyEnabled)
				pBackground->drawTransparent (pContext, size, where);
			else
				pBackground->draw (pContext, size, where);
		}
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CAutoAnimation::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;
	
	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (!(button & kLButton))
		return;

	if (!isWindowOpened ())
	{	
		value = 0;
		openWindow ();
		setDirty (); // force to redraw
		if (listener)
			listener->valueChanged (this);
	}
	else
	{                                                                       
		// stop info animation
		value = 0; // draw first pic of bitmap
		setDirty ();
		closeWindow ();
	}
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CAutoAnimation::onMouseDown (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (!isWindowOpened ())
		{	
			value = 0;
			openWindow ();
			invalid ();
			if (listener)
				listener->valueChanged (this);
		}
		else
		{                                                                       
			// stop info animation
			value = 0; // draw first pic of bitmap
			invalid ();
			closeWindow ();
			if (listener)
				listener->valueChanged (this);
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CAutoAnimation::openWindow ()
{
	bWindowOpened = true;
}

//------------------------------------------------------------------------
void CAutoAnimation::closeWindow ()
{
	bWindowOpened = false;
}

//------------------------------------------------------------------------
void CAutoAnimation::nextPixmap ()
{
	value += heightOfOneImage;
	if (value >= (totalHeightOfBitmap - heightOfOneImage))
		value = 0;
}

//------------------------------------------------------------------------
void CAutoAnimation::previousPixmap ()
{
	value -= heightOfOneImage;
	if (value < 0.f)
		value = (float)(totalHeightOfBitmap - heightOfOneImage - 1);
}


//------------------------------------------------------------------------
// CSlider
//------------------------------------------------------------------------
/*! @class CSlider
Define a slider with a given background and handle.
The range of variation of the handle should be defined.
By default the handler is drawn with transparency.
By clicking Alt+Left Mouse the default value is used.
*/
//------------------------------------------------------------------------
/**
 * CSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
 */
//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CControl (rect, listener, tag, background)
, offset (offset)
, pHandle (handle)
, style (style)
, bFreeClick (true)
{
	setDrawTransparentHandle (true);

	if (pHandle)
	{
		pHandle->remember ();
		widthOfSlider  = pHandle->getWidth ();
		heightOfSlider = pHandle->getHeight ();
	}
	else
	{
		widthOfSlider  = 1; 
		heightOfSlider = 1;
	}

	widthControl  = size.width ();
	heightControl = size.height ();

	if (style & kHorizontal)
	{
		minPos = iMinPos - size.left;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
		CPoint p (0, 0);
		setOffsetHandle (p);
	}
	else
	{
		minPos = iMinPos - size.top;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
		CPoint p (0, 0);
		setOffsetHandle (p);
	}

	zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param _rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
 */
//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long _rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CControl (rect, listener, tag, background)
, offset (offset)
, pHandle (handle) 
, style (style)
, minPos (0)
, bFreeClick (true)
{
	setDrawTransparentHandle (true);

	if (pHandle)
	{
		pHandle->remember ();
		widthOfSlider  = pHandle->getWidth ();
		heightOfSlider = pHandle->getHeight ();
	}
	else
	{
		widthOfSlider  = 1; 
		heightOfSlider = 1;
	}

	widthControl  = size.width ();
	heightControl = size.height ();
	if (style & kHorizontal)
		rangeHandle = _rangeHandle - widthOfSlider;
	else
		rangeHandle = _rangeHandle - heightOfSlider;

	setOffsetHandle (offsetHandle);
	
	zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSlider::CSlider (const CSlider& v)
: CControl (v)
, offset (v.offset)
, offsetHandle (v.offsetHandle)
, pHandle (v.pHandle)
, style (v.style)
, widthOfSlider (v.widthOfSlider)
, heightOfSlider (v.heightOfSlider)
, rangeHandle (v.rangeHandle)
, minTmp (v.minTmp)
, maxTmp (v.maxTmp)
, minPos (v.minPos)
, widthControl (v.widthControl)
, heightControl (v.heightControl)
, zoomFactor (v.zoomFactor)
, bDrawTransparentEnabled (v.bDrawTransparentEnabled)
, bFreeClick (v.bFreeClick)
{
	if (pHandle)
		pHandle->remember ();
}

//------------------------------------------------------------------------
CSlider::~CSlider ()
{
	if (pHandle)
		pHandle->forget ();
}

//------------------------------------------------------------------------
void CSlider::setOffsetHandle (const CPoint &val)
{
	offsetHandle = val;

	if (style & kHorizontal)
	{
		minTmp = offsetHandle.h + minPos;
		maxTmp = minTmp + rangeHandle + widthOfSlider;
	}
	else
	{
		minTmp = offsetHandle.v + minPos;
		maxTmp = minTmp + rangeHandle + heightOfSlider;
	}
}

//------------------------------------------------------------------------
void CSlider::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	float fValue;
	if (style & kLeft || style & kTop)
		fValue = value;
	else 
		fValue = 1.f - value;
	
	// (re)draw background
	CRect rect (0, 0, widthControl, heightControl);
	rect.offset (size.left, size.top);
	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (drawContext, rect, offset);
		else
			pBackground->draw (drawContext, rect, offset);
	}
	
	// calc new coords of slider
	CRect   rectNew;
	if (style & kHorizontal)
	{
		rectNew.top    = offsetHandle.v;
		rectNew.bottom = rectNew.top + heightOfSlider;	

		rectNew.left   = offsetHandle.h + (int)(fValue * rangeHandle);
		rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

		rectNew.right  = rectNew.left + widthOfSlider;
		rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
	}
	else
	{
		rectNew.left   = offsetHandle.h;
		rectNew.right  = rectNew.left + widthOfSlider;	

		rectNew.top    = offsetHandle.v + (int)(fValue * rangeHandle);
		rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

		rectNew.bottom = rectNew.top + heightOfSlider;
		rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
	}
	rectNew.offset (size.left, size.top);

	// draw slider at new position
	if (pHandle)
	{
		if (bDrawTransparentEnabled)
			pHandle->drawTransparent (drawContext, rectNew);
		else 
			pHandle->draw (drawContext, rectNew);
	}

	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CSlider::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	// check if default value wanted
	if (checkDefaultValue (button))
		return;
	
	// allow left mousebutton only
	if (!(button & kLButton))
		return;

	CCoord delta;
	if (style & kHorizontal)
		delta = size.left + offsetHandle.h;
	else
		delta = size.top + offsetHandle.v;
	if (!bFreeClick)
	{
		float fValue;
		if (style & kLeft || style & kTop)
			fValue = value;
		else 
			fValue = 1.f - value;
		CCoord actualPos;
		CRect rect;

		if (style & kHorizontal)
		{
			actualPos = offsetHandle.h + (int)(fValue * rangeHandle) + size.left;

			rect.left   = actualPos;
			rect.top    = size.top  + offsetHandle.v;
			rect.right  = rect.left + widthOfSlider;
			rect.bottom = rect.top  + heightOfSlider;

			if (!where.isInside (rect))
				return;
			else
				delta += where.h - actualPos;
		}
		else
		{
			actualPos = offsetHandle.v + (int)(fValue * rangeHandle) + size.top;
		
			rect.left   = size.left  + offsetHandle.h;
			rect.top    = actualPos;
			rect.right  = rect.left + widthOfSlider;
			rect.bottom = rect.top  + heightOfSlider;

			if (!where.isInside (rect))
				return;
			else
				delta += where.v - actualPos;
		}
	} 
	else
	{
		if (style & kHorizontal)
			delta += widthOfSlider / 2 - 1;
		else
			delta += heightOfSlider / 2 - 1;
	}
	
	float oldVal    = value;
	long  oldButton = button;

	// begin of edit parameter
	beginEdit ();

	while (1)
	{
		button = pContext->getMouseButtons ();
		if (!(button & kLButton))
			break;

		if ((oldButton != button) && (button & kShift))
		{
			oldVal    = value;
			oldButton = button;
		}
		else if (!(button & kShift))
			oldVal = value;

		if (style & kHorizontal)
			value = (float)(where.h - delta) / (float)rangeHandle;
		else
			value = (float)(where.v - delta) / (float)rangeHandle;
			
		if (style & kRight || style & kBottom)
			value = 1.f - value;

		if (button & kShift)
			value = oldVal + ((value - oldVal) / zoomFactor);
		bounceValue ();
    	    
		if (isDirty () && listener)
			listener->valueChanged (this);

		getMouseLocation (pContext, where);

		doIdleStuff ();
	}

	// end of edit parameter
	endEdit ();
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if (style & kHorizontal)
		delta = size.left + offsetHandle.h;
	else
		delta = size.top + offsetHandle.v;
	if (!bFreeClick)
	{
		float fValue;
		if (style & kLeft || style & kTop)
			fValue = value;
		else 
			fValue = 1.f - value;
		CCoord actualPos;
		CRect rect;

		if (style & kHorizontal)
		{
			actualPos = offsetHandle.h + (int)(fValue * rangeHandle) + size.left;

			rect.left   = actualPos;
			rect.top    = size.top  + offsetHandle.v;
			rect.right  = rect.left + widthOfSlider;
			rect.bottom = rect.top  + heightOfSlider;

			if (!where.isInside (rect))
				return kMouseEventNotHandled;
			else
				delta += where.h - actualPos;
		}
		else
		{
			actualPos = offsetHandle.v + (int)(fValue * rangeHandle) + size.top;
		
			rect.left   = size.left  + offsetHandle.h;
			rect.top    = actualPos;
			rect.right  = rect.left + widthOfSlider;
			rect.bottom = rect.top  + heightOfSlider;

			if (!where.isInside (rect))
				return kMouseEventNotHandled;
			else
				delta += where.v - actualPos;
		}
	} 
	else
	{
		if (style & kHorizontal)
			delta += widthOfSlider / 2 - 1;
		else
			delta += heightOfSlider / 2 - 1;
	}
	
	oldVal    = vmin-1;
	oldButton = buttons;

	beginEdit ();
	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	if (buttons & kShift)
		return kMouseEventHandled;
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseUp (CPoint& where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (oldVal == vmin - 1)
			oldVal    = (value - vmin) / (vmax - vmin);
			
		if ((oldButton != buttons) && (buttons & kShift))
		{
			oldVal    = (value - vmin) / (vmax - vmin);
			oldButton = buttons;
		}
		else if (!(buttons & kShift))
			oldVal = (value - vmin) / (vmax - vmin);

		if (style & kHorizontal)
			value = (float)(where.h - delta) / (float)rangeHandle;
		else
			value = (float)(where.v - delta) / (float)rangeHandle;

		if (style & kRight || style & kBottom)
			value = 1.f - value;

		if (buttons & kShift)
			value = oldVal + ((value - oldVal) / zoomFactor);

		if (vmax != 1.f || vmin != 1.f)
			value = vmin + ((vmax - vmin) * value);
			
		bounceValue ();
    	    
		if (isDirty () && listener)
			listener->valueChanged (this);
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CSlider::onWheel (const CPoint& where, const float &distance, const long &buttons)
{
	if (!bMouseEnabled)
		return false;

	if (buttons & kShift)
		value += 0.1f * distance * wheelInc;
	else
		value += distance * wheelInc;
	bounceValue ();

	if (isDirty () && listener)
	{
		// begin of edit parameter
		beginEdit ();
	
		listener->valueChanged (this);
	
		// end of edit parameter
		endEdit ();
	}

	return true;
}

//------------------------------------------------------------------------
long CSlider::onKeyDown (VstKeyCode& keyCode)
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

			if (keyCode.modifier & MODIFIER_SHIFT)
				value += 0.1f * distance * wheelInc;
			else
				value += distance * wheelInc;
			bounceValue ();

			if (isDirty () && listener)
			{
				// begin of edit parameter
				beginEdit ();
			
				listener->valueChanged (this);
			
				// end of edit parameter
				endEdit ();
			}
		} return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
void CSlider::setHandle (CBitmap *_pHandle)
{
	if (pHandle)
		pHandle->forget ();
	pHandle = _pHandle;
	if (pHandle)
	{
		pHandle->remember ();
		widthOfSlider  = pHandle->getWidth ();
		heightOfSlider = pHandle->getHeight ();
	}
}


//------------------------------------------------------------------------
// CVerticalSlider
//------------------------------------------------------------------------
/*! @class CVerticalSlider
This is the vertical slider. See CSlider.
*/
//------------------------------------------------------------------------
/**
 * CVerticalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
/**
 * CVerticalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CVerticalSlider& slider)
: CSlider (slider)
{}

//------------------------------------------------------------------------
// CHorizontalSlider
//------------------------------------------------------------------------
/*! @class CHorizontalSlider
This is the horizontal slider. See CSlider.
*/
//------------------------------------------------------------------------
/**
 * CHorizontalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
/**
 * CHorizontalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CHorizontalSlider& slider)
: CSlider (slider)
{}


//------------------------------------------------------------------------
// CSpecialDigit
//------------------------------------------------------------------------
/*! @class CSpecialDigit
Can be used to display a counter with maximum 7 digits.
All digit have the same size and are stacked in height in the bitmap.
*/
//------------------------------------------------------------------------
/**
 * CSpecialDigit constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param dwPos actual value
 * @param iNumbers amount of numbers (max 7)
 * @param xpos array of all X positions, can be NULL
 * @param ypos array of all Y positions, can be NULL
 * @param width width of one number in the bitmap
 * @param height height of one number in the bitmap
 * @param background bitmap
 */
//------------------------------------------------------------------------
CSpecialDigit::CSpecialDigit (const CRect& size, CControlListener* listener, long tag, long dwPos, long iNumbers, long* xpos, long* ypos, long width, long height, CBitmap* background)
: CControl (size, listener, tag, background)
, iNumbers (iNumbers)
, width (width)
, height (height)
{
	setValue ((float)dwPos);          // actual value

	if (iNumbers > 7)
		iNumbers = 7;

	if (xpos == NULL)
	{
		// automatically init xpos/ypos if not provided by caller
		const int numw = (const int)background->getWidth();
		int x = (int)size.left;
		for (long i = 0; i < iNumbers; i++)
		{
			this->xpos[i] = x; 
			this->ypos[i] = (long)size.top;
			x += numw;
		}
	} 
	else if (xpos && ypos)
	{
		// store coordinates of x/y pos of each digit
		for (long i = 0; i < iNumbers; i++)
		{
			this->xpos[i] = xpos[i];
			this->ypos[i] = ypos[i];
		}
	}

	setMax ((float)pow (10.f, (float)iNumbers) - 1.0f);
	setMin (0.0f);
}

//------------------------------------------------------------------------
CSpecialDigit::CSpecialDigit (const CSpecialDigit& v)
: CControl (v)
, iNumbers (v.iNumbers)
, width (v.width)
, height (v.height)
{
	for (int i = 0; i < 7; i++)
	{
		xpos[i] = v.xpos[i];
		ypos[i] = v.ypos[i];
	}
}

//------------------------------------------------------------------------
CSpecialDigit::~CSpecialDigit ()
{}

//------------------------------------------------------------------------
void CSpecialDigit::draw (CDrawContext *pContext)
{
	CPoint  where;
	CRect   rectDest;
	long    i, j;
	long    dwValue;
	long     one_digit[16];
  
	if ((long)value >= getMax ()) 
		dwValue = (long)getMax ();
	else if ((long)value < getMin ()) 
		dwValue = (long)getMin ();
	else
		dwValue = (long)value;
	
	for (i = 0, j = ((long)getMax () + 1) / 10; i < iNumbers; i++, j /= 10)
	{
		one_digit[i] = dwValue / j;
		dwValue -= (one_digit[i] * j);
	}
	
	where.h = 0;
	for (i = 0; i < iNumbers; i++)
	{	
		j = one_digit[i];
		if (j > 9)
			j = 9;
		
		rectDest.left   = (CCoord)xpos[i];
		rectDest.top    = (CCoord)ypos[i];
		
		rectDest.right  = rectDest.left + width;
		rectDest.bottom = rectDest.top  + height;		
		
		// where = src from bitmap
		where.v = (CCoord)j * height;
		if (pBackground)
		{
			if (bTransparencyEnabled)
				pBackground->drawTransparent (pContext, rectDest, where);
			else
				pBackground->draw (pContext, rectDest, where);
		}
	}
		
	setDirty (false);
}

//------------------------------------------------------------------------
float CSpecialDigit::getNormValue () const
{ 
	float fTemp;
	fTemp = value / getMax ();
	if (fTemp > 1.0f)
		fTemp = 1.0f;
	else if (fTemp < 0.0f) 
		fTemp = 0.0f;
	
	return fTemp;
}


//------------------------------------------------------------------------
// CKickButton
//------------------------------------------------------------------------
/*! @class CKickButton
Define a button with 2 states using 2 subbitmaps.
One click on it, then the second subbitmap is displayed.
When the mouse button is relaxed, the first subbitmap is framed.
*/
//------------------------------------------------------------------------
/**
 * CKickButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CKickButton::CKickButton (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.height ();
}

//------------------------------------------------------------------------
/**
 * CKickButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param heightOfOneImage height of one sub bitmap in background
 * @param background the bitmap
 * @param offset of background
 */
//------------------------------------------------------------------------
CKickButton::CKickButton (const CRect& size, CControlListener* listener, long tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	setHeightOfOneImage (heightOfOneImage);
}

//------------------------------------------------------------------------
CKickButton::CKickButton (const CKickButton& v)
: CControl (v)
, offset (v.offset)
{
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CKickButton::~CKickButton ()
{}

//------------------------------------------------------------------------
void CKickButton::draw (CDrawContext *pContext)
{
	CPoint where (offset.h, offset.v);

	bounceValue ();

	if (value)
		where.v += heightOfOneImage;

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CKickButton::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;
	
	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (!(button & kLButton))
		return;

	// this simulates a real windows button
	float fEntryState = value;

	// begin of edit parameter
	beginEdit ();

	if (pContext->getMouseButtons () == kLButton)
	{
		do
		{
			if (where.h >= size.left && where.v >= size.top  &&
			    where.h <= size.right && where.v <= size.bottom)
				value = !fEntryState;
			else
				value = fEntryState;
			
			if (isDirty () && listener)
				listener->valueChanged (this);
			
			getMouseLocation (pContext, where);
			
			doIdleStuff ();
		}
		while (pContext->getMouseButtons () == kLButton);
	}
	else
	{
		value = !value;
		if (listener)
			listener->valueChanged (this);
	}

	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);

	// end of edit parameter
	endEdit ();
}
#endif

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseUp (CPoint& where, const long& buttons)
{
	if (value && listener)
		listener->valueChanged (this);
	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (where.h >= size.left && where.v >= size.top  &&
			where.h <= size.right && where.v <= size.bottom)
			value = !fEntryState;
		else
			value = fEntryState;
		
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

/// \cond ignore
//------------------------------------------------------------------------
class CDefaultSplashScreenView : public CControl
{
public:
	CDefaultSplashScreenView (const CRect& size, CControlListener* listener, CBitmap* bitmap, const CPoint& offset) : CControl (size, listener), offset (offset) { setBackground (bitmap); }

	void draw (CDrawContext *pContext)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, offset);
		else
			pBackground->draw (pContext, size, offset);
		setDirty (false);
	}

	CMouseEventResult onMouseDown (CPoint& where, const long& buttons)
	{
		if (buttons & kLButton)
		{
			if (listener)
				listener->valueChanged (this);
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
		return kMouseEventNotHandled;
	}
	CLASS_METHODS(CDefaultSplashScreenView, CControl)
protected:
	CPoint offset;
};
/// \endcond

//------------------------------------------------------------------------
// CSplashScreen
//------------------------------------------------------------------------
/*! @class CSplashScreen
One click on its activated region and its bitmap or view is displayed, in this state the other controls can not be used,
and another click on the displayed area will leave the modal mode.
*/
//------------------------------------------------------------------------
/**
 * CSplashScreen constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param toDisplay the region where to display the bitmap
 * @param offset offset of background bitmap
 */
//------------------------------------------------------------------------
CSplashScreen::CSplashScreen (const CRect& size, CControlListener* listener, long tag, CBitmap* background, CRect& toDisplay, const CPoint& offset)
: CControl (size, listener, tag, background)
, toDisplay (toDisplay)
, offset (offset)
, modalView (0)
{
	modalView = new CDefaultSplashScreenView (toDisplay, this, background, offset);
}

//------------------------------------------------------------------------
/**
 * CSplashScreen constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param splashView the view to show
 */
//------------------------------------------------------------------------
CSplashScreen::CSplashScreen (const CRect& size, CControlListener* listener, long tag, CView* splashView)
: CControl (size, listener, tag)
, modalView (splashView)
{
}

//------------------------------------------------------------------------
CSplashScreen::CSplashScreen (const CSplashScreen& v)
: CControl (v)
, toDisplay (v.toDisplay)
, keepSize (v.keepSize)
, offset (v.offset)
{
	modalView = v.modalView->newCopy ();
}

//------------------------------------------------------------------------
CSplashScreen::~CSplashScreen ()
{
	if (modalView)
		modalView->forget ();
}

//------------------------------------------------------------------------
void CSplashScreen::draw (CDrawContext *pContext)
{
	setDirty (false);
}

//------------------------------------------------------------------------
bool CSplashScreen::hitTest (const CPoint& where, const long buttons)
{
	bool result = CView::hitTest (where, buttons);
	if (result && !(buttons & kLButton))
		return false;
	return result;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CSplashScreen::mouse (CDrawContext *pContext, CPoint& where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		if (listener->controlModifierClicked (this, button) != 0)
			return;
	}

	if (!(button & kLButton))
		return;

	value = !value;
	if (value)
	{
		if (getFrame () && getFrame ()->setModalView (modalView))
		{
			if (listener)
				listener->valueChanged (this);
		}
		}
	}
#endif

//------------------------------------------------------------------------
CMouseEventResult CSplashScreen::onMouseDown (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		value = !value;
		if (value)
		{
			if (modalView && getFrame () && getFrame ()->setModalView (modalView))
			{
				if (listener)
					listener->valueChanged (this);
			}
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CSplashScreen::valueChanged (CControl *pControl)
{
	if (pControl == modalView)
	{
		unSplash ();
		if (listener)
			listener->valueChanged (this);
	}
}

//------------------------------------------------------------------------
void CSplashScreen::unSplash ()
{
	value = 0.f;

	if (getFrame ())
	{
		if (getFrame ()->getModalView () == modalView)
		{
			if (modalView)
				modalView->invalid ();
			getFrame ()->setModalView (NULL);
		}
	}
}

//------------------------------------------------------------------------
// CVuMeter
//------------------------------------------------------------------------
/**
 * CVuMeter constructor.
 * @param size the size of this view
 * @param onBitmap TODO
 * @param offBitmap TODO
 * @param nbLed TODO
 * @param style kHorizontal or kVertical
 */
//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, long nbLed, const long style)
: CControl (size, 0, 0)
, onBitmap (onBitmap)
, offBitmap (offBitmap)
, pOScreen (0)
, nbLed (nbLed)
, style (style)
, bUseOffscreen (false)
{
	setDecreaseStepValue (0.1f);

#if (WINDOWS && !USE_LIBPNG && !GDIPLUS)
	setUseOffscreen (true);
#endif
	
	if (onBitmap)
		onBitmap->remember ();
	if (offBitmap)
		offBitmap->remember ();

	rectOn  (size.left, size.top, size.right, size.bottom);
	rectOff (size.left, size.top, size.right, size.bottom);
}

//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CVuMeter& v)
: CControl (v)
, onBitmap (v.onBitmap)
, offBitmap (v.offBitmap)
, pOScreen (0)
, nbLed (v.nbLed)
, style (v.style)
, decreaseValue (v.decreaseValue)
, bUseOffscreen (v.bUseOffscreen)
, rectOn (v.rectOn)
, rectOff (v.rectOff)
{
	if (onBitmap)
		onBitmap->remember ();
	if (offBitmap)
		offBitmap->remember ();
}

//------------------------------------------------------------------------
CVuMeter::~CVuMeter ()
{
	if (onBitmap)
		onBitmap->forget ();
	if (offBitmap)
		offBitmap->forget ();
}

//------------------------------------------------------------------------
void CVuMeter::setDirty (const bool val)
{
	CView::setDirty (val);
}

//-----------------------------------------------------------------------------
bool CVuMeter::attached (CView *parent)
{
	if (pOScreen)
		delete pOScreen;

	if (bUseOffscreen)
	{
		pOScreen = new COffscreenContext (getFrame (), (long)size.width (), (long)size.height (), kBlackCColor);
		rectOn  (0, 0, size.width (), size.height ());
		rectOff (0, 0, size.width (), size.height ());
	}
	else
	{
		rectOn  (size.left, size.top, size.right, size.bottom);
		rectOff (size.left, size.top, size.right, size.bottom);
	}

	return CControl::attached (parent);
}

//------------------------------------------------------------------------
void CVuMeter::setUseOffscreen (bool val)
{
	bUseOffscreen = val;
}

//-----------------------------------------------------------------------------
bool CVuMeter::removed (CView *parent)
{
	if (pOScreen)
	{
		delete pOScreen;
		pOScreen = 0;
	}
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CVuMeter::draw (CDrawContext *_pContext)
{
	if (!onBitmap) 
		return;

	CPoint pointOn;
	CPoint pointOff;
	CDrawContext *pContext = _pContext;

	bounceValue ();
	
	float newValue = oldValue - decreaseValue;
	if (newValue < value)
		newValue = value;
	oldValue = newValue;

	if (bUseOffscreen)
	{
		if (!pOScreen)
		{
			pOScreen = new COffscreenContext (getFrame (), (long)size.width (), (long)size.height (), kBlackCColor);
			rectOn  (0, 0, size.width (), size.height ());
			rectOff (0, 0, size.width (), size.height ());
		}
		pContext = pOScreen;
		if (bTransparencyEnabled)
			pOScreen->copyTo (_pContext, size);
	}

	if (style & kHorizontal) 
	{
		CCoord tmp = (CCoord)(((long)(nbLed * newValue + 0.5f) / (float)nbLed) * onBitmap->getWidth ());
		pointOff (tmp, 0);
		if (!bUseOffscreen)
		tmp += size.left;

		rectOff.left = tmp;
		rectOn.right = tmp;
	}
	else 
	{
		CCoord tmp = (CCoord)(((long)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * onBitmap->getHeight ());
		pointOn (0, tmp);
		if (!bUseOffscreen)
		tmp += size.top;

		rectOff.bottom = tmp;
		rectOn.top     = tmp;
	}

	if (offBitmap)
	{
		if (bTransparencyEnabled)
			offBitmap->drawTransparent (pContext, rectOff, pointOff);
		else
			offBitmap->draw (pContext, rectOff, pointOff);
	}

	if (bTransparencyEnabled)
		onBitmap->drawTransparent (pContext, rectOn, pointOn);
	else
		onBitmap->draw (pContext, rectOn, pointOn);

	if (pOScreen)
		pOScreen->copyFrom (_pContext, size);
	setDirty (false);
}

//------------------------------------------------------------------------
void IMultiBitmapControl::autoComputeHeightOfOneImage ()
{
	CView* view = dynamic_cast<CView*>(this);
	if (view)
	{
		CRect viewSize = view->getViewSize (viewSize);
		heightOfOneImage = viewSize.height ();
	}
}

END_NAMESPACE_VSTGUI
//------------------------------------------------------------------------
// END.
//------------------------------------------------------------------------
