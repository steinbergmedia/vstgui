// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../iuidescwindow.h"
#include "../valuelistener.h"
#include <functional>
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace UIDesc {

//------------------------------------------------------------------------
struct ValueCalls
{
	using Call = std::function<void (IValue&)>;

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

	static ValueCalls onAction (Call&& call)
	{
		ValueCalls c;
		c.onEndEditCall = [call = std::move (call)] (IValue & v)
		{
			if (v.getValue () > 0.5)
				call (v);
		};
		return c;
	}
};

class ModelBindingCallbacks;
using ModelBindingCallbacksPtr = std::shared_ptr<ModelBindingCallbacks>;

//------------------------------------------------------------------------
class ModelBindingCallbacks : public ValueListenerAdapter, public IModelBinding
{
public:
	static ModelBindingCallbacksPtr make () { return std::make_shared<ModelBindingCallbacks> (); }
	ModelBindingCallbacks () = default;
	~ModelBindingCallbacks () override;

	ValuePtr addValue (ValuePtr value, const ValueCalls& callbacks = {});
	ValuePtr addValue (ValuePtr value, ValueCalls&& callbacks);

	ValuePtr getValue (UTF8StringView valueID) const;

	const ValueList& getValues () const override { return valueList; }
private:

	void onBeginEdit (IValue& value) override;
	void onPerformEdit (IValue& value, IValue::Type newValue) override;
	void onEndEdit (IValue& value) override;
	void onStateChange (IValue& value) override;

	using ValueMap = std::unordered_map<const IValue*, ValueCalls>;
	ValueList valueList;
	ValueMap values;
};

//------------------------------------------------------------------------
inline ModelBindingCallbacks::~ModelBindingCallbacks ()
{
	for (auto& v : valueList)
		v->unregisterListener (this);
	values.clear ();
	valueList.clear ();
}

//------------------------------------------------------------------------
inline ValuePtr ModelBindingCallbacks::addValue (ValuePtr value, const ValueCalls& callbacks)
{
	values.emplace (value.get (), callbacks);
	valueList.emplace_back (value);
	value->registerListener (this);
	return value;
}

//------------------------------------------------------------------------
inline ValuePtr ModelBindingCallbacks::addValue (ValuePtr value, ValueCalls&& callbacks)
{
	values.emplace (value.get (), std::move (callbacks));
	valueList.emplace_back (value);
	value->registerListener (this);
	return value;
}

//------------------------------------------------------------------------
inline ValuePtr ModelBindingCallbacks::getValue (UTF8StringView valueID) const
{
	for (auto& v : valueList)
	{
		if (v->getID () == valueID)
			return v;
	}
	return nullptr;
}

//------------------------------------------------------------------------
inline void ModelBindingCallbacks::onBeginEdit (IValue& value)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onBeginEditCall)
		it->second.onBeginEditCall (value);
}

//------------------------------------------------------------------------
inline void ModelBindingCallbacks::onPerformEdit (IValue& value, IValue::Type newValue)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onPerformEditCall)
		it->second.onPerformEditCall (value);
}

//------------------------------------------------------------------------
inline void ModelBindingCallbacks::onEndEdit (IValue& value)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onEndEditCall)
		it->second.onEndEditCall (value);
}

//------------------------------------------------------------------------
inline void ModelBindingCallbacks::onStateChange (IValue& value)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onStateChangeCall)
		it->second.onStateChangeCall (value);
}

//------------------------------------------------------------------------
} // UIDesc
} // Standalone
} // VSTGUI
