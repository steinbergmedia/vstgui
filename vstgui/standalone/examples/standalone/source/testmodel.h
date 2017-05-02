#pragma once

#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/valuelistener.h"

//------------------------------------------------------------------------
namespace MyApp {

using VSTGUI::Standalone::UIDesc::IModelBinding;
using VSTGUI::Standalone::ValueListenerAdapter;

//------------------------------------------------------------------------
class TestModel : public IModelBinding,
                  public ValueListenerAdapter,
                  public std::enable_shared_from_this<TestModel>
{
public:
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using IValue = VSTGUI::Standalone::IValue;

	TestModel ();

	const ValueList& getValues () const override { return values; }

	void onEndEdit (IValue& value) override;

private:
	void addValue (ValuePtr&& value);
	ValueList values;
};

//------------------------------------------------------------------------
} // MyApp
