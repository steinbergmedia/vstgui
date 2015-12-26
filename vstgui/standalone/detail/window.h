#pragma once

#include "../iwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, const WindowControllerPtr& controller);

//------------------------------------------------------------------------
class IPlatformWindowAccess : public Interface
{
public:
	virtual Interface* getPlatformWindow () const = 0;
};

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
