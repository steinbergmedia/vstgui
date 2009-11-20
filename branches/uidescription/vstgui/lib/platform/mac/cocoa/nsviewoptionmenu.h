
#ifndef __nsviewoptionmenu__
#define __nsviewoptionmenu__

#include "../../iplatformoptionmenu.h"

#if MAC_COCOA

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

#endif // MAC_COCOA

#endif // __nsviewoptionmenu__
