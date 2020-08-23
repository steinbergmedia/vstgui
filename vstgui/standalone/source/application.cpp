// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "application.h"
#include "../../lib/cframe.h"
#include "../include/appinit.h"
#include "../include/iapplication.h"
#include "../include/icommand.h"
#include "../include/imenubuilder.h"
#include "../include/iwindowcontroller.h"
#include "shareduiresources.h"
#include "window.h"
#include <algorithm>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
class Application final : public IPlatformApplication
{
public:
	static Application& instance ();
	Application () = default;

	void setDelegate (Standalone::Application::DelegatePtr&& delegate);
	void setConfiguration (Standalone::Application::Configuration&& config);

	// IApplication
	IPreference& getPreferences () const override;
	const CommandLineArguments& getCommandLineArguments () const override;
	const ISharedUIResources& getSharedUIResources () const override;
	const ICommonDirectories& getCommonDirectories () const override;
	Standalone::Application::IDelegate& getDelegate () const override;
	WindowPtr createWindow (const WindowConfiguration& config,
	                        const WindowControllerPtr& controller) override;
	const WindowList& getWindows () const override { return windows; }
	AlertResult showAlertBox (const AlertBoxConfig& config) override;
	void showAlertBoxForWindow (const AlertBoxForWindowConfig& config) override;
	void registerCommand (const Command& command, char16_t defaultCommandKey = 0) override;
	void enableTooltips (bool state) override;
	void quit () override;

	// IWindowListener
	void onSizeChanged (const IWindow& window, const CPoint& newSize) override {};
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) override {};
	void onShow (const IWindow& window) override;
	void onHide (const IWindow& window) override {};
	void onClosed (const IWindow& window) override;
	void onActivated (const IWindow& window) override;
	void onDeactivated (const IWindow& window) override {};

	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	// IPlatformApplication
	void init (const InitParams& params) override;
	CommandList getCommandList (const Platform::IWindow* window) override;
	const CommandList& getKeyCommandList () override;
	bool canQuit () override;
	bool dontClosePopupOnDeactivation (Platform::IWindow* window) override;
	const Configuration& getConfiguration () const override { return config; }

private:
	void registerStandardCommands ();
	bool doCommandHandling (const Command& command, bool checkOnly);
	CommandList getCommandList (const Interface& context, const IMenuBuilder* menuBuilder);
	bool inQuit () const { return hasBit (flags, flagInQuit); }
	void setInQuit (bool state) { setBit (flags, flagInQuit, state); }

	WindowList windows;
	Standalone::Application::DelegatePtr delegate;
	IPreference* preferences {nullptr};
	ICommonDirectories* commonDirectories {nullptr};
	PlatformCallbacks platform;
	CommandList commandList;
	CommandLineArguments commandLineArguments;
	Configuration config;
	uint64_t flags {flagTooltipsEnabled};
	uint16_t commandIDCounter {0};

	enum Flags
	{
		flagInQuit = 1 << 0,
		flagTooltipsEnabled = 1 << 1,
	};
};

//------------------------------------------------------------------------
Application& Application::instance ()
{
	static Application app;
	return app;
}

//------------------------------------------------------------------------
void Application::init (const InitParams& params)
{
	preferences = &params.preferences;
	commonDirectories = &params.commonDirectories;
	commandLineArguments = std::move (params.cmdArgs);
	platform = std::move (params.callbacks);

	// TODO: make command registration configurable
	registerStandardCommands ();

	if (!params.openFiles.empty ())
		getDelegate ().openFiles (params.openFiles);

	getDelegate ().finishLaunching ();
}

//------------------------------------------------------------------------
void Application::registerStandardCommands ()
{
	registerCommand (Commands::About);
	registerCommand (Commands::Preferences);
	registerCommand (Commands::Quit, 'q');
	registerCommand (Commands::CloseWindow, 'w');
	registerCommand (Commands::Undo, 'z');
	registerCommand (Commands::Redo, 'Z');
	registerCommand (Commands::Cut, 'x');
	registerCommand (Commands::Copy, 'c');
	registerCommand (Commands::Paste, 'v');
	registerCommand (Commands::Delete, 0x8);
	registerCommand (Commands::SelectAll, 'a');
}

//------------------------------------------------------------------------
void Application::setDelegate (Standalone::Application::DelegatePtr&& inDelegate)
{
	delegate = std::move (inDelegate);
}

