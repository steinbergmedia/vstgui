#include "../unittests.h"
#include "../../../lib/cstring.h"

namespace VSTGUI {

TESTCASE(UTF8StringViewTest,
	static UTF8StringPtr asciiStr = "This is a simple ASCII String";
	static UTF8StringPtr utf8Str = u8"インターネットをもっと快適に。";

	TEST(CalculateASCIICharacterCountTest,
		UTF8StringView str (asciiStr);
		EXPECT (str.calculateCharacterCount () == 29);
	);

	TEST(CalculateUTF8CharacterCountTest,
		UTF8StringView str (utf8Str);
		EXPECT (str.calculateCharacterCount () == 15);
	);

	TEST(CalculateASCIIByteCountTest,
		UTF8StringView str (asciiStr);
		EXPECT (str.calculateByteCount () == 30);
	);

	TEST(CalculateUTF8ByteCountTest,
		UTF8StringView str (utf8Str);
		EXPECT (str.calculateByteCount () == 46);
	);
	
	TEST(ContainsTest,
		UTF8StringView str (asciiStr);
		EXPECT (str.contains ("simple") == true);
		EXPECT (str.contains ("not") == false);
	);

	TEST(EndsWithTest,
		UTF8StringView str (asciiStr);
		EXPECT (str.endsWith ("String") == true);
		EXPECT (str.endsWith ("This") == false);
	);

);
}
