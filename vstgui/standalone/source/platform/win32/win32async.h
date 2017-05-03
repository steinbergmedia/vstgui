// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../include/iasync.h"
#include "../../../../lib/platform/win32/win32support.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

void initAsyncHandling (HINSTANCE instance);
void terminateAsyncHandling ();

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI
