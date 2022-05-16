// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cstring.h"
#include "../unittests.h"

namespace VSTGUI {

static UTF8StringPtr asciiStr = "This is a simple ASCII String";
static UTF8StringPtr utf8Str =
    "\xe3\x82\xa4\xe3\x83\xb3\xe3\x82\xbf\xe3\x83\xbc\xe3\x83\x8d\xe3\x83\x83\xe3\x83\x88\xe3\x82\x92\xe3\x82\x82\xe3\x81\xa3\xe3\x81\xa8\xe5\xbf\xab\xe9\x81\xa9\xe3\x81\xab\xe3\x80\x82\0"; // u8"インターネットをもっと快適に。";

TEST_CASE (UTF8StringViewTest, CalculateASCIICharacterCount)
{
	UTF8StringView str (asciiStr);
	EXPECT (str.calculateCharacterCount () == 29);
}

TEST_CASE (UTF8StringViewTest, CalculateUTF8CharacterCount)
{
	UTF8StringView str (utf8Str);
	EXPECT (str.calculateCharacterCount () == 15);
	UTF8StringView str2 ("\xc3\x84\xe0\xa5\xb4\xf0\xaa\x80\x9a\0"); // u8"Äॴ𪀚"
	EXPECT (str2.calculateCharacterCount () == 3);
	EXPECT (str2.calculateByteCount () == 10);
}

TEST_CASE (UTF8StringViewTest, CalculateEmptyCharacterCount)
{
	UTF8StringView str (nullptr);
	EXPECT (str.calculateCharacterCount () == 0);
}

TEST_CASE (UTF8StringViewTest, calculateASCIIByteCount)
{
	UTF8StringView str (asciiStr);
	EXPECT (str.calculateByteCount () == 30);
}

TEST_CASE (UTF8StringViewTest, calculateUTF8ByteCount)
{
	UTF8StringView str (utf8Str);
	EXPECT (str.calculateByteCount () == 46);
}

TEST_CASE (UTF8StringViewTest, Contains)
{
	UTF8StringView str (asciiStr);
	EXPECT (str.contains ("simple") == true);
	EXPECT (str.contains ("not") == false);
}

TEST_CASE (UTF8StringViewTest, EndsWith)
{
	UTF8StringView str (asciiStr);
	EXPECT (str.endsWith ("String") == true);
	EXPECT (str.endsWith ("This") == false);
	EXPECT (str.endsWith ("This is a simple ASCII String which is longer") == false);
}

TEST_CASE (UTF8StringViewTest, DoubleConversion)
{
	UTF8StringView str ("32.56789");
	double value = str.toDouble ();
	EXPECT (value == 32.56789);
}

TEST_CASE (UTF8StringViewTest, FloatConversion)
{
	UTF8StringView str ("32.56789");
	float value = str.toFloat ();
	EXPECT (value == 32.56789f);
}

TEST_CASE (UTF8StringViewTest, Compare)
{
	std::string test ("This is a simple ASCII String");
	UTF8StringView str1 (test.data ());
	UTF8StringView str2 (asciiStr);
	EXPECT (str1 == str2);
	UTF8StringView str3 (utf8Str);
	EXPECT (str1 != str3);
}

TEST_CASE (UTF8StringViewTest, ContainsCheckCase)
{
	UTF8StringView str (asciiStr);
	EXPECT (str.contains ("simple"));
	EXPECT (str.contains ("Simple") == false);
	EXPECT (str.contains ("SiMpLe", true));
}

TEST_CASE (UTF8StringViewTest, StartsWith)
{
	UTF8StringView str (asciiStr);
	EXPECT (str.startsWith ("This"));
}

TEST_CASE (UTF8StringViewTest, ToDouble)
{
	UTF8StringView str ("5.1");
	EXPECT (str.toDouble () == 5.1);
}

TEST_CASE (UTF8StringViewTest, ToFloat)
{
	UTF8StringView str ("5.1");
	EXPECT (str.toFloat () == 5.1f);
}

TEST_CASE (UTF8StringViewTest, toInteger)
{
	UTF8StringView str ("5");
	EXPECT (str.toInteger () == 5);
}

TEST_CASE (UTF8StringViewTest, toNumber)
{
	UTF8StringView str ("300");
	auto res = str.toNumber<int32_t> ();
	EXPECT (res);
	EXPECT (*res == 300);
	auto res2 = str.toNumber<uint8_t> ();
	EXPECT(!res2);
}

} // VSTGUI
