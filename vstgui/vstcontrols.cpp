//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
// Standard Control Objects
//
// Version 2.2         Date : 14/05/03
//
// First version            : Wolfgang Kundrus         06.97
// Added new objects        : Michael Schmidt          08.97
// Added new objects        : Yvan Grabit              01.98
// Added BeOS version       : Georges-Edouard Berenger 05.99
// Added MacOSX version     : Arne Scheffler           02.03
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2003, Steinberg Media Technologies, All Rights Reserved
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

BEGIN_NAMESPACE_VSTGUI

#ifdef __MACH__
 #ifndef cosf
 #define cosf (float)cos
 #endif

 #ifndef sinf
 #define sinf (float)sin
 #endif
#endif

//------------------------------------------------------------------------
// CControl
//------------------------------------------------------------------------
CControl::CControl (const CRect &size, CControlListener *listener, long tag,
 CBitmap *pBackground)
:	CView (size), 
	listener (listener), tag (tag), oldValue (1), defaultValue (0.5f),
	value (0), vmin (0), vmax (1.f), wheelInc (0.1f), lastTicks (-1), pBackground (pBackground)
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

	if (pBackground)
		pBackground->remember ();
}

//------------------------------------------------------------------------
CControl::~CControl ()
{
	if (pBackground)
		pBackground->forget ();
}

//------------------------------------------------------------------------
bool CControl::isDirty ()
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
void CControl::setBackground (CBitmap *_pBackground)
{
	if (pBackground)
		pBackground->forget ();
	pBackground = _pBackground;
	if (pBackground)
		pBackground->remember ();
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
bool CControl::isDoubleClick ()
{
	long ticks = getParent ()->getTicks ();
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
COnOffButton::COnOffButton (const CRect &size, CControlListener *listener, long tag,
                            CBitmap *background)
:	CControl (size, listener, tag, background)
{}

//------------------------------------------------------------------------
COnOffButton::~COnOffButton ()
{}

//------------------------------------------------------------------------
void COnOffButton::draw (CDrawContext *pContext)
{
	long off;

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
void COnOffButton::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

 	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	value = ((long)value) ? 0.f : 1.f;
	doIdleStuff ();
	if (listener)
		listener->valueChanged (pContext, this);
}


//------------------------------------------------------------------------
// CKnob
//------------------------------------------------------------------------
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
		long width  = pHandle->getWidth ();
		long height = pHandle->getHeight ();
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
void CKnob::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	// set the default value
	if (button == (kControl|kLButton))
	{
		value = getDefaultValue ();
		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		return;
	}

	float old = oldValue;
	CPoint firstPoint;
	bool  modeLinear = false;
	float fEntryState = value;
	float middle = (vmax - vmin) / 2.f;
	float range = 200.f;
	float coef = (vmax - vmin) / range;
	long  oldButton = button;

	long mode    = kCircularMode;
	long newMode = getParent ()->getKnobMode ();
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
	getParent ()->beginEdit (tag);
	getParent ()->setEditView (this);
	do
	{
		button = pContext->getMouseButtons ();
		if (where != oldWhere)
		{
			oldWhere = where;
			if (modeLinear)
			{
				long diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
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
				listener->valueChanged (pContext, this);
		}

		pContext->getMouseLocation (where);
		doIdleStuff ();
	
	} while (button & kLButton);

	// end of edit parameter
	getParent ()->endEdit (tag);
}

//------------------------------------------------------------------------
bool CKnob::onWheel (CDrawContext *pContext, const CPoint &where, float distance)
{
	if (!bMouseEnabled)
		return false;

	long buttons = pContext->getMouseButtons ();
	if (buttons & kShift)
		value += 0.1f * distance * wheelInc;
	else
		value += distance * wheelInc;
	bounceValue ();

	if (isDirty () && listener)
		listener->valueChanged (pContext, this);
	return true;
}

//------------------------------------------------------------------------
long CKnob::onKeyDown (VstKeyCode& keyCode)
{
#if !PLUGGUI
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
				listener->valueChanged (0, this);
		} return 1;
	}
#endif
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
	halfAngle = ((float)k2PI - fabsf (rangeAngle)) / 2.f;
	setDirty ();
}

//------------------------------------------------------------------------
void CKnob::valueToPoint (CPoint &point)
{
	float alpha = (value - bCoef) / aCoef;
	point.h = (long)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.v = (long)(radius - sinf (alpha) * (radius - inset) + 0.5f);
}

//------------------------------------------------------------------------
float CKnob::valueFromPoint (CPoint &point)
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

	if (stringConvert2)
	{
		string[0] = 0;
		stringConvert2 (value, string, userData);
	}
	else if (stringConvert)
	{
		string[0] = 0;
		stringConvert (value, string);
	}
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
	
			if (!(style & (k3DIn|k3DOut))) 
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
// CTextEdit
//------------------------------------------------------------------------
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
}

//------------------------------------------------------------------------
void CTextEdit::getText (char *txt)
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
		TXNDraw ((TXNObject)platformControl, NULL);
		#endif
		setDirty (false);
		return;
	}

	char string[256];

	if (editConvert2)
	{
		string[0] = 0;
		editConvert2 (text, string, userData);
	}
	else if (editConvert)
	{
		string[0] = 0;
		editConvert (text, string);
	}
	else
		sprintf (string, "%s", text);

	drawText (pContext, string);
	setDirty (false);
}

