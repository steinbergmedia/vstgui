// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../include/fwd.h"
#include "../iplatformwindow.h"

#ifndef _WINDEF_
struct HWND__;
typedef HWND__* HWND;
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
class IWin32Window : public Platform::IWindow
{
public:
	virtual void updateCommands () const = 0;
	virtual void onQuit () = 0;
	virtual HWND getHWND () const = 0;
	virtual void setModalWindow (const VSTGUI::Standalone::WindowPtr& modalWindow) = 0;
};

} // Win32
} // Platform
} // Standalone
} // VSTGUI
