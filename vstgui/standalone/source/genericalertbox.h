// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