//------------------------------------------------------------------------
void CTextEdit::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if (button & kLButton)
	{
		if (getParent ()->getEditView () != this)
		{
			if (style & kDoubleClickStyle)
				if (!isDoubleClick ())
					return;
		
			getParent ()->setEditView (this);
			takeFocus (pContext);
		}
	}
}

//------------------------------------------------------------------------
#if WINDOWS
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

END_NAMESPACE_VSTGUI

#if PLUGGUI
	extern HINSTANCE ghInst;
	inline HINSTANCE GetInstance () { return ghInst; }
#else
	extern void* hInstance;
	inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }
#endif

BEGIN_NAMESPACE_VSTGUI

extern long standardFontSize [];
extern const char *standardFontName [];

#ifdef STRICT
#define WINDOWSPROC WNDPROC
#else
#define WINDOWSPROC FARPROC
#endif

static WINDOWSPROC oldWndProcEdit;
LONG WINAPI WindowProcEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LONG WINAPI WindowProcEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
				CTextEdit *textEdit = (CTextEdit*) GetWindowLong (hwnd, GWL_USERDATA);
				if (textEdit)
				{
					textEdit->bWasReturnPressed = true;
					textEdit->looseFocus ();
				}
			}
		} break;

		case WM_KILLFOCUS:
		{
			CTextEdit *textEdit = (CTextEdit*) GetWindowLong (hwnd, GWL_USERDATA);
			if (textEdit)
				textEdit->looseFocus ();
		} break;
	}

	return CallWindowProc (oldWndProcEdit, hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
#elif MOTIF
#include <Xm/Text.h>
extern XFontStruct *fontStructs[];
#endif

//------------------------------------------------------------------------
#if BEOS
#include <TextView.h>
#include <Window.h>

class BeTextView : public BTextView
{
public:
					BeTextView (  CTextEdit* 	cTextEdit,
								  BRect			frame,
								  const char	*name,
								  BRect			textRect);
			void	MakeFocus (bool focusState = true);
			void	KeyDown (const char *bytes, int32 numBytes);
private:
	CTextEdit *cTextEdit;
};

//------------------------------------------------------------------------
BeTextView::BeTextView (CTextEdit* cTextEdit, BRect frame, const char *name, BRect textRect) :
				BTextView (frame, name, textRect, B_FOLLOW_NONE), cTextEdit (cTextEdit)
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
	EventClass eventClass = GetEventClass (inEvent);
	EventKind eventKind = GetEventKind (inEvent);
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
			GetEventParameter (inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof (WindowRef), NULL, &window);
			switch (eventKind)
			{
				case kEventWindowDeactivated:
				{
					result = CallNextEventHandler (inHandlerCallRef, inEvent);
					ClearKeyboardFocus (window);
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
void CTextEdit::takeFocus (CDrawContext *pContext)
{
	bWasReturnPressed = false;

#if WINDOWS
	int wstyle = 0;
	if (horiTxtAlign == kLeftText)
		wstyle |= ES_LEFT;
	else if (horiTxtAlign == kRightText)
		wstyle |= ES_RIGHT;
	else
		wstyle |= ES_CENTER;

	CRect rect (size);
	if (pContext)
		rect.offset (pContext->offset.h, pContext->offset.v);

	wstyle |= WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	platformControl = (void*)CreateWindow (
		"EDIT", text, wstyle,
		rect.left, rect.top, rect.width () + 1, rect.height () + 1,
		(HWND)getParent ()->getSystemWindow (), NULL, GetInstance (), 0);

	// get/set the current font
	LOGFONT logfont = {0};

	long fontH = standardFontSize [fontID];
	if (fontH > rect.height () - 2)
		fontH = rect.height () - 2;

	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = -fontH;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	strcpy (logfont.lfFaceName, standardFontName[fontID]);

	logfont.lfClipPrecision	 = CLIP_STROKE_PRECIS;
	logfont.lfOutPrecision	 = OUT_STRING_PRECIS;
	logfont.lfQuality 	     = DEFAULT_QUALITY;
	logfont.lfCharSet        = ANSI_CHARSET;
  
	platformFont = (HANDLE)CreateFontIndirect (&logfont);
	platformFontColor = 0;

	SetWindowLong ((HWND)platformControl, GWL_USERDATA, (long)this);
	SendMessage ((HWND)platformControl, WM_SETFONT, (unsigned int)platformFont, true);
	SendMessage ((HWND)platformControl, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG (0, 0));
	SendMessage ((HWND)platformControl, EM_SETSEL, 0, -1);
	SendMessage ((HWND)platformControl, EM_LIMITTEXT, 255, 0);
	SetFocus ((HWND)platformControl);

	oldWndProcEdit = (WINDOWSPROC)SetWindowLong ((HWND)platformControl, GWL_WNDPROC, (long)WindowProcEdit);

#elif MAC
	extern long standardFontSize[];
#if MACX
	gTextEditCanceled = false;
	WindowRef window = (WindowRef)getParent ()->getSystemWindow ();
	TXNFrameOptions iFrameOptions = kTXNMonostyledTextMask | kTXNDisableDragAndDropMask; //kTXNNoKeyboardSyncMask | kTXNDisableDragAndDropMask | kTXNSingleLineOnlyMask | kTXNMonostyledTextMask;
	TXNObject txnObj = 0;
	TXNFrameID frameID = 0;
	TXNObjectRefcon iRefCon = 0;
	Rect rect;
	rect.left   = size.left;
	rect.right  = size.right;
	rect.top    = size.top;
	rect.bottom = size.bottom;
	if (pContext)
	{
		rect.left   += pContext->offset.h;
		rect.right  += pContext->offset.h;
		rect.top    += pContext->offset.v;
		rect.bottom += pContext->offset.v;
	}
	OSStatus err = TXNNewObject (NULL, window, &rect, iFrameOptions, kTXNTextEditStyleFrameType, kTXNSingleStylePerTextDocumentResType, kTXNMacOSEncoding, &txnObj, &frameID, iRefCon);
	if (err == noErr)
	{
		TXNSetFrameBounds (txnObj, rect.top, rect.left, rect.bottom, rect.right, frameID);
		platformControl = txnObj;

		if (strlen (text) > 0)
			TXNSetData ((TXNObject)platformControl, kTXNTextData, (void*)text, strlen (text), kTXNStartOffset, kTXNEndOffset);
		// set background
		extern void CColor2RGBColor (const CColor &cc, RGBColor &rgb);
		RGBColor rgbBackColor;
		CColor2RGBColor (backColor, rgbBackColor);
		RGBColor rgbTextColor = { 0, 0, 0 };
		CColor2RGBColor (fontColor, rgbTextColor);
		RGBBackColor (&rgbBackColor);
		RGBForeColor (&rgbTextColor);
		TXNBackground txnBackground;
		txnBackground.bgType = kTXNBackgroundTypeRGB;
		txnBackground.bg.color = rgbBackColor;
		TXNSetBackground (txnObj, &txnBackground);
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
		TXNSetTXNObjectControls (txnObj, false, 1, controlTag, controlData);
		// set font
		TXNTypeAttributes attributes[3];
		// font name
		extern const unsigned char* macXfontNames[];
		
		short familyID;
		GetFNum (macXfontNames[fontID], &familyID);

		ATSUFontID fontNameID;

		ATSUFONDtoFontID (familyID, 0, &fontNameID);
		
		attributes[0].tag = kATSUFontTag;
		attributes[0].size = sizeof(ATSUFontID);
		attributes[0].data.dataPtr = &fontNameID;
		// size
		long fontSize = standardFontSize [fontID] << 16; // needs to be in Fixed format
		attributes[1].tag = kTXNQDFontSizeAttribute;
		attributes[1].size = kTXNFontSizeAttributeSize;
		attributes[1].data.dataValue = fontSize;
		// color
		attributes[2].tag = kTXNQDFontColorAttribute;
		attributes[2].size = kTXNQDFontColorAttributeSize;
		attributes[2].data.dataPtr = &rgbTextColor;

		TXNSetTypeAttributes (txnObj, 3, attributes, kTXNStartOffset, kTXNEndOffset);

		TXNActivate (txnObj, frameID, false);
		SetUserFocusWindow (window);
		AdvanceKeyboardFocus (window);
		TXNFocus (txnObj, true);
		TXNSelectAll ((TXNObject)platformControl);
			
		EventTypeSpec eventTypes[] = { { kEventClassMouse, kEventMouseMoved }, { kEventClassMouse, kEventMouseDown }, { kEventClassMouse, kEventMouseUp }, { kEventClassWindow, kEventWindowDeactivated }, { kEventClassKeyboard, kEventRawKeyDown }, { kEventClassKeyboard, kEventRawKeyRepeat } };
		InstallWindowEventHandler (window, CarbonEventsTextControlProc, GetEventTypeCount (eventTypes), eventTypes, this, &gTextEditEventHandler);
	}

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
	#if !CARBON
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
	#if CARBON
	vrect.bottom--;
	#endif
			
	RGBColor blackrgb = {0, 0, 0};
	RGBColor whitergb = {0xffff, 0xffff, 0xffff};
	RGBForeColor (&blackrgb);
	RGBBackColor (&whitergb);

	EraseRect (&rect);
	//FrameRect (&rect); // Dave
	#if !CARBON
	InsetRect (&vrect, 0, -2);
	#endif
	
	TextFont (kFontIDHelvetica);
	TextSize (standardFontSize [fontID]);

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
				if (!ende)
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
#elif MOTIF
	// we have to add the Text to the parent !!
	Dimension posX, posY;
	Widget widget = (Widget)(getParent ()->getSystemWindow ());
	XtVaGetValues (widget, XmNx, &posX, XmNy, &posY, 0);
  
	Arg args[20];
	int n = 0;
	XtSetArg (args[n], XmNx, size.left + posX); n++;
	XtSetArg (args[n], XmNy, size.top + posY); n++;
	XtSetArg (args[n], XmNwidth, size.width () + 1); n++;
	XtSetArg (args[n], XmNheight, size.height () + 2); n++;
	
	XtSetArg (args[n], XmNvalue, text); n++;
	
	XtSetArg (args[n], XmNshadowType, XmSHADOW_IN); n++;
	XtSetArg (args[n], XmNshadowThickness, 0); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, true); n++;

	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNresizeHeight, True); n++;
	XtSetArg (args[n], XmNborderWidth, 0); n++;
	XtSetArg (args[n], XmNeditMode, XmSINGLE_LINE_EDIT); n++;
	
	// get/set the current font
	XmFontList fl = 0;
	XFontStruct* fs = fontStructs[fontID];
	if (fs)
	{
		XmFontListEntry entry = XmFontListEntryCreate (XmFONTLIST_DEFAULT_TAG, XmFONT_IS_FONT, fs);
		XmFontList fl = XmFontListAppendEntry (0, entry);
		XtSetArg (args[n], XmNfontList, fl); n++;
	}

	platformControl = XmCreateText (XtParent (widget), "Text", args, n);
	XtManageChild ((Widget)platformControl);
	if (fl)
		XmFontListFree (fl);
	XmTextSetSelection ((Widget)platformControl, 0, strlen (text), 0);
	XmTextSetHighlight ((Widget)platformControl, 0, strlen (text), XmHIGHLIGHT_SELECTED);

#elif BEOS
	BView* plugView = (BView*) getParent ()->getSystemWindow ();
	CRect rect;
	getParent ()->getSize (&rect);
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
	extern long standardFontSize[];
	extern const char* standardFontName[];
	extern const char* standardFontStyle[];
	font.SetFamilyAndStyle (standardFontName[fontID], standardFontStyle[fontID]);
	font.SetSize (standardFontSize[fontID]);
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
void CTextEdit::looseFocus (CDrawContext *pContext)
{
	#if MACX
	if (platformControl == 0)
		return;
	#else
	if (platformControl == 0 || getParent ()->getEditView () != this) 
		return;
	#endif
	
	// Call this yet to avoid recursive call
	getParent ()->setEditView (0);

	char oldText[256];
	strcpy (oldText, text);
	
#if WINDOWS
	HWND hwnd;
	HDC hdc;
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
	}

	TXNFocus ((TXNObject)platformControl, false);
	
	Rect size;
	TXNGetViewRect ((TXNObject)platformControl, &size);
	TXNDeleteObject ((TXNObject)platformControl);
	platformControl = 0;

	setDirty (true);
	doIdleStuff ();
	
	#else

	strcpy (text, (char*)platformControl);
	delete[] platformControl;
	#endif
#elif MOTIF
	char *pNewText = XmTextGetString ((Widget)platformControl);
	strcpy (text, pNewText);
	XtFree (pNewText);
	
	XtUnmanageChild ((Widget)platformControl);
	XtDestroyWidget ((Widget)platformControl);

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
	CDrawContext *pContextTemp = 0;
	if (!pContext)
	{
		// create a local context
#if WINDOWS
		hwnd = (HWND)getParent ()->getSystemWindow ();
		hdc = GetDC (hwnd);
		pContextTemp = new CDrawContext (getParent (), hdc, hwnd);

#elif MAC
		pContextTemp = new CDrawContext (getParent (), getParent ()->getSystemWindow (), getParent ()->getSystemWindow ());

#elif MOTIF
		pContextTemp = new CDrawContext (getParent (), getParent ()->getGC (), (void *)getParent ()->getWindow ());

#elif BEOS
		pContextTemp = new CDrawContext (getParent (), getParent ()->getSystemWindow (), NULL);
#endif
		if (getParentView ())
		{
			CView *view= getParentView ();
			CRect rect2;
			view->getViewSize (rect2);
			pContextTemp->offset.h = rect2.left;
			pContextTemp->offset.v = rect2.top;
		}
	}
	else
		pContextTemp = pContext;

	// update dependency
	if (strcmp (oldText, text) && listener)
		listener->valueChanged (pContextTemp, this);
	
	if (!pContext)
	{
		if (pContextTemp)
			delete pContextTemp;

		#if WINDOWS
		ReleaseDC (hwnd, hdc);
		#endif
	}

	platformControl = 0;

	CView* receiver = pParentView ? pParentView : pParent;
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

COptionMenuScheme::COptionMenuScheme ()
: nbReference (1)
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
}

//------------------------------------------------------------------------
void COptionMenuScheme::remember ()
{
	nbReference++;
}

//------------------------------------------------------------------------
void COptionMenuScheme::forget ()
{
	if (nbReference > 0)
	{
		nbReference--;
		if (nbReference == 0)
			delete this;
	}
}

//------------------------------------------------------------------------
void COptionMenuScheme::getItemSize (const char* text, CDrawContext* pContext, CPoint& size)
{
	if (!strcmp (text, kMenuSeparator)) // separator
	{
		// was: size.h = size.v = 6;
		size.h = 6;
		size.v = 18;
		// separators must have same height, otherwise we have problems
		// in multi-column menus :(
	}
	else
	{
		size.h = pContext->getStringWidth (text) + 18;
		size.v = 18;
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
		long y = rect.top + rect.height () / 2;
		
		pContext->setFrameColor (kBlackCColor);
		pContext->moveTo (CPoint (rect.left + 2, y - 1));
		pContext->lineTo (CPoint (rect.right - 2, y - 1));
		pContext->setFrameColor (kWhiteCColor);
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

	char *ptr = strstr (text, "\t");
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

//------------------------------------------------------------------------
// COptionMenu
//------------------------------------------------------------------------
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
long COptionMenu::getCurrent (char *txt, bool countSeparator)
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

	// to force the redraw
	setDirty ();

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::getEntry (long index, char *txt)
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
long COptionMenu::getIndex (char *txt)
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
bool COptionMenu::isCheckEntry (long index)
{
	if (index < 0 || index >= nbEntries)
		return false;

	return check[index];
}

//------------------------------------------------------------------------
void COptionMenu::draw (CDrawContext *pContext)
{
	if (currentIndex >= 0 && nbEntries) 
		drawText (pContext, entry[currentIndex] + prefixNumbers);
	else
		drawText (pContext, NULL);
}

//------------------------------------------------------------------------
void COptionMenu::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled || !getParent () || !pContext)
		return;

	lastButton = pContext->getMouseButtons ();
	if (lastButton & (kLButton|kRButton|kApple))
	{
		if (bgWhenClick)
		{
			char string[256];
			sprintf (string, "%s", entry[currentIndex]);
		
			drawText (pContext, string, bgWhenClick);
		}

		getParent ()->setEditView (this);
		takeFocus (pContext);
	}
}

//------------------------------------------------------------------------
#if MOTIF
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/SeparatoG.h>

static void _unmapCallback (Widget item, XtPointer clientData, XtPointer callData);
static void _activateCallback (Widget item, XtPointer clientData, XtPointer callData);

//------------------------------------------------------------------------
static void _unmapCallback (Widget item, XtPointer clientData, XtPointer callData)
{
	COptionMenu *optionMenu= (COptionMenu*)clientData;
	optionMenu->looseFocus ();
}

//------------------------------------------------------------------------
static void _activateCallback (Widget item, XtPointer clientData, XtPointer callData)
{
	COptionMenu *optionMenu= (COptionMenu*)clientData;
	optionMenu->setCurrentSelected ((void*)item);
}
#endif

//------------------------------------------------------------------------
#if BEOS
#include <PopUpMenu.h>
#include <MenuItem.h>
#endif

//------------------------------------------------------------------------
COptionMenu *COptionMenu::getLastItemMenu (long &idxInMenu)
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
	
	bool ownerDraw = scheme != 0;

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
						AppendMenu ((HMENU)menu, flags|MF_POPUP|MF_ENABLED, (UINT)submenu, entry[i] + 2);
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
		
	//---Create the menu
	MenuHandle theMenu = NewMenu (menuID, "\pPopUp");

	Str255 menuItem;
	char text2[256];
	long keyChar;
	long idxSubmenu = 0;
	long offset = offsetIdx;
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
				#if CARBON
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
void COptionMenu::takeFocus (CDrawContext *pContext)
{
	if (!getParent ())
		return;

	bool multipleCheck = style & (kMultipleCheckStyle & ~kCheckStyle);
	lastResult = -1;
	lastMenu = 0;

#if WINDOWS
	MSG msg;
	long result = -1;
	HWND hwnd = (HWND)getParent ()->getSystemWindow ();

	//---Get the position of the Parent
	CRect rect;
	if (pContext)
	{
		rect.left = pContext->offsetScreen.h;
		rect.top  = pContext->offsetScreen.v;
	}
	else
	{
		RECT rctWinParent;
		GetWindowRect (hwnd, &rctWinParent);
		rect.left = rctWinParent.left;
		rect.top  = rctWinParent.top;
	}

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
	
	getParent ()->setEditView (0);

	//---Update the dependencies
	if (result != -1 || bgWhenClick)
	{
		CDrawContext *pContextTemp = 0;
		HDC hdc;
		if (!pContext && result != -1)
		{
	 		// create a local context
			hdc = GetDC (hwnd);
			pContextTemp = new CDrawContext (getParent (), hdc, hwnd);
		}
		else
			pContextTemp = pContext;
	
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
					listener->valueChanged (pContextTemp, menu);
			}
		}

		// redraw the display
		// AAAAARRRRGHHHHHHHHHHHHH!!
		//doIdleStuff ();
		//setDirty (false);

		if (!pContext && pContextTemp)
		{
			delete pContextTemp;
			ReleaseDC (hwnd, hdc);
		}
	}

