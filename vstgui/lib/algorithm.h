// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "optional.h"
#include "vstguidebug.h"
#include <cstdint>
#include <algorithm>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** Returns the index of the value */
template <typename Iter, typename Type, typename ResultType = int32_t>
Optional<ResultType> indexOf (Iter first, Iter last, const Type& value)
{
	auto it = std::find (first, last, value);
	if (it == last)
		return {};
	return {static_cast<ResultType> (std::distance (first, it))};
}

//------------------------------------------------------------------------
/** Returns the index of the element for which predicate p returns true */
template <typename Iter, typename Proc, typename ResultType = int32_t>
Optional<ResultType> indexOfTest (Iter first, Iter last, Proc p)
{
	auto it = std::find_if (first, last, p);
	if (it == last)
		return {};
	return {static_cast<ResultType> (std::distance (first, it))};
}

//------------------------------------------------------------------------
/** Returns the value clamped to min and max */
template <typename T>
T clamp (T value, T min, T max)
{
	return std::min (max, std::max (value, min));
}

//------------------------------------------------------------------------
/** Returns the value clamped to zero and one */
template <typename T>
T clampNorm (T value)
{
	static_assert (std::is_floating_point<T>::value, "Only floating point types allowed");
	return clamp (value, static_cast<T> (0), static_cast<T> (1));
}

//------------------------------------------------------------------------
/** Returns the value projected lineary between stepOffset and stepOffset + steps */
template<typename NormT, typename StepT = int32_t>
StepT normalizedToSteps (NormT value, StepT numSteps, StepT stepStart = static_cast<StepT> (0))
{
	static_assert (std::is_integral<StepT>::value, "Step type must be integral");
	vstgui_assert (value >= 0. && value <= 1., "Only normalized values are allowed");
	return std::min<StepT> (numSteps, static_cast<StepT> ((numSteps + 1) * value)) + stepStart;
}

//------------------------------------------------------------------------
/** Returns the normalized value from the step value */
template<typename NormT, typename StepValueT, typename StepT>
NormT stepsToNormalized (StepValueT value, StepT steps, StepT stepOffset = static_cast<StepT> (0))
{
	static_assert (std::is_integral<StepT>::value, "Step type must be integral");
	vstgui_assert ((value - stepOffset) <= steps, "Value must be smaller or equal then steps");
	return static_cast<NormT> (value - stepOffset) / static_cast<NormT> (steps);
}

//------------------------------------------------------------------------
/** Returns the normalized value from a plain one */
template<typename NormT, typename PlainT>
NormT plainToNormalized (PlainT value, PlainT minValue, PlainT maxValue)
{
	static_assert (std::is_floating_point<NormT>::value,
				   "Only floating point types for NormT allowed");
	vstgui_assert (maxValue - minValue != 0., "min and max value must be different");
	return (value - minValue) / static_cast<NormT> (maxValue - minValue);
}

//------------------------------------------------------------------------
/** Returns the plain value from a normalized one */
template<typename PlainT, typename NormT>
PlainT normalizedToPlain (NormT value, PlainT minValue, PlainT maxValue)
{
	static_assert (std::is_floating_point<NormT>::value,
				   "Only floating point types for NormT allowed");
	vstgui_assert (maxValue - minValue != 0., "min and max value must be different");
	return static_cast<PlainT> ((maxValue - minValue) * value + minValue);
}

//------------------------------------------------------------------------
} // VSTGUI
