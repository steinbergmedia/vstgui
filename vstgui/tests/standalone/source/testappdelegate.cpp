#include "testappdelegate.h"
#include "testwindow.h"
#include "vstgui/standalone/iappdelegate.h"
#include "vstgui/standalone/iapplication.h"
#include "vstgui/standalone/iwindow.h"
#include "vstgui/standalone/icommand.h"
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
	void finishLaunching () override;
	void onQuit () override;
	bool canQuit () override;
	void showAboutDialog () override;
	bool hasAboutDialog () override;
	void showPreferenceDialog () override;
	bool hasPreferenceDialog () override;

	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;
private:
};

//------------------------------------------------------------------------
void Delegate::finishLaunching ()
{
	IApplication::instance ().registerCommand (Commands::NewDocument, 'n');
}

//------------------------------------------------------------------------
bool Delegate::canHandleCommand (const Command& command)
{
	return command == Commands::NewDocument;
}

//------------------------------------------------------------------------
bool Delegate::handleCommand (const Command& command)
{
	if (command == Commands::NewDocument)
	{
		WindowController::makeWindow ();
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
	
}

//------------------------------------------------------------------------
bool Delegate::hasAboutDialog ()
{
	return false;
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

