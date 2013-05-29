//
//  ccolor_test.cpp
//  vstgui
//
//  Created by Arne Scheffler on 5/29/13.
//
//

#include "../unittests.h"
#include "../../../lib/ccolor.h"

namespace VSTGUI {

TESTCASE(CColorTest,

	TEST(hsvTest,
		double hue;
		double saturation;
		double value;
		for (uint16_t red = 0; red <= 255; red++)
		{
			for (uint16_t green = 0; green <= 255; green++)
			{
				for (uint16_t blue = 0; blue <= 255; blue++)
				{
					CColor c (red, green, blue);
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

	TEST(hslTest,
		double hue;
		double saturation;
		double lightness;
		for (uint16_t red = 0; red <= 255; red++)
		{
			for (uint16_t green = 0; green <= 255; green++)
			{
				for (uint16_t blue = 0; blue <= 255; blue++)
				{
					CColor c (red, green, blue);
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
