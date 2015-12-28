#include "testwindow.h"
#include "vstgui/standalone/iapplication.h"
#include "vstgui/standalone/ialertbox.h"

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
TestModel::TestModel ()
{
	addValue (IValue::make ("Activate", 1.));
	addValue (IValue::make ("Test", 0.));
	addValue (IStepValue::make ("StepTest", 5, 0));
	addValue (IValue::make ("ShowAlert", 0.));
}

//------------------------------------------------------------------------
void TestModel::addValue (const ValuePtr& value)
{
	value->registerListener (this);
	values.push_back (value);
}

//------------------------------------------------------------------------
void TestModel::onBeginEdit (const IValue& value) {}

//------------------------------------------------------------------------
void TestModel::onPerformEdit (const IValue& value, IValue::Type newValue) {}

//------------------------------------------------------------------------
void TestModel::onEndEdit (const IValue& value)
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
			if (res == AlertResult::thirdButton)
				IApplication::instance ().quit ();
		};
		if (config.window)
			IApplication::instance ().showAlertBoxForWindow (config);
		else
			IApplication::instance ().showAlertBox (config);
	}
}

//------------------------------------------------------------------------
void TestModel::onStateChange (const IValue& value) {}

//------------------------------------------------------------------------
} // MyApp