#elif MAC
	// no entries, no menu
	if (nbEntries == 0)
	{
		getParent ()->setEditView (0);
		return;
	}
	
	//---Get the position of the Parent
	WindowPtr theWindow = (WindowPtr)getParent ()->getSystemWindow ();
	
	//---Transform local coordinates to global coordinates
	long offset;
	if (style & kPopupStyle)
		offset = (int)size.top;
	else
		offset = (int)size.bottom;
	
	Rect bounds;
	GetPortBounds (GetWindowPort (theWindow), &bounds);
	Point LToG;
	LToG.v = bounds.top + offset;
	LToG.h = bounds.left + size.left;
	
	if (pContext)
	{
		LToG.h += pContext->offset.h;
		LToG.v += pContext->offset.v;
	}
	
	LocalToGlobal (&LToG);
	
	//---Create the popup menu---
	long offIdx = 0;
	MenuHandle theMenu = (MenuHandle)appendItems (offIdx);
		
	// Calculate the menu size (height and width)
	CalcMenuSize (theMenu);
	
	// Get a handle to the screen
	RgnHandle rgn = GetGrayRgn ();
	#if CARBON
	GetRegionBounds (rgn, &bounds);
	int bottom      = bounds.bottom;
	long menuHeight = GetMenuHeight (theMenu);
	#else
	int bottom      = (*rgn)->rgnBBox.bottom;
	long menuHeight = (*theMenu)->menuHeight;
	#endif

	// Calculate the size of one menu item (round to the next int)
	int menuItemSize = (menuHeight + nbEntries - 1) / nbEntries;
	
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

		CDrawContext *pContextTemp = 0;
		if (!pContext && menuIDResult != 0)
		{
			// create a local context
			pContextTemp = new CDrawContext (getParent (), 0, theWindow);
		}
		else
			pContextTemp = pContext;
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
					listener->valueChanged (pContextTemp, menu);
			}
		}

		// redraw the display
		doIdleStuff ();
		setDirty (false);

		if (!pContext && pContextTemp)
			delete pContextTemp;
	}
	if (getParent ()->getEditView () == this)
		getParent ()->setEditView (0);

