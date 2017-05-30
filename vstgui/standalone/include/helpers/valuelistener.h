// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../ivaluelistener.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** %Value listener adapter
 *
 *	@ingroup standalone
 */
class ValueListenerAdapter : public IValueListener
{
public:
	void onBeginEdit (IValue& value) override {}
	void onPerformEdit (IValue& value, IValue::Type newValue) override {}
	void onEndEdit (IValue& value) override {}
	void onStateChange (IValue& value) override {}
};

//------------------------------------------------------------------------
namespace Value {
namespace Detail {

//------------------------------------------------------------------------
class ListenerBase : public IValueListener
{
public:
	ListenerBase (IValue& value) : value (value)
	{
		value.registerListener (this);
	}
	~ListenerBase () noexcept override
	{
		value.unregisterListener (this);
	}
	IValue& getValueObject () const { return value; }
private:
	IValue& value;
};

//------------------------------------------------------------------------
} // Detail

//------------------------------------------------------------------------
/** %Value listener
 *
 *	@ingroup standalone
 */
template <typename Context>
class ListenerT : public Detail::ListenerBase
{
public:
	ListenerT (IValue& value, Context context) : Detail::ListenerBase (value), context (context) {}
	
	using OnBeginEditFunc = void(*) (IValue&, Context&);
	using OnEndEditFunc = void(*) (IValue&, Context&);
	using OnStateChangeFunc = void(*) (IValue&, Context&);
	using OnPerformEditFunc = void(*) (IValue&, IValue::Type, Context&);
	
	OnBeginEditFunc onBeginEditFunc {nullptr};
	OnEndEditFunc onEndEditFunc {nullptr};
	OnStateChangeFunc onStateChangeFunc {nullptr};
	OnPerformEditFunc onPerformEditFunc {nullptr};
	
private:
	void onBeginEdit (IValue& value) final
	{
		if (onBeginEditFunc)
			onBeginEditFunc (value, context);
	}
	void onPerformEdit (IValue& value, IValue::Type newValue) final
	{
		if (onPerformEditFunc)
			onPerformEditFunc (value, newValue, context);
	}
	void onEndEdit (IValue& value) final
	{
		if (onEndEditFunc)
			onEndEditFunc (value, context);
	}
	void onStateChange (IValue& value) final
	{
		if (onStateChangeFunc)
			onStateChangeFunc (value, context);
	}
	
	Context context;
};

//------------------------------------------------------------------------
/** %Value listener
 *
 *	@ingroup standalone
 */
class Listener : public Detail::ListenerBase
{
public:
	Listener (IValue& value) : Detail::ListenerBase (value) {}
	
	using OnBeginEditFunc = void(*) (IValue&);
	using OnEndEditFunc = void(*) (IValue&);
	using OnStateChangeFunc = void(*) (IValue&);
	using OnPerformEditFunc = void(*) (IValue&, IValue::Type);
	
	OnBeginEditFunc onBeginEditFunc {nullptr};
	OnEndEditFunc onEndEditFunc {nullptr};
	OnStateChangeFunc onStateChangeFunc {nullptr};
	OnPerformEditFunc onPerformEditFunc {nullptr};
private:
	void onBeginEdit (IValue& value) final
	{
		if (onBeginEditFunc)
			onBeginEditFunc (value);
	}
	void onPerformEdit (IValue& value, IValue::Type newValue) final
	{
		if (onPerformEditFunc)
			onPerformEditFunc (value, newValue);
	}
	void onEndEdit (IValue& value) final
	{
		if (onEndEditFunc)
			onEndEditFunc (value);
	}
	void onStateChange (IValue& value) final
	{
		if (onStateChangeFunc)
			onStateChangeFunc (value);
	}
};

//------------------------------------------------------------------------
} // Value
} // Standalone
} // VSTGUI
