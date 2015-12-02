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

namespace VSTGUI {

//-----------------------------------------------------------------------------
CString::CString (UTF8StringPtr string)
: utf8String (string)
{
	platformString = IPlatformString::createWithUTF8String (string);
}

//-----------------------------------------------------------------------------
CString::~CString ()
{
	if (platformString)
		platformString->forget ();
}

//-----------------------------------------------------------------------------
void CString::setUTF8String (UTF8StringPtr string)
{
	utf8String = string;
	if (platformString)
		platformString->setUTF8String (string);
}

//-----------------------------------------------------------------------------
UTF8String::UTF8String (UTF8StringPtr str)
{
	if (str)
		string = str;
}

//-----------------------------------------------------------------------------
UTF8String::UTF8String (const UTF8String& other)
{
	*this = other;
}

//-----------------------------------------------------------------------------
UTF8String& UTF8String::operator=(const UTF8String& other)
{
	string = other.string;
	platformString = other.platformString;
	return *this;
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
UTF8String::UTF8String (UTF8String&& other)
{
	*this = std::move (other);
}

//-----------------------------------------------------------------------------
UTF8String& UTF8String::operator=(UTF8String&& other)
{
	string = std::move (other.string);
	platformString = std::move (other.platformString);
	return *this;
}
#endif

//-----------------------------------------------------------------------------
void UTF8String::set (UTF8StringPtr str)
{
	if (str == 0 || string != str)
	{
		platformString = 0;
		string = str ? str : "";
	}
}

//-----------------------------------------------------------------------------
IPlatformString* UTF8String::getPlatformString () const
{
	if (platformString == 0)
		platformString = owned (IPlatformString::createWithUTF8String (get ()));
	return platformString;
}

namespace String {

//-----------------------------------------------------------------------------
UTF8StringBuffer newWithString (UTF8StringPtr string)
{
	if (string == 0)
		return 0;
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

} // namespace
