//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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

#include "ctextedit.h"
#include "../cframe.h"

#if WINDOWS
	#include "../win32support.h"
	#include "../coffscreencontext.h"
#endif

#if MAC_COCOA
	#include "../cocoasupport.h"
#endif

#if MAC_CARBON
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // we know that we use deprecated functions from Carbon, so we don't want to be warned
#endif

BEGIN_NAMESPACE_VSTGUI

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
		drawBack (pContext);
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
			else if (wParam == VK_TAB)
			{
				CTextEdit *textEdit = (CTextEdit*)(LONG_PTR) GetWindowLongPtr (hwnd, GWLP_USERDATA);
				if (textEdit)
				{
					if (textEdit->getFrame ())
						textEdit->getFrame ()->advanceNextFocusView (textEdit, GetKeyState (VK_SHIFT) < 0 ? true : false);
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

	// calculate offset for CViewContainers
	CRect rect (size);
	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);

#if WINDOWS
	int wstyle = 0;
	if (horiTxtAlign == kLeftText)
		wstyle |= ES_LEFT;
	else if (horiTxtAlign == kRightText)
		wstyle |= ES_RIGHT;
	else
		wstyle |= ES_CENTER;

	CPoint textInset = getTextInset ();
	rect.offset (textInset.x, textInset.y);
	rect.right -= textInset.x*2;
	rect.bottom -= textInset.y*2;

	// get/set the current font
	LOGFONT logfont = {0};

	CCoord fontH = fontID->getSize ();
	if (fontH > rect.height ())
		fontH = rect.height () - 3;
	if (fontH < rect.height ())
	{
		CCoord adjust = (rect.height () - (fontH + 3)) / (CCoord)2;
		rect.top += adjust;
		rect.bottom -= adjust;
	}
	UTF8StringHelper stringHelper (text);

	wstyle |= WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	platformControl = (void*)CreateWindowEx (WS_EX_TRANSPARENT,
		TEXT("EDIT"), stringHelper, wstyle,
		(int)rect.left, (int)rect.top, (int)rect.width (), (int)rect.height (),
		(HWND)getFrame ()->getSystemWindow (), NULL, GetInstance (), 0);

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

END_NAMESPACE_VSTGUI

