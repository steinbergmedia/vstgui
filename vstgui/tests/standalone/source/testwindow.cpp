#include "testwindow.h"

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
}

//------------------------------------------------------------------------
void TestModel::onStateChange (const IValue& value) {}

//------------------------------------------------------------------------
} // MyApp

