// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "optional.h"
#include "platform/iplatformstring.h"
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cstddef>

namespace VSTGUI {

//-----------------------------------------------------------------------------
template<typename BaseIterator>
class UTF8CodePointIterator
{
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = char32_t;
	using difference_type = ptrdiff_t;
	using pointer = char32_t*;
	using reference = char32_t&;

	using CodePoint = value_type;

	UTF8CodePointIterator () = default;
	UTF8CodePointIterator (const UTF8CodePointIterator& o) noexcept : it (o.it) {}
	explicit UTF8CodePointIterator (const BaseIterator& iterator) noexcept : it (iterator) {}
	
	UTF8CodePointIterator& operator++ () noexcept;
	UTF8CodePointIterator& operator-- () noexcept;
	UTF8CodePointIterator operator++ (int) noexcept;
	UTF8CodePointIterator operator-- (int) noexcept;
	
	bool operator== (const UTF8CodePointIterator& other) const noexcept;
	bool operator!= (const UTF8CodePointIterator& other) const noexcept;
	
	CodePoint operator* () const noexcept;
	
	BaseIterator base () const noexcept { return it; }
private:
	BaseIterator it;
	
	static constexpr uint8_t kFirstBitMask = 128u; // 1000000
	static constexpr uint8_t kSecondBitMask = 64u; // 0100000
	static constexpr uint8_t kThirdBitMask = 32u; // 0010000
	static constexpr uint8_t kFourthBitMask = 16u; // 0001000
	static constexpr uint8_t kFifthBitMask = 8u; // 0000100
};

/**
 *  @brief holds an UTF8 encoded string and a platform representation of it
 *
 */
//-----------------------------------------------------------------------------
class UTF8String
{
public:
	using StringType = std::string;
	using SizeType = StringType::size_type;
	using CodePointIterator = UTF8CodePointIterator<StringType::const_iterator>;
	
	UTF8String (UTF8StringPtr str = nullptr);
	UTF8String (const UTF8String& other);
	explicit UTF8String (const StringType& str);
	UTF8String (UTF8String&& other) noexcept;
	UTF8String (StringType&& str) noexcept;

	UTF8String& operator= (const UTF8String& other);
	UTF8String& operator= (const StringType& other);
	UTF8String& operator= (UTF8String&& other) noexcept;
	UTF8String& operator= (StringType&& str) noexcept;
	UTF8String& operator= (UTF8StringPtr str) { assign (str); return *this; }

	SizeType length () const noexcept { return string.length (); }
	bool empty () const noexcept { return string.empty (); }

	void copy (UTF8StringBuffer dst, SizeType dstSize) const noexcept;

	CodePointIterator begin () const noexcept;
	CodePointIterator end () const noexcept;

	bool operator== (UTF8StringPtr str) const noexcept;
	bool operator!= (UTF8StringPtr str) const noexcept;
	bool operator== (const UTF8String& str) const noexcept;
	bool operator!= (const UTF8String& str) const noexcept;
	bool operator== (const StringType& str) const noexcept;
	bool operator!= (const StringType& str) const noexcept;

	UTF8String& operator+= (const UTF8String& other);
	UTF8String& operator+= (StringType::value_type ch);
	UTF8String& operator+= (const StringType::value_type* other);
	UTF8String operator+ (const UTF8String& other);
	UTF8String operator+ (StringType::value_type ch);
	UTF8String operator+ (const StringType::value_type* other);

	void assign (UTF8StringPtr str);
	void clear () noexcept;

	const UTF8StringPtr data () const noexcept { return string.data (); }
	operator const UTF8StringPtr () const noexcept { return data (); }
	const StringType& getString () const noexcept { return string; }
	IPlatformString* getPlatformString () const noexcept;

	explicit operator bool () const = delete;
//-----------------------------------------------------------------------------
private:
	StringType string;
	mutable SharedPointer<IPlatformString> platformString;
};

inline bool operator== (const UTF8String::StringType& lhs, const UTF8String& rhs) noexcept { return lhs == rhs.getString (); }
inline bool operator!= (const UTF8String::StringType& lhs, const UTF8String& rhs) noexcept { return lhs != rhs.getString (); }

inline UTF8String operator+ (UTF8StringPtr lhs, const UTF8String& rhs) { return UTF8String (lhs) += rhs; }

//-----------------------------------------------------------------------------
template<typename T>
inline UTF8String toString (const T& value)
{
	return UTF8String (std::to_string (value));
}

//-----------------------------------------------------------------------------
/** white-character test
 *	@param character UTF-32 character
 *	@return true if character is a white-character
 */
bool isSpace (char32_t character) noexcept;

//-----------------------------------------------------------------------------
struct TrimOptions
{
	using CharTestFunc = std::function<bool (char32_t)>;
	TrimOptions (CharTestFunc&& func = [] (char32_t c) { return isSpace (c); }) : test (std::move (func)) {}

