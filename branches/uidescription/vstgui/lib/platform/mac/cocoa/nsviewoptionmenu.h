
#ifndef __nsviewoptionmenu__
#define __nsviewoptionmenu__

#include "../../iplatformoptionmenu.h"

#if MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION

namespace VSTGUI {

//-----------------------------------------------------------------------------
class NSViewOptionMenu : public IPlatformOptionMenu
{
public:

	PlatformOptionMenuResult popup (COptionMenu* optionMenu);

//-----------------------------------------------------------------------------
protected:
	static bool initClass ();
};

} // namespace

#endif // MAC_CARBON && VSTGUI_PLATFORM_ABSTRACTION

#endif // __nsviewoptionmenu__