#elif MOTIF
	Arg args[10];
	int n = 0;
	
	// get the position of the pParent
	CRect rect;
	getParent ()->getSize (&rect);

	if (pContext)
	{
		rect.left += pContext->offset.h;
		rect.top  += pContext->offset.v;
	}

	// create a popup menu
	int offset;
	if (style & kPopupStyle)
		offset = (int)(rect.top + size.top);
	else
		offset = (int)(rect.top + size.bottom);

	XtSetArg (args[n], XmNx, rect.left + size.left); n++;
	XtSetArg (args[n], XmNy, offset); n++;
	XtSetArg (args[n], XmNmenuHistory, currentIndex); n++;
	XtSetArg (args[n], XmNtraversalOn, true); n++;

	platformControl = (void*)XmCreatePopupMenu ((Widget)(getParent ()->getSystemWindow ()), 
			"popup", args, n);

	XtAddCallback ((Widget)platformControl, XmNunmapCallback, _unmapCallback, this);

	// insert the menu items
	for (long i = 0; i < nbEntries; i++)
	{
		if (!strcmp (entry[i], kMenuSeparator))
		{
			itemWidget[i] = (void*)XtCreateManagedWidget ("separator", 
							 xmSeparatorGadgetClass, (Widget)platformControl, 0, 0);
		}
		else
		{
			if (multipleCheck)
			{
				itemWidget[i] = (void*)XtVaCreateManagedWidget (entry[i], 
					xmToggleButtonWidgetClass, (Widget)platformControl,
					XmNset, check[i], XmNvisibleWhenOff, false, 0);
				XtAddCallback ((Widget)itemWidget[i], XmNvalueChangedCallback, _activateCallback, this);
			}
			else if (style & kCheckStyle)
			{
				itemWidget[i] = (void*)XtVaCreateManagedWidget (entry[i], 
					xmToggleButtonWidgetClass, (Widget)platformControl,
					XmNset, (i == currentIndex) ? true : false, XmNvisibleWhenOff, false, 0);
				XtAddCallback ((Widget)itemWidget[i], XmNvalueChangedCallback, _activateCallback, this);
			}
			else 
			{
				itemWidget[i] = (void*)XtVaCreateManagedWidget (entry[i], 
					xmPushButtonWidgetClass, (Widget)platformControl, 0);
				XtAddCallback ((Widget)itemWidget[i], XmNactivateCallback, _activateCallback, this);
			}
		}
	}

	XtManageChild ((Widget)platformControl);

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
	getParent ()->getSize (&rect);
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
	BView* plugView = (BView*) getParent ()->getSystemWindow ();
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
				pContextTemp = new CDrawContext (getParent (), plugView, NULL);

			setValue (message->what);
			
			if (listener)
				listener->valueChanged (pContextTemp, this);
	
			// redraw the display
			draw (pContextTemp);

			if (!pContext && pContextTemp)
				delete pContextTemp;
		}
	}
	delete popup;
	getParent ()->setEditView (0);
