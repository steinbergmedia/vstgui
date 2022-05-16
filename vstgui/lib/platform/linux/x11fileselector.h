// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformfileselector.h"
#include "x11frame.h"

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//-----------------------------------------------------------------------------
PlatformFileSelectorPtr createFileSelector (PlatformFileSelectorStyle style, Frame* frame);

//-----------------------------------------------------------------------------
} // X11
} // VSTGUI
