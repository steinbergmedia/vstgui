//
//  crect_test.cpp
//  vstgui
//
//  Created by Arne Scheffler on 4/6/13.
//
//

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
