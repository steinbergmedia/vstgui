#include "vstgui/standalone/iapplication.h"
#include "vstgui/standalone/iuidescwindow.h"
#include "vstgui/standalone/helpers/appdelegate.h"
#include "vstgui/standalone/helpers/windowlistener.h"

using namespace VSTGUI::Standalone;
using namespace VSTGUI::Standalone::Application;

class MyApplication : public DelegateAdapter, public WindowListenerAdapter
{
public:
	MyApplication ()
	: DelegateAdapter ({"simple_standalone", "1.0.0", "com.mycompany.simple_standalone"})
	{}
	
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

static Init gAppDelegate (std::make_shared<MyApplication> ());