	TrimOptions& left () { setBit (flags, Flags::kLeft, true); return *this; }
	TrimOptions& right () { setBit (flags, Flags::kRight, true); return *this; }
	
	bool trimLeft () const { return hasBit (flags, Flags::kLeft); }
	bool trimRight () const { return hasBit (flags, Flags::kRight); }

	bool operator() (char32_t c) const { return !test (c); }

private:
	enum Flags : uint8_t {
		kLeft = 1 << 0,
		kRight = 1 << 1
	};
	uint8_t flags {0};
	CharTestFunc test;
};

//-----------------------------------------------------------------------------
UTF8String trim (const UTF8String& str, TrimOptions options = TrimOptions ().left ().right ());

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
namespace String {
	VSTGUI_DEPRECATED(/** @deprecated Allocates a new UTF8StringBuffer with enough size for string and copy the string into it. Returns nullptr if string is a nullptr. */
	UTF8StringBuffer newWithString (UTF8StringPtr string);)
	VSTGUI_DEPRECATED(/** @deprecated Frees an UTF8StringBuffer. If buffer is a nullptr it does nothing. */
	void free (UTF8StringBuffer buffer);)
}
#endif

//-----------------------------------------------------------------------------
/** @brief a view on a null terminated UTF-8 String
	
	It does not copy the string.
	It's allowed to put null pointers into it.
	A null pointer is treaded different than an empty string as they are not equal and the byte
	count of a null pointer is zero while the empty string has a byte count of one.
*/
//-----------------------------------------------------------------------------
class UTF8StringView
{
public:
	UTF8StringView () : str (nullptr), byteCount (0) {}
	UTF8StringView (const UTF8StringPtr string) : str (string) {}
	UTF8StringView (const UTF8String& string) : str (string.data ()), byteCount (string.length () + 1) {}
	UTF8StringView (const std::string& string) : str (string.data ()), byteCount (string.size () + 1) {}

	UTF8StringView (const UTF8StringView& other) noexcept;
	UTF8StringView& operator= (const UTF8StringView& other) noexcept;
	UTF8StringView (UTF8StringView&& other) noexcept = default;
	UTF8StringView& operator= (UTF8StringView&& other) = default;

	/** calculates the bytes used by this string, including null-character */
	size_t calculateByteCount () const;

	/** calculates the number of UTF-8 characters in the string */
	size_t calculateCharacterCount () const;

	/** checks this string if it contains a subString */
	bool contains (const UTF8StringPtr subString, bool ignoreCase = false) const;

	/** checks this string if it starts with startString */
	bool startsWith (const UTF8StringView& startString) const;

	/** checks this string if it ends with endString */
	bool endsWith (const UTF8StringView& endString) const;

	/** converts the string to a double */
	double toDouble (uint32_t precision = 8) const;
	
	/** converts the string to a float */
	float toFloat (uint32_t precision = 8) const;
	
	/** converts the string to an integer */
	int64_t toInteger () const;

	template<typename T>
	Optional<T> toNumber () const;

	bool operator== (const UTF8StringPtr otherString) const;
	bool operator!= (const UTF8StringPtr otherString) const;
	bool operator== (UTF8StringView otherString) const;
	operator const UTF8StringPtr () const;
//-----------------------------------------------------------------------------
private:
	UTF8StringPtr str;
	mutable Optional<size_t> byteCount;
};

//-----------------------------------------------------------------------------
class UTF8CharacterIterator
{
public:
	UTF8CharacterIterator (const UTF8StringPtr utf8Str)
	: startPos ((uint8_t*)utf8Str)
	, currentPos (nullptr)
	, strLen (std::strlen (utf8Str))
	{
		begin ();
	}

	UTF8CharacterIterator (const UTF8StringPtr utf8Str, size_t strLen)
	: startPos ((uint8_t*)utf8Str)
	, currentPos (nullptr)
	, strLen (strLen)
	{
		begin ();
	}
	
	UTF8CharacterIterator (const std::string& stdStr)
	: startPos ((uint8_t*)stdStr.c_str ())
	, currentPos (nullptr)
	, strLen (stdStr.size ())
	{
		begin ();
	}
	
