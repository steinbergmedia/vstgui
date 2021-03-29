// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platform_helper.h"
#include "../../../lib/platform/win32/win32support.h"
#include <windows.h>

namespace VSTGUI {
namespace UnitTest {

static LRESULT CALLBACK MainWndProc (HWND h, UINT m , WPARAM w, LPARAM l)
{
	return DefWindowProc (h, m, w, l);
}

BOOL InitApplication(HINSTANCE hinstance) 
{
    WNDCLASSEX wcx; 
 
    // Fill in the window class structure with parameters 
    // that describe the main window. 
 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | 
        CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = MainWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = hinstance;         // handle to instance 
    wcx.hIcon = LoadIcon(NULL, 
        IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, 
        IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = nullptr;             // white background brush 
    wcx.lpszMenuName =  nullptr;    // name of menu resource 
    wcx.lpszClassName = L"MainWClass";  // name of window class 
    wcx.hIconSm = nullptr; 
 
    // Register the window class. 

    return RegisterClassEx(&wcx); 
}

struct Initializer
{
	static Initializer& instance ()
	{
		static Initializer gInstance;
		return gInstance;
	}
	Initializer ()
	{
		InitApplication (GetInstance ());
	}
};

struct WinPlatformHandle : PlatformParentHandle
{
	HWND window{ nullptr };

	WinPlatformHandle ()
	{
		Initializer::instance ();
		window = CreateWindow (L"MainWClass", L"Test", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, GetInstance (), 0);
	}
	
	~WinPlatformHandle ()
	{
		DestroyWindow (window);
	}

	PlatformType getType () const override { return PlatformType::kHWND; }
	void* getHandle () const override { return window; };
	void forceRedraw () override {};
};

SharedPointer<PlatformParentHandle> PlatformParentHandle::create()
{
	return owned (dynamic_cast<PlatformParentHandle*> (new WinPlatformHandle ()));
}

} // UnitTest
} // VSTGUI

