//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
// Standard Control Objects
//
// Version 3.5       $Date: 2005-09-02 09:55:06 $
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

BEGIN_NAMESPACE_VSTGUI

#ifdef __MACH__ && __MWERKS__
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
// CControl
//------------------------------------------------------------------------
/*! @class CControl
This object manages the tag identification and the value of a control object.

Note:
Since version 2.1, when an object uses the transparency for its background and draws on it (tranparency area)
or the transparency area changes during different draws (CMovieBitmap ,...), the background will be false (not updated),
you have to rewrite the draw function in order to redraw the background and then call the draw of the object.
*/
CControl::CControl (const CRect &size, CControlListener *listener, long tag,
					CBitmap *pBackground)
:	CView (size), 
	listener (listener), tag (tag), oldValue (1), defaultValue (0.5f),
	value (0), vmin (0), vmax (1.f), wheelInc (0.1f), lastTicks (-1)
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
CControl::~CControl ()
{
}

//------------------------------------------------------------------------
void CControl::beginEdit ()
{
	// begin of edit parameter
	if (listener)
		listener->controlBeginEdit (this);
	getFrame ()->setFocusView(this);
	getFrame ()->beginEdit (tag);
}

//------------------------------------------------------------------------
void CControl::endEdit ()
{
	// end of edit parameter
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

//------------------------------------------------------------------------
// COnOffButton
//------------------------------------------------------------------------
/*! @class COnOffButton
Define a button with 2 positions.
The pixmap includes the 2 subpixmaps (i.e the rectangle used for the display of this button is half-height of the pixmap).
When its value changes, the listener is called.
*/
COnOffButton::COnOffButton (const CRect &size, CControlListener *listener, long tag,
                            CBitmap *background, long style)
: CControl (size, listener, tag, background)
, style (style)
{}

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
void COnOffButton::mouse (CDrawContext *pContext, CPoint &where, long button)
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
	
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	invalid ();
	#endif
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

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseDown (CPoint &where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	value = ((long)value) ? 0.f : 1.f;

	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	invalid ();
	#endif

	if (listener)
		listener->valueChanged (this);

	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//------------------------------------------------------------------------
// CKnob
//------------------------------------------------------------------------
/*! @class CKnob
Define a knob with a given background and foreground handle.
The handle describes a circle over the background (between -45deg and +225deg).
By clicking Alt+Left Mouse the default value is used.
By clicking Alt+Left Mouse the value changes with a vertical move (version 2.1)
*/
CKnob::CKnob (const CRect &size, CControlListener *listener, long tag,
              CBitmap *background, CBitmap *handle, const CPoint &offset)
:	CControl (size, listener, tag, background), offset (offset), pHandle (handle)
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
		long width  = (long)pHandle->getWidth ();
		long height = (long)pHandle->getHeight ();
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
void CKnob::mouse (CDrawContext *pContext, CPoint &where, long button)
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
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			if (isDirty ())
				invalid ();
			#endif
		}
		getMouseLocation (pContext, where);
		doIdleStuff ();

	} while (button & kLButton);

	// end of edit parameter
	endEdit ();
}

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseDown (CPoint &where, const long& buttons)
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
CMouseEventResult CKnob::onMouseUp (CPoint &where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKnob::onMouseMoved (CPoint &where, const long& buttons)
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
			if (isDirty () && listener)
				listener->valueChanged (this);
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			if (isDirty ())
				invalid ();
			#endif
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CKnob::onWheel (const CPoint &where, const float &distance, const long &buttons)
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
	point.h = (long)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.v = (long)(radius - sinf (alpha) * (radius - inset) + 0.5f);
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
void CKnob::setHandleBitmap (CBitmap *bitmap)
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
CParamDisplay::CParamDisplay (const CRect &size, CBitmap *background, const long style)
:	CControl (size, 0, 0, background), stringConvert (0), stringConvert2 (0), string2FloatConvert (0),
	horiTxtAlign (kCenterText), style (style), bTextTransparencyEnabled (true)
{
	backOffset (0, 0);

	fontID      = kNormalFont;
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
CParamDisplay::~CParamDisplay ()
{}

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
			pContext->fillRect (size);
	
			if (!(style & (k3DIn|k3DOut|kNoFrame))) 
			{
				pContext->setFrameColor (frameColor);
				pContext->drawRect (size);
			}
		}
	}
	// draw the frame for the 3D effect
	if (style & (k3DIn|k3DOut)) 
	{
		if (style & k3DIn)
			pContext->setFrameColor (backColor);
		else
			pContext->setFrameColor (frameColor);
		CPoint p;
		pContext->moveTo (p (size.left, size.bottom));
		pContext->lineTo (p (size.left, size.top));
		pContext->lineTo (p (size.right + 1, size.top));

		if (style & k3DIn)
			pContext->setFrameColor (frameColor);
		else
			pContext->setFrameColor (backColor);
		pContext->moveTo (p (size.right, size.top + 1));
		pContext->lineTo (p (size.right, size.bottom));
		pContext->lineTo (p (size.left, size.bottom));
	}

	if (!(style & kNoTextStyle) && string)
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
			pContext->drawString (string, newSize, !bTextTransparencyEnabled, horiTxtAlign);
		}
		pContext->setFontColor (fontColor);
		pContext->drawString (string, size, !bTextTransparencyEnabled, horiTxtAlign);
		pContext->setClipRect (oldClip);
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFont (CFont fontID)
{
	// to force the redraw
	if (this->fontID != fontID)
		setDirty ();
	this->fontID = fontID;
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
void CParamDisplay::setStringConvert (void (*convert) (float value, char *string))
{
	stringConvert = convert;
}

//------------------------------------------------------------------------
void CParamDisplay::setStringConvert (void (*convert) (float value, char *string,
									  void *userDta), void *userData)
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
CTextLabel::CTextLabel (const CRect& size, const char* txt, CBitmap* background, const long style)
: CParamDisplay (size, background, style)
, text (0)
{
	setText (txt);
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
	freeText ();
	if (txt)
	{
		text = (char*)malloc (strlen (txt)+1);
		strcpy (text, txt);
	}
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
A pixmap can be used as background.
*/
CTextEdit::CTextEdit (const CRect &size, CControlListener *listener, long tag,
	const char *txt, CBitmap *background, const long style)
:	CParamDisplay (size, background, style), platformFontColor (0), platformControl (0),
	platformFont (0), editConvert (0), editConvert2 (0)
{
#if (MAC && !MACX)
	text_edit = 0;
#endif
	this->listener = listener;
	this->tag = tag;

	if (txt)
		strcpy (text, txt);
	else
		strcpy (text, "");
#if MAC
	// remember our VST plugin's resource map ID (it should be the current one at this moment)
	pluginResID = CurResFile();
#endif
#if QUARTZ
	textControl = 0;
#endif
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::~CTextEdit ()
{}

//------------------------------------------------------------------------
void CTextEdit::setText (char *txt)
{
	if (txt)
	{
		if (strcmp (text, txt))
		{
			strcpy (text, txt);

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
		#if MACX
		#if QUARTZ
		if (textControl)
		{
			HIViewSetNeedsDisplay (textControl, true);
		}
		else
		#endif
		TXNDraw ((TXNObject)platformControl, NULL);
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

//------------------------------------------------------------------------
void CTextEdit::mouse (CDrawContext *pContext, CPoint &where, long button)
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

//------------------------------------------------------------------------
CMouseEventResult CTextEdit::onMouseDown (CPoint &where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (getFrame ()->getFocusView () != this)
		{
			if (style & kDoubleClickStyle)
				if (!isDoubleClick ())
					return kMouseEventNotHandled;
		
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
					textEdit->looseFocus ();
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
#elif BEOS
#include <TextView.h>
#include <Window.h>

class BeTextView : public BTextView
{
public:
	BeTextView (CTextEdit* cTextEdit, BRect frame,
				const char *name, BRect textRect);
	void MakeFocus (bool focusState = true);
	void KeyDown (const char *bytes, int32 numBytes);
private:
	CTextEdit *cTextEdit;
};

//------------------------------------------------------------------------
BeTextView::BeTextView (CTextEdit* cTextEdit, BRect frame, const char *name, BRect textRect)
: BTextView (frame, name, textRect, B_FOLLOW_NONE), cTextEdit (cTextEdit)
{}

//------------------------------------------------------------------------
void BeTextView::MakeFocus (bool focusState)
{
	BTextView::MakeFocus (focusState);
	if (!focusState && cTextEdit)
		cTextEdit->looseFocus ();
}

//------------------------------------------------------------------------
void BeTextView::KeyDown (const char *bytes, int32 numBytes)
{
	if (cTextEdit && (bytes[0] == B_RETURN || bytes[0] == B_TAB))
		cTextEdit->looseFocus ();
	else
		BTextView::KeyDown (bytes, numBytes);
}
#endif

#if MACX
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

						WindowRef window = (WindowRef) (textEdit->getFrame ()->getSystemWindow());
						GrafPtr	savedPort;
						bool portChanged = window ? QDSwapPort (GetWindowPort (window), &savedPort) : false;

						// remember the current resource map ID
						short currentResID = CurResFile();
						short vstResFileID = textEdit->pluginResID;
						// if it's different (and if it's valid), set the current resource map ID to our plugin's resource map
						if ( (vstResFileID != currentResID) && (vstResFileID > 0) )
							UseResFile(vstResFileID);

						textEdit->looseFocus ();

						// revert the window port, if we changed it
						if (portChanged)
							QDSwapPort (savedPort, NULL);
						// revert the current resource map, if we changed it
						if ( (currentResID > 0) && (vstResFileID != currentResID) && (vstResFileID > 0) )
							UseResFile(currentResID);

						result = noErr;
					}
					#if QUARTZ
					else if (textEdit->textControl)
						break;
					#endif
					else if (modifiers & cmdKey)
					{
						result = noErr;
						TXNObject text_edit = (TXNObject) (textEdit->platformControl);
						switch (toupper(macCharCode))
						{
							// copy
							case 'C':
								if (!TXNIsSelectionEmpty(text_edit))
								{
									OSStatus scrapErr = ClearCurrentScrap();
									scrapErr = TXNCopy(text_edit);
									result = noErr;
								}
								break;
							// cut
							case 'X':
								if (!TXNIsSelectionEmpty(text_edit))
								{
									OSStatus scrapErr = ClearCurrentScrap();
									scrapErr = TXNCut(text_edit);
									result = noErr;
								}
								break;
							// paste
							case 'V':
								TXNPaste(text_edit);
								result = noErr;
								break;

							// select all
							case 'A':
								TXNSelectAll(text_edit);
								break;

							default:
								break;
						}
					}
					else
					{
						EventRecord eventRecord;
						if (ConvertEventRefToEventRecord (inEvent, &eventRecord))
							TXNKeyDown ((TXNObject)textEdit->platformControl, &eventRecord);
						result = noErr;
					}
					break;
				}
			}
			break;
		}
		case kEventClassMouse:
		{
			switch (eventKind)
			{
				case kEventMouseDown:
				case kEventMouseUp:
				{
					WindowRef window;
					GetEventParameter (inEvent, kEventParamWindowRef, typeWindowRef, NULL, sizeof (WindowRef), NULL, &window);
					HIPoint p;
					GetEventParameter (inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &p);
					Point point = {(short)p.y, (short)p.x};
					QDGlobalToLocalPoint (GetWindowPort (window), &point);
					Rect rect;
					TXNGetViewRect ((TXNObject)textEdit->platformControl, &rect);
					if (PtInRect (point, &rect))
					{
						EventRecord eventRecord;
						if (eventKind == kEventMouseDown && ConvertEventRefToEventRecord (inEvent, &eventRecord))
							TXNClick ((TXNObject)textEdit->platformControl, &eventRecord);
						result = noErr;
					}
					break;
				}
				case kEventMouseMoved:
				{
					TXNAdjustCursor ((TXNObject)textEdit->platformControl, NULL);
					break;
				}
			}
			break;
		}
		case kEventClassWindow:
		{
			WindowRef window;
			if (GetEventParameter (inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof (WindowRef), NULL, &window) != noErr)
				break;
			switch (eventKind)
			{
				case kEventWindowDeactivated:
				{
					result = CallNextEventHandler (inHandlerCallRef, inEvent);
					ClearKeyboardFocus (window);

					// set up the correct drawing port for the window
					GrafPtr	savedPort;
					bool portChanged = QDSwapPort (GetWindowPort (window), &savedPort);

					// remember the current resource map ID
					short currentResID = CurResFile();
					short vstResFileID = textEdit->pluginResID;
					// if it's different (and if it's valid), set the current resource map ID to our plugin's resource map
					if ( (vstResFileID != currentResID) && (vstResFileID > 0) )
						UseResFile(vstResFileID);

					textEdit->looseFocus ();

					// revert the window port, if we changed it
					if (portChanged)
						QDSwapPort (savedPort, NULL);
					// revert the current resource map, if we changed it
					if ( (currentResID > 0) && (vstResFileID != currentResID) && (vstResFileID > 0) )
						UseResFile(currentResID);

					break;
				}
			}
			break;
		}
	}
	return result;
}
#endif

#if MAC && CALL_NOT_IN_CARBON
#include <Scrap.h>
#include <ctype.h>
#define ClearCurrentScrap	ZeroScrap
#endif

//------------------------------------------------------------------------
void CTextEdit::takeFocus ()
{
	bWasReturnPressed = false;

#if WINDOWS || MACX
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

	wstyle |= WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	platformControl = (void*)CreateWindow (
		"EDIT", text, wstyle,
		rect.left, rect.top, rect.width () + 1, rect.height () + 1,
		(HWND)getFrame ()->getSystemWindow (), NULL, GetInstance (), 0);

	// get/set the current font
	LOGFONT logfont = {0};

	long fontH = gStandardFontSize [fontID];
	if (fontH > rect.height () - 2)
		fontH = rect.height () - 2;

	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = -fontH;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	strcpy (logfont.lfFaceName, gStandardFontName[fontID]);

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

#elif MAC
#if MACX
	WindowRef window = (WindowRef)getFrame ()->getSystemWindow ();
	#if QUARTZ
	Rect r;
	r.left   = (short)rect.left;// + 2;
	r.right  = (short)rect.right;// - 4;
	r.top    = (short)rect.top;// + 2;
	r.bottom = (short)rect.bottom;// - 4;
	if (rect.getHeight () > gStandardFontSize [fontID])
	{
		r.top = (short)(rect.top + rect.getHeight () / 2 - gStandardFontSize [fontID] / 2 + 1);
		r.bottom = (short)(r.top + gStandardFontSize [fontID]);
	}
	if (CreateEditUnicodeTextControl (NULL, &r, NULL, false, NULL, &textControl) == noErr)
	{
		HIViewAddSubview ((HIViewRef)getFrame ()->getPlatformControl (), textControl);
		HIViewSetFirstSubViewFocus ((HIViewRef)getFrame ()->getPlatformControl (), textControl);
		SetKeyboardFocus ((WindowRef)getFrame ()->getSystemWindow (), textControl, kControlEditTextPart);
		EventTypeSpec eventTypes[] = { { kEventClassWindow, kEventWindowDeactivated }, { kEventClassKeyboard, kEventRawKeyDown }, { kEventClassKeyboard, kEventRawKeyRepeat } };
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
		fontStyle.size = gStandardFontSize [fontID];
		extern const char* gMacXfontNames[];
		Str255 fontName;
		CopyCStringToPascal ((const char*)gMacXfontNames[fontID], fontName); 
		GetFNum (fontName, &fontStyle.font);
		SetControlData (textControl, kControlEditTextPart, kControlFontStyleTag, sizeof (fontStyle), &fontStyle);
		HIViewSetVisible (textControl, true);
	}
	#else
	static bool gTXNInititalized = false;
	if (!gTXNInititalized)
	{
		TXNMacOSPreferredFontDescription defaults;  // fontID, pointSize, encoding, and fontStyle
		defaults.fontID = 0;
		defaults.pointSize = kTXNDefaultFontSize;
		defaults.encoding = CreateTextEncoding(kTextEncodingMacRoman, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
		defaults.fontStyle = kTXNDefaultFontStyle;
		TXNInitOptions options = 0;
		TXNInitTextension(&defaults, 1, options);
		gTXNInititalized = true;
	}
	gTextEditCanceled = false;
	TXNFrameOptions iFrameOptions = kTXNMonostyledTextMask | kTXNDisableDragAndDropMask | kTXNSingleLineOnlyMask; //kTXNNoKeyboardSyncMask | kTXNDisableDragAndDropMask | kTXNSingleLineOnlyMask | kTXNMonostyledTextMask;
	TXNFrameID frameID = 0;
	TXNObjectRefcon iRefCon = 0;
	TXNObject object;
	Rect r;
	r.left   = (short)rect.left;
	r.right  = (short)rect.right;
	r.top    = (short)rect.top;
	r.bottom = (short)rect.bottom;
	OSStatus err;
	err = TXNNewObject (NULL, window, &r, iFrameOptions, kTXNTextEditStyleFrameType, kTXNSingleStylePerTextDocumentResType, kTXNMacOSEncoding, &object, &frameID, iRefCon);
	if (err == noErr)
	{
		platformControl = object;
		TXNSetFrameBounds ((TXNObject)platformControl, r.top, r.left, r.bottom, r.right, frameID);

		if (strlen (text) > 0)
			TXNSetData ((TXNObject)platformControl, kTXNTextData, (void*)text, strlen (text), kTXNStartOffset, kTXNEndOffset);
		// set background
		extern void CColor2RGBColor (const CColor &cc, RGBColor &rgb);
		RGBColor rgbBackColor = { 0, 0, 0};
		CColor2RGBColor (kWhiteCColor /*backColor*/, rgbBackColor);
		RGBColor rgbTextColor = { 32767, 32767, 32767 };
		CColor2RGBColor (kBlackCColor /*fontColor*/, rgbTextColor);
		RGBBackColor (&rgbBackColor);
		RGBForeColor (&rgbTextColor);
		TXNBackground txnBackground;
		txnBackground.bgType = kTXNBackgroundTypeRGB;
		txnBackground.bg.color = rgbBackColor;
		TXNSetBackground ((TXNObject)platformControl, &txnBackground);
		// set justification
		TXNControlTag	controlTag[1];
		TXNControlData	controlData[1];
		SInt32			just;
		switch (horiTxtAlign)
		{
			case kLeftText :	just = kTXNFlushLeft; break;
			case kRightText :	just = kTXNFlushRight; break;
			default :			just = kTXNCenter; break;
		}
		controlTag[0] = kTXNJustificationTag;
		controlData[0].sValue = just;
		TXNSetTXNObjectControls ((TXNObject)platformControl, false, 1, controlTag, controlData);
		// set font
		TXNTypeAttributes attributes[3];
		// font name
		extern const unsigned char* gMacXfontNames[];
		
		short familyID;
		#if QUARTZ
		Str255 fontName;
		CopyCStringToPascal ((const char*)gMacXfontNames[fontID], fontName); 
		GetFNum (fontName, &familyID);
		#else
		GetFNum (gMacXfontNames[fontID], &familyID);
		#endif

		ATSUFontID fontNameID;

		ATSUFONDtoFontID (familyID, 0, &fontNameID);
		
		attributes[0].tag = kATSUFontTag;
		attributes[0].size = sizeof(ATSUFontID);
		attributes[0].data.dataPtr = &fontNameID;
		// size
		long fontSize = gStandardFontSize [fontID] << 16; // needs to be in Fixed format
		attributes[1].tag = kTXNQDFontSizeAttribute;
		attributes[1].size = kTXNFontSizeAttributeSize;
		attributes[1].data.dataValue = fontSize;
		// color
		attributes[2].tag = kTXNQDFontColorAttribute;
		attributes[2].size = kTXNQDFontColorAttributeSize;
		attributes[2].data.dataPtr = &rgbTextColor;

		TXNSetTypeAttributes ((TXNObject)platformControl, 3, attributes, kTXNStartOffset, kTXNEndOffset);

		SetUserFocusWindow (window);
		AdvanceKeyboardFocus (window);
		TXNActivate ((TXNObject)platformControl, frameID, false);
		TXNFocus ((TXNObject)platformControl, true);
		EventTypeSpec eventTypes[] = { { kEventClassMouse, kEventMouseMoved }, { kEventClassMouse, kEventMouseDown }, { kEventClassMouse, kEventMouseUp }, { kEventClassWindow, kEventWindowDeactivated }, { kEventClassKeyboard, kEventRawKeyDown }, { kEventClassKeyboard, kEventRawKeyRepeat } };
		InstallWindowEventHandler (window, CarbonEventsTextControlProc, GetEventTypeCount (eventTypes), eventTypes, this, &gTextEditEventHandler);
		TXNSelectAll ((TXNObject)platformControl);

	}
	#endif //QUARTZ

#else
	bool ende = false;
	char c;
	EventRecord theEvent;
	Rect rect, vrect;

	platformControl = new char[256];
	
	rect.left   = size.left;
	rect.right  = size.right;
	rect.top    = size.top;
	rect.bottom = size.bottom;
	#if !TARGET_API_MAC_CARBON
	rect.bottom++;
	rect.right++;
	#endif
		
	if (pContext)
	{
		rect.left   += pContext->offset.h;
		rect.right  += pContext->offset.h;
		rect.top    += pContext->offset.v;
		rect.bottom += pContext->offset.v;
	}
	vrect = rect;
	
	vrect.top++;
	vrect.left++;
	vrect.right--;
	#if TARGET_API_MAC_CARBON
	vrect.bottom--;
	#endif
			
	RGBColor blackrgb = {0, 0, 0};
	RGBColor whitergb = {0xffff, 0xffff, 0xffff};
	RGBForeColor (&blackrgb);
	RGBBackColor (&whitergb);

	EraseRect (&rect);
	//FrameRect (&rect); // Dave
	#if !TARGET_API_MAC_CARBON
	InsetRect (&vrect, 0, -2);
	#endif
	
	TextFont (kFontIDHelvetica);
	TextSize (gStandardFontSize [fontID]);

	text_edit = TENew (&rect, &vrect);
	
	if (horiTxtAlign == kLeftText)
		TESetAlignment (teJustLeft, (TEHandle)text_edit);
	else if (horiTxtAlign == kRightText)
		TESetAlignment (teJustRight, (TEHandle)text_edit);
	else
		TESetAlignment (teJustCenter, (TEHandle)text_edit);

	char string[256];
	strcpy (string, text);
	TESetText (string, strlen (string), (TEHandle)text_edit);
	TESetSelect (0, strlen (string), (TEHandle)text_edit);
	TEUpdate (&(**(TEHandle)text_edit).viewRect, (TEHandle)text_edit);
	TEActivate ((TEHandle)text_edit);
	HLock ((Handle)text_edit);

	(**(TEHandle)text_edit).crOnly = -1;

	bLoosefocusWanted = false;

	while (!ende && !bLoosefocusWanted)
	{
		GetNextEvent (everyEvent, &theEvent);
		switch (theEvent.what)
		{
			case nullEvent:
				doIdleStuff ();
				break;
			case autoKey :
			case keyDown :
				RGBForeColor (&blackrgb);
				RGBBackColor (&whitergb);
				
				c = theEvent.message & charCodeMask;
				if (c == 13 || c == 3 || c == 27)
				{
					if (c == 13)
						bWasReturnPressed = true;
					ende = true;
				}
				else if (((theEvent.message >> 8) & 0xFF) == 0x75)
				{
			 		if ((**(TEHandle)text_edit).selEnd < (**(TEHandle)text_edit).teLength) 
					{
				 		if (((**(TEHandle)text_edit).selEnd - (**(TEHandle)text_edit).selStart) > 1)
				 			TEDelete ((TEHandle)text_edit);
				 		else
				 			TEKey (0x1D, (TEHandle)text_edit);
						c = 8;
					}
					else
						c = -1;
				}
				if (theEvent.modifiers & cmdKey)
				{
					switch (toupper(c))
					{
						// copy
						case 'C':
							if ((**(TEHandle)text_edit).selEnd > (**(TEHandle)text_edit).selStart)
							{
								OSStatus scrapErr = ClearCurrentScrap();
								TECopy((TEHandle)text_edit);
								if (scrapErr == noErr)
									scrapErr = TEToScrap();
							}
							break;
						// cut
						case 'X':
							if ((**(TEHandle)text_edit).selEnd > (**(TEHandle)text_edit).selStart)
							{
								OSStatus scrapErr = ClearCurrentScrap();
								TECut((TEHandle)text_edit);
								if (scrapErr == noErr)
									scrapErr = TEToScrap();
							}
							break;
						// paste
						case 'V':
							{
								OSErr scrapErr = TEFromScrap();
								TEPaste((TEHandle)text_edit);
							}
							break;
						// select all
						case 'A':
							TESetSelect(0, (**(TEHandle)text_edit).teLength, (TEHandle)text_edit);
							break;
						default:
							break;
					}
				}
				else if (!ende)
					TEKey (c, (TEHandle)text_edit);
				break;
			case mouseDown :
				GlobalToLocal (&theEvent.where);
				if (PtInRect (theEvent.where, &rect))
				{
					bool shiftdown = (theEvent.modifiers & shiftKey) != 0;
					TEClick (theEvent.where, shiftdown, (TEHandle)text_edit);
				}
				else
					ende = true;
				break;
		}
	}
	CharsHandle h = TEGetText ((TEHandle)text_edit);
	short length = (**(TEHandle)text_edit).teLength;
	if (length > 255)
		length = 255;
	strncpy ((char*)platformControl, (char*)*h, length);
	((char*)platformControl)[length] = 0;

	HUnlock ((Handle)text_edit);
	TEDeactivate ((TEHandle)text_edit);
	TEDispose ((TEHandle)text_edit);
	text_edit = 0;
	looseFocus ();
	
#endif
#elif BEOS
	BView* plugView = (BView*) getFrame ()->getSystemWindow ();
	CRect rect;
	getFrame ()->getSize (&rect);
	BRect r (rect.left + size.left, rect.top + size.top, rect.left + size.right, rect.top + size.bottom);
	BRect tr = r;
	tr.OffsetTo (B_ORIGIN);
	tr.InsetBy (1, 1);
	BeTextView* textView = new BeTextView (this, r, NULL, tr);
	plugView->Window ()->Activate (true);
	plugView->AddChild (textView);
	BFont	font;
	if (fontID < 0 || fontID >= kNumStandardFonts)
		fontID = kSystemFont;
	extern const char* gStandardFontStyle[];
	font.SetFamilyAndStyle (gStandardFontName[fontID], gStandardFontStyle[fontID]);
	font.SetSize (gStandardFontSize[fontID]);
	rgb_color c = { fontColor.red, fontColor.green, fontColor.blue, 255 };
	textView->SetFontAndColor (&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE, &c);
	rgb_color cv = { backColor.red, backColor.green, backColor.blue, 255 };
	textView->SetMaxBytes (255);
	textView->SetViewColor (cv);
	alignment a;
	if (horiTxtAlign == kRightText)
		a = B_ALIGN_RIGHT;
	else if (horiTxtAlign == kLeftText)
		a = B_ALIGN_LEFT;
	else
		a = B_ALIGN_CENTER;
	textView->SetAlignment (a);
	textView->SetText (text);
	textView->DisallowChar ('\n');
	textView->DisallowChar ('\t');
	textView->MakeResizable (true);
	textView->MakeFocus (true);
	textView->SelectAll ();
	platformControl = textView;
#endif
}

//------------------------------------------------------------------------
void CTextEdit::looseFocus ()
{
	// Call this yet to avoid recursive call
	endEdit();
	if (getFrame ()->getFocusView () == this)
		getFrame ()->setFocusView (0);

	if (platformControl == 0)
		return;

	char oldText[256];
	strcpy (oldText, text);
	
#if WINDOWS
	char newText[255];
	GetWindowText ((HWND)platformControl, newText, 255);
	strcpy (text, newText);

	DestroyWindow ((HWND)platformControl);
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

#elif MAC

	#if MACX
	if (platformControl == 0)
		return;

	if (gTextEditEventHandler)
		RemoveEventHandler (gTextEditEventHandler);
	gTextEditEventHandler = 0;
	#if QUARTZ
	if (textControl)
	{
		CFStringRef cfstr;
		if (!gTextEditCanceled && GetControlData (textControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof cfstr, (void*)&cfstr, NULL) == noErr)
		{
			CFStringGetCString (cfstr, text, 255, kCFStringEncodingUTF8);
			CFRelease (cfstr);
		}
		HIViewSetVisible (textControl, false);
		HIViewRemoveFromSuperview (textControl);
		textControl = 0;
		pParentFrame->setCursor (kCursorDefault);
	}
	else
	#endif
	{
		if (!gTextEditCanceled)
		{
			CharsHandle dataHandle;
			TXNGetDataEncoded ((TXNObject)platformControl, kTXNStartOffset, kTXNEndOffset, &dataHandle, kTXNTextData);
			if (dataHandle != NULL && GetHandleSize (dataHandle) > 0)
			{
				long s = GetHandleSize (dataHandle);
				strncpy (text, *dataHandle, (s > 255) ? 255 : s);
				text [(s > 255) ? 255 : s] = 0;
				DisposeHandle (dataHandle);
			}
			else
				text[0] = 0;
		}
		TXNFocus ((TXNObject)platformControl, false);
		TXNDeleteObject ((TXNObject)platformControl);
	}
	
	platformControl = 0;

	setDirty (true);
	
	#else

	strcpy (text, (char*)platformControl);
	delete[] platformControl;
	#endif
#elif BEOS
	BTextView* textView = (BTextView*) platformControl;
	strncpy (text, textView->Text (), 255);
	if (textView->LockLooper ())
	{
		BWindow* w = textView->Window ();
		textView->RemoveSelf ();
		w->Unlock ();
	}
	delete textView;

#endif

	// update dependency
	bool change = false;
	if (strcmp (oldText, text))
	{
		change = true;
		if (listener)
			listener->valueChanged (this);
	}

	platformControl = 0;

	if (change)
		doIdleStuff ();

	CView* receiver = pParentView ? pParentView : pParentFrame;
	if (receiver)
		receiver->notify (this, "LooseFocus");
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
	font = kNormalFontSmall;
	#if MAC_ENABLE_MENU_SCHEME
	registerWithToolbox ();
	#endif
}

//------------------------------------------------------------------------
COptionMenuScheme::~COptionMenuScheme ()
{
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
	pContext->fillRect (rect);
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
		pContext->fillEllipse (r);
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
	char *ptr = (char*)strstr (text, "\t");
	if (ptr)
	{
		char modifier[32];
		strcpy (modifier, ptr + 1);
		*ptr = 0;
		pContext->drawString (text, r, false, kLeftText);

		*ptr = '\t';
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
					ControlPartCode partHit = mouseLoc.y / kItemHeight + 1;
					char temp[1024];
					CPoint size;
					long yPos = 0;
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
					HIRect r = { {0, 0}, { 0, 0 }};
					ControlPartCode whichItem;
					RgnHandle outRegion = NULL;
					GetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof (whichItem), NULL, &whichItem);
					GetEventParameter (inEvent, kEventParamControlRegion, typeQDRgnHandle, NULL, sizeof(outRegion), NULL, &outRegion);
					if (whichItem <= 0)
					{
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
					SetRectRgn (outRegion, (short) r.origin.x, (short) r.origin.y,(short) r.origin.x + r.size.width, (short) r.origin.y+ r.size.height + 1);
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
					CRect rect (0, 0);
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
							context.setClipRect (rect);
							scheme->scheme->drawItem (entryText+offset, i, state, &context, rect);
							rect.offset (0, size.y);
						}
					}
					break;
				}
				case kEventControlGetFrameMetrics:
				{
					err = CallNextEventHandler (inCallRef, inEvent);
					HIViewFrameMetrics	metrics;
					GetEventParameter (inEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, NULL, sizeof (metrics), NULL, &metrics);
					metrics.top = metrics.bottom = 0;
					SetEventParameter (inEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, sizeof (metrics), &metrics);
					break;
				}
				case kEventControlOwningWindowChanged:
				{
					WindowRef newWindow = GetControlOwner (control);
					HIWindowChangeFeatures (newWindow, 0, kWindowIsOpaque);
					err = noErr;
					HIViewRef root = HIViewGetRoot (newWindow);
					if (root)
					{
						HIRect bounds, frame;
						HIViewGetBounds (root, &bounds);
						HIViewGetFrame (root, &frame);
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
						EventTypeSpec events [] = { { kEventClassControl, kEventControlDraw }, { kEventClassControl, kEventControlOwningWindowChanged }, { kEventClassControl, kEventControlGetFrameMetrics } };
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
				case kEventScrollableGetInfo:
				{
					HISize size;
					HIPoint origin = { 0, 0 };

					size.width = 200;
					size.height = kItemHeight * (scheme->menu->getNbEntries () + 1);;

					SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof( size ), &size );
					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof( size ), &size );
					SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof( origin ), &origin );

					// line size is 1/10th total size
					size.width /= 10;
					size.height /= 10;

					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof( size ), &size );

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
A pixmap can be used as background, a second pixmap can be used when the option menu is popuped.
There are 2 styles with or without a shadowed text. When a mouse click occurs, a popup menu is displayed.
*/
COptionMenu::COptionMenu (const CRect &size, CControlListener *listener, long tag,
                          CBitmap *background, CBitmap *bgWhenClick, const long style)
:	CParamDisplay (size, background, style), bgWhenClick (bgWhenClick), nbItemsPerColumn (-1),
	prefixNumbers (0), scheme (0)
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
bool COptionMenu::addEntry (COptionMenu *subMenu, char *txt)
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
bool COptionMenu::addEntry (char *txt, long index)
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

//------------------------------------------------------------------------
void COptionMenu::mouse (CDrawContext *pContext, CPoint &where, long button)
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

//------------------------------------------------------------------------
CMouseEventResult COptionMenu::onMouseDown (CPoint &where, const long& buttons)
{
	lastButton = buttons;
	if (lastButton & (kLButton|kRButton|kApple))
	{
		beginEdit ();
		if (bgWhenClick)
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			invalid ();
		#else
			setDirty ();
		#endif
		takeFocus ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
#if BEOS
#include <PopUpMenu.h>
#include <MenuItem.h>
#endif

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
		//---Separator-----
		if (!strcmp (entry[i], kMenuSeparator))
		{
			if (ownerDraw)
				AppendMenu ((HMENU)menu, MF_OWNERDRAW|MF_SEPARATOR, 0, entry[i]);
			else
				AppendMenu ((HMENU)menu, MF_SEPARATOR, 0, entry[i]);
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
						AppendMenu ((HMENU)menu, flags|MF_POPUP|MF_ENABLED, (UINT_PTR)submenu, entry[i] + 2);
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
				AppendMenu ((HMENU)menu, flags|MF_GRAYED, offset + inc, entry[i] + 2);
			}
			//---Disable entry--------
			else if (!strncmp (entry[i], kMenuTitle, 2))
			{
				AppendMenu ((HMENU)menu, flags|MF_DISABLED, offset + inc, entry[i] + 2);
			}
			//---Multiple Checked entry---
			else if (multipleCheck)
			{
				AppendMenu ((HMENU)menu, flags|MF_ENABLED | 
					(check[i] ? MF_CHECKED : MF_UNCHECKED), offset + inc, entry[i]);
			}
			//---Checked Entry---------
			else if (style & kCheckStyle)
			{
				AppendMenu ((HMENU)menu, flags|MF_ENABLED | 
					((i == currentIndex) ? MF_CHECKED : MF_UNCHECKED), offset + inc, entry[i]);
			}
			else
				AppendMenu ((HMENU)menu, flags|MF_ENABLED, offset + inc, entry[i]);
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
		theMenu = NewMenu (menuID, "\pPopUp");

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
		#if TARGET_API_MAC_CARBON

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

		#else
		Str255 menuItem;
		strcpy ((char*)menuItem, (const char*)"\p\0");
		menuItem[0] = strlen ((const char*)text2);
		AppendMenu (theMenu, "\pjunk");
		
		if (!strncmp (text2, kMenuTitle, 2) || !strncmp (text2, kMenuDisable, 2) || !strncmp (text2, kMenuSubMenu, 2))
		{
			menuItem[0] -= 2;
			strcat ((char*)menuItem, text2 + 2);
	
			//---Disable item--------
			if (!strncmp (text2, kMenuDisable, 2))
			{
				#if TARGET_API_MAC_CARBON
				DisableMenuItem (theMenu, i + 1);
				#else
				DisableItem (theMenu, i + 1);
				#endif
			}
			//---Submenu-------------
			else if (!strncmp (text2, kMenuSubMenu, 2))
			{
				if (idxSubmenu < nbSubMenus)
				{
					void *submenu = submenuEntry[idxSubmenu]->appendItems (offsetIdx);
					if (submenu)
					{
						SetMenuItemHierarchicalID (theMenu, i + 1, submenuEntry[idxSubmenu]->getMenuID ());
						idxSubmenu++;
					}
					else
						continue;
				}
				else
					continue;
			}
		}
		else
		{
			strcat ((char*)menuItem, text2);
		}
		SetMenuItemText (theMenu, i + 1, menuItem);

		#endif // !TARGET_API_MAC_CARBON

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

	bool multipleCheck = style & (kMultipleCheckStyle & ~kCheckStyle);
	lastResult = -1;
	lastMenu = 0;

#if MAC || WINDOWS
	// calculate Screen Position
	#if WINDOWS
	HWND hwnd = (HWND)getFrame ()->getSystemWindow ();

	#endif

	CRect rect;
/*	if (pContext)
	{
		rect.left = pContext->offsetScreen.h;
		rect.top  = pContext->offsetScreen.v;
	}
	else*/
	{
		#if WINDOWS
		RECT rctWinParent;
		GetWindowRect (hwnd, &rctWinParent);
		rect.left = rctWinParent.left;
		rect.top  = rctWinParent.top;
		#elif QUARTZ
		HIRect bounds;
		HIViewRef control = (HIViewRef)getFrame ()->getPlatformControl ();
		HIViewGetFrame (control, &bounds);
		WindowRef window = (WindowRef)getFrame ()->getSystemWindow ();
		WindowAttributes attr;
		GetWindowAttributes (window, &attr);
		if (attr & kWindowCompositingAttribute)
		{
			HIViewRef contentView;
			HIViewFindByID (HIViewGetRoot (window), kHIViewWindowContentID, &contentView);
			if (HIViewGetSuperview (control) != contentView)
				HIViewConvertRect (&bounds, control, contentView);
			bounds.origin.x += getFrame ()->hiScrollOffset.x;
			bounds.origin.y += getFrame ()->hiScrollOffset.y;
		}
		rect.left = (CCoord)bounds.origin.x;
		rect.top = (CCoord)bounds.origin.y;
		#endif
	}
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
		CDrawContext *pContextTemp = 0;
		HDC hdc;
		if (result != -1)
		{
	 		// create a local context
			hdc = GetDC (hwnd);
			pContextTemp = new CDrawContext (getFrame (), hdc, hwnd);
		}
	
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

		// redraw the display
		// AAAAARRRRGHHHHHHHHHHHHH!!
		//doIdleStuff ();
		//setDirty (false);

		if (pContextTemp)
		{
			delete pContextTemp;
			ReleaseDC (hwnd, hdc);
		}
	}

#elif MAC
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
	#if TARGET_API_MAC_CARBON
	Rect bounds;
	GetRegionBounds (rgn, &bounds);
	int bottom      = bounds.bottom;
	long menuHeight = GetMenuHeight (theMenu);
	#else
	int bottom      = (*rgn)->rgnBBox.bottom;
	long menuHeight = (*theMenu)->menuHeight;
	#endif

	// Calculate the size of one menu item (round to the next int)
	int menuItemSize = (menuHeight + nbEntries - 1) / nbEntries;

	setDirty (false);	

	//---Popup the Menu
	long popUpItem = 1;
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

#elif BEOS
	BPopUpMenu* popup = new BPopUpMenu ("popup", false, false);
	BMessage*	message;
	BMenuItem*	item;
	for (long i = 0; i < nbEntries; i++)
	{	
		if (strcmp (entry[i], kMenuSeparator) == 0)
			popup->AddSeparatorItem ();
		else
		{
			message = new BMessage (i);
			item = new BMenuItem (entry[i], message);
			popup->AddItem (item);
			if (multipleCheck)
			{
				if (check[i])
					item->SetMarked (true);
			}
			else if ((style & kCheckStyle) && currentIndex == i)
				item->SetMarked (true);
		}
	}
	CRect rect;
	getFrame ()->getSize (&rect);
	if (pContext)
	{
		rect.left += pContext->offset.h;
		rect.top  += pContext->offset.v;
	}
	long offset;
	if (style & kPopupStyle)
		offset = (int)(rect.top + size.top);
	else
		offset = (int)(rect.top + size.bottom);
	BPoint where (rect.left + size.left, offset);
	BView* plugView = (BView*) getFrame ()->getSystemWindow ();
	plugView->ConvertToScreen (&where);
	item = popup->Go (where);
	if (item)
	{
		message = item->Message ();
		if (message)
		{
			CDrawContext *pContextTemp = pContext;
			// create a local context
			if (!pContextTemp)
				pContextTemp = new CDrawContext (getFrame (), plugView, NULL);

			setValue (message->what);
			
			if (listener)
				listener->valueChanged (this);
	
			// redraw the display
			draw (pContextTemp);

			if (!pContext && pContextTemp)
				delete pContextTemp;
		}
	}
	delete popup;
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
}

//------------------------------------------------------------------------
// CAnimKnob
//------------------------------------------------------------------------
/*! @class CAnimKnob
Such as a CKnob control object, but there is a unique pixmap which contains different views (subpixmaps) of this knob.
According to the value, a specific subpixmap is displayed. The different subpixmaps are stacked in the pixmap object.
*/
CAnimKnob::CAnimKnob (const CRect &size, CControlListener *listener, long tag,
                      CBitmap *background, CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset), bInverseBitmap (false)
{
	heightOfOneImage = size.height ();
	subPixmaps = (short)(background->getHeight () / heightOfOneImage);
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect &size, CControlListener *listener, long tag,
                      long subPixmaps,         // number of subPixmaps
                      CCoord heightOfOneImage,   // height of one image in pixel
                      CBitmap *background, CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset), 
   subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage), bInverseBitmap (false)
{
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::~CAnimKnob ()
{}

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
			where.v = (long)((1 - value) * (float)tmp);
		else
			where.v = (long)(value * (float)tmp);
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
Each position has its subpixmap, each subpixmap is stacked in the given handle pixmap.
By clicking Alt+Left Mouse the default value is used.
*/
CVerticalSwitch::CVerticalSwitch (const CRect &size, CControlListener *listener, long tag,
                                  CBitmap *background, CPoint &offset)
: CControl (size, listener, tag, background), offset (offset)
{
	heightOfOneImage = size.height ();
	subPixmaps = (long)(background->getHeight () / heightOfOneImage);
	iMaxPositions = subPixmaps;

	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CRect &size, CControlListener *listener, long tag,
                                  long subPixmaps,       // number of subPixmaps
                                  CCoord heightOfOneImage, // height of one image in pixel
                                  long iMaxPositions,
                                  CBitmap *background, CPoint &offset)
: CControl (size, listener, tag, background), offset (offset),
	subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage),
	iMaxPositions (iMaxPositions)
{
	setDefaultValue (0.f);
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

//------------------------------------------------------------------------
void CVerticalSwitch::mouse (CDrawContext *pContext, CPoint &where, long button)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
		
		getMouseLocation (pContext, where);
		
		doIdleStuff ();
	}
	while (pContext->getMouseButtons () == button);

	// end of edit parameter
	endEdit ();
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseDown (CPoint &where, const long& buttons)
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
CMouseEventResult CVerticalSwitch::onMouseUp (CPoint &where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseMoved (CPoint &where, const long& buttons)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CHorizontalSwitch
//------------------------------------------------------------------------
/*! @class CHorizontalSwitch
Same as the CVerticalSwitch but horizontal.
*/
CHorizontalSwitch::CHorizontalSwitch (const CRect &size, CControlListener *listener, long tag,
								  CBitmap *background, CPoint &offset)
: CControl (size, listener, tag, background), offset (offset)
{
	heightOfOneImage = size.width ();
	subPixmaps = (long)(background->getWidth () / heightOfOneImage);
	iMaxPositions = subPixmaps;

	setDefaultValue (0.f);
}

//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect &size, CControlListener *listener, long tag,
                                  long subPixmaps,   // number of subPixmaps
                                  CCoord heightOfOneImage, // height of one image in pixel
                                  long iMaxPositions,
                                  CBitmap *background, CPoint &offset)
