
#ifndef __iplatformoptionmenu__
#define __iplatformoptionmenu__

#include "../vstguibase.h"

#if VSTGUI_PLATFORM_ABSTRACTION

namespace VSTGUI {
class COptionMenu;
class CMenuItem;

//-----------------------------------------------------------------------------
struct PlatformOptionMenuResult
{
	COptionMenu* menu;
	long index;
};

//-----------------------------------------------------------------------------
class IPlatformOptionMenu : public CBaseObject
{
public:
	virtual PlatformOptionMenuResult popup (COptionMenu* optionMenu) = 0;
//-----------------------------------------------------------------------------
protected:
};

} // namespace

#endif // VSTGUI_PLATFORM_ABSTRACTION
#endif // __iplatformtextedit__
