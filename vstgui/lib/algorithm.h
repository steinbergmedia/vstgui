// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "optional.h"
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
} // VSTGUI
