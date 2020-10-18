// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../include/helpers/value.h"
#include "../../../lib/algorithm.h"
#include "../../../lib/dispatchlist.h"
#include "../../include/ivaluelistener.h"
#include <algorithm>
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {
namespace /* anonymous */ {

//------------------------------------------------------------------------
IValue::Type convertStepToValue (IStepValue::StepType step, IStepValue::StepType steps)
{
	return static_cast<IValue::Type> (step) / static_cast<IValue::Type> (steps);
}

//------------------------------------------------------------------------
IStepValue::StepType convertValueToStep (IValue::Type value, IStepValue::StepType steps)
{
	return std::min (
	    steps, static_cast<IStepValue::StepType> (value * static_cast<IValue::Type> (steps + 1)));
}

//------------------------------------------------------------------------
class PercentValueConverter : public IValueConverter
{
public:
	UTF8String valueAsString (IValue::Type value) const override
	{
		auto v = static_cast<uint32_t> (value * 100.);
		return toString (v) + " %";
	}

	IValue::Type stringAsValue (const UTF8String& string) const override
	{
		auto v = UTF8StringView (string).toDouble ();
		return v / 100.;
	}

	IValue::Type plainToNormalized (IValue::Type plain) const override { return plain / 100.; }

	IValue::Type normalizedToPlain (IValue::Type normalized) const override
	{
		return normalized * 100.;
	}
};

//------------------------------------------------------------------------
class DefaultValueConverter : public IValueConverter
{
public:
	DefaultValueConverter (uint32_t stringPrecision = 40) : stringPrecision (stringPrecision) {}

	UTF8String valueAsString (IValue::Type value) const override
	{
		UTF8String result;
		if (value < 0. || value > 1.)
			return result;
		value = normalizedToPlain (value);
		std::stringstream sstream;
		sstream.imbue (std::locale::classic ());
		sstream.precision (stringPrecision);
		if (stringPrecision)
			sstream << std::showpoint;
		sstream << std::fixed;
		sstream << value;
		result = sstream.str ();
		return result;
	}

	IValue::Type stringAsValue (const UTF8String& string) const override
	{
		IValue::Type value;
		std::istringstream sstream (string.getString ());
		sstream.imbue (std::locale::classic ());
		sstream.precision (stringPrecision);
		sstream >> value;
		value = plainToNormalized (value);
		if (value < 0. || value > 1.)
			return IValue::InvalidValue;
		return value;
	}

	IValue::Type plainToNormalized (IValue::Type plain) const override { return plain; }

	IValue::Type normalizedToPlain (IValue::Type normalized) const override { return normalized; }
private:
	uint32_t stringPrecision {40};
};

//------------------------------------------------------------------------
class RangeValueConverter : public DefaultValueConverter, public IRangeValueConverter
{
public:
	RangeValueConverter (IValue::Type minValue, IValue::Type maxValue, uint32_t stringPrecision)
	: DefaultValueConverter (stringPrecision), minValue (minValue), maxValue (maxValue)
	{
	}

	IValue::Type plainToNormalized (IValue::Type plain) const override
	{
		return (plain - minValue) / (maxValue - minValue);
	}

	IValue::Type normalizedToPlain (IValue::Type normalized) const override
	{
		return normalized * (maxValue - minValue) + minValue;
	}

	void setRange (IValue::Type _min, IValue::Type _max) override
	{
		minValue = _min;
		maxValue = _max;
	}

private:
	IValue::Type minValue;
	IValue::Type maxValue;
};

//------------------------------------------------------------------------
class StringListValueConverter : public IValueConverter
{
public:
	explicit StringListValueConverter (const std::initializer_list<UTF8String>& list)
	: strings (list)
	{
	}

	explicit StringListValueConverter (const IStringListValue::StringList& list) : strings (list) {}

	UTF8String valueAsString (IValue::Type value) const override
	{
		if (strings.empty ())
			return "";
		auto index =
		    convertValueToStep (value, static_cast<IStepValue::StepType> (strings.size () - 1));
		return strings[index];
	}

	IValue::Type stringAsValue (const UTF8String& string) const override
	{
		if (auto index = indexOf (strings.begin (), strings.end (), string))
		{
			return convertStepToValue (static_cast<IStepValue::StepType> (*index),
			                           static_cast<IStepValue::StepType> (strings.size () - 1));
		}
		return IValue::InvalidValue;
	}

	IValue::Type plainToNormalized (IValue::Type plain) const override
	{
		return convertStepToValue (static_cast<IStepValue::StepType> (plain),
		                           static_cast<IStepValue::StepType> (strings.size () - 1));
	}

	IValue::Type normalizedToPlain (IValue::Type normalized) const override
	{
		return convertValueToStep (normalized,
		                           static_cast<IStepValue::StepType> (strings.size () - 1));
	}

private:
	IStringListValue::StringList strings;
};

//------------------------------------------------------------------------
struct ValueBase : public IValue
{
	ValueBase (const UTF8String& id) : idString (id) {}

	const UTF8String& getID () const override { return idString; }

	using Listeners = DispatchList<IValueListener*>;
	
	void registerListener (IValueListener* listener) override { listeners.add (listener); }
	void unregisterListener (IValueListener* listener) override { listeners.remove (listener); }

	Listeners& getListeners () { return listeners; }

private:
	UTF8String idString;
	Listeners listeners;
};

//------------------------------------------------------------------------
class StaticStringValue : public ValueBase, public IValueConverter
{
public:
	StaticStringValue (const UTF8String& id, const UTF8String& value)
	: ValueBase (id), value (value)
	{}

	StaticStringValue (const UTF8String& id, UTF8String&& value)
	: ValueBase (id), value (std::move (value))
	{}

	void beginEdit () override {}
	bool performEdit (Type newValue) override { return false; }
	void endEdit () override {}
	
	void setActive (bool state) override {}
	bool isActive () const override { return false; }
	
	Type getValue () const override { return 0.; }
	bool isEditing () const override { return false; }
	
	const IValueConverter& getConverter () const override { return *this; }

	UTF8String valueAsString (IValue::Type) const override { return value; }
	IValue::Type stringAsValue (const UTF8String&) const override { return 0.; }
	IValue::Type plainToNormalized (IValue::Type) const override { return 0.; }
	IValue::Type normalizedToPlain (IValue::Type) const override { return 0.; }
private:
	UTF8String value;
};

//------------------------------------------------------------------------
class Value : public ValueBase
{
public:
	Value (const UTF8String& id, Type initialValue, const ValueConverterPtr& valueConverter);

	void beginEdit () override;
	bool performEdit (Type newValue) override;
	void endEdit () override;

	void setActive (bool state) override;
	bool isActive () const override;

	Type getValue () const override;
	bool isEditing () const override;

	const IValueConverter& getConverter () const override;

	bool hasValueConverter () const { return valueConverter != nullptr; }
	void setValueConverter (const ValueConverterPtr& stringConverter);

	void dispatchStateChange ();

private:
	Type value;
	bool active {true};
	uint32_t editCount {0};
	ValueConverterPtr valueConverter;
};

//------------------------------------------------------------------------
class StringValue : public Value, public IValueConverter, public IStringValue
{
public:
	StringValue (const UTF8String& id, const UTF8String& value)
	: Value (id, 0, nullptr), str (value)
	{
	}

	StringValue (const UTF8String& id, UTF8String&& value)
	: Value (id, 0, nullptr), str (std::move (value))
	{
	}

	const IValueConverter& getConverter () const override { return *this; }

	UTF8String valueAsString (IValue::Type) const override { return str; }
	IValue::Type stringAsValue (const UTF8String& s) const override
	{
		if (isEditing ())
			str = s;
		return 0.;
	}
	IValue::Type plainToNormalized (IValue::Type) const override { return 0.; }
	IValue::Type normalizedToPlain (IValue::Type) const override { return 0.; }
	
	void setString (const UTF8String& s) override
	{
		str = s;
		if (isEditing ())
			performEdit (0.);
	}
	const UTF8String& getString () const override
	{
		return str;
	}

private:
	mutable UTF8String str;
};

//------------------------------------------------------------------------
class StepValue : public Value, public IStepValue, public IValueConverter, public IMutableStepValue
{
public:
	StepValue (const UTF8String& id, StepType initialSteps, Type initialValue,
	           const ValueConverterPtr& stringConverter);

	bool performEdit (Type newValue) override;

	StepType getSteps () const override;
	IValue::Type stepToValue (StepType step) const override;
	StepType valueToStep (IValue::Type) const override;

	UTF8String valueAsString (IValue::Type value) const override;
	IValue::Type stringAsValue (const UTF8String& string) const override;
	IValue::Type plainToNormalized (IValue::Type plain) const override;
	IValue::Type normalizedToPlain (IValue::Type normalized) const override;

	const IValueConverter& getConverter () const override;

	void setNumSteps (StepType numSteps) override;

private:
	StepType steps;
};

//------------------------------------------------------------------------
class StringListValue : public StepValue, public IStringListValue
{
public:
	StringListValue (const UTF8String& id, StepType initialSteps, Type initialValue,
	                 const ValueConverterPtr& stringConverter);

	bool updateStringList (const StringList& newStrings) override;
};

//------------------------------------------------------------------------
Value::Value (const UTF8String& id, Type initialValue, const ValueConverterPtr& valueConverter)
: ValueBase (id), value (initialValue), valueConverter (valueConverter)
{
}

//------------------------------------------------------------------------
void Value::beginEdit ()
{
	++editCount;

	if (editCount == 1)
	{
		getListeners ().forEach ([this] (IValueListener* l) { l->onBeginEdit (*this); });
	}
}

//------------------------------------------------------------------------
bool Value::performEdit (Type newValue)
{
	if (newValue < 0. || newValue > 1.)
		return false;
	//	if (newValue == value)
	//		return true;
	value = newValue;

	getListeners ().forEach ([this] (IValueListener* l) { l->onPerformEdit (*this, value); });

	return true;
}

//------------------------------------------------------------------------
void Value::endEdit ()
{
	vstgui_assert (editCount > 0);
	--editCount;

	if (editCount == 0)
	{
		getListeners ().forEach ([this] (IValueListener* l) { l->onEndEdit (*this); });
	}
}

//------------------------------------------------------------------------
void Value::setActive (bool state)
{
	if (state == active)
		return;
	active = state;
	dispatchStateChange ();
}

//------------------------------------------------------------------------
bool Value::isActive () const
{
	return active;
}

//------------------------------------------------------------------------
Value::Type Value::getValue () const
{
	return value;
}

//------------------------------------------------------------------------
bool Value::isEditing () const
{
	return editCount != 0;
}

//------------------------------------------------------------------------
const IValueConverter& Value::getConverter () const
{
	return *valueConverter.get ();
}

//------------------------------------------------------------------------
void Value::dispatchStateChange ()
{
	getListeners ().forEach ([this] (IValueListener* l) { l->onStateChange (*this); });
}

//------------------------------------------------------------------------
void Value::setValueConverter (const ValueConverterPtr& converter)
{
	valueConverter = converter;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
StepValue::StepValue (const UTF8String& id, StepType initialSteps, Type initialValue,
                      const ValueConverterPtr& stringConverter)
: Value (id, initialValue, stringConverter), steps (initialSteps - 1)
{
}

//------------------------------------------------------------------------
bool StepValue::performEdit (Type newValue)
{
	return Value::performEdit (stepToValue (valueToStep (newValue)));
}

//------------------------------------------------------------------------
StepValue::StepType StepValue::getSteps () const
{
	return steps + 1;
}

//------------------------------------------------------------------------
IValue::Type StepValue::stepToValue (StepType step) const
{
	if (steps == 0)
		return 0.;
	return convertStepToValue (step, steps);
}

//------------------------------------------------------------------------
StepValue::StepType StepValue::valueToStep (IValue::Type value) const
{
	return convertValueToStep (value, steps);
}

//------------------------------------------------------------------------
UTF8String StepValue::valueAsString (IValue::Type value) const
{
	auto v = valueToStep (value);
	return UTF8String (std::to_string (v));
}

//------------------------------------------------------------------------
IValue::Type StepValue::stringAsValue (const UTF8String& string) const
{
	StepType v;
	std::istringstream sstream (string.getString ());
	sstream.imbue (std::locale::classic ());
	sstream >> v;
	if (v > steps)
		return IValue::InvalidValue;
	return stepToValue (v);
}

//------------------------------------------------------------------------
IValue::Type StepValue::plainToNormalized (IValue::Type plain) const
{
	return stepToValue (static_cast<IStepValue::StepType> (plain));
}

//------------------------------------------------------------------------
IValue::Type StepValue::normalizedToPlain (IValue::Type normalized) const
{
	return valueToStep (normalized);
}

//------------------------------------------------------------------------
const IValueConverter& StepValue::getConverter () const
{
	if (!hasValueConverter ())
		return *this;
	return Value::getConverter ();
}

//------------------------------------------------------------------------
void StepValue::setNumSteps (StepType numSteps)
{
	steps = numSteps - 1;
	dispatchStateChange ();
}

//------------------------------------------------------------------------
StringListValue::StringListValue (const UTF8String& id, StepType initialSteps, Type initialValue,
                                  const ValueConverterPtr& stringConverter)
: StepValue (id, initialSteps, initialValue, stringConverter)
{
}

//------------------------------------------------------------------------
bool StringListValue::updateStringList (const StringList& newStrings)
{
	setValueConverter (std::make_shared<Detail::StringListValueConverter> (newStrings));
	setNumSteps (static_cast<IStepValue::StepType> (newStrings.size ()));
	return true;
}

//------------------------------------------------------------------------
ValueConverterPtr getDefaultConverter ()
{
	static ValueConverterPtr gInstance = std::make_shared<Detail::DefaultValueConverter> ();
	return gInstance;
}

//------------------------------------------------------------------------
} // anonymous
} // Detail

//------------------------------------------------------------------------
namespace Value {

//------------------------------------------------------------------------
ValuePtr make (const UTF8String& id, IValue::Type initialValue,
               const ValueConverterPtr& stringConverter)
{
	vstgui_assert (id.empty () == false);
	return std::make_shared<Detail::Value> (id, initialValue, stringConverter.get () ?
	                                                              stringConverter :
	                                                              Detail::getDefaultConverter ());
}

//------------------------------------------------------------------------
ValuePtr makeStepValue (const UTF8String& id, IStepValue::StepType initialSteps,
                        IValue::Type initialValue, const ValueConverterPtr& stringConverter)
{
	vstgui_assert (id.empty () == false);
	return std::make_shared<Detail::StepValue> (id, initialSteps, initialValue, stringConverter);
}

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id,
                              const std::initializer_list<IStringListValue::StringType>& strings,
                              IValue::Type initialValue)
{
	vstgui_assert (id.empty () == false);
	return std::make_shared<Detail::StringListValue> (
	    id, static_cast<IStepValue::StepType> (strings.size ()), initialValue,
	    std::make_shared<Detail::StringListValueConverter> (strings));
}

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id, const IStringListValue::StringList& strings)
{
	vstgui_assert (id.empty () == false);
	return std::make_shared<Detail::StringListValue> (
	    id, static_cast<IStepValue::StepType> (strings.size ()), 0,
	    std::make_shared<Detail::StringListValueConverter> (strings));
}

