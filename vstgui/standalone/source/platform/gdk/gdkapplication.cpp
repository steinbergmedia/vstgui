// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../application.h"
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
	static void gdkEventCallback (GdkEvent* ev, gpointer data);

	CommonDirectories commonDirectories;
	Preference prefs;

	bool doRunning{true};
};

//------------------------------------------------------------------------
bool Application::init (int argc, char* argv[])
{
    Glib::init();
    Gio::init();
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
	auto execPath = dirname (result);
	VSTGUI::GDK::Frame::createResourceInputStreamFunc = [&](const CResourceDescription& desc) {
		if (desc.type == CResourceDescription::kIntegerType)
			return IPlatformResourceInputStream::Ptr ();
		std::string path (execPath);
		path += "/Resources/";
		path += desc.u.name;
		return FileResourceInputStream::create (path);
	};

	PlatformCallbacks callbacks;
	callbacks.quit = [this]() { quit (); };
	callbacks.onCommandUpdate = []() {};
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
int Application::run ()
{
	gdk_event_handler_set (gdkEventCallback, nullptr, nullptr);
	RunLoop::instance ().run ();
	return 0;
}

//------------------------------------------------------------------------
void Application::quit ()
{
	RunLoop::instance ().quit ();
}

//------------------------------------------------------------------------
void Application::gdkEventCallback (GdkEvent* ev, gpointer data)
{
	GdkWindow* gdkWindow = reinterpret_cast<GdkEventAny*> (ev)->window;
	if (!gdkWindow)
		return;

	if (auto window = IGdkWindow::find (gdkWindow))
		window->handleEvent (ev);
	else
	{
		printf ("unknown gdk window \n");
	}
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