: CControl (size, listener, tag, background), offset (offset),
	subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage),
	iMaxPositions (iMaxPositions)
{
	setDefaultValue (0.f);
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

//------------------------------------------------------------------------
void CHorizontalSwitch::mouse (CDrawContext *pContext, CPoint &where, long button)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
		
		getMouseLocation (pContext, where);
		
		doIdleStuff ();
	}
	while (pContext->getMouseButtons () == button);

	// end of edit parameter
	endEdit ();
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseDown (CPoint &where, const long& buttons)
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
CMouseEventResult CHorizontalSwitch::onMouseUp (CPoint &where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseMoved (CPoint &where, const long& buttons)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CRockerSwitch
//------------------------------------------------------------------------
/*! @class CRockerSwitch
Define a rocker switch with 3 states using 3 subpixmaps.
One click on its leftside, then the first subpixmap is displayed.
One click on its rightside, then the third subpixmap is displayed.
When the mouse button is relaxed, the second subpixmap is framed.
*/
CRockerSwitch::CRockerSwitch (const CRect &size, CControlListener *listener, long tag,              // identifier tag (ID)
                              CBitmap *background, CPoint &offset, const long style)
:	CControl (size, listener, tag, background), offset (offset), style (style)
{
	heightOfOneImage = size.width ();
}

//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRect &size, CControlListener *listener, long tag,              // identifier tag (ID)
                              CCoord heightOfOneImage, // height of one image in pixel
                              CBitmap *background, CPoint &offset, const long style)
