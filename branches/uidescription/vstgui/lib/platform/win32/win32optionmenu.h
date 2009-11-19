
#ifndef __win32optionmenu__
#define __win32optionmenu__

#include "../iplatformoptionmenu.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include <windows.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32OptionMenu : public IPlatformOptionMenu
{
public:
	Win32OptionMenu (HWND windowHandle);
	
	PlatformOptionMenuResult popup (COptionMenu* optionMenu);

protected:
	HMENU createMenu (COptionMenu* menu, long& offsetIdx);
	
	HWND windowHandle;
};

} // namespace

#endif // MAC_CARBON && VSTGUI_PLATFORM_ABSTRACTION
#endif // __win32optionmenu__
