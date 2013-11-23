//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "../../../lib/cpoint.h"
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
		EXPECT(r.rectOverlap (r2) == false)
	);

	TEST(bound,
		CRect r (50, 50, 100, 100);
		CRect r2 ( 90, 90, 200, 200);
		r.bound (r2);
		EXPECT(r.left == 90 && r.top == 90 && r.right == 100 && r.bottom == 100)
		r = CRect (0, 0, 80, 80);
		r.bound (r2);
		EXPECT(r.left == 90 && r.top == 90 && r.right == 90 && r.bottom == 90)
	);

	TEST(unite,
		CRect r (20, 20, 40, 40);
		CRect r2 (40, 40, 80, 80);
		r.unite (r2);
		EXPECT(r.left == 20 && r.top == 20 && r.right == 80 && r.bottom == 80)
	);

); // TESTCASE

} // namespace