:	CControl (size, listener, tag, background), offset (offset), 
	heightOfOneImage (heightOfOneImage), style (style)
{}

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
void CRockerSwitch::mouse (CDrawContext *pContext, CPoint &where, long button)
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
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			if (isDirty ())
				invalid ();
			#endif

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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}

	value = 0.f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	if (isDirty ())
		invalid ();
	#endif

	// end of edit parameter
	endEdit ();
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseDown (CPoint &where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseUp (CPoint &where, const long& buttons)
{
	value = 0.f;
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	if (isDirty ())
		invalid ();
	#endif
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseMoved (CPoint &where, const long& buttons)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CRockerSwitch::onWheel (const CPoint &where, const float &distance, const long &buttons)
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
/*! @class CMovieBitmap
A movie pixmap allows to display different subpixmaps according to its current value.
*/
CMovieBitmap::CMovieBitmap (const CRect &size, CControlListener *listener, long tag,
                            CBitmap *background, CPoint &offset)
  :	CControl (size, listener, tag, background), offset (offset),
		subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage)
{
	heightOfOneImage = size.height ();
	subPixmaps = (long)(background->getHeight () / heightOfOneImage);
}

//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect &size, CControlListener *listener, long tag,
                            long subPixmaps,        // number of subPixmaps
                            CCoord heightOfOneImage,  // height of one image in pixel
                            CBitmap *background, CPoint &offset)
  :	CControl (size, listener, tag, background), offset (offset),
		subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage)
{}

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
/*! @class CMovieButton
A movie button is a bi-states button with 2 subpixmaps. These subpixmaps are stacked in the pixmap.
*/
CMovieButton::CMovieButton (const CRect &size, CControlListener *listener, long tag,              // identifier tag (ID)
                            CBitmap *background, CPoint &offset)
