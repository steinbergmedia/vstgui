// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtextedit.h"

#if WINDOWS

#include <windows.h>

#ifdef STRICT
#define WINDOWSPROC WNDPROC
#else
#define WINDOWSPROC FARPROC
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32TextEdit final : public IPlatformTextEdit
{
public:
	Win32TextEdit (HWND parent, IPlatformTextEditCallback* textEdit);
	~Win32TextEdit () noexcept;
	
	UTF8String getText () override;
	bool setText (const UTF8String& text) override;
	bool updateSize () override;
	bool drawsPlaceholder () const override { return false; }

	HWND getPlatformControl () const { return platformControl; }
	HBRUSH getPlatformBackColor () const { return platformBackColor; }
	IPlatformTextEditCallback* getTextEdit () const { return textEdit; }

//-----------------------------------------------------------------------------
protected:
	void textChanged ();

	HWND platformControl;
	HANDLE platformFont;
	HBRUSH platformBackColor;
	WINDOWSPROC oldWndProcEdit;
	std::string text;

	static LONG_PTR WINAPI procEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // VSTGUI

#endif // WINDOWS
