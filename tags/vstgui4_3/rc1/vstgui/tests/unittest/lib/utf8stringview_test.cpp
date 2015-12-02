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

namespace VSTGUI {

TESTCASE(UTF8StringViewTest,
	static UTF8StringPtr asciiStr = "This is a simple ASCII String";
	static UTF8StringPtr utf8Str = u8"インターネットをもっと快適に。";

	TEST(calculateASCIICharacterCount,
		UTF8StringView str (asciiStr);
		EXPECT(str.calculateCharacterCount () == 29);
	);

	TEST(calculateUTF8CharacterCount,
		UTF8StringView str (utf8Str);
		EXPECT(str.calculateCharacterCount () == 15);
		UTF8StringView str2 (u8"Äॴ𪀚");
		EXPECT(str2.calculateCharacterCount () == 3);
		EXPECT(str2.calculateByteCount () == 10);
	);

	TEST(calculateEmptyCharacterCount,
		UTF8StringView str (nullptr);
		EXPECT(str.calculateCharacterCount () == 0);
	);

	TEST(calculateASCIIByteCount,
		UTF8StringView str (asciiStr);
		EXPECT(str.calculateByteCount () == 30);
	);

	TEST(calculateUTF8ByteCount,
		UTF8StringView str (utf8Str);
		EXPECT(str.calculateByteCount () == 46);
	);
	
	TEST(contains,
		UTF8StringView str (asciiStr);
		EXPECT(str.contains ("simple") == true);
		EXPECT(str.contains ("not") == false);
	);

	TEST(endsWith,
		UTF8StringView str (asciiStr);
		EXPECT(str.endsWith ("String") == true);
		EXPECT(str.endsWith ("This") == false);
		EXPECT(str.endsWith ("This is a simple ASCII String which is longer") == false);
	);
	
	TEST(doubleConversion,
		UTF8StringView str ("32.56789");
		double value = str.toDouble ();
		EXPECT(value == 32.56789);
	);

	TEST(floatConversion,
		UTF8StringView str ("32.56789");
		float value = str.toFloat ();
		EXPECT(value == 32.56789f);
	);

	TEST(compare,
		std::string test ("This is a simple ASCII String");
		UTF8StringView str1 (test.c_str ());
		UTF8StringView str2 (asciiStr);
		EXPECT(str1 == str2);
		UTF8StringView str3 (utf8Str);
		EXPECT(str1 != str3);
	);

);
}