: CControl (size, listener, tag, background), offset (offset), buttonState (value)
{
	heightOfOneImage = size.height ();
}

//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CRect &size, CControlListener *listener, long tag,
                            CCoord heightOfOneImage, // height of one image in pixel
                            CBitmap *background, CPoint &offset)
	:	CControl (size, listener, tag, background), offset (offset),
		heightOfOneImage (heightOfOneImage), buttonState (value)
{}

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
void CMovieButton::mouse (CDrawContext *pContext, CPoint &where, long button)
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
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			if (isDirty ())
				invalid ();
			#endif
	    
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}

	// end of edit parameter
	endEdit ();

	buttonState = value;
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseDown (CPoint &where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseUp (CPoint &where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseMoved (CPoint &where, const long& buttons)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CAutoAnimation
//------------------------------------------------------------------------
/*! @class CAutoAnimation
An auto-animation control contains a given number of subpixmap which can be displayed in loop.
Two functions allows to get the previous or the next subpixmap (these functions increase or decrease the current value of this control).
*/
// displays bitmaps within a (child-) window
CAutoAnimation::CAutoAnimation (const CRect &size, CControlListener *listener, long tag,
                                CBitmap *background, CPoint &offset)
: CControl (size, listener, tag, background), offset (offset), bWindowOpened (false)
{
	heightOfOneImage = size.height ();
	subPixmaps = (long)(background->getHeight () / heightOfOneImage);

	totalHeightOfBitmap = heightOfOneImage * subPixmaps;
}

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect &size, CControlListener *listener, long tag,
                                long subPixmaps,	 // number of subPixmaps...
                                CCoord heightOfOneImage, // height of one image in pixel
                                CBitmap *background, CPoint &offset)
	:	CControl (size, listener, tag, background), offset (offset),
		subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage),
		bWindowOpened (false)
{
	totalHeightOfBitmap = heightOfOneImage * subPixmaps;
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
void CAutoAnimation::mouse (CDrawContext *pContext, CPoint &where, long button)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		invalid ();
		#else
		setDirty (); // force to redraw
		#endif
		if (listener)
			listener->valueChanged (this);
	}
	else
	{                                                                       
		// stop info animation
		value = 0; // draw first pic of bitmap
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		invalid ();
		#else
		setDirty ();
		#endif
		closeWindow ();
	}
}

