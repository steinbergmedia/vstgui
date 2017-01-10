//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "platform_helper.h"
#include "../../../lib/platform/win32/win32support.h"
#include <Windows.h>

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

	PlatformType getType () const override { return kHWND; }
	void* getHandle () const override { return window; };
	void forceRedraw () override {};
};

SharedPointer<PlatformParentHandle> PlatformParentHandle::create()
{
	return owned (dynamic_cast<PlatformParentHandle*> (new WinPlatformHandle ()));
}

} // UnitTest
} // VSTGUI

