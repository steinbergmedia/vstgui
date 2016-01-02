#pragma once

#include "vstgui/standalone/iwindowcontroller.h"
#include "vstgui/standalone/ivalue.h"
#include "vstgui/standalone/iuidescwindow.h"
#include <unordered_map>

//------------------------------------------------------------------------
namespace MyApp {

//------------------------------------------------------------------------
struct ValueCallbacks
{
	using IValue = VSTGUI::Standalone::IValue;
	
	std::function<void (const IValue&)> onBeginEditCall;
	std::function<void (const IValue&)> onPerformEditCall;
	std::function<void (const IValue&)> onEndEditCall;
	std::function<void (const IValue&)> onStateChangeCall;
};

//------------------------------------------------------------------------
class ModelBindingCallbacks : public VSTGUI::Standalone::ValueListenerAdapter, public VSTGUI::Standalone::UIDesc::IModelBinding
{
public:
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using IValue = VSTGUI::Standalone::IValue;
	
	~ModelBindingCallbacks ();
	
	void addValue (ValuePtr value, const ValueCallbacks& callbacks = {});
private:
	const ValueList& getValues () const override { return valueList; }

	void onBeginEdit (const IValue& value) override;
	void onPerformEdit (const IValue& value, IValue::Type newValue) override;
	void onEndEdit (const IValue& value) override;
	void onStateChange (const IValue& value) override;
	
	using ValueMap = std::unordered_map<const IValue*, ValueCallbacks>;
	ValueList valueList;
	ValueMap values;
};

//------------------------------------------------------------------------
class About : public VSTGUI::Standalone::WindowListenerAdapter
{
public:
	static void show ();
	
private:
	static std::unique_ptr<About> gInstance;

	using IWindow = VSTGUI::Standalone::IWindow;
	using WindowPtr = VSTGUI::Standalone::WindowPtr;

	void onClosed (const IWindow& window) override;

	WindowPtr window;
};

//------------------------------------------------------------------------
} // MyApp
