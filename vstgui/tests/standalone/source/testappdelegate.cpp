#include "testappdelegate.h"
#include "testwindow.h"
#include "testabout.h"
#include "vstgui/standalone/iappdelegate.h"
#include "vstgui/standalone/iapplication.h"
#include "vstgui/standalone/iwindow.h"
#include "vstgui/standalone/icommand.h"
#include "vstgui/standalone/iuidescwindow.h"
#include "vstgui/standalone/ialertbox.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/crect.h"
#include <memory>

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
class Delegate : public Application::IDelegate, public ICommandHandler
{
public:
	// Application::IDelegate
	void finishLaunching () override;
	void onQuit () override;
	bool canQuit () override;
	void showAboutDialog () override;
	bool hasAboutDialog () override;
	void showPreferenceDialog () override;
	bool hasPreferenceDialog () override;

	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;
private:
	std::shared_ptr<TestModel> model;
};

static Command NewPopup {CommandGroup::File, "New Popup"};
//------------------------------------------------------------------------
void Delegate::finishLaunching ()
{
	model = std::make_shared<TestModel> ();
	IApplication::instance ().registerCommand (Commands::NewDocument, 'n');
	IApplication::instance ().registerCommand (NewPopup, 'N');
	handleCommand (Commands::NewDocument);
}

//------------------------------------------------------------------------
bool Delegate::canHandleCommand (const Command& command)
{
	return command == Commands::NewDocument || command == NewPopup;
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
			config.windowConfig.style.transparent ();
		}
		else
		{
			config.uiDescFileName = "test.uidesc";
			config.windowConfig.style.border ();
		}
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void Delegate::onQuit ()
{
}

//------------------------------------------------------------------------
bool Delegate::canQuit ()
{
	return true;
}

//------------------------------------------------------------------------
void Delegate::showAboutDialog ()
{
	About::show ();
}

//------------------------------------------------------------------------
bool Delegate::hasAboutDialog ()
{
	return true;
}

//------------------------------------------------------------------------
void Delegate::showPreferenceDialog ()
{
	
}

//------------------------------------------------------------------------
bool Delegate::hasPreferenceDialog ()
{
	return false;
}

//------------------------------------------------------------------------
Application::Init gAppDelegate (std::make_shared<Delegate> ());

} // MyApp

