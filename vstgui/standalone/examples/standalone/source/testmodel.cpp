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
	addValue (Value::make ("Test", 0, Value::makeRangeConverter (0, 10, 2)));
	addValue (Value::makeStepValue ("StepTest", 5, 0));
	addValue (Value::make ("ShowAlert"));
	addValue (Value::make ("ShowAlert2"));
	addValue (Value::make ("ShowAlert3"));
	addValue (Value::makeStringListValue ("StringList", {"one", "two", "three", "four", "five"}));
	addValue (Value::make ("ShowPopup"));
	addValue (Value::makeStringValue ("MutableString", "This is a string value"));
	addValue (
	    Value::makeStringListValue ("Weekdays", {"Weekdays", "Monday", "Tuesday", "Wednesday",
	                                             "Thirsday", "Friday", "Saturday", "Sunday"}));
	Value::performSingleStepEdit (*values.back (), 3);
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
	auto v = value.getValue ();

	auto activeValue = values[0];
	if (&value == activeValue.get ())
	{
		for (auto& val : values)
		{
			if (val != values[0])
				val->setActive (activeValue->getValue () == 1.);
		}
	}
	else if (value.getID () == "ShowAlert")
	{
		value.performEdit (0.);
		if (v > 0.5)
		{
			AlertBoxForWindowConfig config;
			config.headline = "Test Alert";
			config.description =
			    "This is an example alert box.\nWith more than one line.\nIt even has more than two lines.";
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
	}
	else if (value.getID () == "ShowAlert2")
	{
		value.performEdit (0.);
		if (v > 0.5)
		{
			AlertBoxForWindowConfig config;
			config.headline = "Test Alert 2";
			config.description =
			    "This is an example alert box.\nWith more than one line.\nIt even has more than two lines.";
			config.defaultButton = "Close";
			config.secondButton = "Cancel";
			config.window = IApplication::instance ().getWindows ().front ();
			IApplication::instance ().showAlertBox (config);
		}
	}
	else if (value.getID () == "ShowAlert3")
	{
		value.performEdit (0.);
		if (v > 0.5)
		{
			AlertBoxForWindowConfig config;
			config.headline = "Test Alert 2";
			config.defaultButton = "YES";
			config.window = IApplication::instance ().getWindows ().front ();
			IApplication::instance ().showAlertBox (config);
		}
	}
	else if (value.getID () == "ShowPopup" && value.getValue () > 0.5)
	{
		value.performEdit (0.);
		CRect rect;
		for (auto& window : IApplication::instance ().getWindows ())
		{
			if (window->getType () == WindowType::Document)
			{
				rect = window->getFocusViewRect ();
				rect.offset (window->getPosition ());
				break;
			}
		}
		if (rect.isEmpty ())
			return;
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