	uint8_t* next ()
	{
		if (currentPos)
		{
			if (currentPos == back ())
			{}
			else if (*currentPos <= 0x7F) // simple ASCII character
				currentPos++;
			else
			{
				uint8_t characterLength = getByteLength ();
				if (characterLength)
					currentPos += characterLength;
				else
					currentPos = end (); // error, not an allowed UTF-8 character at this position
			}
		}
		return currentPos;
	}
	
	uint8_t* previous ()
	{
		while (currentPos)
		{
			--currentPos;
			if (currentPos < front ())
			{
				currentPos = begin ();
				break;
			}
			else
			{
				if (*currentPos <= 0x7f || (*currentPos >= 0xC0 && *currentPos <= 0xFD))
					break;
			}
		}
		return currentPos;
	}
	
	uint8_t getByteLength () const
	{
		if (currentPos && currentPos != back ())
		{
			if (*currentPos <= 0x7F)
				return 1;
			else
			{
				if (*currentPos >= 0xC0 && *currentPos <= 0xFD)
				{
					if ((*currentPos & 0xF8) == 0xF8)
						return 5;
					else if ((*currentPos & 0xF0) == 0xF0)
						return 4;
					else if ((*currentPos & 0xE0) == 0xE0)
						return 3;
					else if ((*currentPos & 0xC0) == 0xC0)
						return 2;
				}
			}
		}
		return 0;
	}

	uint8_t* begin () { currentPos = startPos; return currentPos;}
	uint8_t* end () { currentPos = startPos + strLen; return currentPos; }

	const uint8_t* front () const { return startPos; }
	const uint8_t* back () const { return startPos + strLen; }