#endif
}

//------------------------------------------------------------------------
void COptionMenu::looseFocus (CDrawContext *pContext)
{	
	if (platformControl == 0) 
		return;

#if WINDOWS
#elif MAC
#elif MOTIF
	for (long i = 0; i < nbEntries; i++)
		if (itemWidget[i])
			XtDestroyWidget ((Widget)itemWidget[i]);

	if (platformControl)
	{
		XtUnmanageChild ((Widget)platformControl);
		XtDestroyWidget ((Widget)platformControl);
	}
#endif

	platformControl = 0;
}

#if MOTIF
//------------------------------------------------------------------------
void COptionMenu::setCurrentSelected (void *itemSelected)
{
	// retrieve the current index
	if (itemSelected != 0)
	{
		for (long i = 0; i < nbEntries; i++)
			if (itemWidget[i] == itemSelected)
			{
				currentIndex = i;
				break;
			}
	}

	// update dependency
	CDrawContext *pContext = new CDrawContext (getParent (), (void*)getParent ()->getGC (), (void*)getParent ()->getWindow ());

	setValue (currentIndex);

	if (listener)
		listener->valueChanged (pContext, this);
	delete pContext;
}
#endif


//------------------------------------------------------------------------
// CAnimKnob
//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect &size, CControlListener *listener, long tag,
                      long subPixmaps,         // number of subPixmaps
                      long heightOfOneImage,   // height of one image in pixel
                      CBitmap *background, CPoint &offset)
