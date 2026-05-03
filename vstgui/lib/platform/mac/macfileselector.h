// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#if !TARGET_OS_IPHONE

#include "../iplatformfileselector.h"
#include "cocoa/nsviewframe.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
PlatformFileSelectorPtr createCocoaFileSelector (PlatformFileSelectorStyle style,
												 NSViewFrame* frame);

} // VSTGUI

#endif

