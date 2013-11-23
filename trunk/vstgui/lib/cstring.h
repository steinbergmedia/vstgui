//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "vstguibase.h"
#include <string>

namespace VSTGUI {

class IPlatformString;

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
	UTF8StringPtr utf8String;
	IPlatformString* platformString;
};

//---------------------------------------------------------------------------------------------------------------------
class UTF8CharacterIterator
{
public:
	UTF8CharacterIterator (const std::string& str) : str (str), currentPos (0) { begin (); }
	
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

	uint8_t* begin () { currentPos = ((uint8_t*)str.c_str ()); return currentPos;}
	uint8_t* end () { currentPos = ((uint8_t*)str.c_str ()) + str.size (); return currentPos; }

	const uint8_t* front () const { return (const uint8_t*)str.c_str (); }
	const uint8_t* back () const { return (const uint8_t*)str.c_str () + str.size (); }

	const uint8_t* operator++() { return next (); }
	const uint8_t* operator--() { return previous (); }
	bool operator==(uint8_t i) { if (currentPos) return *currentPos == i; return false; }
	operator uint8_t* () const { return (uint8_t*)currentPos; }

protected:
	const std::string& str;
	uint8_t* currentPos;
	
};

} // namespace

#endif // __cstring__
