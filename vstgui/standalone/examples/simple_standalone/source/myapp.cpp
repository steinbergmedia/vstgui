// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"

using namespace VSTGUI::Standalone;

class MyApplication : public Application::DelegateAdapter, public WindowListenerAdapter
{
public:
	MyApplication ()
	: Application::DelegateAdapter (
	      {"simple_standalone", "1.0.0", "vstgui.examples.simplestandalone"})
	{
	}

	void finishLaunching () override
	{
		UIDesc::Config config;
		config.uiDescFileName = "Window.uidesc";
		config.viewName = "Window";
		config.windowConfig.title = "Sample App";
		config.windowConfig.autoSaveFrameName = "SampleAppWindow";
		config.windowConfig.style.border ().close ().size ().centered ();
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
			window->registerWindowListener (this);
		}
		else
		{
			IApplication::instance ().quit ();
		}
	}
	void onClosed (const IWindow& window) override
	{
		IApplication::instance ().quit ();
	}
	
};

static Application::Init gAppDelegate (std::make_unique<MyApplication> ());
