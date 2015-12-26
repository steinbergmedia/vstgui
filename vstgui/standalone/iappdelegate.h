#pragma once

#include "interface.h"
#include "fwd.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Application {
	
//------------------------------------------------------------------------
class IDelegate : public Interface
{
public:
	virtual void finishLaunching () = 0;
	virtual void onQuit () = 0;
	virtual bool canQuit () = 0;
	virtual void showAboutDialog () = 0;
	virtual bool hasAboutDialog () = 0;
	virtual void showPreferenceDialog () = 0;
	virtual bool hasPreferenceDialog () = 0;
};

//------------------------------------------------------------------------
struct Init
{
	Init (const DelegatePtr& delegate);
};

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