//------------------------------------------------------------------------
void Application::setConfiguration (Standalone::Application::Configuration&& configuration)
{
	using namespace VSTGUI::Standalone::Application;

	for (auto c : configuration)
	{
		switch (c.first)
		{
			case ConfigKey::UseCompressedUIDescriptionFiles:
			{
				vstgui_assert (c.second.type == ConfigValue::Type::Integer);
				config.useCompressedUIDescriptionFiles = c.second.value.integer != 0;
				break;
			}
		}
	}
}

//------------------------------------------------------------------------
Standalone::Application::IDelegate& Application::getDelegate () const
{
	vstgui_assert (delegate.get (), "Delegate cannot be nullptr");
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
const ISharedUIResources& Application::getSharedUIResources () const
{
	return Detail::getSharedUIResources ();
}

//------------------------------------------------------------------------
const ICommonDirectories& Application::getCommonDirectories () const
{
	vstgui_assert (commonDirectories);
	return *commonDirectories;
}

//------------------------------------------------------------------------
WindowPtr Application::createWindow (const WindowConfiguration& inConfig,
                                     const WindowControllerPtr& controller)
{
	auto window = makeWindow (inConfig, controller);
	if (window)
	{
		windows.emplace_back (window);
		window->registerWindowListener (this);
	}
	return window;
}

//------------------------------------------------------------------------
AlertResult Application::showAlertBox (const AlertBoxConfig& inConfig)
{
	if (platform.showAlert)
		return platform.showAlert (inConfig);
	return AlertResult::Error;
}

//------------------------------------------------------------------------
void Application::showAlertBoxForWindow (const AlertBoxForWindowConfig& inConfig)
{
	vstgui_assert (inConfig.window);
	if (platform.showAlertForWindow)
		platform.showAlertForWindow (inConfig);
}

//------------------------------------------------------------------------
void Application::enableTooltips (bool state)
{
	setBit (flags, flagTooltipsEnabled, state);
	for (auto& window : windows)
	{
		if (auto frame = staticPtrCast<IPlatformWindowAccess> (window)->getFrame ())
			frame->enableTooltips (state);
	}
}

//------------------------------------------------------------------------
void Application::quit ()
{
	if (inQuit () || !canQuit ())
		return;
	setInQuit (true);
	if (platform.quit)
		platform.quit ();
	setInQuit (false);
}

//------------------------------------------------------------------------
bool Application::canQuit ()
{
	if (!delegate->canQuit ())
		return false;

	auto currentWindows = windows; // make a copy
	for (auto& window : currentWindows)
	{
		if (window->getController () && !window->getController ()->canClose (*window))
			return false;
	}

	return true;
}

//------------------------------------------------------------------------
auto Application::getCommandList (const Interface& context, const IMenuBuilder* menuBuilder)
    -> CommandList
{
	CommandList menuCommandList;
	if (menuBuilder)
	{
		for (auto& catList : commandList)
		{
			if (catList.second.empty ())
				continue;
			if (!menuBuilder->showCommandGroupInMenu (context, catList.first))
				continue;
			auto catListCopy = catList;
			for (auto it = catListCopy.second.begin (); it != catListCopy.second.end ();)
			{
				auto current = it++;
				if (!menuBuilder->showCommandInMenu (context, *current))
					it = catListCopy.second.erase (current);
			}
			if (catListCopy.second.empty ())
				continue;
			if (auto func = menuBuilder->getCommandGroupSortFunction (context, catListCopy.first))
			{
				std::sort (catListCopy.second.begin (), catListCopy.second.end (),
				           [&] (const CommandWithKey& lhs, const CommandWithKey& rhs) {
					           return func (lhs.name, rhs.name);
				           });
			}
			for (auto it = ++catListCopy.second.begin (); it != catListCopy.second.end (); ++it)
			{
				if (menuBuilder->prependMenuSeparator (context, *it))
				{
					CommandWithKey separator {};
					separator.name = CommandName::MenuSeparator;
					it = catListCopy.second.emplace (it, std::move (separator));
					++it;
				}
			}

			menuCommandList.emplace_back (std::move (catListCopy));
		}
		return menuCommandList;
	}
	menuCommandList.clear ();
	for (auto& catList : commandList)
	{
		if (catList.second.empty ())
			continue;
		auto catListCopy = catList;
		if (catList.first == CommandGroup::Edit)
		{
			for (auto it = ++catListCopy.second.begin (); it != catListCopy.second.end (); ++it)
			{
				if (it->name == CommandName::Cut || it->name == CommandName::SelectAll)
				{
					CommandWithKey separator {};
					separator.name = CommandName::MenuSeparator;
					it = catListCopy.second.emplace (it, std::move (separator));
					++it;
				}
			}
		}
		menuCommandList.emplace_back (std::move (catListCopy));
	}
	return menuCommandList;
}

//------------------------------------------------------------------------
auto Application::getCommandList (const Platform::IWindow* window) -> CommandList
{
	if (window)
	{
		for (auto& w : getWindows ())
		{
			if (staticPtrCast<IPlatformWindowAccess> (w)->getPlatformWindow ().get () == window)
			{
				auto menuBuilder = w->getController ()->getWindowMenuBuilder (*w.get ());
				if (!menuBuilder)
					menuBuilder = delegate.get ()->dynamicCast<IMenuBuilder> ();
				return getCommandList (asInterface<IWindow> (*w.get ()), menuBuilder);
			}
		}
		return {};
	}
	return getCommandList (asInterface<IApplication> (*this),
	                       delegate.get ()->dynamicCast<IMenuBuilder> ());
}

//------------------------------------------------------------------------
auto Application::getKeyCommandList () -> const CommandList&
{
	return commandList;
}

//------------------------------------------------------------------------
void Application::registerCommand (const Command& command, char16_t defaultCommandKey)
{
	CommandWithKey c;
	c.group = command.group;
	c.name = command.name;
	c.defaultKey = defaultCommandKey;
	c.id = ++commandIDCounter;
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
			entry.second.emplace_back (c);
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
	if (auto commandHandler = dynamic_cast<ICommandHandler*> (delegate.get ()))
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
		else if (command == Commands::About)
		{
			if (!checkOnly)
			{
				delegate->showAboutDialog ();
				return true;
			}
			return delegate->hasAboutDialog ();
		}
		else if (command == Commands::Preferences)
		{
			if (!checkOnly)
			{
				delegate->showPreferenceDialog ();
				return true;
			}
			return delegate->hasPreferenceDialog ();
		}
	}
	return result;
}

