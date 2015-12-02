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

#include "../unittests.h"
#include "../../../lib/cstring.h"

#if MAC
#include "../../../lib/platform/mac/macstring.h"
#endif

namespace VSTGUI {

TESTCASE(CStringTest,

	TEST(test,
		CString str ("Test");
		EXPECT(str.getPlatformString ());
		EXPECT(UTF8String ("Test") == str.getUTF8String ());
		str.setUTF8String ("Other");
		EXPECT(str.getPlatformString ());
		EXPECT(UTF8String ("Other") == str.getUTF8String ());
	);

);

TESTCASE(UTF8StringTest,

	TEST(empty,
		UTF8String str;
		EXPECT (str.empty ());
		EXPECT (str.getByteCount () == 0);
	);
	
	TEST(set,
		UTF8String str;
		EXPECT (str.empty ());
		str.set ("Test");
		EXPECT(str.empty () == false);
		EXPECT(str.get () == std::string ("Test"));
	);

	TEST(equalOperator,
		EXPECT(UTF8String ("bla") == "bla");
		EXPECT(UTF8String ("bla") != "uhh");
		std::string str ("bla");
		std::string str2 ("uhh");
		EXPECT(UTF8String ("bla") == str);
		EXPECT(UTF8String ("bla") != str2);
	);

	TEST(setOperator,
		UTF8String str;
		str = "Test";
		EXPECT(str == "Test");
	);

	TEST(getOperator,
		UTF8String str ("Test");
		UTF8StringPtr cstr = str;
		EXPECT(cstr == std::string ("Test"));
	);
	
	TEST(copyOperator,
		UTF8String str1 ("str");
		UTF8String str2 (str1);
		EXPECT(str1 == str2);
	);

	TEST(moveOperator,
		UTF8String str1 ("str");
		UTF8String str2 (std::move (str1));
		EXPECT(str1 != str2);
	);

);

#if MAC
TESTCASE(UTF8StringMacTest,

	TEST(platformString,
		UTF8String str1 ("Test");
		auto platformStr = str1.getPlatformString ();
		auto macStr = dynamic_cast<MacString*>(platformStr);
		EXPECT(macStr);
		auto cfStr = macStr->getCFString ();
		auto cfStr2 = CFStringCreateWithCString (kCFAllocatorDefault, "Test", kCFStringEncodingUTF8);
		EXPECT(CFStringCompare (cfStr, cfStr2, 0) == kCFCompareEqualTo);
		CFRelease (cfStr2);
	);

);
#endif


} // VSTGUI
