// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <initializer_list>
#include <type_traits>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** enum class bitset
 *
 *	example:
 *
 *		enum class Flag {
 *			One,
 *			Two,
 *			Three
 *		};
 *		using Flags = EnumBitset<Flag>;
 *		Flags f;
 *		f = Flag::One
 *		f |= Flag::Two;
 *		if (f & Flag::One) {}
 *
 */
template<typename Enum>
struct EnumBitset
{
	using value_type = std::underlying_type_t<Enum>;

	constexpr EnumBitset () {}
	constexpr EnumBitset (Enum initialValue) { exlusive (initialValue); }
	constexpr EnumBitset (const std::initializer_list<Enum>& list)
	{
		for (auto& v : list)
			add (v);
	}
	explicit constexpr EnumBitset (value_type v) : val (v) {}

	constexpr EnumBitset (const EnumBitset&) = default;
	constexpr EnumBitset& operator= (const EnumBitset&) = default;

	constexpr void exlusive (Enum e) { val = to_value_type (e); }
	constexpr void add (Enum e) { val |= to_value_type (e); }
	constexpr void remove (Enum e) { val &= ~to_value_type (e); }
	constexpr void clear () { val = {}; }

	constexpr bool test (Enum e) const { return (val & to_value_type (e)) != 0; }
	constexpr bool empty () const { return val == 0; }

	constexpr EnumBitset& operator= (Enum e)
	{
		exlusive (e);
		return *this;
	}
	constexpr EnumBitset& operator|= (Enum e)
	{
		add (e);
		return *this;
	}
	constexpr EnumBitset& operator^= (Enum e)
	{
		remove (e);
		return *this;
	}
	constexpr EnumBitset& operator&= (const EnumBitset& other)
	{
		val &= other.value;
		return *this;
	}
	constexpr EnumBitset& operator<< (Enum e)
	{
		add (e);
		return *this;
	}
	constexpr EnumBitset& operator<< (const EnumBitset& other)
	{
		val |= other.val;
		return *this;
	}
	constexpr EnumBitset& operator>> (Enum e)
	{
		remove (e);
		return *this;
	}
	constexpr EnumBitset& operator>> (const EnumBitset& other)
	{
		val &= ~other.val;
		return *this;
	}

	[[nodiscard]] constexpr EnumBitset operator| (Enum e) const
	{
		auto v = *this;
		v.add (e);
		return v;
	}
	constexpr bool operator& (Enum e) const { return test (e); }
	constexpr bool operator== (const EnumBitset& other) const { return val == other.val; }
	constexpr bool operator!= (const EnumBitset& other) const { return val != other.val; }

	constexpr value_type value () const { return val; }

private:
	constexpr value_type to_value_type (Enum e) const { return 1 << static_cast<value_type> (e); }

	value_type val {};
};

//------------------------------------------------------------------------
} // VSTGUI
