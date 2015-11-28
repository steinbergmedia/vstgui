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

#ifndef __cstring__
#define __cstring__

#include "vstguifwd.h"
#include "platform/iplatformstring.h"
#include <string>
#include <sstream>

namespace VSTGUI {

//-----------------------------------------------------------------------------
/** @brief a string holder class

	It's main propose is to hold a platform dependent string represenation when the string is used more than once.
	You should currently don't use this, it's used internally.
*/
//-----------------------------------------------------------------------------
class CString : public CBaseObject
{
public:
	CString (UTF8StringPtr string = 0);
	~CString ();

	void setUTF8String (UTF8StringPtr string);
	UTF8StringPtr getUTF8String () const { return utf8String; }

	IPlatformString* getPlatformString () const { return platformString; }

//-----------------------------------------------------------------------------
protected:
	CString (const CString&) {}
	UTF8StringPtr utf8String;
	IPlatformString* platformString;
};

/**
 *  @brief holds an UTF8 encoded string and a platform representation of it
 *
 *	You should currently don't use this, it's used internally.
 *
 */
//-----------------------------------------------------------------------------
class UTF8String
{
public:
	UTF8String (UTF8StringPtr str = 0);
	UTF8String (const UTF8String& other);
	UTF8String& operator=(const UTF8String& other);

	void set (UTF8StringPtr str);
	UTF8StringPtr get () const { return string.c_str (); }
	size_t getByteCount () const { return string.length (); }
	bool empty () const { return string.empty (); }

	bool operator== (UTF8StringPtr str) const { return string == str; }
	bool operator!= (UTF8StringPtr str) const { return string != str; }
	bool operator== (const std::string& str) const { return string == str; }
	bool operator!= (const std::string& str) const { return string != str; }

	void operator= (UTF8StringPtr str) { set (str); }
	operator UTF8StringPtr () const { return get (); }

	IPlatformString* getPlatformString () const;

#if VSTGUI_RVALUE_REF_SUPPORT
	UTF8String (UTF8String&& other);
	UTF8String& operator=(UTF8String&& other);
#endif
//-----------------------------------------------------------------------------
private:
	std::string string;
	mutable SharedPointer<IPlatformString> platformString;
};

//-----------------------------------------------------------------------------
namespace String {
	/** Allocates a new UTF8StringBuffer with enough size for string and copy the string into it. Returns nullptr if string is a nullptr. */
	UTF8StringBuffer newWithString (UTF8StringPtr string);
	/** Frees an UTF8StringBuffer. If buffer is a nullptr it does nothing. */
	void free (UTF8StringBuffer buffer);
}

//-----------------------------------------------------------------------------
/** @brief a view on an UTF-8 String
	
	It does not copy the string.
	It's allowed to put null pointers into it.
	A null pointer is treaded different than an empty string.
*/
//-----------------------------------------------------------------------------
class UTF8StringView
{
public:
	UTF8StringView (const UTF8StringPtr string) : str (string) {}
	
	/** calculates the bytes used by this string, including null-character */
	size_t calculateByteCount () const;

	/** calculates the number of UTF-8 characters in the string */
	size_t calculateCharacterCount () const;

	/** checks this string if it contains a subString */
	bool contains (const UTF8StringPtr subString) const;

	/** checks this string if it ends with endString */
	bool endsWith (const UTF8StringView& endString) const;

	/** converts the string to a double */
	double toDouble (uint32_t precision = 8) const;
	
	/** converts the string to a float */
	float toFloat (uint32_t precision = 8) const;
	
	bool operator== (const UTF8StringPtr otherString) const;
	bool operator!= (const UTF8StringPtr otherString) const;
	operator const UTF8StringPtr () const;
//-----------------------------------------------------------------------------
private:
	UTF8StringView () : str (0) {}
	const UTF8StringPtr str;
};

//-----------------------------------------------------------------------------
class UTF8CharacterIterator
{
public:
	UTF8CharacterIterator (const UTF8StringPtr utf8Str)
	: startPos ((uint8_t*)utf8Str)
	, currentPos (0)
	, strLen (std::strlen (utf8Str))
	{
		begin ();
	}

	UTF8CharacterIterator (const std::string& stdStr)
	: startPos ((uint8_t*)stdStr.c_str ())
	, currentPos (0)
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
	uint8_t* currentPos;
	uint8_t* startPos;
	size_t strLen;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline size_t UTF8StringView::calculateCharacterCount () const
{
	size_t count = 0;
	if (str == 0)
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
	return str ? std::strlen (str) + 1 : 0;
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::contains (const UTF8StringPtr subString) const
{
	return (!str || !subString || std::strstr (str, subString) == 0) ? false : true;
}

//-----------------------------------------------------------------------------
inline bool UTF8StringView::endsWith (const UTF8StringView& endString) const
{
	uint64_t endStringLen = endString.calculateByteCount ();
	uint64_t thisLen = calculateByteCount ();
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
inline UTF8StringView::operator const UTF8StringPtr () const
{
	return str;
}

} // namespace

#endif // __cstring__
