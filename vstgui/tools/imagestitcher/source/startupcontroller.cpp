// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "startupcontroller.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/iappdelegate.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iasync.h"
#include <cassert>
#include <memory>

using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {
namespace {

//------------------------------------------------------------------------
class StartupWindowController : public WindowControllerAdapter,
                                public UIDesc::CustomizationAdapter,
                                public ICommandHandler
{
public:
	static std::shared_ptr<StartupWindowController> getInstance ()
	{
		if (!instance)
		{
			instance = std::make_shared<StartupWindowController> ();

			UIDesc::Config config;
			config.uiDescFileName = "StartupWindow.uidesc";
			config.viewName = "Window";
			config.windowConfig.style.transparent ()
			    .close ()
			    .centered ()
			    .movableByWindowBackground ();
			config.modelBinding = instance->createModelBinding ();
			config.customization = instance;

			instance->window = UIDesc::makeWindow (config);
		}
		return instance;
	}

	void showWindow ()
	{
		window->show ();
		window->activate ();
	}

private:
	void createNewDocument ()
	{
		Async::schedule (Async::mainQueue (), [] () {
			auto commandHandler =
			    IApplication::instance ().getDelegate ().dynamicCast<ICommandHandler> ();
			assert (commandHandler);
			commandHandler->handleCommand (Commands::NewDocument);
		});
	}

	void openDocument ()
	{
		Async::schedule (Async::mainQueue (), [] () {
			auto commandHandler =
			    IApplication::instance ().getDelegate ().dynamicCast<ICommandHandler> ();
			assert (commandHandler);
			commandHandler->handleCommand (Commands::OpenDocument);
		});
	}

	UIDesc::ModelBindingPtr createModelBinding ()
	{
		auto binding = UIDesc::ModelBindingCallbacks::make ();
		binding->addValue (Value::make ("CreateNewDocument"),
		                   UIDesc::ValueCalls::onAction ([this] (auto& v) {
			                   v.performEdit (0.);
							   this->handleCommand (Commands::NewDocument);
		                   }));
		binding->addValue (Value::make ("OpenDocument"),
		                   UIDesc::ValueCalls::onAction ([this] (auto& v) {
			                   v.performEdit (0.);
							   this->handleCommand (Commands::OpenDocument);
		                   }));
		binding->addValue (Value::make ("CloseWindow"),
		                   UIDesc::ValueCalls::onAction ([] (auto& v) {
			                   v.performEdit (0.);
			                   IApplication::instance ().quit ();
		                   }));
		return binding;
	}

	bool canHandleCommand (const Command& command) override
	{
		if (command == Commands::OpenDocument)
			return true;
		if (command == Commands::NewDocument)
			return true;
		return false;
	}
	
	bool handleCommand (const Command& command) override
	{
		if (command == Commands::OpenDocument)
		{
			openDocument ();
			window->close ();
			return true;
		}
		if (command == Commands::NewDocument)
		{
			createNewDocument ();
			window->close ();
			return true;
		}
		return false;
	}

	void onClosed (const IWindow&) override
	{
		window = nullptr;
		instance = nullptr;
		if (IApplication::instance ().getWindows ().empty ())
			IApplication::instance ().quit ();
	}

	static std::shared_ptr<StartupWindowController> instance;

	WindowPtr window;
};
std::shared_ptr<StartupWindowController> StartupWindowController::instance;

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
void showStartupController ()
{
	StartupWindowController::getInstance ()->showWindow ();
}

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
