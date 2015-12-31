#pragma once

#include "vstgui/standalone/iuidescwindow.h"

//------------------------------------------------------------------------
namespace MyApp {

using VSTGUI::Standalone::UIDesc::IModelBinding;
using VSTGUI::Standalone::IValueListener;

//------------------------------------------------------------------------
class TestModel : public IModelBinding, public IValueListener
{
public:
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using IValue = VSTGUI::Standalone::IValue;
	
	TestModel ();
	
	const ValueList& getValues () const override { return values; }

	void onBeginEdit (const IValue& value) override;
	void onPerformEdit (const IValue& value, IValue::Type newValue) override;
	void onEndEdit (const IValue& value) override;
	void onStateChange (const IValue& value) override;
private:
	void addValue (const ValuePtr& value);
	ValueList values;
};

//------------------------------------------------------------------------
} // MyApp