//------------------------------------------------------------------------
ValuePtr makeStaticStringValue (const UTF8String& id, const UTF8String& value)
{
	return std::make_shared<Detail::StaticStringValue> (id, value);
}

//------------------------------------------------------------------------
ValuePtr makeStaticStringValue (const UTF8String& id, UTF8String&& value)
{
	return std::make_shared<Detail::StaticStringValue> (id, std::move (value));
}

//------------------------------------------------------------------------
ValuePtr makeStringValue (const UTF8String& id, const UTF8String& initialString)
{
	return std::make_shared<Detail::StringValue> (id, initialString);
}

//------------------------------------------------------------------------
ValuePtr makeStringValue (const UTF8String& id, UTF8String&& initialString)
{
	return std::make_shared<Detail::StringValue> (id, std::move (initialString));
}

//------------------------------------------------------------------------
ValueConverterPtr makePercentConverter ()
{
	return std::make_shared<Detail::PercentValueConverter> ();
}

//------------------------------------------------------------------------
ValueConverterPtr makeRangeConverter (IValue::Type minValue, IValue::Type maxValue,
                                      uint32_t stringPrecision)
{
	return std::make_shared<Detail::RangeValueConverter> (minValue, maxValue, stringPrecision);
}

//------------------------------------------------------------------------
} // Value
} // Standalone
} // VSTGUI
