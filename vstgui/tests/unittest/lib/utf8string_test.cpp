// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/cstring.h"

#if MAC
#include "../../../lib/platform/mac/macstring.h"
#endif

namespace VSTGUI {

TESTCASE(UTF8StringTest,

	TEST(empty,
		UTF8String str;
		EXPECT (str.empty ());
		EXPECT (str.length () == 0);
	);
	
	TEST(set,
		UTF8String str;
		EXPECT (str.empty ());
		str.assign ("Test");
		EXPECT(str.empty () == false);
		EXPECT(str.data () == std::string ("Test"));
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

	TEST(codePointIterator,
		 UTF8String str ("\xc3\x84\xe0\xa5\xb4\xf0\xaa\x80\x9a\0"); // u8"Äॴ𪀚"
		 auto charCount = 0;
		 for (auto it = str.begin (); it != str.end (); ++it)
		 {
			 charCount++;
		 }
		 EXPECT(charCount == 3);
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
