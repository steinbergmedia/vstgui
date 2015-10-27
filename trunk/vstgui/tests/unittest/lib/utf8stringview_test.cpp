#include "../unittests.h"
#include "../../../lib/cstring.h"

namespace VSTGUI {

TESTCASE(UTF8StringViewTest,
	static UTF8StringPtr asciiStr = "This is a simple ASCII String";
	static UTF8StringPtr utf8Str = u8"インターネットをもっと快適に。";

	TEST(calculateASCIICharacterCount,
		UTF8StringView str (asciiStr);
		EXPECT (str.calculateCharacterCount () == 29);
	);

	TEST(calculateUTF8CharacterCount,
		UTF8StringView str (utf8Str);
		EXPECT (str.calculateCharacterCount () == 15);
	);

	TEST(calculateASCIIByteCount,
		UTF8StringView str (asciiStr);
		EXPECT (str.calculateByteCount () == 30);
	);

	TEST(calculateUTF8ByteCount,
		UTF8StringView str (utf8Str);
		EXPECT (str.calculateByteCount () == 46);
	);
	
	TEST(contains,
		UTF8StringView str (asciiStr);
		EXPECT (str.contains ("simple") == true);
		EXPECT (str.contains ("not") == false);
	);

	TEST(endsWith,
		UTF8StringView str (asciiStr);
		EXPECT (str.endsWith ("String") == true);
		EXPECT (str.endsWith ("This") == false);
	);
	
	TEST(doubleConversion,
		UTF8StringView str ("32.56789");
		double value = str.toDouble ();
		EXPECT (value == 32.56789);
	);

	TEST(floatConversion,
		UTF8StringView str ("32.56789");
		float value = str.toFloat ();
		EXPECT (value == 32.56789f);
	);

	TEST(compare,
		std::string test ("This is a simple ASCII String");
		UTF8StringView str1 (test.c_str ());
		UTF8StringView str2 (asciiStr);
		EXPECT (str1 == str2);
	);

);
}
