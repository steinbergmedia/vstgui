#pragma once

#include "../iwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, const WindowControllerPtr& controller);

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