//------------------------------------------------------------------------
CMouseEventResult CAutoAnimation::onMouseDown (CPoint &where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (!isWindowOpened ())
		{	
			value = 0;
			openWindow ();
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			invalid ();
			#else
			setDirty (); // force to redraw
			#endif
			if (listener)
				listener->valueChanged (this);
		}
		else
		{                                                                       
			// stop info animation
			value = 0; // draw first pic of bitmap
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			invalid ();
			#else
			setDirty ();
			#endif
			closeWindow ();
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
By default the handler is drawn with transparency (white color).
By clicking Alt+Left Mouse the default value is used.
*/
CSlider::CSlider (const CRect &rect, CControlListener *listener, long tag,
                  long      iMinPos, // min position in pixel
                  long      iMaxPos, // max position in pixel
                  CBitmap  *handle,  // bitmap of slider
                  CBitmap  *background, // bitmap of background
                  CPoint   &offset,  // offset in the background
                  const long style)  // style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
  :	CControl (rect, listener, tag, background),	offset (offset), pHandle (handle),
	pOScreen (0), style (style), bFreeClick (true)
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
		rangeHandle = iMaxPos - iMinPos;
		CPoint p (0, 0);
		setOffsetHandle (p);
	}
	else
	{
		minPos = iMinPos - size.top;
		rangeHandle = iMaxPos - iMinPos;
		CPoint p (0, 0);
		setOffsetHandle (p);
	}

	zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, CControlListener *listener, long tag,
                  CPoint   &offsetHandle,    // handle offset
                  long     _rangeHandle, // size of handle range
                  CBitmap  *handle,     // bitmap of slider
                  CBitmap  *background, // bitmap of background
                  CPoint   &offset,     // offset in the background
                  const long style)     // style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
