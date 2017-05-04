// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __nsviewoptionmenu__
#define __nsviewoptionmenu__

#include "../../iplatformoptionmenu.h"

#if MAC_COCOA

namespace VSTGUI {

//-----------------------------------------------------------------------------
class NSViewOptionMenu : public IPlatformOptionMenu
{
public:

	PlatformOptionMenuResult popup (COptionMenu* optionMenu) override;

//-----------------------------------------------------------------------------
protected:
	static bool initClass ();
};

} // namespace

#endif // MAC_COCOA

#endif // __nsviewoptionmenu__