	const uint8_t* operator++() { return next (); }
	const uint8_t* operator--() { return previous (); }
	bool operator==(uint8_t i) { if (currentPos) return *currentPos == i; return false; }
	operator uint8_t* () const { return (uint8_t*)currentPos; }

protected:
	uint8_t* startPos;
	uint8_t* currentPos;
	size_t strLen;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline UTF8StringView::UTF8StringView (const UTF8StringView& other) noexcept
{
	*this = other;
}

//------------------------------------------------------------------------
inline UTF8StringView& UTF8StringView::operator= (const UTF8StringView& other) noexcept
{
	str = other.str;
	if (other.byteCount)
		byteCount = makeOptional (*other.byteCount);
	return *this;
}

//------------------------------------------------------------------------
inline size_t UTF8StringView::calculateCharacterCount () const
{
	size_t count = 0;
	if (str == nullptr)
		return count;
	
	UTF8CharacterIterator it (str);
	while (it != it.back ())
	{
		count++;
		++it;
	}
	return count;
}

//-----------------------------------------------------------------------------
inline size_t UTF8StringView::calculateByteCount () const
{
	if (!byteCount)
		byteCount = makeOptional<size_t> (str ? std::strlen (str) + 1 : 0);
	return *byteCount;
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::contains (const UTF8StringPtr subString, bool ignoreCase) const
{
	if (ignoreCase)
	{
		if (!str || !subString)
			return false;
		UTF8CharacterIterator subIt (subString);
		UTF8CharacterIterator it (str, calculateByteCount ());
		auto foundIt = std::search (
		    it.begin (), it.end (), subIt.begin (), subIt.end (),
		    [] (uint8_t c1, uint8_t c2) { return std::toupper (c1) == std::toupper (c2); });
		return foundIt != it.end ();
	}
	return (!str || !subString || std::strstr (str, subString) == nullptr) ? false : true;
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::startsWith (const UTF8StringView& startString) const
{
	if (!str || !startString.str)
		return false;
	size_t startStringLen = startString.calculateByteCount ();
	size_t thisLen = calculateByteCount ();
	if (startStringLen > thisLen)
		return false;
	return std::strncmp (str, startString.str, startStringLen - 1) == 0;
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::endsWith (const UTF8StringView& endString) const
{
	size_t endStringLen = endString.calculateByteCount ();
	size_t thisLen = calculateByteCount ();
	if (endStringLen > thisLen)
		return false;
	return endString == UTF8StringView (str + (thisLen - endStringLen));
}

//-----------------------------------------------------------------------------
inline double UTF8StringView::toDouble (uint32_t precision) const
{
	std::istringstream sstream (str);
	sstream.imbue (std::locale::classic ());
	sstream.precision (static_cast<std::streamsize> (precision));
	double result;
	sstream >> result;
	return result;
}

//-----------------------------------------------------------------------------
inline float UTF8StringView::toFloat (uint32_t precision) const
{
	return static_cast<float>(toDouble (precision));
}

//------------------------------------------------------------------------
inline int64_t UTF8StringView::toInteger () const
{
	if (auto number = toNumber<int64_t> ())
		return *number;
	return 0;
}

//------------------------------------------------------------------------
template<typename T>
inline Optional<T> UTF8StringView::toNumber () const
{
	static_assert (std::is_arithmetic<T>::value, "only arithmetic types allowed");
	std::istringstream sstream (str);
	sstream.imbue (std::locale::classic ());
	T number {};
	sstream >> number;
	if (!sstream.fail ())
		return makeOptional (number);
	return {};
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::operator== (const UTF8StringPtr otherString) const
{
	if (str == otherString) return true;
	return (str && otherString) ? (std::strcmp (str, otherString) == 0) : false;
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::operator!= (const UTF8StringPtr otherString) const
{
	return !(*this == otherString);
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::operator== (UTF8StringView otherString) const
{
	if (byteCount && otherString.byteCount && *byteCount != *otherString.byteCount)
		return false;
	return operator==(otherString.str);
}

//-----------------------------------------------------------------------------
inline UTF8StringView::operator const UTF8StringPtr () const
{
	return str;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline UTF8CodePointIterator<BaseIterator>& UTF8CodePointIterator<BaseIterator>::operator++ () noexcept
{
	auto firstByte = *it;
 
	difference_type offset = 1;
 
	if (firstByte & kFirstBitMask)
	{
		if (firstByte & kThirdBitMask)
		{
			if (firstByte & kFourthBitMask)
				offset = 4;
			else
				offset = 3;
		}
		else
		{
			offset = 2;
		}
	}
	it += offset;
	return *this;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline UTF8CodePointIterator<BaseIterator>& UTF8CodePointIterator<BaseIterator>::operator-- () noexcept
{
	--it;
 	if (*it & kFirstBitMask)
	{
		--it;
		if ((*it & kSecondBitMask) == 0)
		{
			--it;
			if ((*it & kSecondBitMask) == 0)
			{
				--it;
			}
		}
	}
	return *this;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline UTF8CodePointIterator<BaseIterator> UTF8CodePointIterator<BaseIterator>::operator++ (int) noexcept
{
	auto result = *this;
	++(*this);
	return result;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline UTF8CodePointIterator<BaseIterator> UTF8CodePointIterator<BaseIterator>::operator-- (int) noexcept
{
	auto result = *this;
	--(*this);
	return result;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline bool UTF8CodePointIterator<BaseIterator>::operator== (const UTF8CodePointIterator<BaseIterator>& other) const noexcept
{
	return it == other.it;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline bool UTF8CodePointIterator<BaseIterator>::operator!= (const UTF8CodePointIterator<BaseIterator>& other) const noexcept
{
	return it != other.it;
}

//-----------------------------------------------------------------------------
template<typename BaseIterator>
inline typename UTF8CodePointIterator<BaseIterator>::CodePoint UTF8CodePointIterator<BaseIterator>::operator* () const noexcept
{
	CodePoint codePoint = 0;
 
	auto firstByte = *it;
 
	if (firstByte & kFirstBitMask)
	{
		if (firstByte & kThirdBitMask)
		{
			if (firstByte & kFourthBitMask)
			{
				codePoint = static_cast<CodePoint> ((firstByte & 0x07) << 18);
				auto secondByte = *(it + 1);
				codePoint +=  static_cast<CodePoint> ((secondByte & 0x3f) << 12);
				auto thirdByte = *(it + 2);
				codePoint +=  static_cast<CodePoint> ((thirdByte & 0x3f) << 6);
				auto fourthByte = *(it + 3);
				codePoint += (fourthByte & 0x3f);
			}
			else
			{
				codePoint = static_cast<CodePoint> ((firstByte & 0x0f) << 12);
				auto secondByte = *(it + 1);
				codePoint += static_cast<CodePoint> ((secondByte & 0x3f) << 6);
				auto thirdByte = *(it + 2);
				codePoint +=  static_cast<CodePoint> ((thirdByte & 0x3f));
			}
		}
		else
		{
			codePoint = static_cast<CodePoint> ((firstByte & 0x1f) << 6);
			auto secondByte = *(it + 1);
			codePoint +=  (secondByte & 0x3f);
		}
	}
	else
	{
		codePoint = static_cast<CodePoint> (firstByte);
	}
	return codePoint;
}

} // VSTGUI
