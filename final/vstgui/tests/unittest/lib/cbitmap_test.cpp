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

#include "../../../lib/cbitmap.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/platform/iplatformbitmap.h"
#include "../unittests.h"

namespace VSTGUI {

TESTCASE(CBitmapTest,

	TEST(scaleFactor,
		CPoint p (10, 10);
		auto b1 = owned (IPlatformBitmap::create (&p));
		CBitmap bitmap (b1);
		p (20, 20);
		auto b2 = owned (IPlatformBitmap::create (&p));
		b2->setScaleFactor (2.);
		EXPECT (bitmap.addBitmap (b2));
		
		p (21, 21);
		auto b3 = owned(IPlatformBitmap::create (&p));
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
);

} // VSTGUI
