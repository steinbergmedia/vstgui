// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "platform/platformfwd.h"

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
/** Init to use VSTGUI. Must be done before using the VSTGUI library.
 *
 *	The instance is depended on the platform:
 *	- HINSTANCE on Windows
 *	- CFBundleRef on macOS
 *	- void* on Linux (the handle returned from dlopen)
 *
 *	@ingroup new_in_4_10
 */
void init (PlatformInstanceHandle instance);

//-----------------------------------------------------------------------------
/** exit using the VSTGUI library.
 *	@ingroup new_in_4_10
 */
void exit ();

//-----------------------------------------------------------------------------
} // VSTGUI
