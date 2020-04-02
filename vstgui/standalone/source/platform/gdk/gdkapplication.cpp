// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../application.h"
#include "../../../include/iappdelegate.h"
#include "../../../include/iapplication.h"
#include "../../../../lib/vstkeycode.h"
#include "../../../../lib/platform/linux/x11frame.h"
#include "../../../../lib/platform/common/fileresourceinputstream.h"
#include "gdkcommondirectories.h"
#include "gdkpreference.h"
#include "gdkwindow.h"
#include "gdkrunloop.h"
#include <gtkmm.h>
#include <libgen.h>
#include <unordered_map>

//------------------------------------------------------------------------
namespace std {

//------------------------------------------------------------------------
template<>
struct hash<VstKeyCode>
{
	std::size_t operator() (const VstKeyCode& k) const
	{
		return ((hash<int32_t> () (k.character) ^ (hash<unsigned char> () (k.modifier) << 1)) >>
				1) ^
			   (hash<unsigned char> () (k.virt) << 1);
	}
};

//------------------------------------------------------------------------
} // std

//------------------------------------------------------------------------
bool operator== (const VstKeyCode& k1, const VstKeyCode& k2)
{
	return k1.virt == k2.virt && k1.modifier == k2.modifier && k1.character == k2.character;
}

//------------------------------------------------------------------------
namespace VSTGUI {
void* soHandle = nullptr;

namespace Standalone {
namespace Platform {
namespace GDK {

using namespace VSTGUI::Standalone::Detail;

//------------------------------------------------------------------------
Glib::RefPtr<Gtk::Application> app;

//------------------------------------------------------------------------
Glib::RefPtr<Gtk::Application> gtkApp ()
{
	return app;
}

//------------------------------------------------------------------------
class Application
{
public:
	bool init (int argc, char* argv[]);
	int run ();

	void quit ();

private:
	void doCommandUpdate ();
	void handleCommand (const CommandWithKey& command);

	CommonDirectories commonDirectories;
	Preference prefs;

	bool isInitialized {false};
};

//------------------------------------------------------------------------
bool Application::init (int argc, char* argv[])
{
	const auto& appInfo = IApplication::instance ().getDelegate ().getInfo ();
	app = Gtk::Application::create (argc, argv, appInfo.uri.data ());
	Glib::set_application_name (appInfo.name.getString ());

	IApplication::CommandLineArguments cmdArgs;
	for (auto i = 0; i < argc; ++i)
		cmdArgs.push_back (argv[i]);

	app->signal_startup ().connect ([cmdArgs = std::move (cmdArgs), this] () mutable {
		char result[PATH_MAX];
		ssize_t count = readlink ("/proc/self/exe", result, PATH_MAX);
		if (count == -1)
			exit (-1);
		std::string execPath = dirname (result);
		VSTGUI::X11::Frame::createResourceInputStreamFunc =
			[execPath] (const CResourceDescription& desc) {
				if (desc.type == CResourceDescription::kIntegerType)
					return PlatformResourceInputStreamPtr ();
				std::string path (execPath);
				path += "/Resources/";
				path += desc.u.name;
				return FileResourceInputStream::create (path);
			};
		VSTGUI::X11::Frame::userDefinedResourcePath = execPath + "/Resources/";

		PlatformCallbacks callbacks;
		callbacks.quit = [this] () { quit (); };
		callbacks.onCommandUpdate = [this] () { doCommandUpdate (); };
		callbacks.showAlert = [] (const AlertBoxConfig& config) { return AlertResult::Error; };
		callbacks.showAlertForWindow = [] (const AlertBoxForWindowConfig& config) {
			if (config.callback)
				config.callback (AlertResult::Error);
		};

		auto appAccess = Detail::getApplicationPlatformAccess ();
		vstgui_assert (appAccess);
		IPlatformApplication::OpenFilesList openFilesList;
		/* TODO: fill openFilesList */
		appAccess->init ({prefs, commonDirectories, std::move (cmdArgs), std::move (callbacks),
						  std::move (openFilesList)});
		isInitialized = true;
		doCommandUpdate ();
	});
	return true;
}

//------------------------------------------------------------------------
int Application::run ()
{
	return app->run ();
}

//------------------------------------------------------------------------
void Application::quit ()
{
	app->quit ();
}

//------------------------------------------------------------------------
void Application::handleCommand (const CommandWithKey& command) {}

//------------------------------------------------------------------------
void Application::doCommandUpdate ()
{
	if (!isInitialized)
		return;
	auto mainMenu = Gio::Menu::create ();
	auto commandList = Detail::getApplicationPlatformAccess ()->getCommandList ();
	for (auto& e : commandList)
	{
		auto subMenu = Gio::Menu::create ();
		for (auto& command : e.second)
		{
			if (command.name == CommandName::MenuSeparator)
			{
				continue;
			}
			auto actionName = command.group.getString () + "." + command.name.getString ();
			std::replace (actionName.begin (), actionName.end (), ' ', '_');
			auto item = Gio::MenuItem::create (command.name.getString (), actionName);
			if (command.defaultKey)
			{
				std::string accelKey ("<Primary>");
				accelKey += command.defaultKey;
				// TODO: map virtual characters
				item->set_attribute_value ("accel",
										   Glib::Variant<Glib::ustring>::create (accelKey));
			}
			subMenu->append_item (item);

			if (!app->has_action (actionName))
			{
				if (auto action = app->add_action (actionName,
												   [this, command] () { handleCommand (command); }))
				{
				}
			}
		}
		mainMenu->append_submenu (e.first.getString (), subMenu);
	}
	app->set_menubar (mainMenu);
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI

//------------------------------------------------------------------------
int main (int argc, char* argv[])
{
	VSTGUI::Standalone::Platform::GDK::Application app;
	if (app.init (argc, argv))
		return app.run ();
	return -1;
}
