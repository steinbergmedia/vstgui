// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __win32optionmenu__
#define __win32optionmenu__

#include "../iplatformoptionmenu.h"

#if WINDOWS

#include <windows.h>
#include <list>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32OptionMenu : public IPlatformOptionMenu
{
public:
	Win32OptionMenu (HWND windowHandle);
	
	PlatformOptionMenuResult popup (COptionMenu* optionMenu) override;

protected:
	HMENU createMenu (COptionMenu* menu, int32_t& offsetIdx);
	
	HWND windowHandle;
	
	std::list<HBITMAP> bitmaps;
};

} // namespace

#endif // WINDOWS
#endif // __win32optionmenu__
