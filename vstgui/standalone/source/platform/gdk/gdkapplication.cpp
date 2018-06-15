#include "../../application.h"
#include "../../../../lib/platform/linux/gdkframe.h"
#include "../../../../lib/platform/common/fileresourceinputstream.h"
#include "gdkcommondirectories.h"
#include "gdkpreference.h"
#include <gdkmm.h>
#include <gdkmm/event.h>
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
	CommonDirectories commonDirectories;
	Preference prefs;

	bool doRunning{true};
	GMainLoop* mainLoop {nullptr};
};

//------------------------------------------------------------------------
bool Application::init (int argc, char* argv[])
{
	if (!gdk_init_check (&argc, &argv))
		return false;
	IApplication::CommandLineArguments cmdArgs;
	for (auto i = 0; i < argc; ++i)
		cmdArgs.push_back (argv[i]);

	char result[PATH_MAX];
	ssize_t count = readlink ("/proc/self/exe", result, PATH_MAX);
	if (count == -1)
		exit (-1);
	auto execPath = dirname (result);
	VSTGUI::GDK::Frame::createResourceInputStreamFunc = [&] (const CResourceDescription& desc) {
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
	mainLoop = g_main_loop_new (nullptr, true);
	g_main_loop_run (mainLoop);
	g_main_loop_unref (mainLoop);
	mainLoop = nullptr;
	return 0;
}

//------------------------------------------------------------------------
void Application::quit ()
{
	if (mainLoop)
		g_main_loop_quit (mainLoop);
}

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