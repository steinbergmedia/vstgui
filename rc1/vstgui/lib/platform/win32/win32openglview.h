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

#ifndef __win32openglview__
#define __win32openglview__

#include "../iplatformopenglview.h"

#if VSTGUI_OPENGL_SUPPORT
#if WINDOWS

#include <windows.h>

namespace VSTGUI {
class Win32Frame;

//-----------------------------------------------------------------------------
class Win32OpenGLView : public IPlatformOpenGLView
{
public:
	Win32OpenGLView (Win32Frame* win32Frame);
	~Win32OpenGLView ();

	virtual bool init (IOpenGLView* view, PixelFormat* pixelFormat = 0) VSTGUI_OVERRIDE_VMETHOD;
	virtual void remove () VSTGUI_OVERRIDE_VMETHOD;

	virtual void invalidRect (const CRect& rect) VSTGUI_OVERRIDE_VMETHOD;
	virtual void viewSizeChanged (const CRect& visibleSize) VSTGUI_OVERRIDE_VMETHOD; ///< visibleSize is cframe relative

	virtual bool makeContextCurrent () VSTGUI_OVERRIDE_VMETHOD;
	virtual bool lockContext () VSTGUI_OVERRIDE_VMETHOD;
	virtual bool unlockContext () VSTGUI_OVERRIDE_VMETHOD;

	virtual void swapBuffers () VSTGUI_OVERRIDE_VMETHOD;
protected:
	static void initWindowClass ();
	static void destroyWindowClass ();
	static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool createWindow ();
	bool setupPixelFormt ();

	Win32Frame* win32Frame;
	IOpenGLView* view;
	HWND windowHandle;
	PixelFormat pixelFormat;

	HDC deviceContext;
	HGLRC openGLContext;
	CRITICAL_SECTION lock;
	static int32_t instanceCount;
};

} // namespace

#endif // WINDOWS
#endif // VSTGUI_OPENGL_SUPPORT

#endif // __win32openglview__
