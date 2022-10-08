// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cbitmap.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/platform/iplatformbitmap.h"
#include "../../../lib/platform/platformfactory.h"
#include "../unittests.h"

namespace VSTGUI {

//------------------------------------------------------------------------
TEST_CASE (CBitmap, ScaleFactor)
{
	CPoint p (10, 10);
	auto b1 = getPlatformFactory ().createBitmap (p);
	CBitmap bitmap (b1);
	p (20, 20);
	auto b2 = getPlatformFactory ().createBitmap (p);
	b2->setScaleFactor (2.);
	EXPECT_TRUE (bitmap.addBitmap (b2));

	p (21, 21);
	auto b3 = getPlatformFactory ().createBitmap (p);
	EXPECT_EXCEPTION (bitmap.addBitmap (b3), "wrong bitmap size");

	EXPECT_EQ (bitmap.getBestPlatformBitmapForScaleFactor (0.5), b1);
	EXPECT_EQ (bitmap.getBestPlatformBitmapForScaleFactor (1.0), b1);
	EXPECT_EQ (bitmap.getBestPlatformBitmapForScaleFactor (1.4), b1);
	EXPECT_EQ (bitmap.getBestPlatformBitmapForScaleFactor (1.5), b2);
	EXPECT_EQ (bitmap.getBestPlatformBitmapForScaleFactor (1.6), b2);
	EXPECT_EQ (bitmap.getBestPlatformBitmapForScaleFactor (2.6), b2);
}

//------------------------------------------------------------------------
TEST_CASE (CBitmap, PixelAccess)
{
	CBitmap bitmap (10, 10);
	EXPECT_EQ (bitmap.getWidth (), 10);
	EXPECT_EQ (bitmap.getHeight (), 10);

	auto accessor = owned (CBitmapPixelAccess::create (&bitmap));
	EXPECT (accessor);
	uint32_t x = 0;
	uint32_t y = 0;
	CColor color;
	do
	{
		accessor->getColor (color);
		EXPECT_EQ (color, CColor (0, 0, 0, 0));
		accessor->setColor (kRedCColor);
		accessor->getColor (color);
		EXPECT_EQ (color, kRedCColor);
		accessor->setColor (kGreenCColor);
		accessor->getColor (color);
		EXPECT_EQ (color, kGreenCColor);
		accessor->setColor (kBlueCColor);
		accessor->getColor (color);
		EXPECT_EQ (color, kBlueCColor);
		EXPECT_EQ (accessor->getX (), x);
		EXPECT_EQ (accessor->getY (), y);
		if (++x == accessor->getBitmapWidth ())
		{
			++y;
			x = 0;
		}
	} while (++(*accessor));
}

//------------------------------------------------------------------------
TEST_CASE (CBitmap, PixelAccess2)
{
	CBitmap bitmap (10, 10);
	CColor color (255, 1, 2, 150);
	if (auto accessor = owned (CBitmapPixelAccess::create (&bitmap)))
	{
		do
		{
			accessor->setColor (color);
		} while (++(*accessor));
	}
	if (auto accessor = owned (CBitmapPixelAccess::create (&bitmap)))
	{
		do
		{
			CColor c;
			accessor->getColor (c);
			EXPECT_EQ (c, color);
		} while (++(*accessor));
	}
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
TEST_CASE (CMultiFrameBitmap, NotInitialized)
{
	CMultiFrameBitmap bitmap (100, 100);
	auto fr = bitmap.calcFrameRect (0);
	EXPECT_EQ (fr, CRect (0, 0, 100, 100));
}

//------------------------------------------------------------------------
TEST_CASE (CMultiFrameBitmap, CalcFrameRect)
{
	CMultiFrameBitmap bitmap (100, 100);
	EXPECT_TRUE (bitmap.setMultiFrameDesc ({{50, 50}, 4, 2}));
	EXPECT_EQ (bitmap.getFrameSize (), CPoint (50., 50.));
	EXPECT_EQ (bitmap.getNumFrames (), 4);
	EXPECT_EQ (bitmap.getNumFramesPerRow (), 2);
	EXPECT_EQ (bitmap.calcFrameRect (0), CRect (0, 0, 50, 50));
	EXPECT_EQ (bitmap.calcFrameRect (1), CRect (50, 0, 100, 50));
	EXPECT_EQ (bitmap.calcFrameRect (2), CRect (0, 50, 50, 100));
	EXPECT_EQ (bitmap.calcFrameRect (3), CRect (50, 50, 100, 100));
	EXPECT_EQ (bitmap.calcFrameRect (4), CRect (50, 50, 100, 100));
}

//------------------------------------------------------------------------
TEST_CASE (CMultiFrameBitmap, InvalidFrameDesc)
{
	CMultiFrameBitmap bitmap (100, 100);
	EXPECT_FALSE (bitmap.setMultiFrameDesc ({{50, 50}, 4, 1}));
	EXPECT_FALSE (bitmap.setMultiFrameDesc ({{50, 50}, 4, 4}));
}

} // VSTGUI
