#include "../../../../lib/cvstguitimer.h"
#include "../../../../lib/platform/win32/win32support.h"
#include "../../../iappdelegate.h"
#include "../../../iapplication.h"
#include "../../application.h"
#include "../../genericalertbox.h"
#include "../../window.h"
#include "../iplatformwindow.h"
#include "win32preference.h"
#include "win32window.h"

#include <ShellScalingAPI.h>
#include <Windows.h>

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

using namespace VSTGUI::Standalone;
using VSTGUI::Standalone::Detail::IApplicationPlatformAccess;
using VSTGUI::Standalone::Detail::CommandWithKey;
using VSTGUI::Standalone::Detail::IPlatformWindowAccess;
using CommandWithKeyList =
    VSTGUI::Standalone::Detail::IApplicationPlatformAccess::CommandWithKeyList;
using VSTGUI::Standalone::Detail::PlatformCallbacks;

//------------------------------------------------------------------------
static IWin32Window* toWin32Window (const VSTGUI::Standalone::WindowPtr& window)
{
	auto platformWindow = window->dynamicCast<Detail::IPlatformWindowAccess> ();
	if (!platformWindow)
		return nullptr;
	return platformWindow->getPlatformWindow ()->dynamicCast<IWin32Window> ();
}

//------------------------------------------------------------------------
class Application
{
public:
	Application () = default;

	void init ();
	void run ();

	void quit ();
	void onCommandUpdate ();
	AlertResult showAlert (const AlertBoxConfig& config);
	void showAlertForWindow (const AlertBoxForWindowConfig& config);

private:
	Win32Preference prefs;
	HACCEL keyboardAccelerators {nullptr};
};

//------------------------------------------------------------------------
void Application::init ()
{
	SetProcessDpiAwareness (PROCESS_PER_MONITOR_DPI_AWARE);

	IApplication::CommandLineArguments cmdArgs;
	auto commandLine = GetCommandLine ();
	int numArgs = 0;
	auto cmdArgsArray = CommandLineToArgvW (commandLine, &numArgs);
	for (int i = 0; i < numArgs; ++i)
	{
		UTF8StringHelper str (cmdArgsArray[i]);
		cmdArgs.push_back (str.getUTF8String ());
	}
	LocalFree (cmdArgsArray);

	PlatformCallbacks callbacks;
	callbacks.quit = [this] () { quit (); };
	callbacks.onCommandUpdate = [this] () { onCommandUpdate (); };
	callbacks.showAlert = [this] (const AlertBoxConfig& config) { return showAlert (config); };
	callbacks.showAlertForWindow = [this] (const AlertBoxForWindowConfig& config) {
		showAlertForWindow (config);
	};

	auto app = Detail::getApplicationPlatformAccess ();
	vstgui_assert (app);
	app->init (prefs, std::move (cmdArgs), std::move (callbacks));
}

//------------------------------------------------------------------------
AlertResult Application::showAlert (const AlertBoxConfig& config)
{
	AlertResult result = AlertResult::error;
	if (auto window = Detail::createAlertBox (config, [&] (AlertResult r) { result = r; }))
	{
		auto winModalWindow = toWin32Window (window);
		vstgui_assert (winModalWindow);
		for (auto& w : IApplication::instance ().getWindows ())
		{
			if (w == window)
				continue;
			if (auto winWindow = toWin32Window (w))
				winWindow->setModalWindow (window);
		}
		auto platformWindow = window->dynamicCast<Detail::IPlatformWindowAccess> ()
		                          ->getPlatformWindow ()
		                          ->dynamicCast<IWindow> ();
		platformWindow->center ();

		window->show ();
		MSG msg;
		BOOL gmResult;
		while (result == AlertResult::error && (gmResult = GetMessage (&msg, NULL, 0, 0)))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		for (auto& w : IApplication::instance ().getWindows ())
		{
			if (auto winWindow = toWin32Window (w))
				winWindow->setModalWindow (nullptr);
		}
	}
	return result;
}

//------------------------------------------------------------------------
void Application::showAlertForWindow (const AlertBoxForWindowConfig& config)
{
	auto callback = config.callback;
	auto parentWindow = config.window;
	if (auto window = Detail::createAlertBox (config, [=] (AlertResult r) {
		    auto parentWinWindow = toWin32Window (parentWindow);
		    vstgui_assert (parentWinWindow);
		    parentWinWindow->setModalWindow (nullptr);
		    Call::later ([callback, r, parentWindow] () {
			    callback (r);
			    if (auto winWindow = toWin32Window (parentWindow))
				    winWindow->dynamicCast<IWindow> ()->activate ();
			});
		}))
	{
		auto parentWinWindow = toWin32Window (config.window);
		vstgui_assert (parentWinWindow);
		parentWinWindow->setModalWindow (window);
		CRect r;
		r.setTopLeft (parentWindow->getPosition ());
		r.setSize (parentWindow->getSize ());
		CRect r2;
		r2.setSize (window->getSize ());
		r2.centerInside (r);
		window->setPosition (r2.getTopLeft ());
		window->show ();
	}
}

//------------------------------------------------------------------------
void Application::onCommandUpdate ()
{
	if (keyboardAccelerators)
	{
		DestroyAcceleratorTable (keyboardAccelerators);
		keyboardAccelerators = nullptr;
	}
	auto& windows = IApplication::instance ().getWindows ();
	for (auto& w : windows)
	{
		if (auto winWindow = toWin32Window (w))
			winWindow->updateCommands ();
	}
	auto app = Detail::getApplicationPlatformAccess ();
	std::vector<ACCEL> accels;
	WORD cmd = 0;
	for (auto& grp : app->getCommandList ())
	{
		for (auto& e : grp.second)
		{
			if (e.defaultKey)
			{
				BYTE virt = FVIRTKEY | FCONTROL;
				auto upperKey = toupper (e.defaultKey);
				if (upperKey == e.defaultKey)
					virt |= FSHIFT;
				accels.push_back ({virt, static_cast<WORD> (upperKey), cmd});
			}
			++cmd;
		}
	}
	if (!accels.empty ())
		keyboardAccelerators =
		    CreateAcceleratorTable (accels.data (), static_cast<int> (accels.size ()));
}

//------------------------------------------------------------------------
void Application::quit ()
{
	Call::later ([] () {
		auto windows = IApplication::instance ().getWindows (); // Yes, copy the window list
		for (auto& w : windows)
		{
			if (auto winWindow = toWin32Window (w))
				winWindow->onQuit ();
		}
		IApplication::instance ().getDelegate ().onQuit ();
		PostQuitMessage (0);
	});
}

//------------------------------------------------------------------------
void Application::run ()
{
	MSG msg;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		if (TranslateAccelerator (msg.hwnd, keyboardAccelerators, &msg))
			continue;

		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI

void* hInstance = nullptr; // for VSTGUI

//------------------------------------------------------------------------
int APIENTRY wWinMain (_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance,
                       _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	HRESULT hr = CoInitialize (NULL);
	if (FAILED (hr))
		return FALSE;

	hInstance = instance;

	VSTGUI::useD2DHardwareRenderer (true);
	VSTGUI::Standalone::Platform::Win32::Application app;
	app.init ();
	app.run ();

	CoUninitialize ();
	return 0;
}
