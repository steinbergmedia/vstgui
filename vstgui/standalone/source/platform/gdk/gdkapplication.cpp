#include "../../application.h"
#include "gdkcommondirectories.h"
#include "gdkpreference.h"
#include <gdkmm.h>
#include <gdkmm/event.h>

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
};

//------------------------------------------------------------------------
bool Application::init (int argc, char* argv[])
{
	if (!gdk_init_check (&argc, &argv))
		return false;
	IApplication::CommandLineArguments cmdArgs;
	for (auto i = 0; i < argc; ++i)
		cmdArgs.push_back (argv[i]);

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
	return 0;
}

//------------------------------------------------------------------------
int Application::run ()
{
	while (doRunning)
	{
		Gdk::Event event = Gdk::Event::get ();
	}
	return 0;
}

//------------------------------------------------------------------------
void Application::quit () {}

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