
#ifndef __win32frame__
#define __win32frame__

#include "../../cframe.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include <windows.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32Frame : public IPlatformFrame
{
public:
	Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent);
	~Win32Frame ();

	HWND getPlatformWindow () const { return windowHandle; }
	HWND getParentPlatformWindow () const { return parentWindow; }
	HWND getOuterWindow () const;
	IPlatformFrameCallback* getFrame () const { return frame; }
	
	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const;
	bool setSize (const CRect& newSize);
	bool getSize (CRect& size) const;
	bool getCurrentMousePosition (CPoint& mousePosition) const;
	bool getCurrentMouseButtons (long& buttons) const;
	bool setMouseCursor (CCursorType type);
	bool invalidRect (const CRect& rect);
	bool scrollRect (const CRect& src, const CPoint& distance);
	unsigned long getTicks () const;
	bool showTooltip (const CRect& rect, const char* utf8Text);
	bool hideTooltip ();
	void* getPlatformRepresentation () const { return windowHandle; }
	IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit);
	IPlatformOptionMenu* createPlatformOptionMenu ();
	COffscreenContext* createOffscreenContext (CCoord width, CCoord height);
	
//-----------------------------------------------------------------------------
protected:
	void initTooltip ();

	static void initWindowClass ();
	static void destroyWindowClass ();
	static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static long gUseCount;

	HWND parentWindow;
	HWND windowHandle;
	HWND tooltipWindow;

	COffscreenContext* backBuffer;
};

} // namespace

#endif // WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#endif // __win32frame__
