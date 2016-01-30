//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cstring.h"
#include <string.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
UTF8String::UTF8String (UTF8StringPtr str)
{
	if (str)
		string = str;
}

//-----------------------------------------------------------------------------
UTF8String::UTF8String (const StringType& str)
: string (str)
{
}

//-----------------------------------------------------------------------------
UTF8String::UTF8String (StringType&& str) noexcept
{
	*this = std::move (str);
}

//-----------------------------------------------------------------------------
UTF8String& UTF8String::operator= (StringType&& str) noexcept
{
	string = std::move (str);
	platformString = nullptr;
	return *this;
}

//-----------------------------------------------------------------------------
UTF8String::UTF8String (const UTF8String& other)
{
	*this = other;
}

//-----------------------------------------------------------------------------
UTF8String::UTF8String (UTF8String&& other) noexcept
{
	*this = std::move (other);
}

//-----------------------------------------------------------------------------
UTF8String& UTF8String::operator=(UTF8String&& other) noexcept
{
	string = std::move (other.string);
	platformString = std::move (other.platformString);
	return *this;
}

//-----------------------------------------------------------------------------
UTF8String& UTF8String::operator=(const StringType& other)
{
	if (string != other)
	{
		string = other;
		platformString = nullptr;
	}
	return *this;
}

//-----------------------------------------------------------------------------
bool UTF8String::operator== (UTF8StringPtr str) const noexcept { return str ? string == str : false; }
bool UTF8String::operator!= (UTF8StringPtr str) const noexcept { return str ? string != str : true; }
bool UTF8String::operator== (const UTF8String& str) const noexcept { return string == str.getString (); }
bool UTF8String::operator!= (const UTF8String& str) const noexcept { return string != str.getString (); }
bool UTF8String::operator== (const StringType& str) const noexcept { return string == str; }
bool UTF8String::operator!= (const StringType& str) const noexcept { return string != str; }

//-----------------------------------------------------------------------------
UTF8String& UTF8String::operator+= (const UTF8String& other)
{
	if (!other.empty ())
	{
		string += other.getString ();
		platformString = nullptr;
	}
	return *this;
}

//-----------------------------------------------------------------------------
UTF8String UTF8String::operator+ (const UTF8String& other)
{
	return UTF8String (*this) += other;
}

//-----------------------------------------------------------------------------
void UTF8String::assign (UTF8StringPtr str)
{
	if (str == nullptr || string != str)
	{
		platformString = nullptr;
		string = str ? str : "";
	}
}

//-----------------------------------------------------------------------------
void UTF8String::clear () noexcept
{
	string.clear ();
	platformString = nullptr;
}

//-----------------------------------------------------------------------------
void UTF8String::copy (UTF8StringBuffer dst, SizeType dstSize) const noexcept
{
#if WINDOWS
	strcpy_s (dst, dstSize, string.data ());
#else
	strlcpy (dst, string.data (), dstSize);
#endif
}

//-----------------------------------------------------------------------------
IPlatformString* UTF8String::getPlatformString () const noexcept
{
	if (platformString == nullptr)
		platformString = owned (IPlatformString::createWithUTF8String (data ()));
	return platformString;
}

//-----------------------------------------------------------------------------
UTF8String::CodePointIterator UTF8String::begin () const noexcept
{
	return UTF8String::CodePointIterator (string.begin ());
}

//-----------------------------------------------------------------------------
UTF8String::CodePointIterator UTF8String::end () const noexcept
{
	return UTF8String::CodePointIterator (string.end ());
}

//-----------------------------------------------------------------------------
bool isspace (char32_t character) noexcept
{
	switch (character)
	{
		case 0x0009: // CHARACTER TABULATION
		case 0x000A: // LINE FEED
		case 0x000B: // LINE TABULATION
		case 0x000C: // FORM FEED
		case 0x000D: // CARRIAGE RETURN
		case 0x0020: // SPACE
		case 0x0085: // NEXT LINE (NEL)
		case 0x00A0: // NO-BREAK SPACE
		case 0x2000: // EN QUAD
		case 0x2001: // EM QUAD
		case 0x2002: // EN SPACE
		case 0x2003: // EM SPACE
		case 0x2004: // THREE-PER-EM SPACE
		case 0x2005: // FOUR-PER-EM SPACE
		case 0x2006: // SIX-PER-EM SPACE
		case 0x2007: // FIGURE SPACE
		case 0x2008: // PUNCTUATION SPACE
		case 0x2009: // THIN SPACE
		case 0x200A: // HAIR SPACE
		case 0x200B: // ZERO WIDTH SPACE
		case 0x202F: // NARROW NO-BREAK SPACE
		case 0x205F: // MEDIUM MATHEMATICAL SPACE
		case 0x3000: // IDEOGRAPHIC SPACE
		case 0xFEFF: // ZERO WIDTH NO-BREAK SPACE
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
UTF8String trim (const UTF8String& str, TrimOptions options)
{
	using Iterator = UTF8String::CodePointIterator;
	auto string = str.getString ();
	if (options.trimLeft ())
	{
		auto it = std::find_if (Iterator (string.begin ()), Iterator (string.end ()), options);
		string.erase (string.begin (), it.base ());
	}
	if (options.trimRight ())
	{
		auto pos = Iterator (string.end ());
		for (auto it = Iterator (string.end ()); it != Iterator (string.begin ()); --it)
		{
			if (options (*it))
				break;
			pos = it;
		}
		string.erase (pos.base (), string.end ());
	}
	return UTF8String (std::move (string));
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS

namespace String {

//-----------------------------------------------------------------------------
UTF8StringBuffer newWithString (UTF8StringPtr string)
{
	if (string == nullptr)
		return nullptr;
	UTF8StringBuffer buffer = (UTF8StringBuffer)std::malloc (UTF8StringView (string).calculateByteCount ());
	std::strcpy (buffer, string);
	return buffer;
}

//-----------------------------------------------------------------------------
void free (UTF8StringBuffer buffer)
{
	if (buffer)
		std::free (buffer);
}

} // StringCreate

#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

} // namespace
