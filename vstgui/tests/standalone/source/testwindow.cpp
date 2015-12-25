#include "testwindow.h"

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
TestModelHandler::TestModelHandler ()
{
	addValue (IValue::make ("Activate", 1.));
	addValue (IValue::make ("Test", 0.));
	addValue (IStepValue::make ("StepTest", 5, 0));
}

//------------------------------------------------------------------------
void TestModelHandler::addValue (const ValuePtr& value)
{
	value->registerListener (this);
	values.push_back (value);
}

//------------------------------------------------------------------------
bool TestModelHandler::canHandleCommand (const Command& command)
{
	return false;
}

//------------------------------------------------------------------------
bool TestModelHandler::handleCommand (const Command& command)
{
	return false;
}

void TestModelHandler::onBeginEdit (const IValue& value) {}
void TestModelHandler::onPerformEdit (const IValue& value, IValue::Type newValue) {}
void TestModelHandler::onEndEdit (const IValue& value)
{
	auto activeValue = values[0];
	if (&value == activeValue.get ())
	{
		for (auto& v : values)
		{
			if (v != values[0])
				v->setActive(activeValue->getValue () == 1.);
		}
	}
}
void TestModelHandler::onStateChange (const IValue& value) {}

//------------------------------------------------------------------------
} // MyApp

