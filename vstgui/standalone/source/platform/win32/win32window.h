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