:	CControl (rect, listener, tag, background), offset (offset), pHandle (handle),
	pOScreen (0), style (style), minPos (0), bFreeClick (true)
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
CSlider::~CSlider ()
{
	if (pHandle)
		pHandle->forget ();
}

//------------------------------------------------------------------------
void CSlider::setOffsetHandle (CPoint &val)
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

//-----------------------------------------------------------------------------
bool CSlider::attached (CView *parent)
{
	if (pOScreen)
		delete pOScreen;
	#if !MACX
	pOScreen = new COffscreenContext (getFrame (), widthControl, heightControl, kBlackCColor);
	#endif		
	return CControl::attached (parent);
}

//-----------------------------------------------------------------------------
bool CSlider::removed (CView *parent)
{
	if (pOScreen)
	{
		delete pOScreen;
		pOScreen = 0;
	}
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CSlider::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pOScreen ? pOScreen : pContext;

	#if 1
	if (pOScreen && bTransparencyEnabled)
		pOScreen->copyTo (pContext, size);
	#endif
	float fValue;
	if (style & kLeft || style & kTop)
		fValue = value;
	else 
		fValue = 1.f - value;
	
	// (re)draw background
	CRect rect (0, 0, widthControl, heightControl);
	if (!pOScreen)
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
	if (!pOScreen)
		rectNew.offset (size.left, size.top);

	// draw slider at new position
	if (pHandle)
	{
		if (bDrawTransparentEnabled)
			pHandle->drawTransparent (drawContext, rectNew);
		else 
			pHandle->draw (drawContext, rectNew);
	}

	if (pOScreen)
		pOScreen->copyFrom (pContext, size);
	
	setDirty (false);
}