: CKnob (size, listener, tag, background, 0, offset), 
   subPixmaps (subPixmaps), heightOfOneImage (heightOfOneImage), bInverseBitmap (false)
{
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::~CAnimKnob ()
{}

//------------------------------------------------------------------------
void CAnimKnob::draw (CDrawContext *pContext)
{
	CPoint where (0, 0);
	if (value >= 0.f) 
	{
		long tmp = heightOfOneImage * (subPixmaps - 1);
		if (bInverseBitmap)
			where.v = (long)((1 - value) * (float)tmp);
		else
			where.v = (long)(value * (float)tmp);
		for (long realY = 0; realY <= tmp; realY += heightOfOneImage) 
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
	setDirty (false);
}

//------------------------------------------------------------------------
// CVerticalSwitch
//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CRect &size, CControlListener *listener, long tag,
                                  long subPixmaps,       // number of subPixmaps
                                  long heightOfOneImage, // height of one image in pixel
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
void CVerticalSwitch::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;
	
	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	// set the default value
	if (button == (kControl|kLButton))
	{
		value = getDefaultValue ();
		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		return;
	}

	double coef = (double)heightOfOneImage / (double)iMaxPositions;

	// begin of edit parameter
	getParent ()->beginEdit (tag);
	do
	{
		value = (long)((where.v - size.top) / coef) / (float)(iMaxPositions - 1);
		if (value > 1.f)
			value = 1.f;
		else if (value < 0.f)
			value = 0.f;

		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		
		pContext->getMouseLocation (where);
		
		doIdleStuff ();
	}
	while (pContext->getMouseButtons () == button);

	// end of edit parameter
	getParent ()->endEdit (tag);
}


//------------------------------------------------------------------------
// CHorizontalSwitch
//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect &size,
                                  CControlListener *listener,
                                  long tag,
                                  long subPixmaps,   // number of subPixmaps
                                  long heightOfOneImage, // height of one image in pixel
                                  long iMaxPositions,
                                  CBitmap *background,
                                  CPoint &offset)
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
void CHorizontalSwitch::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;
	
	// set the default value
	if (button == (kControl|kLButton))
	{
		value = getDefaultValue ();

		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		return;
	}

	double coef = (double)pBackground->getWidth () / (double)iMaxPositions;

	// begin of edit parameter
	getParent ()->beginEdit (tag);
	do
	{
		value = (long)((where.h - size.left) / coef) / (float)(iMaxPositions - 1);
		if (value > 1.f)
			value = 1.f;
		else if (value < 0.f)
			value = 0.f;

		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		
		pContext->getMouseLocation (where);
		
		doIdleStuff ();
	}
	while (pContext->getMouseButtons () == button);

	// end of edit parameter
	getParent ()->endEdit (tag);
}


