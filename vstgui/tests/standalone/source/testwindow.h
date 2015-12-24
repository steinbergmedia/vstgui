#pragma once

#include "vstgui/standalone/iwindow.h"
#include "vstgui/standalone/icommand.h"

//------------------------------------------------------------------------
namespace MyApp {

//------------------------------------------------------------------------
class WindowController : public VSTGUI::Standalone::WindowControllerAdapter, public VSTGUI::Standalone::ICommandHandler
{
public:
	using WindowPtr = VSTGUI::Standalone::WindowPtr;
	using Command = VSTGUI::Standalone::Command;
	
	static WindowPtr makeWindow ();

	WindowController ();
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;
private:
	struct Impl;
	struct EditImpl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // MyApp