// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../application.h"
#include "../../../../lib/vstkeycode.h"
#include "../../../../lib/platform/linux/gdkframe.h"
#include "../../../../lib/platform/common/fileresourceinputstream.h"
#include "gdkcommondirectories.h"
#include "gdkpreference.h"
#include "gdkwindow.h"
#include "gdkrunloop.h"
#include <gdkmm.h>
#include <glibmm.h>
#include <giomm.h>
#include <gdkmm/event.h>
#include <gdkmm/wrap_init.h>
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
class Application
{
public:
	bool init (int argc, char* argv[]);
	int run ();

	void quit ();

private:
	void doCommandUpdate ();
	bool handleKeyEvent (GdkEvent* event);

	static void gdkEventCallback (GdkEvent* ev, gpointer data);

	using KeyCommands = std::unordered_map<VstKeyCode, Command>;

	KeyCommands keyCommands;
	CommonDirectories commonDirectories;
	Preference prefs;

	bool doRunning{true};
};

//------------------------------------------------------------------------
bool Application::init (int argc, char* argv[])
{
	Glib::init ();
	Gio::init ();
	if (!gdk_init_check (&argc, &argv))
		return false;
	Gdk::wrap_init ();

	IApplication::CommandLineArguments cmdArgs;
	for (auto i = 0; i < argc; ++i)
		cmdArgs.push_back (argv[i]);

	char result[PATH_MAX];
	ssize_t count = readlink ("/proc/self/exe", result, PATH_MAX);
	if (count == -1)
		exit (-1);
	std::string execPath = dirname (result);
	VSTGUI::GDK::Frame::createResourceInputStreamFunc =
		[execPath](const CResourceDescription& desc) {
			if (desc.type == CResourceDescription::kIntegerType)
				return IPlatformResourceInputStream::Ptr ();
			std::string path (execPath);
			path += "/Resources/";
			path += desc.u.name;
			return FileResourceInputStream::create (path);
		};

	PlatformCallbacks callbacks;
	callbacks.quit = [this]() { quit (); };
	callbacks.onCommandUpdate = [this]() { doCommandUpdate (); };
	callbacks.showAlert = [](const AlertBoxConfig& config) { return AlertResult::Error; };
	callbacks.showAlertForWindow = [](const AlertBoxForWindowConfig& config) {
		if (config.callback)
			config.callback (AlertResult::Error);
	};

	auto app = Detail::getApplicationPlatformAccess ();
	vstgui_assert (app);
	IPlatformApplication::OpenFilesList openFilesList;
	/* TODO: fill openFilesList */
	app->init ({prefs, commonDirectories, std::move (cmdArgs), std::move (callbacks),
				std::move (openFilesList)});
	return true;
}

//------------------------------------------------------------------------
void Application::doCommandUpdate ()
{
	keyCommands.clear ();
	for (auto& grp : Detail::getApplicationPlatformAccess ()->getCommandList ())
	{
		for (auto& e : grp.second)
		{
			if (e.defaultKey)
			{
				VstKeyCode keyCode{};
				keyCode.character = e.defaultKey;
				keyCode.modifier = MODIFIER_CONTROL;
				auto upperKey = toupper (e.defaultKey);
				if (upperKey == e.defaultKey)
				{
					keyCode.modifier |= MODIFIER_SHIFT;
					keyCode.character = tolower (e.defaultKey);
				}
				keyCommands.emplace (keyCode, e);
			}
		}
	}
}

//------------------------------------------------------------------------
int Application::run ()
{
	gdk_event_handler_set (gdkEventCallback, this, nullptr);
	RunLoop::instance ().run ();
	return 0;
}

//------------------------------------------------------------------------
void Application::quit ()
{
	RunLoop::instance ().quit ();
}

//------------------------------------------------------------------------
static void handleEvent (GdkEvent* event, GdkWindow* gdkWindow)
{
	if (auto window = IGdkWindow::find (gdkWindow))
	{
		if (!window->handleEvent (event))
		{
			if (auto parent = gdk_window_get_parent (gdkWindow))
				handleEvent (event, parent);
		}
	}
	else
	{
		printf ("unknown gdk window \n");
	}
}

//------------------------------------------------------------------------
bool Application::handleKeyEvent (GdkEvent* event)
{
	auto keyCode = VSTGUI::GDK::Frame::keyCodeFromEvent (event);
	auto it = keyCommands.find (keyCode);
	if (it != keyCommands.end ())
	{
		auto& windows = Standalone::IApplication::instance ().getWindows ();
		if (!windows.empty ())
		{
			if (auto handler = windows.front ()->dynamicCast<ICommandHandler> ())
			{
				if (handler->canHandleCommand (it->second))
				{
					if (handler->handleCommand (it->second))
						return true;
				}
			}
		}

		if (auto commandHandler = Detail::getApplicationPlatformAccess ())
		{
			if (commandHandler->canHandleCommand (it->second))
			{
				if (commandHandler->handleCommand (it->second))
					return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
void Application::gdkEventCallback (GdkEvent* ev, gpointer data)
{
	if (ev->type == GDK_KEY_PRESS)
	{
		auto app = reinterpret_cast<Application*> (data);
		if (app)
		{
			if (app->handleKeyEvent (ev))
				return;
		}
	}

	GdkWindow* gdkWindow = reinterpret_cast<GdkEventAny*> (ev)->window;
	if (!gdkWindow)
		return;
	handleEvent (ev, gdkWindow);
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