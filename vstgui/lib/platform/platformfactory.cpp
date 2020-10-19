// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platformfactory.h"

#if MAC
#include "mac/macfactory.h"
using VSTGUIPlatformFactory = VSTGUI::MacFactory;
#elif WINDOWS
#include "win32/win32factory.h"
using VSTGUIPlatformFactory = VSTGUI::Win32Factory;
#elif LINUX
#include "linux/linuxfactory.h"
using VSTGUIPlatformFactory = VSTGUI::LinuxFactory;
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {

static PlatformFactoryPtr factory;

//-----------------------------------------------------------------------------
void setPlatformFactory (PlatformFactoryPtr&& f)
{
	factory = std::move (f);
}

//-----------------------------------------------------------------------------
void initPlatform (PlatformInstanceHandle instance)
{
	vstgui_assert (!factory);
	setPlatformFactory (std::unique_ptr<IPlatformFactory> (new VSTGUIPlatformFactory (instance)));
}

//-----------------------------------------------------------------------------
void exitPlatform ()
{
	setPlatformFactory (nullptr);
}

//-----------------------------------------------------------------------------
const IPlatformFactory& getPlatformFactory ()
{
	return *factory.get ();
}

//-----------------------------------------------------------------------------
} // VSTGUI
