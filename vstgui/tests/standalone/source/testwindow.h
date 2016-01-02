#pragma once

#include "vstgui/standalone/iuidescwindow.h"

//------------------------------------------------------------------------
namespace MyApp {

using VSTGUI::Standalone::UIDesc::IModelBinding;
using VSTGUI::Standalone::ValueListenerAdapter;

//------------------------------------------------------------------------
class TestModel : public IModelBinding, public ValueListenerAdapter
{
public:
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using IValue = VSTGUI::Standalone::IValue;
	
	TestModel ();
	
	const ValueList& getValues () const override { return values; }

	void onEndEdit (const IValue& value) override;
private:
	void addValue (const ValuePtr& value);
	ValueList values;
};

//------------------------------------------------------------------------
} // MyApp
