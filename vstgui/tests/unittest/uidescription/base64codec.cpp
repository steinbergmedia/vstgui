// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../uidescription/base64codec.h"
#include "../unittests.h"
#include <string>

namespace VSTGUI {

TEST_CASE (Base64CodecTest, EncodeAscii)
{
	std::string test ("ABCD");
	auto result = Base64Codec::encode (test.data (), test.size ());
	EXPECT (result.dataSize == 8);
	uint8_t* ptr = result.data.get ();
	EXPECT (ptr[0] == 'Q');
	EXPECT (ptr[1] == 'U');
	EXPECT (ptr[2] == 'J');
	EXPECT (ptr[3] == 'D');
	EXPECT (ptr[4] == 'R');
	EXPECT (ptr[5] == 'A');
	EXPECT (ptr[6] == '=');
	EXPECT (ptr[7] == '=');
}

TEST_CASE (Base64CodecTest, EncodeBinary)
{
	uint8_t binary[6];
	binary[0] = 0x89;
	binary[1] = 0x50;
	binary[2] = 0x4E;
	binary[3] = 0x47;
	binary[4] = 0x0D;
	binary[5] = 0x0A;
	auto result = Base64Codec::encode (binary, 6);
	EXPECT (result.dataSize == 8);
	uint8_t* ptr = result.data.get ();
	EXPECT (ptr[0] == 'i');
	EXPECT (ptr[1] == 'V');
	EXPECT (ptr[2] == 'B');
	EXPECT (ptr[3] == 'O');
	EXPECT (ptr[4] == 'R');
	EXPECT (ptr[5] == 'w');
	EXPECT (ptr[6] == '0');
	EXPECT (ptr[7] == 'K');
}

TEST_CASE (Base64CodecTest, DecodeAscii)
{
	std::string test ("QUJDRA");
	auto result = Base64Codec::decode (test);
	EXPECT (result.dataSize == 4);
	uint8_t* ptr = result.data.get ();
	EXPECT (ptr[0] == 'A');
	EXPECT (ptr[1] == 'B');
	EXPECT (ptr[2] == 'C');
	EXPECT (ptr[3] == 'D');
}

TEST_CASE (Base64CodecTest, DecodeBinary)
{
	std::string test ("iVBORw0K");
	auto result = Base64Codec::decode (test);
	EXPECT (result.dataSize == 6);
	uint8_t* ptr = result.data.get ();
	EXPECT (ptr[0] == 0x89);
	EXPECT (ptr[1] == 0x50);
	EXPECT (ptr[2] == 0x4E);
	EXPECT (ptr[3] == 0x47);
	EXPECT (ptr[4] == 0x0D);
	EXPECT (ptr[5] == 0x0A);
}

}
