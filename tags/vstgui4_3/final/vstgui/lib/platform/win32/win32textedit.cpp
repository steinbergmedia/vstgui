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

	DWORD wxStyle = 0;
	if (getD2DFactory () == 0 && getSystemVersion ().dwMajorVersion >= 6) // Vista and above
		wxStyle = WS_EX_COMPOSITED;
	wstyle |= WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	platformControl = CreateWindowEx (wxStyle,
		TEXT("EDIT"), stringHelper, wstyle,
		(int)rect.left, (int)rect.top, (int)rect.getWidth (), (int)rect.getHeight (),
		parent, NULL, GetInstance (), 0);

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

	CColor backColor = textEdit->platformGetBackColor ();
	platformBackColor = CreateSolidBrush (RGB (backColor.red, backColor.green, backColor.blue));
}

//-----------------------------------------------------------------------------
Win32TextEdit::~Win32TextEdit ()
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
UTF8StringPtr Win32TextEdit::getText ()
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
bool Win32TextEdit::setText (UTF8StringPtr _text)
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
					if (wParam == VK_RETURN)
					{
						win32TextEdit->textEdit->platformLooseFocus (true);
						return 0;
					}
					else if (wParam == VK_TAB)
					{
						VstKeyCode keyCode = {0};
						keyCode.virt = VKEY_TAB;
						keyCode.modifier = GetKeyState (VK_SHIFT) < 0 ? MODIFIER_SHIFT : 0;
						if (win32TextEdit->textEdit->platformOnKeyDown (keyCode))
							return 0;
					}
					else if (wParam == VK_ESCAPE)
					{
						win32TextEdit->textEdit->platformLooseFocus (false);
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
