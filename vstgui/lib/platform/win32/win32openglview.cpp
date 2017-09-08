// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32openglview.h"

#if WINDOWS && VSTGUI_OPENGL_SUPPORT

#pragma comment (lib,"opengl32.lib")
#pragma comment (lib,"glu32.lib")

#include "win32frame.h"
#include "win32support.h"
#include <cmath>

namespace VSTGUI {

//-----------------------------------------------------------------------------
int32_t Win32OpenGLView::instanceCount = 0;
static TCHAR gGLWindowClassName[100];

//-----------------------------------------------------------------------------
Win32OpenGLView::Win32OpenGLView (Win32Frame* win32Frame)
: win32Frame (win32Frame)
, view (0)
, windowHandle (0)
, deviceContext (0)
, openGLContext (0)
{
	initWindowClass ();
	InitializeCriticalSection (&lock);
}

//-----------------------------------------------------------------------------
Win32OpenGLView::~Win32OpenGLView () noexcept
{
	DeleteCriticalSection (&lock);
	destroyWindowClass ();
}

//-----------------------------------------------------------------------------
void Win32OpenGLView::initWindowClass ()
{
	instanceCount++;
	if (instanceCount == 1)
	{
		VSTGUI_SPRINTF (gGLWindowClassName, TEXT("VSTGUI_OpenGL_%p"), GetInstance ());
		
		WNDCLASS windowClass = {};
		windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_GLOBALCLASS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra  = 0; 
		windowClass.cbWndExtra  = 0; 
		windowClass.hInstance   = GetInstance ();
		windowClass.hIcon = 0; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		windowClass.lpszMenuName  = 0; 
		windowClass.lpszClassName = gGLWindowClassName; 
		RegisterClass (&windowClass);
	}
}

//-----------------------------------------------------------------------------
void Win32OpenGLView::destroyWindowClass ()
{
	instanceCount--;
	if (instanceCount == 0)
	{
		UnregisterClass (gGLWindowClassName, GetInstance ());
	}
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32OpenGLView::WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32OpenGLView* oglView = (Win32OpenGLView*)(LONG_PTR)GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (oglView)
	{
		switch (message)
		{
			case WM_ERASEBKGND:
			{
				return 1; // don't draw background
			}
			case WM_PAINT:
			{
				oglView->lockContext ();
				if (oglView->makeContextCurrent ())
				{
					RECT updateRect;
					GetUpdateRect (hwnd, &updateRect, FALSE);
					oglView->view->drawOpenGL (CRect (updateRect.left, updateRect.top, updateRect.right, updateRect.bottom));
				}
				oglView->unlockContext ();
				break;
			}
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
bool Win32OpenGLView::setupPixelFormt ()
{
	// TODO: support for custom PixelFormat
	if (deviceContext)
	{
		PIXELFORMATDESCRIPTOR pfd = { 
			sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd  
			1,                                // version number  
			PFD_DRAW_TO_WINDOW |              // support window  
			PFD_SUPPORT_OPENGL |              // support OpenGL  
			PFD_DOUBLEBUFFER,                 // double buffered  
			PFD_TYPE_RGBA,                    // RGBA type  
			24,                               // 24-bit color depth  
			0, 0, 0, 0, 0, 0,                 // color bits ignored  
			0,                                // no alpha buffer  
			0,                                // shift bit ignored  
			0,                                // no accumulation buffer  
			0, 0, 0, 0,                       // accum bits ignored  
			32,                               // 32-bit z-buffer      
			0,                                // no stencil buffer  
			0,                                // no auxiliary buffer  
			PFD_MAIN_PLANE,                   // main layer  
			0,                                // reserved  
			0, 0, 0                           // layer masks ignored  
		}; 
		int  iPixelFormat; 
		 
		// get the device context's best, available pixel format match  
		iPixelFormat = ChoosePixelFormat (deviceContext, &pfd); 
		 
		// make that match the device context's current pixel format  
		return SetPixelFormat (deviceContext, iPixelFormat, &pfd) ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32OpenGLView::createWindow ()
{
	if (windowHandle)
		return false;

	DWORD style = 0;//transparent ? WS_EX_LAYERED : 0;
	windowHandle = CreateWindowEx (style, gGLWindowClassName, TEXT("Window"),
									WS_CHILD | WS_VISIBLE, 
									0, 0, (int)0, (int)0, 
									win32Frame->getPlatformWindow (), NULL, GetInstance (), NULL);

	if (windowHandle)
	{
		EnableWindow (windowHandle, false);	// we don't handle mouse and keyboard events
		SetWindowLongPtr (windowHandle, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
		deviceContext = GetDC (windowHandle);
		if (deviceContext && setupPixelFormt ())
		{
			openGLContext = wglCreateContext (deviceContext);
			return true;
		}
		remove ();
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32OpenGLView::init (IOpenGLView* view, PixelFormat* _pixelFormat)
{
	if (windowHandle)
		return false;
	if (_pixelFormat)
		pixelFormat = *_pixelFormat;

	if (createWindow ())
	{
		this->view = view;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void Win32OpenGLView::remove ()
{
	if (windowHandle)
	{
		if (openGLContext)
		{
			if (wglGetCurrentContext () == openGLContext)
				wglMakeCurrent (0, 0);
			ReleaseDC (windowHandle, deviceContext);
			wglDeleteContext (openGLContext);
			openGLContext = 0;
		}
		SetWindowLongPtr (windowHandle, GWLP_USERDATA, (LONG_PTR)NULL);
		DestroyWindow (windowHandle);
		windowHandle = 0;
	}
}

//-----------------------------------------------------------------------------
void Win32OpenGLView::invalidRect (const CRect& rect)
{
	if (windowHandle)
	{
		RECT r = {(LONG)rect.left, (LONG)rect.top, (LONG)ceil (rect.right), (LONG)ceil (rect.bottom)};
		InvalidateRect (windowHandle, &r, true);
	}
}

//-----------------------------------------------------------------------------
void Win32OpenGLView::viewSizeChanged (const CRect& visibleSize)
{
	if (windowHandle)
	{
		lockContext ();
		SetWindowPos (windowHandle, HWND_TOP, (int)visibleSize.left, (int)visibleSize.top, (int)visibleSize.getWidth (), (int)visibleSize.getHeight (), SWP_NOCOPYBITS|SWP_DEFERERASE);
		unlockContext ();
	}
}

//-----------------------------------------------------------------------------
bool Win32OpenGLView::makeContextCurrent ()
{
	if (openGLContext && deviceContext)
	{
		return wglMakeCurrent (deviceContext, openGLContext) ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32OpenGLView::lockContext ()
{
	EnterCriticalSection (&lock);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32OpenGLView::unlockContext ()
{
	LeaveCriticalSection (&lock);
	return true;
}

//-----------------------------------------------------------------------------
void Win32OpenGLView::swapBuffers ()
{
	if (deviceContext)
	{
		wglMakeCurrent (deviceContext, 0);
		SwapBuffers (deviceContext);
	}
}

} // namespace

#endif // WINDOWS && VSTGUI_OPENGL_SUPPORT
