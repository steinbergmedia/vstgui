// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "mandelbrotwindow.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/standalone/include/iwindow.h"
#include <atomic>

//------------------------------------------------------------------------
namespace Mandelbrot {
using namespace VSTGUI;
using namespace VSTGUI::Standalone;
using namespace VSTGUI::Standalone::Application;

//------------------------------------------------------------------------
struct AppDelegate : DelegateAdapter, WindowListenerAdapter, ICommandHandler
{
	AppDelegate () : DelegateAdapter ({"mandelbrot", "1.0.0", VSTGUI_STANDALONE_APP_URI}) {}

	void finishLaunching () override
	{
		IApplication::instance ().registerCommand (Commands::NewDocument, 'n');
		if (!handleCommand (Commands::NewDocument))
			IApplication::instance ().quit ();
	}

	bool canHandleCommand (const Command& command) override
	{
		if (command == Commands::NewDocument)
			return true;
		return false;
	}

	bool handleCommand (const Command& command) override
	{
		if (command == Commands::NewDocument)
		{
			if (auto window = makeMandelbrotWindow ())
			{
				window->show ();
				window->registerWindowListener (this);
				return true;
			}
		}
		return false;
	}

	void onClosed (const IWindow& window) override
	{
		if (IApplication::instance ().getWindows ().empty ())
			IApplication::instance ().quit ();
	}
};

static Init gAppDelegate (std::make_unique<AppDelegate> ());

//------------------------------------------------------------------------
} // Mandelbrot
