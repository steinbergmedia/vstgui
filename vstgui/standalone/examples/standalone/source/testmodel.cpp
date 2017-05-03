// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "testmodel.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cvstguitimer.h"

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
TestModel::TestModel ()
{
	addValue (Value::make ("Activate", 1.));
	addValue (Value::make ("Test"));
	addValue (Value::makeStepValue ("StepTest", 5, 0));
	addValue (Value::make ("ShowAlert"));
	addValue (Value::makeStringListValue ("StringList", {"one","two","three","four","five"}));
	addValue (Value::make ("ShowPopup"));
}

//------------------------------------------------------------------------
void TestModel::addValue (ValuePtr&& value)
{
	value->registerListener (this);
	values.emplace_back (std::move (value));
}

//------------------------------------------------------------------------
void TestModel::onEndEdit (IValue& value)
{
	auto activeValue = values[0];
	if (&value == activeValue.get ())
	{
		for (auto& v : values)
		{
			if (v != values[0])
				v->setActive (activeValue->getValue () == 1.);
		}
	}
	else if (value.getID () == "ShowAlert")
	{
		AlertBoxForWindowConfig config;
		config.headline = "Test Alert";
		config.description = "This is an example alert box.";
		config.secondButton = "Cancel";
		config.thirdButton = "Do Quit";
		config.window = IApplication::instance ().getWindows ().front ();
		config.callback = [] (AlertResult res) {
			if (res == AlertResult::ThirdButton)
				IApplication::instance ().quit ();
		};
		if (config.window)
			IApplication::instance ().showAlertBoxForWindow (config);
		else
			IApplication::instance ().showAlertBox (config);
	}
	else if (value.getID () == "ShowPopup" && value.getValue () > 0.5)
	{
		auto v = values[5];
		v->performEdit (0.);
		auto window = IApplication::instance ().getWindows ().front ();
		vstgui_assert (window);
		auto rect = window->getFocusViewRect ();
		if (rect.isEmpty ())
			return;
		rect.offset (window->getPosition ());
		UIDesc::Config config;
		config.viewName = "view";
		config.modelBinding = shared_from_this ();
		config.uiDescFileName = "testpopup.uidesc";
		config.windowConfig.type = WindowType::Popup;
		if (auto popup = UIDesc::makeWindow (config))
		{
			auto size = popup->getSize ();
			CRect r;
			r.setSize (size);
			r.centerInside (rect);
			popup->setPosition (r.getTopLeft ());
			popup->show ();
		}
		
	}
}

//------------------------------------------------------------------------
} // MyApp

