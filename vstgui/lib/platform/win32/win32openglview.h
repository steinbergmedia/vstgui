// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformopenglview.h"

#if VSTGUI_OPENGL_SUPPORT
#if WINDOWS

#include <windows.h>

namespace VSTGUI {
class Win32Frame;

//-----------------------------------------------------------------------------
class Win32OpenGLView final : public IPlatformOpenGLView
{
public:
	Win32OpenGLView (Win32Frame* win32Frame);
	~Win32OpenGLView () noexcept;

	bool init (IOpenGLView* view, PixelFormat* pixelFormat = nullptr) override;
	void remove () override;

	void invalidRect (const CRect& rect) override;
	void viewSizeChanged (const CRect& visibleSize) override;

	bool makeContextCurrent () override;
	bool lockContext () override;
	bool unlockContext () override;

	void swapBuffers () override;
protected:
	static void initWindowClass ();
	static void destroyWindowClass ();
	static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool createWindow (PixelFormat* pixelFormat);
	bool setupPixelFormt (PixelFormat* pixelFormat);

	Win32Frame* win32Frame;
	IOpenGLView* view;
	HWND windowHandle;

	HDC deviceContext;
	HGLRC openGLContext;
	CRITICAL_SECTION lock;
	static int32_t instanceCount;
};

} // VSTGUI

#endif // WINDOWS
#endif // VSTGUI_OPENGL_SUPPORT
