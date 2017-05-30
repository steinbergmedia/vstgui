// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/platform/iplatformframe.h"
#include "vstgui/standalone/include/fwd.h"

namespace Mandelbrot {

#if MAC && defined (MAC_OS_X_VERSION_10_12)
void installTouchbarSupport (VSTGUI::IPlatformFrameTouchBarExtension*, const VSTGUI::Standalone::ValuePtr&);
#else
inline void installTouchbarSupport (VSTGUI::IPlatformFrameTouchBarExtension*, const VSTGUI::Standalone::ValuePtr&) {}
#endif // MAC

} // Mandelbrot

