#pragma once

#include "../include/ialertbox.h"
#include "../include/iuidescwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

using AlertBoxCallback = std::function<void (AlertResult)>;
WindowPtr createAlertBox (const AlertBoxConfig& config, const AlertBoxCallback& callback);

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
