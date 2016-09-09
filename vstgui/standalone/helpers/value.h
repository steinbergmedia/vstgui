#pragma once

#include "../ivalue.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class IStringListValue : public Interface
{
public:
	using StringList = std::vector<UTF8String>;
	virtual bool updateStringList (const StringList& newStrings) = 0;
};

//------------------------------------------------------------------------
namespace Value {

//------------------------------------------------------------------------
ValuePtr make (const UTF8String& id, IValue::Type initialValue = 0.,
               const ValueConverterPtr& stringConverter = nullptr);

//------------------------------------------------------------------------
ValuePtr makeStepValue (const UTF8String& id, IStepValue::StepType numSteps,
                        IValue::Type initialValue = 0.,
                        const ValueConverterPtr& stringConverter = nullptr);

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id,
                              const std::initializer_list<UTF8String>& strings,
                              IValue::Type initialValue = 0.);

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id, const IStringListValue::StringList& strings);

//------------------------------------------------------------------------
ValueConverterPtr makePercentConverter ();
ValueConverterPtr makeRangeConverter (IValue::Type minValue, IValue::Type maxValue);

//------------------------------------------------------------------------
inline IValue::Type plainToNormalize (IValue& value, IValue::Type plainValue)
{
	return value.getConverter ().plainToNormalized (plainValue);
}

//------------------------------------------------------------------------
inline IValue::Type normalizeToPlain (IValue& value, IValue::Type normalizeValue)
{
	return value.getConverter ().normalizedToPlain (normalizeValue);
}

//------------------------------------------------------------------------
inline IValue::Type stepToNormalize (IValue& value, IStepValue::StepType stepValue)
{
	if (auto sv = value.dynamicCast<IStepValue> ())
	{
		return sv->stepToValue (stepValue);
	}
	return IValue::InvalidValue;
}

//------------------------------------------------------------------------
inline IStepValue::StepType normalizeToStep (IValue& value, IValue::Type normalizeValue)
{
	if (auto sv = value.dynamicCast<IStepValue> ())
	{
		return sv->valueToStep (normalizeValue);
	}
	return IStepValue::InvalidStep;
}

//------------------------------------------------------------------------
inline IValue::Type currentPlainValue (IValue& value)
{
	return normalizeToPlain (value, value.getValue ());
}

//------------------------------------------------------------------------
inline IStepValue::StepType currentStepValue (IValue& value)
{
	return normalizeToStep (value, value.getValue ());
}

//------------------------------------------------------------------------
inline void performSingleEdit (IValue& value, IValue::Type newValue)
{
	value.beginEdit ();
	value.performEdit (newValue);
	value.endEdit ();
}

//------------------------------------------------------------------------
inline void performSinglePlainEdit (IValue& value, IValue::Type plainValue)
{
	performSingleEdit (value, plainToNormalize (value, plainValue));
}

//------------------------------------------------------------------------
inline bool performSingleStepEdit (IValue& value, IStepValue::StepType step)
{
	if (auto stepValue = value.dynamicCast<IStepValue> ())
	{
		performSingleEdit (value, stepValue->stepToValue (step));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // Value
} // Standalone
} // VSTGUI
