
#ifndef __win32textedit__
#define __win32textedit__

#include "../iplatformtextedit.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include <windows.h>

#ifdef STRICT
#define WINDOWSPROC WNDPROC
#else
#define WINDOWSPROC FARPROC
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32TextEdit : public IPlatformTextEdit
{
public:
	Win32TextEdit (HWND parent, IPlatformTextEditCallback* textEdit);
	~Win32TextEdit ();
	
	bool getText (char* text, long maxSize);
	bool setText (const char* text);
	bool updateSize ();

	HWND getPlatformControl () const { return platformControl; }
	HBRUSH getPlatformBackColor () const { return platformBackColor; }
	IPlatformTextEditCallback* getTextEdit () const { return textEdit; }

//-----------------------------------------------------------------------------
protected:
	HWND platformControl;
	HANDLE platformFont;
	HBRUSH platformBackColor;
	WINDOWSPROC oldWndProcEdit;

	static LONG_PTR WINAPI procEdit (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace

#endif // WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#endif // __win32textedit__
