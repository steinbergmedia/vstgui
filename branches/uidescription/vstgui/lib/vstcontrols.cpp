//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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
#include <list>

#include "vstcontrols.h"
#include "cbitmap.h"
#include "coffscreencontext.h"
#include "cfont.h"
#include "vstkeycode.h"
#include "win32support.h"

#ifdef check
#undef check
#endif

#if MAC_COCOA
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
	#elif MAC_CARBON
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
void CControl::setBackOffset (const CPoint &offset)
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
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COnOffButton::COnOffButton (const COnOffButton& v)
: CControl (v)
, style (v.style)
{
	setWantsFocus (true);
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

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	value = ((long)value) ? 0.f : 1.f;

	invalid ();

	beginEdit ();

	if (listener)
		listener->valueChanged (this);

	endEdit ();

	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//------------------------------------------------------------------------
long COnOffButton::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
	{
		value = ((long)value) ? 0.f : 1.f;
		invalid ();
		beginEdit ();
		if (listener)
			listener->valueChanged (this);
		endEdit ();
		return 1;
	}
	return -1;
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
void CKnob::setViewSize (CRect &rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	radius = (float)(size.right - size.left) / 2.f;
	compute ();
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
: CControl (size, 0, -1, background)
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
, fontColor (v.fontColor)
, backColor (v.backColor)
, frameColor (v.frameColor)
, shadowColor (v.shadowColor)
, textInset (v.textInset)
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
	if (style & kNoDrawStyle)
		return;

	char string[256];
	string[0] = 0;

	if (stringConvert2)
		stringConvert2 (value, string, userData);
	else if (stringConvert)
		stringConvert (value, string);
	else
		sprintf (string, "%2.2f", value);

	drawBack (pContext);
	drawText (pContext, string);
	setDirty (false);
}

//------------------------------------------------------------------------
void CParamDisplay::drawBack (CDrawContext* pContext, CBitmap* newBack)
{
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
}

//------------------------------------------------------------------------
void CParamDisplay::drawText (CDrawContext *pContext, const char *string)
{
	if (!(style & kNoTextStyle) && string && strlen (string))
	{
		CRect textRect (size);
		textRect.inset (textInset.x, textInset.y);
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (textRect);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		pContext->setFont (fontID);
	
		// draw darker text (as shadow)
		if (style & kShadowText) 
		{
			CRect newSize (textRect);
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
		pContext->drawStringUTF8 (string, textRect, horiTxtAlign, bAntialias);
		#else
		pContext->drawString (string, textRect, !bTextTransparencyEnabled, horiTxtAlign);
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
	drawBack (pContext);
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
		#if MAC_CARBON
		HIViewSetNeedsDisplay ((HIViewRef)platformControl, true);
		#endif
		#if MAC_COCOA
		drawBack (pContext);
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

	drawBack (pContext);
	drawText (pContext, string);
	setDirty (false);
}

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
long CTextEdit::onKeyDown (VstKeyCode& keyCode)
{
	if (platformControl)
	{
		if (keyCode.virt == VKEY_ESCAPE)
		{
			bWasReturnPressed = false;
			endEdit ();
			looseFocus ();
			return 1;
		}
		else if (keyCode.virt == VKEY_RETURN)
		{
			bWasReturnPressed = true;
			endEdit ();
			looseFocus ();
			return 1;
		}
	}
	return -1;
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
				CTextEdit *textEdit = (CTextEdit*)(LONG_PTR) GetWindowLongPtr (hwnd, GWLP_USERDATA);
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
			CTextEdit *textEdit = (CTextEdit*)(LONG_PTR) GetWindowLongPtr (hwnd, GWLP_USERDATA);
			if (textEdit)
				textEdit->looseFocus ();
		} break;
	}

	return CallWindowProc (oldWndProcEdit, hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
#endif

#if MAC_CARBON
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
						#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
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
	if (getFrame () && getFrame ()->getNSView ())
	{
		moveNSTextField (platformControl, this);
		return;
	}
	#endif
	
	#if MAC_CARBON
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
	if (getFrame () && getFrame ()->getNSView ())
	{
		CView::setViewSize (newSize, invalid);
		moveNSTextField (platformControl, this);
		return;
	}
	#endif
	
	#if MAC_CARBON
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

	SetWindowLongPtr ((HWND)platformControl, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
	SendMessage ((HWND)platformControl, WM_SETFONT, (WPARAM)platformFont, true);
	SendMessage ((HWND)platformControl, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG (0, 0));
	SendMessage ((HWND)platformControl, EM_SETSEL, 0, -1);
	SendMessage ((HWND)platformControl, EM_LIMITTEXT, 255, 0);
	SetFocus ((HWND)platformControl);

	oldWndProcEdit = (WINDOWSPROC)(LONG_PTR)SetWindowLongPtr ((HWND)platformControl, GWLP_WNDPROC, (__int3264)(LONG_PTR)WindowProcEdit);
#endif // WINDOWS

#if MAC_COCOA
	if (getFrame ()->getNSView ())
	{
		platformControl = addNSTextField (getFrame (), this);
		return;
	}
#endif // MAC_COCOA

#if MAC_CARBON
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

#endif // MAC_CARBON
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
	if (getFrame () && getFrame ()->getNSView ())
	{
		getNSTextFieldText(platformControl, text, 255);
		removeNSTextField (platformControl);
	}
	#if MAC_CARBON
	else
	{
	#endif
#endif

#if MAC_CARBON

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
#endif // MAC_CARBON

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
// CMenuItem
//------------------------------------------------------------------------
/*! @class CMenuItem
Defines an item of a VSTGUI::COptionMenu
*/
//------------------------------------------------------------------------
/**
 * CMenuItem constructor.
 * @param inTitle title of item
 * @param inFlags CMenuItem::Flags of item
 * @param inKeycode keycode of item
 * @param inKeyModifiers keymodifiers of item
 * @param inIcon icon of item
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const char* inTitle, const char* inKeycode, long inKeyModifiers, CBitmap* inIcon, long inFlags)
: title (0)
, flags (inFlags)
, keycode (0)
, keyModifiers (0)
, submenu (0)
, icon (0)
, tag (-1)
{
	setTitle (inTitle);
	setKey (inKeycode, inKeyModifiers);
	setIcon (inIcon);
}

//------------------------------------------------------------------------
/**
 * CMenuItem constructor.
 * @param inTitle title of item
 * @param inSubmenu submenu of item
 * @param inIcon icon of item
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const char* inTitle, COptionMenu* inSubmenu, CBitmap* inIcon)
: title (0)
, flags (0)
, keycode (0)
, keyModifiers (0)
, submenu (0)
, icon (0)
, tag (-1)
{
	setTitle (inTitle);
	setSubmenu (inSubmenu);
	setIcon (inIcon);
}

//------------------------------------------------------------------------
/**
 * CMenuItem constructor.
 * @param inTitle title of item
 * @param inTag tag of item
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const char* inTitle, long inTag)
: title (0)
, flags (0)
, keycode (0)
, keyModifiers (0)
, submenu (0)
, icon (0)
, tag (-1)
{
	setTitle (inTitle);
	setTag (inTag);
}

//------------------------------------------------------------------------
/**
 * CMenuItem copy constructor.
 * @param item item to copy
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const CMenuItem& item)
: title (0)
, flags (item.flags)
, keycode (0)
, keyModifiers (0)
, submenu (0)
, icon (0)
{
	setTitle (item.getTitle ());
	setIcon (item.getIcon ());
	setKey (item.getKeycode (), item.getKeyModifiers ());
	setTag (item.getTag ());
	*submenu = *item.getSubmenu ();
}

//------------------------------------------------------------------------
CMenuItem::~CMenuItem ()
{
	setIcon (0);
	setSubmenu (0);
	setTitle (0);
	setKey (0);
}

//------------------------------------------------------------------------
void CMenuItem::setTitle (const char* inTitle)
{
	if (title)
		free (title);
	title = 0;
	if (inTitle)
	{
		title = (char*)malloc (strlen (inTitle) + 1);
		strcpy (title, inTitle);
	}
}

//------------------------------------------------------------------------
void CMenuItem::setKey (const char* inKeycode, long inKeyModifiers)
{
	if (keycode)
		free (keycode);
	keycode = 0;
	if (inKeycode)
	{
		keycode = (char*)malloc (strlen (inKeycode) + 1);
		strcpy (keycode, inKeycode);
	}
	keyModifiers = inKeyModifiers;
}

//------------------------------------------------------------------------
void CMenuItem::setSubmenu (COptionMenu* inSubmenu)
{
	if (submenu)
		submenu->forget ();
	submenu = inSubmenu;
	if (submenu)
		submenu->remember ();
}

//------------------------------------------------------------------------
void CMenuItem::setIcon (CBitmap* inIcon)
{
	if (icon)
		icon->forget ();
	icon = inIcon;
	if (icon)
		icon->remember ();
}

//------------------------------------------------------------------------
void CMenuItem::setTag (long t)
{
	tag = t;
}

//------------------------------------------------------------------------
void CMenuItem::setEnabled (bool state)
{
	if (state)
		flags &= ~kDisabled;
	else
		flags |= kDisabled;
}

//------------------------------------------------------------------------
void CMenuItem::setChecked (bool state)
{
	if (state)
		flags |= kChecked;
	else
		flags &= ~kChecked;
}

//------------------------------------------------------------------------
void CMenuItem::setIsTitle (bool state)
{
	if (state)
		flags |= kTitle;
	else
		flags &= ~kTitle;
}

//------------------------------------------------------------------------
void CMenuItem::setIsSeparator (bool state)
{
	if (state)
		flags |= kSeparator;
	else
		flags &= ~kSeparator;
}

class CMenuItemList : public std::list<CMenuItem*>
{
public:
	CMenuItemList () {}
	CMenuItemList (const CMenuItemList& inList) : std::list<CMenuItem*> (inList) {}
};

typedef std::list<CMenuItem*>::iterator CMenuItemIterator;

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
, inPopup (false)
{
	this->listener = listener;
	this->tag = tag;

	currentIndex = -1;
	lastButton = kRButton;
	platformControl = 0;
	lastResult = -1;
	lastMenu = 0;

	#if MAC_CARBON
	menuID = 0;
	#endif
	
	if (bgWhenClick)
		bgWhenClick->remember ();

	menuItems = new CMenuItemList;
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COptionMenu::COptionMenu ()
: CParamDisplay (CRect (0, 0, 0, 0))
, platformControl (0)
, currentIndex (-1)
, bgWhenClick (0)
, lastButton (0)
, nbItemsPerColumn (-1)
, lastResult (-1)
, prefixNumbers (0)
, lastMenu (0)
, inPopup (false)
{
	menuItems = new CMenuItemList;
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COptionMenu::COptionMenu (const COptionMenu& v)
: CParamDisplay (v)
, platformControl (0)
, currentIndex (-1)
, bgWhenClick (v.bgWhenClick)
, lastButton (0)
, nbItemsPerColumn (v.nbItemsPerColumn)
, lastResult (-1)
, prefixNumbers (0)
, lastMenu (0)
, menuItems (new CMenuItemList (*v.menuItems))
, inPopup (false)
{
	if (bgWhenClick)
		bgWhenClick->remember ();

	CMenuItemIterator it = menuItems->begin ();
	while (it != menuItems->end ())
	{
		(*it)->remember ();
		it++;
	}
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COptionMenu::~COptionMenu ()
{
	removeAllEntry ();

	if (bgWhenClick)
		bgWhenClick->forget ();

	delete menuItems;
}

//------------------------------------------------------------------------
long COptionMenu::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.character == 0)
	{
		if (keyCode.virt == VKEY_RETURN)
		{
			if (bgWhenClick)
				invalid ();
			popup ();
			if (bgWhenClick)
				invalid ();
			return 1;
		}
		if (!(style & (kMultipleCheckStyle & ~kCheckStyle)))
		{
			if (keyCode.virt == VKEY_UP)
			{
				long value = (long)getValue ()-1;
				if (value >= 0)
				{
					CMenuItem* entry = getEntry (value);
					while (entry && (entry->isSeparator () || entry->isTitle () || !entry->isEnabled () || entry->getSubmenu ()))
						entry = getEntry (--value);
					if (entry)
					{
						beginEdit ();
						setValue (value);
						lastResult = getValue ();
						if (listener)
							listener->valueChanged (this);
						endEdit ();
						invalid ();
					}
				}
				return 1;
			}
			if (keyCode.virt == VKEY_DOWN)
			{
				long value = (long)getValue ()+1;
				if (value < getNbEntries ())
				{
					CMenuItem* entry = getEntry (value);
					while (entry && (entry->isSeparator () || entry->isTitle () || !entry->isEnabled () || entry->getSubmenu ()))
						entry = getEntry (++value);
					if (entry)
					{
						beginEdit ();
						setValue (value);
						lastResult = getValue ();
						if (listener)
							listener->valueChanged (this);
						endEdit ();
						invalid ();
					}
				}
				return 1;
			}
		}
	}
	return CParamDisplay::onKeyDown (keyCode);
}

//------------------------------------------------------------------------
bool COptionMenu::popup ()
{
	bool popupResult = false;
	if (!getFrame ())
		return popupResult;

	inPopup = true;

	beginEdit ();

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
				popupResult = true;
			}
		}
	}

	//---Destroy the menu----
	removeItems ();
	
	//---Update the dependencies
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
			popupResult = true;
		}
		endEdit();
		inPopup = false;
		return popupResult;
	}
#endif // MAC_COCOA
	
#if MAC_CARBON
	// no entries, no menu
	if (getNbEntries () == 0)
	{
		endEdit();
		inPopup = false;
		return popupResult;
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

	setDirty (false);	

	//---Popup the Menu
	long popUpItem = style & kPopupStyle ? (value + 1) : 1;
	long PopUpMenuItem = PopUpMenuItem = PopUpMenuSelect (theMenu, LToG.v, LToG.h, popUpItem);

	//---Destroy the menu----
	removeItems ();
	
	// HiWord indicates MenuID, LoWord indicates the item index
	short result = LoWord (PopUpMenuItem) - 1;	
	lastResult = result;
	short menuIDResult = HiWord (PopUpMenuItem);
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
			popupResult = true;
		}
	}

#endif
	endEdit ();
	inPopup = false;
	return popupResult;
}

//------------------------------------------------------------------------
bool COptionMenu::popup (CFrame* frame, const CPoint& frameLocation)
{
	if (frame == 0)
		return false;
	if (isAttached ())
		return false;
	CView* oldFocusView = frame->getFocusView ();
	CRect size (frameLocation, CPoint (0, 0));
	setViewSize (size);
	frame->addView (this);
	popup ();
	frame->removeView (this, false);
	frame->setFocusView (oldFocusView);
	long index;
	if (getLastItemMenu (index))
		return true;
	return false;
}

//------------------------------------------------------------------------
void COptionMenu::setPrefixNumbers (long preCount)
{
	if (preCount >= 0 && preCount <= 4)
		prefixNumbers = preCount;
}

/**
 * @param item menu item to add. Takes ownership of item.
 * @param index position of insertation. -1 appends the item
 */
//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addEntry (CMenuItem* item, long index)
{
	if (index == -1)
		menuItems->push_back (item);
	else
	{
		CMenuItemIterator it = menuItems->begin ();
		for (int i = 0; i < index && it != menuItems->end (); i++, it++);
		menuItems->insert (it, item);
	}
	return item;
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addEntry (COptionMenu* submenu, const char* title)
{
	CMenuItem* item = new CMenuItem (title, submenu);
	return addEntry (item);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addEntry (const char* title, long index, long itemFlags)
{
	if (title && strcmp (title, "-") == 0)
		return addSeparator ();
	CMenuItem* item = new CMenuItem (title, 0, 0, 0, itemFlags);
	return addEntry (item, index);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addSeparator ()
{
	CMenuItem* item = new CMenuItem ("", 0, 0, 0, CMenuItem::kSeparator);
	return addEntry (item);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::getCurrent () const
{
	return getEntry (currentIndex);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::getEntry (long index) const
{
	if (menuItems->empty())
		return 0;
	
	CMenuItemIterator it = menuItems->begin ();
	for (int i = 0; i < index && it != menuItems->end (); i++, it++);
	if (it == menuItems->end ())
		return 0;
	return (*it);
}

//-----------------------------------------------------------------------------
long COptionMenu::getNbEntries () const
{
	return (long) menuItems->size ();
}

//------------------------------------------------------------------------
COptionMenu* COptionMenu::getSubMenu (long idx) const
{
	CMenuItem* item = getEntry (idx);
	if (item)
		return item->getSubmenu ();
	return 0;
}

//------------------------------------------------------------------------
long COptionMenu::getCurrentIndex (bool countSeparator) const
{
	if (countSeparator)
		return currentIndex;
	long i = 0;
	long numSeparators = 0;
	CMenuItemIterator it = menuItems->begin ();
	while (it != menuItems->end ())
	{
		if ((*it)->isSeparator ())
			numSeparators++;
		if (i == currentIndex)
			break;
		it++;
		i++;
	}
	return currentIndex - numSeparators;
}

//------------------------------------------------------------------------
bool COptionMenu::setCurrent (long index, bool countSeparator)
{
	CMenuItem* item = 0;
	if (countSeparator)
	{
		item = getEntry (index);
		if (!item || (item && item->isSeparator ()))
			return false;
		currentIndex = index;
	}
	else
	{
		long i = 0;
		CMenuItemIterator it = menuItems->begin ();
		while (it != menuItems->end ())
		{
			if (i > index)
				break;
			if ((*it)->isSeparator ())
				index++;
			it++;
			i++;
		}
		currentIndex = index;
		item = getEntry (currentIndex);
	}
	if (item && style & (kMultipleCheckStyle & ~kCheckStyle))
		item->setChecked (!item->isChecked ());
	
	// to force the redraw
	setDirty ();

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::removeEntry (long index)
{
	CMenuItem* item = getEntry (index);
	if (item)
	{
		menuItems->remove (item);
		item->forget ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool COptionMenu::removeAllEntry ()
{
	for (CMenuItemIterator it = menuItems->begin(); it != menuItems->end(); ++it)
	{
		(*it)->forget();
	}
	menuItems->clear ();
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::checkEntry (long index, bool state)
{
	CMenuItem* item = getEntry (index);
	if (item)
	{
		item->setChecked (state);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool COptionMenu::checkEntryAlone (long index)
{
	CMenuItemIterator it = menuItems->begin ();
	long pos = 0;
	while (it != menuItems->end ())
	{
		(*it)->setChecked (pos == index);
		it++;
		pos++;
	}
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::isCheckEntry (long index) const
{
	CMenuItem* item = getEntry (index);
	if (item && item->isChecked ())
		return true;
	return false;
}

//------------------------------------------------------------------------
void COptionMenu::draw (CDrawContext *pContext)
{
	CMenuItem* item = getEntry (currentIndex);
	drawBack (pContext, inPopup ? bgWhenClick : 0);
	drawText (pContext, item ? item->getTitle () : 0);
}

//------------------------------------------------------------------------
CMouseEventResult COptionMenu::onMouseDown (CPoint& where, const long& buttons)
{
	lastButton = buttons;
	if (lastButton & (kLButton|kRButton|kApple))
	{
		if (bgWhenClick)
			invalid ();
		popup ();
		if (bgWhenClick)
			invalid ();
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
	offsetIdx += getNbEntries ();

	if (idx < offsetIdx)
	{
		idxInMenu = idx - oldIDx;
		return this;
	}
	
#elif MAC_CARBON
	if (menuID == offsetIdx)
	{
		idxInMenu = idx;
		return this;
	}
#endif
	COptionMenu *menu = 0;
	CMenuItemIterator it = menuItems->begin ();
	while (it != menuItems->end ())
	{
		if ((*it)->getSubmenu ())
		{
			menu = (*it)->getSubmenu ()->getItemMenu (idx, idxInMenu, offsetIdx);
			if (menu)
				break;
		}
		it++;
	}
	return menu;
}

//------------------------------------------------------------------------
void COptionMenu::removeItems ()
{
	CMenuItemIterator it = menuItems->begin ();
	while (it != menuItems->end ())
	{
		if ((*it)->getSubmenu ())
			(*it)->getSubmenu ()->removeItems ();
		it++;
	}
	
#if WINDOWS
	#if GDIPLUS
	// destroy item bitmaps
	long idx = 0;
	it = menuItems->begin ();
	while (it != menuItems->end ())
	{
		if ((*it)->getIcon ())
		{
			MENUITEMINFO mInfo = {0};
			mInfo.cbSize = sizeof (MENUITEMINFO);
			mInfo.fMask = MIIM_BITMAP;
			if (GetMenuItemInfo ((HMENU)platformControl, idx, TRUE, &mInfo))
			{
				if (mInfo.hbmpItem)
					DeleteObject (mInfo.hbmpItem);
			}
		}
		it++;
		idx++;
	}

	#endif
	// destroy the menu
	if (platformControl)
		DestroyMenu ((HMENU)platformControl);
	platformControl = 0;

#elif MAC_CARBON
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
#if WINDOWS || MAC_CARBON
	bool multipleCheck = style & (kMultipleCheckStyle & ~kCheckStyle);
#endif

#if WINDOWS
	void *menu = (void*)CreatePopupMenu ();
	
	bool ownerDraw = false;

	int flags = 0;
	long idxSubmenu = 0;
	long offset = offsetIdx;
	long nbEntries = getNbEntries ();
	offsetIdx += nbEntries;
	long inc = 0;
	CMenuItemIterator it = menuItems->begin ();
	while (it != menuItems->end ())
	{
		CMenuItem* item = (*it);
		if (item->isSeparator ())
		{
			if (ownerDraw)
				AppendMenu ((HMENU)menu, MF_OWNERDRAW|MF_SEPARATOR, 0, 0);
			else
				AppendMenu ((HMENU)menu, MF_SEPARATOR, 0, 0);
		}
		else
		{
			char* titleWithPrefixNumbers = 0;
			if (getPrefixNumbers ())
			{
				titleWithPrefixNumbers = (char*)malloc (strlen (item->getTitle ()) + 50);
				switch (getPrefixNumbers ())
				{
					case 2:
					{
						sprintf (titleWithPrefixNumbers, "%1d %s", inc+1, item->getTitle ());
						break;
					}
					case 3:
					{
						sprintf (titleWithPrefixNumbers, "%02d %s", inc+1, item->getTitle ());
						break;
					}
					case 4:
					{
						sprintf (titleWithPrefixNumbers, "%03d %s", inc+1, item->getTitle ());
						break;
					}
				}
			}
			UTF8StringHelper entryText (titleWithPrefixNumbers ? titleWithPrefixNumbers : item->getTitle ());
			flags = ownerDraw ? MF_OWNERDRAW : MF_STRING;
			if (nbEntries < 160 && nbItemsPerColumn > 0 && inc && !(inc % nbItemsPerColumn))
				flags |= MF_MENUBARBREAK;

			if (item->getSubmenu ())
			{
				void *submenu = item->getSubmenu ()->appendItems (offsetIdx);
				if (submenu)
				{
					AppendMenu ((HMENU)menu, flags|MF_POPUP|MF_ENABLED, (UINT_PTR)submenu, (const TCHAR*)entryText);
				}
			}
			else
			{
				if (item->isEnabled ())
					flags |= MF_ENABLED;
				else
					flags |= MF_GRAYED;
				if (item->isTitle ())
					flags |= MF_DISABLED;
				if (multipleCheck && item->isChecked ())
					flags |= MF_CHECKED;
				if (style & kCheckStyle && inc == currentIndex && item->isChecked ())
					flags |= MF_CHECKED;
				if (!(flags & MF_CHECKED))
					flags |= MF_UNCHECKED;
				AppendMenu ((HMENU)menu, flags, offset + inc, entryText);
				if (item->getIcon ())
				{
					MENUITEMINFO mInfo = {0};
					mInfo.cbSize = sizeof (MENUITEMINFO);
					mInfo.fMask = MIIM_BITMAP;
					#if GDIPLUS
					Gdiplus::Bitmap* bitmap = item->getIcon ()->getBitmap ();
					HBITMAP hBmp = NULL;
					bitmap->GetHBITMAP (Gdiplus::Color (0, 0, 0, 0), &hBmp);
					mInfo.hbmpItem = hBmp;
					#else
					mInfo.hbmpItem = item->getIcon ()->getHandle ();
					#endif
					SetMenuItemInfo ((HMENU)menu, offset + inc, TRUE, &mInfo);
				}
			}
			if (titleWithPrefixNumbers)
				free (titleWithPrefixNumbers);
		}
		inc++;
		it++;
	}
	platformControl = menu;
	return menu;
	
#elif MAC_CARBON
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
		CreateNewMenu (menuID, kMenuAttrCondenseSeparators, &theMenu);
		
	CMenuItemIterator it = menuItems->begin ();
	long i = 0;
	while (it != menuItems->end ())
	{
		i++;
		CMenuItem* item = (*it);
		if (item->isSeparator ())
			AppendMenuItemTextWithCFString (theMenu, CFSTR(""), kMenuItemAttrSeparator, 0, NULL);
		else
		{
			CFStringRef itemString = CFStringCreateWithCString (NULL, item->getTitle (), kCFStringEncodingUTF8);
			if (getPrefixNumbers ())
			{
				CFStringRef prefixString = 0;
				switch (getPrefixNumbers ())
				{
					case 2:
						prefixString = CFStringCreateWithFormat (NULL, 0, CFSTR("%1d "),i); break;
					case 3:
						prefixString = CFStringCreateWithFormat (NULL, 0, CFSTR("%02d "),i); break;
					case 4:
						prefixString = CFStringCreateWithFormat (NULL, 0, CFSTR("%03d "),i); break;
				}
				CFMutableStringRef newItemString = CFStringCreateMutable (0, 0);
				CFStringAppend (newItemString, prefixString);
				CFStringAppend (newItemString, itemString);
				CFRelease (itemString);
				CFRelease (prefixString);
				itemString = newItemString;
			}
			if (itemString == 0)
				continue;
			MenuItemAttributes itemAttribs = kMenuItemAttrIgnoreMeta;
			if (!item->isEnabled ())
				itemAttribs |= kMenuItemAttrDisabled;
			if (item->isTitle ())
				itemAttribs |= kMenuItemAttrSectionHeader;

			InsertMenuItemTextWithCFString (theMenu, itemString, i, itemAttribs, 0);

			if (item->isChecked () && multipleCheck)
				CheckMenuItem (theMenu, i, true);
			if (item->getSubmenu ())
			{
				void *submenu = item->getSubmenu ()->appendItems (offsetIdx);
				if (submenu)
					SetMenuItemHierarchicalID (theMenu, i, item->getSubmenu ()->getMenuID ());
			}
			if (item->getIcon ())
			{
				CGImageRef image = item->getIcon ()->createCGImage ();
				if (image)
				{
					SetMenuItemIconHandle (theMenu, i, kMenuCGImageRefType, (Handle)image);
					CGImageRelease (image);
				}
			}
			if (item->getKeycode ())
			{
				SetItemCmd (theMenu, i, item->getKeycode ()[0]);
				UInt8 keyModifiers = 0;
				long itemModifiers = item->getKeyModifiers ();
				if (itemModifiers & kShift)
					keyModifiers |= kMenuShiftModifier;
				if (!(itemModifiers & kControl))
					keyModifiers |= kMenuNoCommandModifier;
				if (itemModifiers & kAlt)
					keyModifiers |= kMenuOptionModifier;
				if (itemModifiers & kApple)
					keyModifiers |= kMenuControlModifier;
				
				SetMenuItemModifiers (theMenu, i, keyModifiers);
			}
			CFRelease (itemString);
		}
		it++;
	}
	// set the check
	if (style & kCheckStyle && !multipleCheck)
		CheckMenuItem (theMenu, currentIndex + 1, true);

	InsertMenu ((MenuHandle)theMenu, -1);
	
	platformControl = (void*)theMenu;
	return platformControl;
	
#endif
	return 0;
}

//------------------------------------------------------------------------
void COptionMenu::setValue (float val)
{
	if ((long)val < 0 || (long)val >= getNbEntries ())
		return;
	
	currentIndex = (long)val;
	if (style & (kMultipleCheckStyle & ~kCheckStyle))
	{
		CMenuItem* item = getCurrent ();
		if (item)
			item->setChecked (!item->isChecked ());
	}
	CParamDisplay::setValue (val);
	
	// to force the redraw
	setDirty ();
}

//------------------------------------------------------------------------
void COptionMenu::takeFocus ()
{
	CParamDisplay::takeFocus ();
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
	setNumSubPixmaps (background ? (short)(background->getHeight () / heightOfOneImage) : 0);
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
void CAnimKnob::setHeightOfOneImage (const CCoord& height)
{
	IMultiBitmapControl::setHeightOfOneImage (height);
	if (pBackground && heightOfOneImage)
		setNumSubPixmaps ((long)(pBackground->getHeight () / heightOfOneImage));
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setBackground (CBitmap *background)
{
	CKnob::setBackground (background);
	if (heightOfOneImage == 0)
		heightOfOneImage = size.height ();
	if (background && heightOfOneImage)
		setNumSubPixmaps ((long)(background->getHeight () / heightOfOneImage));
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
		CCoord tmp = heightOfOneImage * (getNumSubPixmaps () - 1);
		if (bInverseBitmap)
			where.v = (CCoord)((1 - value) * (float)tmp);
		else
			where.v = (CCoord)(value * (float)tmp);
		for (CCoord realY = 0; realY <= tmp, tmp > 0; realY += heightOfOneImage) 
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
	setNumSubPixmaps (background ? (long)(background->getHeight () / heightOfOneImage) : 0);

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
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CVerticalSwitch& v)
: CControl (v)
, offset (v.offset)
{
	setNumSubPixmaps (v.subPixmaps);
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
		CPoint where (0, heightOfOneImage * ((long)(value * (getNumSubPixmaps () - 1) + 0.5f)));

		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = (double)heightOfOneImage / (double)getNumSubPixmaps ();

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
		value = (long)((where.v - size.top) / coef) / (float)(getNumSubPixmaps () - 1);
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
	setNumSubPixmaps (background ? (long)(background->getWidth () / heightOfOneImage) : 0);

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
 * @param iMaxPositions ignored
 * @param background the switch bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, long iMaxPositions, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CHorizontalSwitch& v)
: CControl (v)
, offset (v.offset)
{
	setNumSubPixmaps (v.subPixmaps);
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
		CPoint where (0, heightOfOneImage * ((long)(value * (getNumSubPixmaps () - 1) + 0.5f)));

		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, where);
		else
			pBackground->draw (pContext, size, where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = (double)pBackground->getWidth () / (double)getNumSubPixmaps ();

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
		value = (long)((where.h - size.left) / coef) / (float)(getNumSubPixmaps () - 1);
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
	setNumSubPixmaps (3);
	setHeightOfOneImage (size.height ());
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
	setNumSubPixmaps (3);
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
{
	setHeightOfOneImage (size.getHeight ());
	setNumSubPixmaps (background ? (long)(background->getHeight () / heightOfOneImage) : 0);
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
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
}

//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CMovieBitmap& v)
: CControl (v)
, offset (v.offset)
{
	setNumSubPixmaps (v.subPixmaps);
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
		where.v += heightOfOneImage * (int)(value * (getNumSubPixmaps () - 1) + 0.5);

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
	setNumSubPixmaps (background ? (long)(background->getHeight () / heightOfOneImage) : 0);

	totalHeightOfBitmap = heightOfOneImage * getNumSubPixmaps ();
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
, bWindowOpened (false)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	totalHeightOfBitmap = heightOfOneImage * getNumSubPixmaps ();
}

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CAutoAnimation& v)
: CControl (v)
, offset (v.offset)
, totalHeightOfBitmap (v.totalHeightOfBitmap)
, bWindowOpened (v.bWindowOpened)
{
	setNumSubPixmaps (v.subPixmaps);
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
, minPos (iMinPos)
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
	}
	else
	{
		minPos = iMinPos - size.top;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
	}

	CPoint p (0, 0);
	setOffsetHandle (p);

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
void CSlider::setStyle (long _style)
{
	style =_style;
}

//------------------------------------------------------------------------
void CSlider::setViewSize (CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	if (style & kHorizontal)
	{
		minPos = rect.left - size.left;
		rangeHandle = rect.getWidth () - (widthOfSlider + offsetHandle.h*2);
	}
	else
	{
		minPos = rect.top - size.top;
		rangeHandle = rect.getHeight () - (heightOfSlider + offsetHandle.v * 2);
	}
	
	widthControl  = rect.width ();
	heightControl = rect.height ();

	setOffsetHandle (offsetHandle);
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
	modalView = (CView*)v.modalView->newCopy ();
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
