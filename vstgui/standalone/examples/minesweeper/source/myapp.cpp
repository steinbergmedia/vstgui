// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "commands.h"
#include "windowcontroller.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/menubuilder.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class MyApplication final : public Application::DelegateAdapter,
                            public WindowListenerAdapter,
                            public MenuBuilderAdapter
{
public:
	MyApplication ()
	: Application::DelegateAdapter ({"Minesweeper", "1.0.0", "vstgui.examples.minesweeper"})
	{
	}

	void finishLaunching () override
	{
		auto windowController = std::make_shared<WindowController> ();
		UIDesc::Config config;
		config.uiDescFileName = "Window.uidesc";
		config.viewName = "Window";
		config.customization = windowController;
		config.modelBinding = windowController;
		config.windowConfig.title = "Minesweeper";
		config.windowConfig.autoSaveFrameName = "MinesweeperWindow";
		config.windowConfig.style.border ().close ().centered ().size ();
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
			window->registerWindowListener (this);
		}
		else
		{
			IApplication::instance ().quit ();
		}
	}
	void onClosed (const IWindow& window) override { IApplication::instance ().quit (); }

	SortFunction getCommandGroupSortFunction (const Interface& context,
	                                          const UTF8String& group) const override
	{
		if (group == GameGroup)
		{
			return [] (const UTF8String& lhs, const UTF8String& rhs) {
				static const auto order = {NewGameCommand.name,
				                           NewBeginnerGameCommand.name,
				                           NewIntermediateGameCommand.name,
				                           NewExpertGameCommand.name,
				                           MouseModeCommand.name,
				                           TouchpadModeCommand.name,
				                           ToggleHighscoresCommand.name};
				auto leftIndex = std::find (order.begin (), order.end (), lhs);
				auto rightIndex = std::find (order.begin (), order.end (), rhs);
				return std::distance (leftIndex, rightIndex) > 0;
			};
		}
		return {};
	}

	bool prependMenuSeparator (const Interface& context, const Command& cmd) const override
	{
		if (cmd == Commands::CloseWindow || cmd == MouseModeCommand || cmd == ToggleHighscoresCommand)
			return true;
		return false;
	}
};

static Application::Init gAppDelegate (std::make_unique<MyApplication> ());

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
