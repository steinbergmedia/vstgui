// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32textedit.h"

#if WINDOWS

#include "win32support.h"
#include "../../vstkeycode.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
Win32TextEdit::Win32TextEdit (HWND parent, IPlatformTextEditCallback* textEdit)
: IPlatformTextEdit (textEdit)
, platformControl (0)
, platformFont (0)
, platformBackColor (0)
, oldWndProcEdit (0)
{
	CRect rect = textEdit->platformGetSize ();
	CFontRef fontID = textEdit->platformGetFont ();
	
	CHoriTxtAlign horiTxtAlign = textEdit->platformGetHoriTxtAlign ();
	int wstyle = 0;
	if (horiTxtAlign == kLeftText)
		wstyle |= ES_LEFT;
	else if (horiTxtAlign == kRightText)
		wstyle |= ES_RIGHT;
	else
		wstyle |= ES_CENTER;

	CPoint textInset = textEdit->platformGetTextInset ();
	rect.offset (textInset.x, textInset.y);
	rect.right -= textInset.x*2;
	rect.bottom -= textInset.y*2;

	// get/set the current font
	LOGFONT logfont = {0};

	CCoord fontH = fontID->getSize ();
	if (fontH > rect.getHeight ())
		fontH = rect.getHeight () - 3;
	if (fontH < rect.getHeight ())
	{
		CCoord adjust = (rect.getHeight () - (fontH + 3)) / (CCoord)2;
		rect.top += adjust;
		rect.bottom -= adjust;
	}
	UTF8StringHelper stringHelper (textEdit->platformGetText ());
	text = stringHelper;

	DWORD wxStyle = WS_EX_LAYERED;
	if (getD2DFactory () == 0 && IsWindowsVistaOrGreater()) // Vista and above
		wxStyle = WS_EX_COMPOSITED;
	wstyle |= WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	if (textEdit->platformIsSecureTextEdit ())
		wstyle |= ES_PASSWORD;
	platformControl = CreateWindowEx (wxStyle,
		TEXT("EDIT"), stringHelper, wstyle,
		(int)rect.left, (int)rect.top, (int)rect.getWidth (), (int)rect.getHeight (),
		parent, NULL, GetInstance (), 0);
	if (!platformControl)
	{
		wxStyle &= ~WS_EX_LAYERED;
		platformControl = CreateWindowEx (wxStyle,
			TEXT("EDIT"), stringHelper, wstyle,
			(int)rect.left, (int)rect.top, (int)rect.getWidth (), (int)rect.getHeight (),
			parent, NULL, GetInstance (), 0);
	}

	CColor backColor = textEdit->platformGetBackColor ();
	SetLayeredWindowAttributes (platformControl, RGB (backColor.red, backColor.green, backColor.blue), 0, LWA_COLORKEY);
	platformBackColor = CreateSolidBrush (RGB (backColor.red, backColor.green, backColor.blue));

	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = (LONG)-fontH;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	UTF8StringHelper fontNameHelper (fontID->getName ());
	VSTGUI_STRCPY (logfont.lfFaceName, fontNameHelper);

	logfont.lfClipPrecision	 = CLIP_STROKE_PRECIS;
	logfont.lfOutPrecision	 = OUT_STRING_PRECIS;
	logfont.lfQuality 	     = DEFAULT_QUALITY;
	logfont.lfCharSet        = ANSI_CHARSET;
  
	platformFont = CreateFontIndirect (&logfont);

	SetWindowLongPtr (platformControl, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
	SendMessage (platformControl, WM_SETFONT, (WPARAM)platformFont, true);
	SendMessage (platformControl, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG (0, 0));
	SendMessage (platformControl, EM_SETSEL, 0, -1);
	SendMessage (platformControl, EM_LIMITTEXT, 255, 0);
	SetFocus (platformControl);

	oldWndProcEdit = (WINDOWSPROC)(LONG_PTR)SetWindowLongPtr (platformControl, GWLP_WNDPROC, (__int3264)(LONG_PTR)procEdit);

}

//-----------------------------------------------------------------------------
Win32TextEdit::~Win32TextEdit () noexcept
{
	if (platformControl)
	{
		SetWindowLongPtr (platformControl, GWLP_WNDPROC, (__int3264)(LONG_PTR)oldWndProcEdit);
		DestroyWindow (platformControl);
	}
	if (platformFont)
		DeleteObject (platformFont);
	if (platformBackColor)
		DeleteObject (platformBackColor);
}

//-----------------------------------------------------------------------------
UTF8String Win32TextEdit::getText ()
{
#if 1
	return text.c_str ();
#else
	if (platformControl)
	{
		int textLength = GetWindowTextLength (platformControl);
		TCHAR* newText = new TCHAR[textLength+1];
		GetWindowText (platformControl, newText, textLength+1);
		UTF8StringHelper windowText (newText);
		text = windowText;		
		delete [] newText;
		return text.c_str ();
	}
	return "";
#endif
}

//-----------------------------------------------------------------------------
bool Win32TextEdit::setText (const UTF8String& _text)
{
	if (platformControl && text != _text)
	{
		UTF8StringHelper windowText (_text);
		return SetWindowText (platformControl, windowText) ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32TextEdit::updateSize ()
{
	// TODO: Implement me !
	return false;
}

//-----------------------------------------------------------------------------
void Win32TextEdit::textChanged ()
{
	if (platformControl)
	{
		int textLength = GetWindowTextLength (platformControl);
		TCHAR* newText = new TCHAR[textLength+1];
		GetWindowText (platformControl, newText, textLength+1);
		UTF8StringHelper windowText (newText);
		text = windowText;		
		delete [] newText;
		textEdit->platformTextDidChange ();
	}
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32TextEdit::procEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	Win32TextEdit* win32TextEdit = (Win32TextEdit*)(LONG_PTR) GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (win32TextEdit)
	{
		WINDOWSPROC oldProc = win32TextEdit->oldWndProcEdit;
		switch (message)
		{
			case WM_GETDLGCODE :
			{
				LONG_PTR flags = DLGC_WANTALLKEYS;
				return flags;
			}

			case WM_KEYDOWN:
			{
				if (win32TextEdit->textEdit)
				{
					if (auto keyCode = keyMessageToKeyCode (wParam, lParam))
					{
						// for now only dispatch virtual keys
						if (keyCode->character == 0 && win32TextEdit->textEdit->platformOnKeyDown (*keyCode))
							return 0;
					}
				}
				break;
			}

			case WM_KILLFOCUS:
			{
				if (win32TextEdit->textEdit)
				{
					win32TextEdit->textEdit->platformLooseFocus (false);
					return 0;
				}
				break;
			}
			case WM_COMMAND:
			{
				if (HIWORD (wParam) == EN_CHANGE && win32TextEdit->textEdit)
				{
					win32TextEdit->textChanged ();
				}
				return 0;
			}
		}
		return CallWindowProc (oldProc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

} // namespace

#endif // WINDOWS
