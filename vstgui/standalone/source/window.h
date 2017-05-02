#pragma once

#include "../include/iwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, const WindowControllerPtr& controller);

//------------------------------------------------------------------------
class IPlatformWindowAccess : public IWindow
{
public:
	virtual InterfacePtr getPlatformWindow () const = 0;
};

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
