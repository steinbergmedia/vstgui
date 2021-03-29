// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platform/platformfactory.h"

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
void init (PlatformInstanceHandle instance)
{
	initPlatform (instance);
}

//-----------------------------------------------------------------------------
void exit ()
{
	exitPlatform ();
}

//-----------------------------------------------------------------------------
} // VSTGUI

