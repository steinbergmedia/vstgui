// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "testabout.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/iuidescwindow.h"

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI::Standalone;
using namespace VSTGUI::Standalone::UIDesc;

//------------------------------------------------------------------------
About::Ptr About::gInstance;

//------------------------------------------------------------------------
void About::show ()
{
	if (gInstance)
	{
		gInstance->window->activate ();
		return;
	}
	gInstance = std::unique_ptr<About> (new About ());
	auto modelBinding = std::make_shared<ModelBindingCallbacks> ();
	modelBinding->addValue (Value::make ("Close"), ValueCalls::onEndEdit ([] (const IValue& value) {
		                        if (gInstance && value.getValue () > 0.)
			                        gInstance->window->close ();
		                    }));

	UIDesc::Config config;
	config.uiDescFileName = "about.uidesc";
	config.viewName = "view";
	config.modelBinding = modelBinding;
	config.windowConfig.type = WindowType::Document;
	config.windowConfig.title = "About";
	config.windowConfig.style.centered ().close ().transparent ().movableByWindowBackground ();
	gInstance->window = UIDesc::makeWindow (config);
	if (gInstance->window)
	{
		gInstance->window->registerWindowListener (gInstance.get ());
		gInstance->window->show ();
	}
}

//------------------------------------------------------------------------
void About::onClosed (const IWindow&) { gInstance = nullptr; }

//------------------------------------------------------------------------
} // MyApp
