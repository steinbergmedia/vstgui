
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

	PlatformOptionMenuResult popup (COptionMenu* optionMenu);

protected:
	MenuRef createMenu (COptionMenu* menu);
};

} // namespace

#endif // MAC_CARBON
#endif // __hiviewoptionmenu__
