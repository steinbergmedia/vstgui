
#ifndef __hiviewoptionmenu__
#define __hiviewoptionmenu__

#include "../../iplatformoptionmenu.h"

#if MAC_CARBON && VSTGUI_PLATFORM_ABSTRACTION

#include <Carbon/Carbon.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class HIViewOptionMenu : public IPlatformOptionMenu
{
public:

	PlatformOptionMenuResult popup (COptionMenu* optionMenu);

protected:
	MenuRef createMenu (COptionMenu* menu);
};

} // namespace

#endif // MAC_CARBON && VSTGUI_PLATFORM_ABSTRACTION
#endif // __hiviewoptionmenu__
