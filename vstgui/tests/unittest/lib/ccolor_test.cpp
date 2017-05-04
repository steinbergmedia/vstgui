// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/ccolor.h"

namespace VSTGUI {

#if DEBUG
static constexpr auto advanceCount = 16;
#else
static constexpr auto advanceCount = 1;
#endif

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
		for (uint16_t red = 0; red <= 255; red+=advanceCount)
		{
			for (uint16_t green = 0; green <= 255; green+=advanceCount)
			{
				for (uint16_t blue = 0; blue <= 255; blue+=advanceCount)
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
		for (uint16_t red = 0; red <= 255; red+=advanceCount)
		{
			for (uint16_t green = 0; green <= 255; green+=advanceCount)
			{
				for (uint16_t blue = 0; blue <= 255; blue+=advanceCount)
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
