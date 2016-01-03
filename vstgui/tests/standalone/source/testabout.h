#pragma once

#include "vstgui/standalone/iwindowcontroller.h"
#include "vstgui/standalone/ivalue.h"
#include "vstgui/standalone/iuidescwindow.h"
#include <unordered_map>
#include <functional>

//------------------------------------------------------------------------
namespace MyApp {

//------------------------------------------------------------------------
struct ValueCalls
{
	using IValue = VSTGUI::Standalone::IValue;
	using Call = std::function<void (const IValue&)>;
	
	Call onBeginEditCall;
	Call onPerformEditCall;
	Call onEndEditCall;
	Call onStateChangeCall;

	static ValueCalls onPerformEdit (Call&& call)
	{
		ValueCalls c;
		c.onPerformEditCall = std::move (call);
		return c;
	}

	static ValueCalls onEndEdit (Call&& call)
	{
		ValueCalls c;
		c.onEndEditCall = std::move (call);
		return c;
	}
};

//------------------------------------------------------------------------
class ModelBindingCallbacks : public VSTGUI::Standalone::ValueListenerAdapter, public VSTGUI::Standalone::UIDesc::IModelBinding
{
public:
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using IValue = VSTGUI::Standalone::IValue;
	
	~ModelBindingCallbacks ();
	
	void addValue (ValuePtr value, const ValueCalls& callbacks = {});
	void addValue (ValuePtr value, ValueCalls&& callbacks);
private:
	const ValueList& getValues () const override { return valueList; }

	void onBeginEdit (const IValue& value) override;
	void onPerformEdit (const IValue& value, IValue::Type newValue) override;
	void onEndEdit (const IValue& value) override;
	void onStateChange (const IValue& value) override;
	
	using ValueMap = std::unordered_map<const IValue*, ValueCalls>;
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
