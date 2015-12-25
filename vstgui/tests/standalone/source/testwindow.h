#pragma once

#include "vstgui/standalone/iuidescwindow.h"

//------------------------------------------------------------------------
namespace MyApp {

using VSTGUI::Standalone::UIDescription::IModelHandler;
using VSTGUI::Standalone::IValueListener;

//------------------------------------------------------------------------
class TestModelHandler : public IModelHandler, public IValueListener
{
public:
	using UTF8String = VSTGUI::UTF8String;
	using Command = VSTGUI::Standalone::Command;
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using IValue = VSTGUI::Standalone::IValue;
	
	TestModelHandler ();
	
	const CommandList& getCommands () const override { return commands; }
	const ValueList& getValues () const override { return values; }

	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	void onBeginEdit (const IValue& value) override;
	void onPerformEdit (const IValue& value, IValue::Type newValue) override;
	void onEndEdit (const IValue& value) override;
	void onStateChange (const IValue& value) override;
private:
	void addValue (const ValuePtr& value);
	CommandList commands;
	ValueList values;
};

//------------------------------------------------------------------------
} // MyApp