//------------------------------------------------------------------------
// CRockerSwitch
//------------------------------------------------------------------------
// Switch, which releases itself after being clicked
CRockerSwitch::CRockerSwitch (const CRect &size,
                              CControlListener *listener, 
                              long tag,              // identifier tag (ID)
                              long heightOfOneImage, // height of one image in pixel
                              CBitmap *background,
                              CPoint  &offset, const long style)
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
void CRockerSwitch::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	float fEntryState = value;

	long  width_2 = size.width () / 2;
	long  height_2 = size.height () / 2;
	if (button)
	{
		// begin of edit parameter
		getParent ()->beginEdit (tag);
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
				listener->valueChanged (pContext, this);

			pContext->getMouseLocation (where);

			doIdleStuff ();
		}
		while (pContext->getMouseButtons ());

		// end of edit parameter
		getParent ()->endEdit (tag);
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
			listener->valueChanged (pContext, this);
	}

	value = 0.f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (pContext, this);
}

//------------------------------------------------------------------------
bool CRockerSwitch::onWheel (CDrawContext *pContext, const CPoint &where, float distance)
{
	if (!bMouseEnabled)
		return false;

	if (distance > 0)
		value = -1.0f;
	else
		value = 1.0f;

	if (isDirty () && listener)
		listener->valueChanged (pContext, this);

	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (pContext, this);
	return true;
}


//------------------------------------------------------------------------
// CMovieBitmap
//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect &size, CControlListener *listener, long tag,
                            long subPixmaps,        // number of subPixmaps
                            long heightOfOneImage,  // height of one image in pixel
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
// simulating a real windows-button
CMovieButton::CMovieButton (const CRect &size,
                            CControlListener *listener, 
                            long tag,              // identifier tag (ID)
                            long heightOfOneImage, // height of one image in pixel
                            CBitmap *background,
                            CPoint  &offset)
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
void CMovieButton::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	// this simulates a real windows button
	float fEntryState = value;

	if (pContext->getMouseButtons ())
	{
		// begin of edit parameter
		getParent ()->beginEdit (tag);
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
				listener->valueChanged (pContext, this);
	    
			pContext->getMouseLocation (where); 

			doIdleStuff ();
		}
		while (pContext->getMouseButtons () == button);
	
		// end of edit parameter
		getParent ()->endEdit (tag);
	}
	else
	{
		value = !value;
		if (listener)
			listener->valueChanged (pContext, this);
	}

	buttonState = value;
}


//------------------------------------------------------------------------
// CAutoAnimation
//------------------------------------------------------------------------
// displays bitmaps within a (child-) window
CAutoAnimation::CAutoAnimation (const CRect &size,
                                CControlListener *listener, 
                                long tag,         // identifier tag (ID)
                                long subPixmaps,	 // number of subPixmaps...
                                long heightOfOneImage, // height of one image in pixel
                                CBitmap *background,
                                CPoint  &offset)
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
		setDirty (false);
	}
}

//------------------------------------------------------------------------
void CAutoAnimation::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;
	
	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	if (!isWindowOpened ())
	{	
		value = 0;
		openWindow ();
		setDirty (); // force to redraw
		if (listener)
			listener->valueChanged (pContext, this);
	}
	else
	{                                                                       
		// stop info animation
		value = 0; // draw first pic of bitmap
		draw (pContext);
		closeWindow ();
	}
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

	pOScreen = new COffscreenContext (getParent (), widthControl, heightControl, kBlackCColor);
		
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
	float fValue;
	if (style & kLeft || style & kTop)
		fValue = value;
	else 
		fValue = 1.f - value;
	
	// (re)draw background
	CRect rect (0, 0, widthControl, heightControl);
	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pOScreen, rect, offset);
		else
			pBackground->draw (pOScreen, rect, offset);
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

	// draw slider at new position
	if (pHandle)
	{
		if (bDrawTransparentEnabled)
			pHandle->drawTransparent (pOScreen, rectNew);
		else 
			pHandle->draw (pOScreen, rectNew);
	}

	pOScreen->copyFrom (pContext, size);
	
	setDirty (false);
}

