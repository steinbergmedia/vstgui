// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cbitmap.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/platform/iplatformbitmap.h"
#include "../../../lib/platform/platformfactory.h"
#include "../unittests.h"

namespace VSTGUI {

TESTCASE(CBitmapTest,

	TEST(scaleFactor,
		CPoint p (10, 10);
		auto b1 = getPlatformFactory ().createBitmap (p);
		CBitmap bitmap (b1);
		p (20, 20);
		auto b2 = getPlatformFactory ().createBitmap (p);
		b2->setScaleFactor (2.);
		EXPECT (bitmap.addBitmap (b2));
		
		p (21, 21);
		auto b3 = getPlatformFactory ().createBitmap (p);
		EXPECT_EXCEPTION (bitmap.addBitmap(b3), "wrong bitmap size");
		
		EXPECT (bitmap.getBestPlatformBitmapForScaleFactor (0.5) == b1);
		EXPECT (bitmap.getBestPlatformBitmapForScaleFactor (1.0) == b1);
		EXPECT (bitmap.getBestPlatformBitmapForScaleFactor (1.4) == b1);
		EXPECT (bitmap.getBestPlatformBitmapForScaleFactor (1.5) == b2);
		EXPECT (bitmap.getBestPlatformBitmapForScaleFactor (1.6) == b2);
		EXPECT (bitmap.getBestPlatformBitmapForScaleFactor (2.6) == b2);
	);

	TEST(pixelAccess,
		CBitmap bitmap (10, 10);
		EXPECT (bitmap.getWidth () == 10);
		EXPECT (bitmap.getHeight () == 10);

		auto accessor = owned (CBitmapPixelAccess::create (&bitmap));
		EXPECT(accessor);
		uint32_t x = 0;
		uint32_t y = 0;
		CColor color;
		do
		{
			accessor->getColor (color);
			EXPECT (color == CColor (0, 0, 0, 0));
			accessor->setColor (kRedCColor);
			accessor->getColor(color);
			EXPECT(color == kRedCColor);
			accessor->setColor (kGreenCColor);
			accessor->getColor(color);
			EXPECT(color == kGreenCColor);
			accessor->setColor (kBlueCColor);
			accessor->getColor(color);
			EXPECT(color == kBlueCColor);
			EXPECT (accessor->getX () == x++);
			EXPECT (accessor->getY () == y);
			if (x == accessor->getBitmapWidth ())
			{
				++y;
				x = 0;
			}
		} while (++(*accessor));
	);
	TEST(pixelAccess2,
		CBitmap bitmap (10, 10);
		CColor color (255, 1, 2, 150);
		if (auto accessor = owned (CBitmapPixelAccess::create(&bitmap)))
		{
			do
			{
				accessor->setColor (color);
			} while (++(*accessor));
		}
		if (auto accessor = owned (CBitmapPixelAccess::create(&bitmap)))
		{
			do
			{
				CColor c;
				accessor->getColor (c);
				EXPECT (c == color);
			} while (++(*accessor));
		}
	);
);

} // VSTGUI
