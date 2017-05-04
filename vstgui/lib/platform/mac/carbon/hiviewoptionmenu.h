// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __hiviewoptionmenu__
#define __hiviewoptionmenu__

#include "../../iplatformoptionmenu.h"

#if MAC_CARBON

#include <Carbon/Carbon.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class HIViewOptionMenu : public IPlatformOptionMenu
{
public:

	PlatformOptionMenuResult popup (COptionMenu* optionMenu) override;

protected:
	MenuRef createMenu (COptionMenu* menu);
};

} // namespace

#endif // MAC_CARBON
#endif // __hiviewoptionmenu__