//------------------------------------------------------------------------
void CSlider::mouse (CDrawContext *pContext, CPoint &where, long button)
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif

		getMouseLocation (pContext, where);

		doIdleStuff ();
	}

	// end of edit parameter
	endEdit ();
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseDown (CPoint &where, const long& buttons)
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
	
	oldVal    = value;
	oldButton = buttons;

	beginEdit ();
	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseUp (CPoint &where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseMoved (CPoint &where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if ((oldButton != buttons) && (buttons & kShift))
		{
			oldVal    = value;
			oldButton = buttons;
		}
		else if (!(buttons & kShift))
			oldVal = value;

		if (style & kHorizontal)
			value = (float)(where.h - delta) / (float)rangeHandle;
		else
			value = (float)(where.v - delta) / (float)rangeHandle;
			
		if (style & kRight || style & kBottom)
			value = 1.f - value;

		if (buttons & kShift)
			value = oldVal + ((value - oldVal) / zoomFactor);
		bounceValue ();
    	    
		if (isDirty () && listener)
			listener->valueChanged (this);
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CSlider::onWheel (const CPoint &where, const float &distance, const long &buttons)
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
CVerticalSlider::CVerticalSlider (const CRect &rect, CControlListener *listener, long tag,
                                  long      iMinPos, // min position in pixel
                                  long      iMaxPos, // max position in pixel
                                  CBitmap  *handle,  // bitmap of slider
                                  CBitmap  *background, // bitmap of background
                                  CPoint   &offset,  // offset in the background
                                  const long style)  // style (kLeft, kRight)
  :	CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, CControlListener *listener, long tag,
                          CPoint   &offsetHandle,    // handle offset
                          long     rangeHandle, // size of handle range
                          CBitmap  *handle,     // bitmap of slider
                          CBitmap  *background, // bitmap of background
                          CPoint   &offset,     // offset in the background
                          const long style)     // style (kLeft, kRight)
:	CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kVertical)
{}


