#include "application.h"
#include "../iappdelegate.h"
#include "../iapplication.h"
#include "../iwindowcontroller.h"
#include "window.h"
#include "../icommand.h"
#include "../../lib/cview.h"
#include "../../lib/cvstguitimer.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
class Application : public IApplication,
                    public IWindowListener,
                    public ICommandHandler,
                    public IApplicationPlatformAccess
{
public:
	static Application& instance ();
	Application () = default;

	// IApplication
	void setDelegate (const Standalone::Application::DelegatePtr& delegate);
	IPreference& getPreferences () const override;
	const CommandLineArguments& getCommandLineArguments () const override;
	Standalone::Application::IDelegate& getDelegate () const override;
	WindowPtr createWindow (const WindowConfiguration& config,
	                        const WindowControllerPtr& controller) override;
	const WindowList& getWindows () const override { return windows; }
	AlertResult showAlertBox (const AlertBoxConfig& config) override;
	void showAlertBoxForWindow (const AlertBoxForWindowConfig& config) override;
	void registerCommand (const Command& command, char16_t defaultCommandKey = 0) override;
	void quit () override;

	// IWindowListener
	void onSizeChanged (const IWindow& window, const CPoint& newSize) override {};
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) override {};
	void onShow (const IWindow& window) override {};
	void onHide (const IWindow& window) override {};
	void onClosed (const IWindow& window) override;
	void onActivated (const IWindow& window) override;
	void onDeactivated (const IWindow& window) override {};

	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	// IApplicationPlatformAccess
	void init (IPreference& preferences, IApplication::CommandLineArguments&& cmdArgs,
	           PlatformCallbacks&& callbacks) override;
	const CommandList& getCommandList () override;
	bool canQuit () override;

private:
	bool doCommandHandling (const Command& command, bool checkOnly);

	WindowList windows;
	Standalone::Application::DelegatePtr delegate;
	IPreference* preferences {nullptr};
	PlatformCallbacks platform;
	CommandList commandList;
	CommandLineArguments commandLineArguments;
	bool inQuit {false};
};

//------------------------------------------------------------------------
Application& Application::instance ()
{
	static Application app;
	return app;
}

//------------------------------------------------------------------------
void Application::init (IPreference& preferences, IApplication::CommandLineArguments&& cmdArgs,
                        PlatformCallbacks&& callbacks)
{
	this->preferences = &preferences;
	commandLineArguments = std::move (cmdArgs);
	platform = std::move (callbacks);

	registerCommand (Commands::About);
	registerCommand (Commands::Preferences);
	registerCommand (Commands::Quit, 'q');
	registerCommand (Commands::CloseWindow, 'w');
	registerCommand (Commands::Undo, 'z');
	registerCommand (Commands::Redo, 'Z');
	registerCommand (Commands::Cut, 'x');
	registerCommand (Commands::Copy, 'c');
	registerCommand (Commands::Paste, 'v');
	registerCommand (Commands::SelectAll, 'a');
	
	getDelegate ().finishLaunching ();
}

//------------------------------------------------------------------------
void Application::setDelegate (const Standalone::Application::DelegatePtr& inDelegate)
{
	delegate = inDelegate;
}

//------------------------------------------------------------------------
Standalone::Application::IDelegate& Application::getDelegate () const
{
	vstgui_assert (delegate, "Delegate cannot be nullptr");
	return *(delegate.get ());
}

//------------------------------------------------------------------------
IPreference& Application::getPreferences () const
{
	vstgui_assert (preferences);
	return *preferences;
}

//------------------------------------------------------------------------
const Application::CommandLineArguments& Application::getCommandLineArguments () const
{
	return commandLineArguments;
}

//------------------------------------------------------------------------
WindowPtr Application::createWindow (const WindowConfiguration& config,
                                     const WindowControllerPtr& controller)
{
	auto window = makeWindow (config, controller);
	if (window)
	{
		windows.push_back (window);
		window->registerWindowListener (this);
	}
	return window;
}

//------------------------------------------------------------------------
AlertResult Application::showAlertBox (const AlertBoxConfig& config)
{
	if (platform.showAlert)
		return platform.showAlert (config);
	return AlertResult::error;
}

//------------------------------------------------------------------------
void Application::showAlertBoxForWindow (const AlertBoxForWindowConfig& config)
{
	vstgui_assert (config.window);
	if (platform.showAlertForWindow)
		platform.showAlertForWindow (config);
}

//------------------------------------------------------------------------
void Application::quit ()
{
	if (inQuit || !canQuit ())
		return;
	inQuit = true;
	if (platform.quit)
		platform.quit ();
	inQuit = false;
}

//------------------------------------------------------------------------
bool Application::canQuit ()
{
	if (!delegate->canQuit ())
		return false;

	for (auto& window : windows)
	{
		if (window->getController () && !window->getController()->canClose (*window))
			return false;
	}
	
	return true;
}

//------------------------------------------------------------------------
const Application::CommandList& Application::getCommandList () { return commandList; }

//------------------------------------------------------------------------
void Application::registerCommand (const Command& command, char16_t defaultCommandKey)
{
	CommandWithKey c;
	c.group = command.group;
	c.name = command.name;
	c.defaultKey = defaultCommandKey;
	bool added = false;
	for (auto& entry : commandList)
	{
		if (entry.first == command.group)
		{
			for (auto& cmd : entry.second)
			{
				if (cmd == command)
					return; // already registered
			}
			entry.second.push_back (c);
			added = true;
			break;
		}
	}
	if (!added)
		commandList.push_back ({command.group, {c}});
	if (platform.onCommandUpdate)
		platform.onCommandUpdate ();
}

//------------------------------------------------------------------------
bool Application::canHandleCommand (const Command& command)
{
	return doCommandHandling (command, true);
}

//------------------------------------------------------------------------
bool Application::handleCommand (const Command& command)
{
	return doCommandHandling (command, false);
}

//------------------------------------------------------------------------
bool Application::doCommandHandling (const Command& command, bool checkOnly)
{
	bool result = false;
	if (auto commandHandler = delegate->dynamicCast<ICommandHandler> ())
		result = checkOnly ? commandHandler->canHandleCommand (command) :
		                     commandHandler->handleCommand (command);
	if (!result)
	{
		if (command == Commands::Quit)
		{
			if (!checkOnly)
			{
				quit ();
				return true;
			}
			return delegate->canQuit ();
		}
	}
	return result;
}

//------------------------------------------------------------------------
void Application::onClosed (const IWindow& window)
{
	auto it = std::find_if (windows.begin (), windows.end (), [&] (const WindowPtr& w) {
		if (&window == w.get ())
			return true;
		return false;
	});
	if (it != windows.end ())
		windows.erase (it);
}

//------------------------------------------------------------------------
void Application::onActivated (const IWindow& window)
{
	// move the window in the window list to the first position
	auto it = std::find_if (windows.begin (), windows.end (), [&] (const WindowPtr& w) {
		if (&window == w.get ())
			return true;
		return false;
	});
	if (it != windows.begin ())
	{
		auto window = *it;
		windows.erase (it);
		windows.insert (windows.begin (), window);
	}
}

//------------------------------------------------------------------------
} // Detail

//------------------------------------------------------------------------
IApplication& IApplication::instance () { return Detail::Application::instance (); }

//------------------------------------------------------------------------
namespace Application {

//------------------------------------------------------------------------
Init::Init (const DelegatePtr& delegate)
{
	CView::kDirtyCallAlwaysOnMainThread = true;
	Detail::Application::instance ().setDelegate (delegate);
}

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
