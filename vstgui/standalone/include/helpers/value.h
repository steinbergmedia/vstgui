// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	using StringType = UTF8String;
	using StringList = std::vector<StringType>;
	virtual bool updateStringList (const StringList& newStrings) = 0;
};

//------------------------------------------------------------------------
/**	%value create and helper functions
 *	@ingroup standalone
 */
namespace Value {

//------------------------------------------------------------------------
/** @name %Create values
 *	@{ */
 
//------------------------------------------------------------------------
/** make a value in the normalized range [0..1]
 *
 *	@param id value ID
 *	@param initialValue initial value
 *	@param valueConverter value converter
 *	@return shared value pointer
 */
ValuePtr make (const UTF8String& id, IValue::Type initialValue = 0.,
               const ValueConverterPtr& valueConverter = nullptr);

//------------------------------------------------------------------------
/** make a step value
 *
 *	@param id value ID
 *	@param numSteps number of discrete steps
 *	@param initialValue initial value in the normalized range [0..1]
 *	@param valueConverter value converter
 *	@return shared value pointer
 */
ValuePtr makeStepValue (const UTF8String& id, IStepValue::StepType numSteps,
                        IValue::Type initialValue = 0.,
                        const ValueConverterPtr& valueConverter = nullptr);

//------------------------------------------------------------------------
/** make a string list value
 *
 *	a string list value is a step value where each step has a string representation.
 *
 *	to modify the string list you can cast the returned value object to IStringListValue
 *	and use the updateStringList method.
 *
 *	@param id value ID
 *	@param strings string list
 *	@param initialValue initial value in the normalized range [0..1]
 *	@return shared value pointer
 */
ValuePtr makeStringListValue (const UTF8String& id,
                              const std::initializer_list<IStringListValue::StringType>& strings,
                              IValue::Type initialValue = 0.);

//------------------------------------------------------------------------
/** make a string list value
 *
 *	the returned value object has the IStringListValue interface
 *
 *	@param id value ID
 *	@param strings string list
 *	@return shared value pointer
 */
ValuePtr makeStringListValue (const UTF8String& id, const IStringListValue::StringList& strings);

//------------------------------------------------------------------------
/** make a static string value
 *
 *	a static string value is an inactive unchangeable value 
 *
 *	@param id value ID
 *	@param value static string
 *	@return shared value pointer
 */
ValuePtr makeStaticStringValue (const UTF8String& id, const UTF8String& value);

//------------------------------------------------------------------------
/** make a static string value
 *
 *	a static string value is an inactive unchangeable value
 *
 *	@param id value ID
 *	@param value static string
 *	@return shared value pointer
 */
ValuePtr makeStaticStringValue (const UTF8String& id, UTF8String&& value);

/** @} */
/** @name %Create value converters
 *	@{ */

//------------------------------------------------------------------------
/** make a percent value converter
 *
 *	converts normalized values to the range [0..100]
 */
ValueConverterPtr makePercentConverter ();

//------------------------------------------------------------------------
/** make a range value converter
 *
 *	converts normalized values to the range [minValue..maxValue]
 */
ValueConverterPtr makeRangeConverter (IValue::Type minValue, IValue::Type maxValue);

/** @} */

//------------------------------------------------------------------------
/** @name %Value helper functions
 *	@{
 */
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
/** @} */

//------------------------------------------------------------------------
} // Value
} // Standalone
} // VSTGUI
