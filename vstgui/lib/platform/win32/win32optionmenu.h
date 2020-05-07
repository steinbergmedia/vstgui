// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformoptionmenu.h"

#if WINDOWS

#include <windows.h>
#include <list>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32OptionMenu final : public IPlatformOptionMenu
{
public:
	Win32OptionMenu (HWND windowHandle);
	
	void popup (COptionMenu* optionMenu, const Callback& callback) override;

protected:
	HMENU createMenu (COptionMenu* menu, int32_t& offsetIdx);
	
	HWND windowHandle;
	
	std::list<HBITMAP> bitmaps;
};

} // VSTGUI

#endif // WINDOWS
