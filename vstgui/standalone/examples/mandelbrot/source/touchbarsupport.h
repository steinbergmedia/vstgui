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

