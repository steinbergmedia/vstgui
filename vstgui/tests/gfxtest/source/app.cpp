#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "drawdevicetests.h"

namespace VSTGUI {
namespace Standalone {
		
using namespace Application;
	
class GFXTestApp : public DelegateAdapter, public WindowListenerAdapter
{
public:
	GFXTestApp ()
	: DelegateAdapter ({"gfxtest", "1.0.0", "vstgui.tests.gfxtest"})
	{}
	
	void finishLaunching () override
	{
		auto modelBinding = UIDesc::ModelBindingCallbacks::make ();
		modelBinding->addValue (Value::make ("ShowDrawDeviceTests"), UIDesc::ValueCalls::onAction ([] (auto& value) {
			makeDrawDeviceTestsWindow ();
			value.performEdit (0.);
		}));
	
		UIDesc::Config config;
		config.uiDescFileName = "Window.uidesc";
		config.viewName = "Window";
		config.modelBinding = modelBinding;
		config.windowConfig.title = "GFXTests";
		config.windowConfig.autoSaveFrameName = "GFXTestsWindow";
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

static Init gAppDelegate (std::make_unique<GFXTestApp> ());

} // Standalone
} // VSTGUI

