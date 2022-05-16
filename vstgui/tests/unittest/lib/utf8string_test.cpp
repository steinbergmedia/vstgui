// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cstring.h"
#include "../unittests.h"

#if MAC
#include "../../../lib/platform/mac/macstring.h"
#endif

namespace VSTGUI {

TEST_CASE (UTF8StringTest, Empty)
{
	UTF8String str;
	EXPECT (str.empty ());
	EXPECT (str.length () == 0);
}

TEST_CASE (UTF8StringTest, Set)
{
	UTF8String str;
	EXPECT (str.empty ());
	str.assign ("Test");
	EXPECT (str.empty () == false);
	EXPECT (str.data () == std::string ("Test"));
}

TEST_CASE (UTF8StringTest, EqualOperator)
{
	EXPECT (UTF8String ("bla") == "bla");
	EXPECT (UTF8String ("bla") != "uhh");
	std::string str ("bla");
	std::string str2 ("uhh");
	EXPECT (UTF8String ("bla") == str);
	EXPECT (UTF8String ("bla") != str2);
}

TEST_CASE (UTF8StringTest, SetOperator)
{
	UTF8String str;
	str = "Test";
	EXPECT (str == "Test");
}

TEST_CASE (UTF8StringTest, GetOperator)
{
	UTF8String str ("Test");
	UTF8StringPtr cstr = str;
	EXPECT (cstr == std::string ("Test"));
}

TEST_CASE (UTF8StringTest, CopyOperator)
{
	UTF8String str1 ("str");
	UTF8String str2 (str1);
	EXPECT (str1 == str2);
}

TEST_CASE (UTF8StringTest, MoveOperator)
{
	UTF8String str1 ("str");
	UTF8String str2 (std::move (str1));
	EXPECT (str1 != str2);
}

TEST_CASE (UTF8StringTest, AddOperator)
{
	UTF8String str1 ("str");
	str1 += "ing";
	EXPECT (str1 == "string");

	auto str2 = str1 + "1";
	EXPECT (str2 == "string1");
	EXPECT (str1 == "string");
}

TEST_CASE (UTF8StringTest, Clear)
{
	UTF8String str1 ("string");
	EXPECT (str1 == "string");
	str1.clear ();
	EXPECT (str1 == "");
}

TEST_CASE (UTF8StringTest, Copy)
{
	UTF8String str1 ("string");
	char buffer[3] {1};
	str1.copy (buffer, sizeof (buffer));
	EXPECT (buffer[0] == 's');
	EXPECT (buffer[1] == 't');
	EXPECT (buffer[2] == 0);
}

TEST_CASE (UTF8StringTest, CodePointIterator)
{
	UTF8String str ("\xc3\x84\xe0\xa5\xb4\xf0\xaa\x80\x9a\0"); // u8"Äॴ𪀚"
	auto charCount = 0;
	for (auto it = str.begin (); it != str.end (); ++it)
	{
		charCount++;
	}
	EXPECT (charCount == 3);
}

#if MAC

TEST_CASE (UTF8StringTest, MacPlatformString)
{
	UTF8String str1 ("Test");
	auto platformStr = str1.getPlatformString ();
	auto macStr = dynamic_cast<MacString*> (platformStr);
	EXPECT (macStr);
	auto cfStr = macStr->getCFString ();
	auto cfStr2 = CFStringCreateWithCString (kCFAllocatorDefault, "Test", kCFStringEncodingUTF8);
	EXPECT (CFStringCompare (cfStr, cfStr2, 0) == kCFCompareEqualTo);
	CFRelease (cfStr2);
}

#endif

} // VSTGUI
