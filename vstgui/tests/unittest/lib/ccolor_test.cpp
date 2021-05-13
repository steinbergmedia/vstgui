// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/ccolor.h"
#include "../../../lib/cstring.h"
#include "../unittests.h"

namespace VSTGUI {

TEST_CASE (CColorTest, MakeCColor)
{
	CColor c = MakeCColor (10, 20, 30, 40);
	EXPECT_EQ (c.red, 10);
	EXPECT_EQ (c.green, 20);
	EXPECT_EQ (c.blue, 30);
	EXPECT_EQ (c.alpha, 40);
}

TEST_CASE (CColorTest, Luma)
{
	CColor c (30, 50, 80, 255);
	EXPECT_EQ (c.getLuma (), 47);
}

TEST_CASE (CColorTest, Lightness)
{
	CColor c (80, 30, 50, 255);
	EXPECT_EQ (c.getLightness (), 55);
}

TEST_CASE (CColorTest, OperatorEqual)
{
	CColor c (30, 40, 50, 60);
	EXPECT_TRUE (c == CColor (30, 40, 50, 60));
}

TEST_CASE (CColorTest, OperatorNotEqual)
{
	CColor c (30, 40, 50, 60);
	EXPECT (c != CColor (50, 40, 50, 60));
	EXPECT (c != CColor (30, 50, 50, 60));
	EXPECT (c != CColor (30, 40, 60, 60));
	EXPECT (c != CColor (30, 40, 50, 70));
}

TEST_CASE (CColorTest, CopyConstructor)
{
	CColor c (60, 50, 40, 100);
	CColor c2 (c);
	EXPECT_EQ (c, c2);
}

TEST_CASE (CColorTest, AssignOperator)
{
	CColor c;
	c (20, 30, 40, 50);
	EXPECT_EQ (c, CColor (20, 30, 40, 50));
}

#if DEBUG
static constexpr auto advanceCount = 16;
#else
static constexpr auto advanceCount = 1;
#endif

TEST_CASE (CColorTest, HSV)
{
	double hue;
	double saturation;
	double value;
	for (uint16_t red = 0; red <= 255; red += advanceCount)
	{
		for (uint16_t green = 0; green <= 255; green += advanceCount)
		{
			for (uint16_t blue = 0; blue <= 255; blue += advanceCount)
			{
				CColor c (static_cast<uint8_t> (red), static_cast<uint8_t> (green),
				          static_cast<uint8_t> (blue));
				c.toHSV (hue, saturation, value);
				c.fromHSV (hue, saturation, value);
				EXPECT_EQ (c.red, red)
				EXPECT_EQ (c.green, green)
				EXPECT_EQ (c.blue, blue)
				EXPECT_EQ (c.alpha, 255);
			}
		}
	}
}

TEST_CASE (CColorTest, HSL)
{
	double hue;
	double saturation;
	double lightness;
	for (uint16_t red = 0; red <= 255; red += advanceCount)
	{
		for (uint16_t green = 0; green <= 255; green += advanceCount)
		{
			for (uint16_t blue = 0; blue <= 255; blue += advanceCount)
			{
				CColor c (static_cast<uint8_t> (red), static_cast<uint8_t> (green),
				          static_cast<uint8_t> (blue));
				c.toHSL (hue, saturation, lightness);
				c.fromHSL (hue, saturation, lightness);
				EXPECT_EQ (c.red, red)
				EXPECT_EQ (c.green, green)
				EXPECT_EQ (c.blue, blue)
				EXPECT_EQ (c.alpha, 255);
			}
		}
	}
}

TEST_CASE (CColorTest, IsColorRepresentation)
{
	EXPECT_TRUE (CColor::isColorRepresentation ("#FF00FFFF"));
	// EXPECT_FALSE (CColor::isColorRepresentation ("#ABCDEFGH"));
	EXPECT_FALSE (CColor::isColorRepresentation ("star"));
}

TEST_CASE (CColorTest, ToString)
{
	auto str = CColor (255, 255, 255).toString ();
	EXPECT_EQ (str, "#ffffffff")
}

TEST_CASE (CColorTest, FromString)
{
	CColor c;
	EXPECT_TRUE (c.fromString ("#FFFFFFFF"));
	EXPECT_EQ (c, CColor (255, 255, 255));
	c.fromString ("#FF0000FF");
	EXPECT_EQ (c, CColor (255, 0, 0));
	c.fromString ("#00FF00FF");
	EXPECT_EQ (c, CColor (0, 255, 0));
	c.fromString ("#0000FFFF");
	EXPECT_EQ (c, CColor (0, 0, 255));
	c.fromString ("#0000FF00");
	EXPECT_EQ (c, CColor (0, 0, 255, 0));
}

TEST_CASE (CColorTest, SetNormalized)
{
	CColor c (0, 0, 0, 0);
	c.setNormRed (1.);
	c.setNormGreen (1.);
	c.setNormBlue (1.);
	c.setNormAlpha (1.);
	EXPECT_EQ (c.red, 255);
	EXPECT_EQ (c.green, 255);
	EXPECT_EQ (c.blue, 255);
	EXPECT_EQ (c.alpha, 255);
	c.setNormRed (0.5);
	c.setNormGreen (0.5);
	c.setNormBlue (0.5);
	c.setNormAlpha (0.5);
	EXPECT_EQ (c.red, 128);
	EXPECT_EQ (c.green, 128);
	EXPECT_EQ (c.blue, 128);
	EXPECT_EQ (c.alpha, 128);
}

TEST_CASE (CColorTest, GetNormalized)
{
	CColor r (kRedCColor);
	EXPECT_EQ (r.normRed<double> (), 1.);
	EXPECT_EQ (r.normGreen<double> (), 0.);
	EXPECT_EQ (r.normBlue<double> (), 0.);
	EXPECT_EQ (r.normAlpha<double> (), 1.);
	r.green = 127;
	EXPECT_EQ (r.normGreen<double> (), 127. / 255.);
	r.blue = 33;
	EXPECT_EQ (r.normBlue<double> (), 33. / 255.);
	r.alpha = 250;
	EXPECT_EQ (r.normAlpha<double> (), 250. / 255.);
}

} // VSTGUI
