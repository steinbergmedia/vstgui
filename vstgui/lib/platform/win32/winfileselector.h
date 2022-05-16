// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformfileselector.h"

#if WINDOWS

#include "win32frame.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
PlatformFileSelectorPtr createWinFileSelector (PlatformFileSelectorStyle style, HWND parent);

} // VSTGUI

#endif
