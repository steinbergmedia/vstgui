// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "testappdelegate.h"
#include "testmodel.h"
#include "testabout.h"
#include "AlertBoxDesign.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iwindow.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/crect.h"

#include "vstgui/standalone/source/genericalertbox.h"

#include <memory>

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
class Delegate : public Application::DelegateAdapter, public ICommandHandler, public WindowListenerAdapter
{
public:
	Delegate ();

	// Application::IDelegate
	void finishLaunching () override;
	void showAboutDialog () override;
	bool hasAboutDialog () override;
	VSTGUI::UTF8StringPtr getSharedUIResourceFilename () const override;

	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	// WindowListenerAdapter
	void onClosed (const IWindow& window) override;

private:
	std::shared_ptr<TestModel> model;
};

//------------------------------------------------------------------------
Application::Init gAppDelegate (std::make_unique<Delegate> ());

static Command NewPopup {CommandGroup::File, "New Popup"};
static Command ShowAlertBoxDesign {CommandGroup::File, "Show AlertBox Design"};

//------------------------------------------------------------------------
Delegate::Delegate ()
: Application::DelegateAdapter (
      {"VSTGUIStandalone", "1.0.0", "vstgui.examples.standalone"})
{
}

//------------------------------------------------------------------------
void Delegate::finishLaunching ()
{
	model = std::make_shared<TestModel> ();
	IApplication::instance ().registerCommand (Commands::NewDocument, 'n');
	IApplication::instance ().registerCommand (NewPopup, 'N');
	IApplication::instance ().registerCommand (ShowAlertBoxDesign, 'b');
	handleCommand (Commands::NewDocument);
}

//------------------------------------------------------------------------
void Delegate::onClosed (const IWindow& window)
{
	if (IApplication::instance ().getWindows ().empty ())
	{
		IApplication::instance ().quit ();
	}
}

//------------------------------------------------------------------------
bool Delegate::canHandleCommand (const Command& command)
{
	return command == Commands::NewDocument || command == NewPopup || command == ShowAlertBoxDesign;
}

//------------------------------------------------------------------------
bool Delegate::handleCommand (const Command& command)
{
	if (command == Commands::NewDocument || command == NewPopup)
	{
		UIDesc::Config config;
		config.windowConfig.title = "Test Window";
		config.windowConfig.autoSaveFrameName = "TestWindowFrame";
		config.windowConfig.style.close ();
		config.windowConfig.size = {100, 100};
		config.viewName = "view";
		config.modelBinding = model;
		if (command == NewPopup)
		{
			config.uiDescFileName = "testpopup.uidesc";
			config.windowConfig.type = WindowType::Popup;
			config.windowConfig.style.movableByWindowBackground ();
		}
		else
		{
			config.uiDescFileName = "test.uidesc";
			config.windowConfig.style.border ();
		}
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
			window->registerWindowListener (this);
		}
		return true;
	}
	else if (command == ShowAlertBoxDesign)
	{
		if (auto window = VSTGUI::Standalone::makeAlertBoxDesignWindow ())
			window->show ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void Delegate::showAboutDialog () { About::show (); }

//------------------------------------------------------------------------
bool Delegate::hasAboutDialog () { return true; }

//------------------------------------------------------------------------
VSTGUI::UTF8StringPtr Delegate::getSharedUIResourceFilename () const { return "resources.uidesc"; }

} // MyApp
