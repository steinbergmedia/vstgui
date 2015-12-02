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
#include "../../../uidescription/base64codec.h"
#include <string>

namespace VSTGUI {

TESTCASE(Base64CodecTest,

	TEST(encodeAscii,
		 std::string test ("ABCD");
		 Base64Codec bd;
		 EXPECT (bd.init (test.c_str (), 4) == true)
		 EXPECT (bd.getDataSize () == 8);
		 uint8_t* ptr = (uint8_t*)bd.getData ();
		 EXPECT (ptr[0] == 'Q');
		 EXPECT (ptr[1] == 'U');
		 EXPECT (ptr[2] == 'J');
		 EXPECT (ptr[3] == 'D');
		 EXPECT (ptr[4] == 'R');
		 EXPECT (ptr[5] == 'A');
		 EXPECT (ptr[6] == '=');
		 EXPECT (ptr[7] == '=');
	);

	TEST(encodeBinary,
		 unsigned char binary [6];
		 binary[0] = 0x89;
		 binary[1] = 0x50;
		 binary[2] = 0x4E;
		 binary[3] = 0x47;
		 binary[4] = 0x0D;
		 binary[5] = 0x0A;
		 Base64Codec bd;
		 EXPECT (bd.init (binary, 6) == true);
		 EXPECT (bd.getDataSize () == 8);
		 uint8_t* ptr = (uint8_t*)bd.getData ();
		 EXPECT (ptr[0] == 'i');
		 EXPECT (ptr[1] == 'V');
		 EXPECT (ptr[2] == 'B');
		 EXPECT (ptr[3] == 'O');
		 EXPECT (ptr[4] == 'R');
		 EXPECT (ptr[5] == 'w');
		 EXPECT (ptr[6] == '0');
		 EXPECT (ptr[7] == 'K');
	);

	TEST(decodeAscii,
		 std::string test ("QUJDRA");
		 Base64Codec bd;
		 EXPECT (bd.init (test) == true)
		 EXPECT (bd.getDataSize () == 4);
		 uint8_t* ptr = (uint8_t*)bd.getData ();
		 EXPECT (ptr[0] == 'A');
		 EXPECT (ptr[1] == 'B');
		 EXPECT (ptr[2] == 'C');
		 EXPECT (ptr[3] == 'D');
	);

	TEST(decodeBinary,
		 std::string test ("iVBORw0K");
		 Base64Codec bd;
		 EXPECT (bd.init (test) == true)
		 EXPECT (bd.getDataSize () == 6);
		 uint8_t* ptr = (uint8_t*)bd.getData ();
		 EXPECT (ptr[0] == 0x89);
		 EXPECT (ptr[1] == 0x50);
		 EXPECT (ptr[2] == 0x4E);
		 EXPECT (ptr[3] == 0x47);
		 EXPECT (ptr[4] == 0x0D);
		 EXPECT (ptr[5] == 0x0A);
	);
);

}
