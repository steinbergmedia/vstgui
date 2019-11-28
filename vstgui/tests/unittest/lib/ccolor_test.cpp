// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/cstring.h"

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
	
	TEST(isColorRepresentation,
		EXPECT(CColor::isColorRepresentation ("#FF00FFFF"));
	);

	TEST(toString,
		auto str = CColor (255, 255, 255).toString ();
		EXPECT(str == "#ffffffff")
	);

	TEST(fromString,
		CColor c;
		EXPECT(c.fromString ("#FFFFFFFF"));
		EXPECT(c == CColor (255, 255, 255));
		c.fromString ("#FF0000FF");
		EXPECT(c == CColor (255, 0, 0));
		c.fromString ("#00FF00FF");
		EXPECT(c == CColor (0, 255, 0));
		c.fromString ("#0000FFFF");
		EXPECT(c == CColor (0, 0, 255));
		c.fromString ("#0000FF00");
		EXPECT(c == CColor (0, 0, 255, 0));
	);
	
	TEST(setNormalized,
		CColor c (0, 0, 0, 0);
		c.setNormRed (1.);
		c.setNormGreen (1.);
		c.setNormBlue (1.);
		c.setNormAlpha (1.);
		EXPECT(c.red == 255);
		EXPECT(c.green == 255);
		EXPECT(c.blue == 255);
		EXPECT(c.alpha == 255);
		c.setNormRed (0.5);
		c.setNormGreen (0.5);
		c.setNormBlue (0.5);
		c.setNormAlpha (0.5);
		EXPECT(c.red == 128);
		EXPECT(c.green == 128);
		EXPECT(c.blue == 128);
		EXPECT(c.alpha == 128);
	);

	TEST(getNormalized,
		CColor r (kRedCColor);
		EXPECT(r.normRed<double> () == 1.);
		EXPECT(r.normGreen<double> () == 0.);
		EXPECT(r.normBlue<double> () == 0.);
		EXPECT(r.normAlpha<double> () == 1.);
		r.green = 127;
		EXPECT(r.normGreen<double> () == 127. / 255.);
		r.blue = 33;
		EXPECT(r.normBlue<double> () == 33. / 255.);
		r.alpha = 250;
		EXPECT(r.normAlpha<double> () == 250. / 255.);
	);

);

} // VSTGUI