//------------------------------------------------------------------------
// CHorizontalSlider
//------------------------------------------------------------------------
/*! @class CHorizontalSlider
This is the horizontal slider. See CSlider.
*/
CHorizontalSlider::CHorizontalSlider (const CRect &rect, CControlListener *listener, long tag,
                                  long      iMinPos, // min Y position in pixel
                                  long      iMaxPos, // max Y position in pixel
                                  CBitmap  *handle,  // bitmap of slider
                                  CBitmap  *background, // bitmap of background
                                  CPoint   &offset,  // offset in the background
                                  const long style)  // style (kLeft, kRight)
  :	CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, CControlListener *listener, long tag,
                          CPoint   &offsetHandle,    // handle offset
                          long     rangeHandle, // size of handle range
                          CBitmap  *handle,     // bitmap of slider
                          CBitmap  *background, // bitmap of background
                          CPoint   &offset,     // offset in the background
                          const long style)     // style (kLeft, kRight)
:	CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kHorizontal)
{}


//------------------------------------------------------------------------
// CSpecialDigit
//------------------------------------------------------------------------
/*! @class CSpecialDigit
Can be used to display a counter with maximum 7 digits.
All digit have the same size and are stacked in height in the pixmap.
*/
CSpecialDigit::CSpecialDigit (const CRect &size,
                              CControlListener *listener,
                              long      tag,        // tag identifier
                              long     dwPos,      // actual value
                              long      iNumbers,   // amount of numbers (max 7)
                              long      *xpos,      // array of all XPOS
                              long      *ypos,      // array of all YPOS
                              long      width,      // width of ONE number
                              long      height,     // height of ONE number
                              CBitmap  *background)    // bitmap numbers
  :	CControl (size, listener, tag, background),
		iNumbers (iNumbers), width (width), height (height)
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
	else 
	{
		// store coordinates of x/y pos of each digit
		for (long i = 0; i < iNumbers; i++)
		{
			this->xpos[i] = xpos[i];
			this->ypos[i] = ypos[i];
		}
	}

	setMax ((float)pow (10, iNumbers) - 1.0f);
	setMin (0.0f);
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
		
		rectDest.left   = xpos[i];
		rectDest.top    = ypos[i];
		
		rectDest.right  = rectDest.left + width;
		rectDest.bottom = rectDest.top  + height;		
		
		// where = src from bitmap
		where.v = j * height;
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
Define a button with 2 states using 2 subpixmaps.
One click on it, then the second subpixmap is displayed.
When the mouse button is relaxed, the first subpixmap is framed.
*/
CKickButton::CKickButton (const CRect &size, CControlListener *listener, long tag,
                          CBitmap *background, CPoint &offset)
:	CControl (size, listener, tag, background), offset (offset)
{
	heightOfOneImage = size.height ();
}

//------------------------------------------------------------------------
CKickButton::CKickButton (const CRect &size, CControlListener *listener, long tag,
                          CCoord heightOfOneImage, // height of one image in pixel
                          CBitmap *background, CPoint &offset)
:	CControl (size, listener, tag, background), offset (offset), 
	heightOfOneImage (heightOfOneImage)
{}

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
void CKickButton::mouse (CDrawContext *pContext, CPoint &where, long button)
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
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			if (isDirty ())
				invalid ();
			#endif
			
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
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		invalid ();
		#endif
	}

	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	if (isDirty ())
		invalid ();
	#endif

	// end of edit parameter
	endEdit ();
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseDown (CPoint &where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseUp (CPoint &where, const long& buttons)
{
	if (value && listener)
		listener->valueChanged (this);
	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);
	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	if (isDirty ())
		invalid ();
	#endif
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseMoved (CPoint &where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (where.h >= size.left && where.v >= size.top  &&
			where.h <= size.right && where.v <= size.bottom)
			value = !fEntryState;
		else
			value = fEntryState;
		
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		if (isDirty ())
			invalid ();
		#endif
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
// CSplashScreen
//------------------------------------------------------------------------
/*! @class CSplashScreen
One click on its activated region and its pixmap is displayed, in this state the other control can not be used,
an another click on the displayed area reinstalls the normal frame.
This can be used to display a help view over the other views.
*/
// one click draw its pixmap, an another click redraw its parent
CSplashScreen::CSplashScreen (const CRect &size, CControlListener *listener, long tag,
                              CBitmap *background,
                              CRect   &toDisplay,
                              CPoint  &offset)
:	CControl (size, listener, tag, background), 
	toDisplay (toDisplay), offset (offset), bitmapTransparency (255)
{}

//------------------------------------------------------------------------
CSplashScreen::~CSplashScreen ()
{}

//------------------------------------------------------------------------
void CSplashScreen::setBitmapTransparency (unsigned char transparency)
{
	bitmapTransparency = transparency;
	setTransparency (bitmapTransparency != 255);
}

//------------------------------------------------------------------------
void CSplashScreen::draw (CDrawContext *pContext)
{
	if (value && pBackground)
	{
		if (bTransparencyEnabled)
		{
			if (bitmapTransparency)
				pBackground->drawAlphaBlend (pContext, toDisplay, offset, bitmapTransparency);
			else
			pBackground->drawTransparent (pContext, toDisplay, offset);
		}
		else
			pBackground->draw (pContext, toDisplay, offset);
	}
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
void CSplashScreen::mouse (CDrawContext *pContext, CPoint &where, long button)
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
		if (getFrame () && getFrame ()->setModalView (this))
		{
			keepSize = size;
			size = toDisplay;
			mouseableArea = size;
//			draw (pContext);
			if (listener)
				listener->valueChanged (this);
		}
		#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
		invalid ();
		#else
		setDirty ();
		#endif
	}
	else
	{
		size = keepSize;
		mouseableArea = size;
		if (listener)
			listener->valueChanged (this);
		if (getFrame ())
		{
			getFrame ()->setDirty (true);
			getFrame ()->setModalView (NULL);
		}
	}
}

//------------------------------------------------------------------------
CMouseEventResult CSplashScreen::onMouseDown (CPoint &where, const long& buttons)
{
	if (buttons & kLButton)
	{
		value = !value;
		if (value)
		{
			if (getFrame () && getFrame ()->setModalView (this))
			{
				keepSize = size;
				size = toDisplay;
				mouseableArea = size;
				if (listener)
					listener->valueChanged (this);
			}
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			invalid ();
			#else
			setDirty ();
			#endif
		}
		else
		{
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			invalid ();
			#endif
			size = keepSize;
			mouseableArea = size;
			if (listener)
				listener->valueChanged (this);
			if (getFrame ())
			{
				#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
				getFrame ()->setDirty (true);
				#endif
				getFrame ()->setModalView (NULL);
			}
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CSplashScreen::unSplash ()
{
	setDirty ();
	value = 0.f;

	size = keepSize;
	if (getFrame ())
	{
		if (getFrame ()->getModalView () == this)
		{
			getFrame ()->setModalView (NULL);
			#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
			getFrame ()->invalid ();
			#else
			getFrame ()->redraw ();
			#endif
		}
	}
}

//------------------------------------------------------------------------
// CVuMeter
//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CRect &size, CBitmap *onBitmap, CBitmap *offBitmap,
                    long nbLed, const long style)
	: CControl (size, 0, 0),
	  onBitmap (onBitmap), offBitmap (offBitmap), pOScreen (0),
	  nbLed (nbLed), style (style)
{
	setDecreaseStepValue (0.1f);

#if WINDOWS && !USE_LIBPNG
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
	}

	if (style & kHorizontal) 
	{
		CCoord tmp = (long)(((long)(nbLed * newValue + 0.5f) / (float)nbLed) * onBitmap->getWidth ());
		pointOff (tmp, 0);
		if (!bUseOffscreen)
		tmp += size.left;

		rectOff.left = tmp;
		rectOn.right = tmp;
	}
	else 
	{
		CCoord tmp = (long)(((long)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * onBitmap->getHeight ());
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

END_NAMESPACE_VSTGUI
//------------------------------------------------------------------------
// END.
//------------------------------------------------------------------------
