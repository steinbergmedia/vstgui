#pragma once

#include "../../../interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
class IWin32Window : public Interface
{
public:
	virtual void updateCommands () const = 0;
};

} // Win32
} // Platform
} // Standalone
} // VSTGUI