//------------------------------------------------------------------------
void Application::onShow (const IWindow& window)
{
	if (auto frame = static_cast<const IPlatformWindowAccess&> (window).getFrame ())
		frame->enableTooltips (hasBit (flags, flagTooltipsEnabled));
}

//------------------------------------------------------------------------
template <typename Cont>
typename Cont::const_iterator findWindow (const Cont& c, const IWindow& window)
{
	return std::find_if (c.begin (), c.end (),
	                     [&] (const WindowPtr& w) { return &window == w.get (); });
}

//------------------------------------------------------------------------
void Application::onClosed (const IWindow& window)
{
	auto it = findWindow (windows, window);
	if (it != windows.end ())
		windows.erase (it);
}

//------------------------------------------------------------------------
void Application::onActivated (const IWindow& window)
{
	// move the window in the window list to the first position
	auto it = findWindow (windows, window);
	if (it != windows.begin ())
	{
		auto windowPtr = *it;
		windows.erase (it);
		windows.insert (windows.begin (), windowPtr);
	}
}

static std::vector<Platform::IWindow*> popupClosePreventionList;

//------------------------------------------------------------------------
bool Application::dontClosePopupOnDeactivation (Platform::IWindow* window)
{
	return std::find (popupClosePreventionList.begin (), popupClosePreventionList.end (), window) !=
	       popupClosePreventionList.end ();
}

//------------------------------------------------------------------------
PreventPopupClose::PreventPopupClose (IWindow& window)
{
	if (auto pwa = static_cast<IPlatformWindowAccess*> (&window))
	{
		if ((platformWindow = dynamicPtrCast<Platform::IWindow> (pwa->getPlatformWindow ())))
			popupClosePreventionList.emplace_back (platformWindow.get ());
	}
}

//------------------------------------------------------------------------
PreventPopupClose::~PreventPopupClose () noexcept
{
	auto it = std::find (popupClosePreventionList.begin (), popupClosePreventionList.end (),
	                     platformWindow.get ());
	if (it != popupClosePreventionList.end ())
	{
		popupClosePreventionList.erase (it);
	}
	platformWindow->activate ();
}

//------------------------------------------------------------------------
} // Detail

//------------------------------------------------------------------------
IApplication& IApplication::instance ()
{
	return Detail::Application::instance ();
}

//------------------------------------------------------------------------
namespace Application {

//------------------------------------------------------------------------
Init::Init (DelegatePtr&& delegate, Configuration&& config)
{
	CView::kDirtyCallAlwaysOnMainThread = true;
	Detail::Application::instance ().setDelegate (std::move (delegate));
	Detail::Application::instance ().setConfiguration (std::move (config));
}

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