//------------------------------------------------------------------------
void CSlider::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();

	// set the default value
	if (button == (kControl|kLButton))
	{
		value = getDefaultValue ();
		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		return;
	}
	
	// allow left mousebutton only
	if (!(button & kLButton))
		return;

	long delta;
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
		long actualPos;
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
	getParent ()->beginEdit (tag);
	getParent ()->setEditView (this);

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
			listener->valueChanged (pContext, this);

		pContext->getMouseLocation (where);

		doIdleStuff ();
	}

	// end of edit parameter
	getParent ()->endEdit (tag);
}

//------------------------------------------------------------------------
bool CSlider::onWheel (CDrawContext *pContext, const CPoint &where, float distance)
{
	if (!bMouseEnabled)
		return false;

	long buttons = pContext->getMouseButtons ();
	if (buttons & kShift)
		value += 0.1f * distance * wheelInc;
	else
		value += distance * wheelInc;
	bounceValue ();

	if (isDirty () && listener)
		listener->valueChanged (pContext, this);
	return true;
}

//------------------------------------------------------------------------
long CSlider::onKeyDown (VstKeyCode& keyCode)
{
#if !PLUGGUI
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
				listener->valueChanged (0, this);
		} return 1;
	}
#endif

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
// special display with custom digits (0...9)
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

	// store coordinates of x/y pos of each digit
	for (long i = 0; i < iNumbers; i++)
	{
		this->xpos[i] = xpos[i];
		this->ypos[i] = ypos[i];
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
float CSpecialDigit::getNormValue ()
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
// Button, which releases itself after being clicked
CKickButton::CKickButton (const CRect &size,
                              CControlListener *listener, 
                              long tag,              // identifier tag (ID)
                              long heightOfOneImage, // height of one image in pixel
                              CBitmap *background,
                              CPoint  &offset)
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
void CKickButton::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;
	
	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	// this simulates a real windows button
	float fEntryState = value;

	if (pContext->getMouseButtons () == kLButton)
	{
		// begin of edit parameter
		getParent ()->beginEdit (tag);
		do
		{
			if (where.h >= size.left && where.v >= size.top  &&
			    where.h <= size.right && where.v <= size.bottom)
				value = !fEntryState;
			else
				value = fEntryState;
			
			if (isDirty () && listener)
				listener->valueChanged (pContext, this);
			
			pContext->getMouseLocation (where);
			
			doIdleStuff ();
		}
		while (pContext->getMouseButtons () == kLButton);
		
		// end of edit parameter
		getParent ()->endEdit (tag);
	}
	else
	{
		value = !value;
		if (listener)
			listener->valueChanged (pContext, this);
	}

	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (pContext, this);
}


//------------------------------------------------------------------------
// CSplashScreen
//------------------------------------------------------------------------
// one click draw its pixmap, an another click redraw its parent
CSplashScreen::CSplashScreen (const CRect &size,
                              CControlListener *listener, 
                              long     tag,
                              CBitmap *background,
                              CRect   &toDisplay,
                              CPoint  &offset)
:	CControl (size, listener, tag, background), 
	toDisplay (toDisplay), offset (offset)
{}

//------------------------------------------------------------------------
CSplashScreen::~CSplashScreen ()
{}

//------------------------------------------------------------------------
void CSplashScreen::draw (CDrawContext *pContext)
{
	if (value && pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, toDisplay, offset);
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
void CSplashScreen::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	value = !value;
	if (value)
	{
		if (getParent () && getParent ()->setModalView (this))
		{
			keepSize = size;
			size = toDisplay;
			draw (pContext);
			if (listener)
				listener->valueChanged (pContext, this);
		}
	}
	else
	{
		size = keepSize;
		if (getParent ())
		{
			getParent ()->setModalView (NULL);
			getParent ()->draw (pContext);
		}
		if (listener)
			listener->valueChanged (pContext, this);
	}
	setDirty ();
}

//------------------------------------------------------------------------
void CSplashScreen::unSplash ()
{
	setDirty ();
	value = 0.f;

	size = keepSize;
	if (getParent ())
	{
		if (getParent ()->getModalView () == this)
		{
			getParent ()->setModalView (NULL);
			getParent ()->redraw ();
		}
	}
}

//------------------------------------------------------------------------
// CVuMeter
//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CRect &size, CBitmap *onBitmap, CBitmap *offBitmap,
                    long nbLed, const long style)
	: CControl (size, 0, 0),
	  onBitmap (onBitmap), offBitmap (offBitmap), nbLed (nbLed), style (style)
{
	setDecreaseStepValue (0.1f);
	
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

//------------------------------------------------------------------------
void CVuMeter::draw (CDrawContext *pContext)
{
	if (!onBitmap) 
		return;

	CPoint pointOn;
	CPoint pointOff;

	bounceValue ();
	
	float newValue = oldValue - decreaseValue;
	if (newValue < value)
		newValue = value;
	oldValue = newValue;

	if (style & kHorizontal) 
	{
		long tmp = (long)(((long)(nbLed * newValue + 0.5f) / (float)nbLed) * onBitmap->getWidth ());
		pointOff (tmp, 0);
		tmp += size.left;

		rectOff.left = tmp;
		rectOn.right = tmp;
	}
	else 
	{
		long tmp = (long)(((long)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * onBitmap->getHeight ());
		pointOn (0, tmp);
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
}

END_NAMESPACE_VSTGUI
//------------------------------------------------------------------------
// END.
//------------------------------------------------------------------------
