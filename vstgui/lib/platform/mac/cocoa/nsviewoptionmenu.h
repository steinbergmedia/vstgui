// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../iplatformoptionmenu.h"

#if MAC_COCOA

namespace VSTGUI {

//-----------------------------------------------------------------------------
class NSViewOptionMenu : public IPlatformOptionMenu
{
public:
	void popup (COptionMenu* optionMenu, const Callback& callback) override;
};

} // VSTGUI

#endif // MAC_COCOA
