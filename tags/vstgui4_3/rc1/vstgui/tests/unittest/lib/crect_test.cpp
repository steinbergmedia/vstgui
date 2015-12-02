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

#include "../../../lib/crect.h"
#include "../unittests.h"

namespace VSTGUI {

TESTCASE(CRectTest,

	TEST(makeIntegral,
		CRect r (0.2, 0.5, 10.8, 12.1);
		r.makeIntegral ();
		EXPECT(r.left == 0.0)
		EXPECT(r.top == 1.0)
		EXPECT(r.right == 11.0)
		EXPECT(r.bottom == 12.0)
	);

	TEST(getCenter,
		CRect r (0, 0, 50, 50);
		CPoint p = r.getCenter ();
		EXPECT(p.x == 25 && p.y == 25)
	);

	TEST(centerInside,
		CRect r (0, 0, 500, 500);
		CRect r2 (0, 0, 10, 10);
		r2.centerInside (r);
		EXPECT(r2.left == 245 && r2.top == 245 && r2.getWidth () == 10 && r2.getHeight () == 10)
	);

	TEST(pointInside,
		CRect r (0, 0, 250, 250);
		CPoint p (50, 50);
		EXPECT(r.pointInside (p))
	);

	TEST(pointNotInside,
		CRect r (0, 0, 250, 250);
		CPoint p (250, 50);
		EXPECT(r.pointInside (p) == false)
	);

	TEST(rectOverlap,
		CRect r (50, 50, 100, 100);
		CRect r2 (90, 90, 120, 120);
		EXPECT(r.rectOverlap (r2))
	);

	TEST(rectNotOverlap,
		CRect r (50, 50, 100, 100);
		CRect r2 (100, 101, 120, 120);
		CRect r3 (101, 100, 120, 120);
		CRect r4 (0, 0, 40, 40);
		CRect r5 (51, 51, 100, 40);
		EXPECT(r.rectOverlap (r2) == false)
		EXPECT(r.rectOverlap (r3) == false)
		EXPECT(r.rectOverlap (r4) == false)
		EXPECT(r.rectOverlap (r5) == false)
	);

	TEST(bound,
		CRect r (50, 50, 100, 100);
		CRect r2 ( 90, 90, 200, 200);
		r.bound (r2);
		EXPECT(r.left == 90 && r.top == 90 && r.right == 100 && r.bottom == 100)
		r (0, 0, 80, 80);
		r.bound (r2);
		EXPECT(r.left == 90 && r.top == 90 && r.right == 90 && r.bottom == 90)
		r (0, 0, 280, 280);
		r.bound (r2);
		EXPECT(r.left == 90 && r.top == 90 && r.right == 200 && r.bottom == 200)
	);

	TEST(unite,
		CRect r (20, 20, 40, 40);
		CRect r2 (40, 40, 80, 80);
		r.unite (r2);
		EXPECT(r.left == 20 && r.top == 20 && r.right == 80 && r.bottom == 80)
		r (50, 50, 120, 120);
		r.unite (r2);
		EXPECT(r.left == 40 && r.top == 40 && r.right == 120 && r.bottom == 120)
	);

	TEST(setWidth,
		CRect r (0., 0., 0., 0.);
		EXPECT(r.getWidth () == 0.);
		r.setWidth (100.);
		EXPECT(r.getWidth () == 100.);
	);

	TEST(setWidth,
		CRect r (0., 0., 0., 0.);
		EXPECT(r.getHeight () == 0.);
		r.setHeight (100.);
		EXPECT(r.getHeight () == 100.);
	);

	TEST(offset,
		CRect r (0, 0, 10, 10);
		r.offset (CPoint (5, 5));
		EXPECT(r.left == 5 && r.top == 5 && r.right == 15 && r.bottom == 15);
	);
	TEST(offsetInverse,
		CRect r (10, 10, 20, 20);
		r.offsetInverse (CPoint (5, 5));
		EXPECT(r.left == 5 && r.top == 5 && r.right == 15 && r.bottom == 15);
	);

	TEST(extend,
		CRect r (5, 5, 10, 10);
		r.extend (CPoint (5, 5));
		EXPECT(r.left == 0 && r.top == 0 && r.right == 15 && r.bottom == 15);
	);

	TEST(assign4CoordOperator,
		CRect r;
		r (0, 0, 15, 15);
		EXPECT(r.left == 0 && r.top == 0 && r.right == 15 && r.bottom == 15);
		r (15, 15, 0, 0);
		EXPECT(r.left == 0 && r.top == 0 && r.right == 15 && r.bottom == 15);
	);

	TEST(originSizeConstructor,
		CRect r (CPoint (10, 10), CPoint (10, 10));
		EXPECT(r.left == 10 && r.top == 10 && r.right == 20 && r.bottom == 20);
	);
	
	TEST(inset,
		CRect r (0., 0., 100., 100.);
		r.inset (CPoint (5, 5));
		EXPECT(r.left == 5.);
		EXPECT(r.top == 5.);
		EXPECT(r.right == 95.);
		EXPECT(r.bottom == 95.);
	);

	TEST(normalize,
		CRect r (50, 50, 0, 0);
		r.normalize ();
		EXPECT(r.left == 0 && r.top == 0 && r.right == 50 && r.bottom == 50);
	);

	TEST(originize,
		CRect r (50, 50, 150, 150);
		r.originize ();
		EXPECT(r.left == 0 && r.top == 0 && r.right == 100 && r.bottom == 100);
	);

	TEST(moveTo,
		CRect r (0, 0, 20, 20);
		r.moveTo (CPoint (20, 20));
		EXPECT(r.left == 20 && r.top == 20 && r.right == 40 && r.bottom == 40);
	);
	
	TEST(corners,
		CRect r (10, 10, 20, 20);
		auto topLeft = r.getTopLeft ();
		EXPECT(topLeft.x == 10 && topLeft.y == 10);
		auto topRight = r.getTopRight ();
		EXPECT(topRight.x == 20 && topRight.y == 10);
		auto bottomLeft = r.getBottomLeft ();
		EXPECT(bottomLeft.x == 10 && bottomLeft.y == 20);
		auto bottomRight = r.getBottomRight ();
		EXPECT(bottomRight.x == 20 && bottomRight.y == 20);
	);

	TEST(setCorners,
		CRect r (0, 0, 0, 0);
		r.setTopLeft (CPoint (10, 10));
		EXPECT(r.left == 10 && r.top == 10);
		r.setTopRight (CPoint (10, 10));
		EXPECT(r.right == 10 && r.top == 10);
		r.setBottomLeft (CPoint (10, 10));
		EXPECT(r.left == 10 && r.bottom == 10);
		r.setBottomRight (CPoint (10, 10));
		EXPECT(r.right == 10 && r.bottom == 10);
	);

	TEST(getSize,
		CRect r (20, 20, 22, 22);
		auto s = r.getSize ();
		EXPECT(s.x == 2 && s.y == 2);
	);

	TEST(isEmpty,
		CRect r (1, 1, 1, 2);
		EXPECT(r.isEmpty ());
		r (1, 1, 2, 1);
		EXPECT(r.isEmpty ());
	);

	TEST(operatorEqual,
		CRect r1 (0, 1, 2, 3);
		CRect r2 (0, 1, 2, 3);
		EXPECT(r1 == r2);
	);

); // TESTCASE

} // namespace
