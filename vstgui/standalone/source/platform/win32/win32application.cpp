// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/platform/win32/win32support.h"
#include "../../../include/iappdelegate.h"
#include "../../../include/iapplication.h"
#include "../../application.h"
#include "../../genericalertbox.h"
#include "../../shareduiresources.h"
#include "../../window.h"
#include "../iplatformwindow.h"
#include "win32async.h"
#include "win32preference.h"
#include "win32window.h"
#include "win32commondirectories.h"
#include <ShellScalingAPI.h>
#include <Windows.h>
#include <array>
#include <chrono>

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#pragma comment(linker, \
                "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

using VSTGUI::Standalone::Detail::IPlatformApplication;
using VSTGUI::Standalone::Detail::CommandWithKey;
using VSTGUI::Standalone::Detail::IPlatformWindowAccess;
using CommandWithKeyList = VSTGUI::Standalone::Detail::IPlatformApplication::CommandWithKeyList;
using VSTGUI::Standalone::Detail::PlatformCallbacks;

//------------------------------------------------------------------------
static IWin32Window* toWin32Window (const VSTGUI::Standalone::WindowPtr& window)
{
	auto platformWindow = dynamicPtrCast<Detail::IPlatformWindowAccess> (window);
	if (!platformWindow)
		return nullptr;
	return staticPtrCast<IWin32Window> (platformWindow->getPlatformWindow ()).get ();
}

//------------------------------------------------------------------------
class Application
{
public:
	Application () = default;

	void init (HINSTANCE instance, LPWSTR commandLine);
	void run ();

	void quit ();
	void onCommandUpdate ();
	AlertResult showAlert (const AlertBoxConfig& config);
	void showAlertForWindow (const AlertBoxForWindowConfig& config);

private:
	static void dispatchPaintMessages ();

	Win32Preference prefs;
	CommonDirectories commonDirectories;
	bool needCommandUpdate {false};
	HACCEL keyboardAccelerators {nullptr};
};

//------------------------------------------------------------------------
void Application::init (HINSTANCE instance, LPWSTR commandLine)
{
	SetProcessDpiAwareness (PROCESS_PER_MONITOR_DPI_AWARE);

	IApplication::CommandLineArguments cmdArgs;
	int numArgs = 0;
	auto cmdArgsArray = CommandLineToArgvW (commandLine, &numArgs);
	for (int i = 0; i < numArgs; ++i)
	{
		UTF8StringHelper str (cmdArgsArray[i]);
		cmdArgs.push_back (str.getUTF8String ());
	}
	LocalFree (cmdArgsArray);

	initAsyncHandling (instance);

	PlatformCallbacks callbacks;
	callbacks.quit = [this] () { quit (); };
	callbacks.onCommandUpdate = [this] () { 
		if (!needCommandUpdate)
		{
			needCommandUpdate = true;
			Async::perform (Async::Context::Main, [this] () {
				onCommandUpdate ();
			});
		}
	};
	callbacks.showAlert = [this] (const AlertBoxConfig& config) { return showAlert (config); };
	callbacks.showAlertForWindow = [this] (const AlertBoxForWindowConfig& config) {
		showAlertForWindow (config);
	};

	auto app = Detail::getApplicationPlatformAccess ();
	vstgui_assert (app);
	app->init ({prefs, commonDirectories, std::move (cmdArgs), std::move (callbacks)});
}

//------------------------------------------------------------------------
AlertResult Application::showAlert (const AlertBoxConfig& config)
{
	AlertResult result = AlertResult::Error;
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
		winModalWindow->center ();

		window->show ();
		MSG msg;
		BOOL gmResult;
		while (result == AlertResult::Error && (gmResult = GetMessage (&msg, NULL, 0, 0)))
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
		    Async::perform (Async::Context::Main, [callback, r, parentWindow] () {
			    callback (r);
			    if (auto winWindow = toWin32Window (parentWindow))
				    winWindow->activate ();
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
	std::vector<ACCEL> accels;
	for (auto& grp : Detail::getApplicationPlatformAccess ()->getCommandList ())
	{
		for (auto& e : grp.second)
		{
			if (e.defaultKey)
			{
				BYTE virt = FVIRTKEY | FCONTROL;
				auto upperKey = toupper (e.defaultKey);
				if (upperKey == e.defaultKey)
					virt |= FSHIFT;
				accels.push_back ({virt, static_cast<WORD> (upperKey), e.id});
			}
		}
	}
	if (!accels.empty ())
		keyboardAccelerators =
		    CreateAcceleratorTable (accels.data (), static_cast<int> (accels.size ()));
	needCommandUpdate = false;
}

//------------------------------------------------------------------------
void Application::quit ()
{
	Async::perform (Async::Context::Main, [] () {
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
void Application::dispatchPaintMessages ()
{
	HWND prevPaintWindow = nullptr;
	MSG msg;
	while (PeekMessage (&msg, nullptr, WM_PAINT, WM_PAINT, PM_REMOVE | PM_QS_PAINT))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
		if (prevPaintWindow == msg.hwnd)
			break;
		prevPaintWindow = msg.hwnd;
	}
}

//------------------------------------------------------------------------
void Application::run ()
{
	using namespace std::chrono;
	auto lastPaintMessageTime = steady_clock::now ();
	MSG msg;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		if (keyboardAccelerators && TranslateAccelerator (msg.hwnd, keyboardAccelerators, &msg))
			continue;

		TranslateMessage (&msg);
		DispatchMessage (&msg);
		if (msg.message == WM_PAINT &&
		    duration_cast<milliseconds> (steady_clock::now () - lastPaintMessageTime).count () >=
		        15)
		{
			dispatchPaintMessages ();
			lastPaintMessageTime = steady_clock::now ();
		}
	}
	terminateAsyncHandling ();
	Detail::cleanupSharedUIResources ();
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
	app.init (instance, lpCmdLine);
	app.run ();

	CoUninitialize ();
	return 0;
}
