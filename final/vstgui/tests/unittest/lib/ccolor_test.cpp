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
#include "../../../lib/ccolor.h"

namespace VSTGUI {

TESTCASE(CColorTest,

	TEST(makeCColor,
		CColor c = MakeCColor (10, 20, 30, 40);
		EXPECT(c.red == 10);
		EXPECT(c.green == 20);
		EXPECT(c.blue == 30);
		EXPECT(c.alpha == 40);
	);
	
	TEST(luma,
		CColor c (30, 50, 80, 255);
		EXPECT(c.getLuma () == 47);
	);

	TEST(lightness,
		CColor c (80, 30, 50, 255);
		EXPECT(c.getLightness () == 55);
	);

	TEST(operatorEqual,
		CColor c (30, 40, 50, 60);
		EXPECT(c == CColor (30, 40, 50, 60));
	);

	TEST(operatorNotEqual,
		CColor c (30, 40, 50, 60);
		EXPECT(c != CColor (50, 40, 50, 60));
		EXPECT(c != CColor (30, 50, 50, 60));
		EXPECT(c != CColor (30, 40, 60, 60));
		EXPECT(c != CColor (30, 40, 50, 70));
	);

	TEST(copyConstructor,
		CColor c (60, 50, 40, 100);
		EXPECT(c == CColor (c));
	);

	TEST(assignOperator,
		CColor c;
		c (20, 30, 40, 50);
		EXPECT(c == CColor (20, 30, 40, 50));
	);

	TEST(hsv,
		double hue;
		double saturation;
		double value;
		for (uint16_t red = 0; red <= 255; red++)
		{
			for (uint16_t green = 0; green <= 255; green++)
			{
				for (uint16_t blue = 0; blue <= 255; blue++)
				{
					CColor c (static_cast<uint8_t> (red), static_cast<uint8_t> (green), static_cast<uint8_t> (blue));
					c.toHSV (hue, saturation, value);
					c.fromHSV (hue, saturation, value);
					EXPECT(c.red == red)
					EXPECT(c.green == green)
					EXPECT(c.blue == blue)
					EXPECT(c.alpha == 255);
				}
			}
		}
	);

	TEST(hsl,
		double hue;
		double saturation;
		double lightness;
		for (uint16_t red = 0; red <= 255; red++)
		{
			for (uint16_t green = 0; green <= 255; green++)
			{
				for (uint16_t blue = 0; blue <= 255; blue++)
				{
					CColor c (static_cast<uint8_t> (red), static_cast<uint8_t> (green), static_cast<uint8_t> (blue));
					c.toHSL (hue, saturation, lightness);
					c.fromHSL (hue, saturation, lightness);
					EXPECT(c.red == red)
					EXPECT(c.green == green)
					EXPECT(c.blue == blue)
					EXPECT(c.alpha == 255);
				}
			}
		}
	);
);

} // namespace
