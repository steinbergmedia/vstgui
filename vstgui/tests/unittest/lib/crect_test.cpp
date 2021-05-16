// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/crect.h"
#include "../unittests.h"

namespace VSTGUI {

TEST_CASE (CRectTest, MakeIntegral)
{
	CRect r (0.2, 0.5, 10.8, 12.1);
	r.makeIntegral ();
	EXPECT (r.left == 0.0)
	EXPECT (r.top == 0.0)
	EXPECT (r.right == 11.0)
	EXPECT (r.bottom == 13.0)
}

TEST_CASE (CRectTest, GetCenter)
{
	CRect r (0, 0, 50, 50);
	CPoint p = r.getCenter ();
	EXPECT (p.x == 25 && p.y == 25)
}

TEST_CASE (CRectTest, CenterInside)
{
	CRect r (0, 0, 500, 500);
	CRect r2 (0, 0, 10, 10);
	r2.centerInside (r);
	EXPECT (r2.left == 245 && r2.top == 245 && r2.getWidth () == 10 && r2.getHeight () == 10)
}

TEST_CASE (CRectTest, PointInside)
{
	CRect r (0, 0, 250, 250);
	CPoint p (50, 50);
	EXPECT (r.pointInside (p))
}

TEST_CASE (CRectTest, PointNotInside)
{
	CRect r (0, 0, 250, 250);
	CPoint p (250, 50);
	EXPECT (r.pointInside (p) == false)
}

TEST_CASE (CRectTest, RectOverlap)
{
	CRect r (50, 50, 100, 100);
	CRect r2 (90, 90, 120, 120);
	EXPECT (r.rectOverlap (r2))
}

TEST_CASE (CRectTest, RectNotOverlap)
{
	CRect r (50, 50, 100, 100);
	CRect r2 (100, 101, 120, 120);
	CRect r3 (101, 100, 120, 120);
	CRect r4 (0, 0, 40, 40);
	CRect r5 (51, 51, 100, 40);
	EXPECT (r.rectOverlap (r2) == false)
	EXPECT (r.rectOverlap (r3) == false)
	EXPECT (r.rectOverlap (r4) == false)
	EXPECT (r.rectOverlap (r5) == false)
}

TEST_CASE (CRectTest, Bound)
{
	CRect r (50, 50, 100, 100);
	CRect r2 (90, 90, 200, 200);
	r.bound (r2);
	EXPECT (r.left == 90 && r.top == 90 && r.right == 100 && r.bottom == 100)
	r (0, 0, 80, 80);
	r.bound (r2);
	EXPECT (r.left == 90 && r.top == 90 && r.right == 90 && r.bottom == 90)
	r (0, 0, 280, 280);
	r.bound (r2);
	EXPECT (r.left == 90 && r.top == 90 && r.right == 200 && r.bottom == 200)
}

TEST_CASE (CRectTest, Unite)
{
	CRect r (20, 20, 40, 40);
	CRect r2 (40, 40, 80, 80);
	r.unite (r2);
	EXPECT (r.left == 20 && r.top == 20 && r.right == 80 && r.bottom == 80)
	r (50, 50, 120, 120);
	r.unite (r2);
	EXPECT (r.left == 40 && r.top == 40 && r.right == 120 && r.bottom == 120)
}

TEST_CASE (CRectTest, SetWidth)
{
	CRect r (0., 0., 0., 0.);
	EXPECT (r.getWidth () == 0.);
	r.setWidth (100.);
	EXPECT (r.getWidth () == 100.);
}

TEST_CASE (CRectTest, setHeight)
{
	CRect r (0., 0., 0., 0.);
	EXPECT (r.getHeight () == 0.);
	r.setHeight (100.);
	EXPECT (r.getHeight () == 100.);
}

TEST_CASE (CRectTest, Offset)
{
	CRect r (0, 0, 10, 10);
	r.offset (CPoint (5, 5));
	EXPECT (r.left == 5 && r.top == 5 && r.right == 15 && r.bottom == 15);
}

TEST_CASE (CRectTest, OffsetInverse)
{
	CRect r (10, 10, 20, 20);
	r.offsetInverse (CPoint (5, 5));
	EXPECT (r.left == 5 && r.top == 5 && r.right == 15 && r.bottom == 15);
}

TEST_CASE (CRectTest, Extend)
{
	CRect r (5, 5, 10, 10);
	r.extend (CPoint (5, 5));
	EXPECT (r.left == 0 && r.top == 0 && r.right == 15 && r.bottom == 15);
}

TEST_CASE (CRectTest, Assign4CoordOperator)
{
	CRect r;
	r (0, 0, 15, 15);
	EXPECT (r.left == 0 && r.top == 0 && r.right == 15 && r.bottom == 15);
	r (15, 15, 0, 0);
	EXPECT (r.left == 0 && r.top == 0 && r.right == 15 && r.bottom == 15);
}

TEST_CASE (CRectTest, OriginSizeConstructor)
{
	CRect r (CPoint (10, 10), CPoint (10, 10));
	EXPECT (r.left == 10 && r.top == 10 && r.right == 20 && r.bottom == 20);
}

TEST_CASE (CRectTest, Inset)
{
	CRect r (0., 0., 100., 100.);
	r.inset (CPoint (5, 5));
	EXPECT (r.left == 5.);
	EXPECT (r.top == 5.);
	EXPECT (r.right == 95.);
	EXPECT (r.bottom == 95.);
}

TEST_CASE (CRectTest, Normalize)
{
	CRect r (50, 50, 0, 0);
	r.normalize ();
	EXPECT (r.left == 0 && r.top == 0 && r.right == 50 && r.bottom == 50);
}

TEST_CASE (CRectTest, Originize)
{
	CRect r (50, 50, 150, 150);
	r.originize ();
	EXPECT (r.left == 0 && r.top == 0 && r.right == 100 && r.bottom == 100);
}

TEST_CASE (CRectTest, MoveTo)
{
	CRect r (0, 0, 20, 20);
	r.moveTo (CPoint (20, 20));
	EXPECT (r.left == 20 && r.top == 20 && r.right == 40 && r.bottom == 40);
}

TEST_CASE (CRectTest, Corners)
{
	CRect r (10, 10, 20, 20);
	auto topLeft = r.getTopLeft ();
	EXPECT (topLeft.x == 10 && topLeft.y == 10);
	auto topRight = r.getTopRight ();
	EXPECT (topRight.x == 20 && topRight.y == 10);
	auto bottomLeft = r.getBottomLeft ();
	EXPECT (bottomLeft.x == 10 && bottomLeft.y == 20);
	auto bottomRight = r.getBottomRight ();
	EXPECT (bottomRight.x == 20 && bottomRight.y == 20);
}

TEST_CASE (CRectTest, SetCorners)
{
	CRect r (0, 0, 0, 0);
	r.setTopLeft (CPoint (10, 10));
	EXPECT (r.left == 10 && r.top == 10);
	r.setTopRight (CPoint (10, 10));
	EXPECT (r.right == 10 && r.top == 10);
	r.setBottomLeft (CPoint (10, 10));
	EXPECT (r.left == 10 && r.bottom == 10);
	r.setBottomRight (CPoint (10, 10));
	EXPECT (r.right == 10 && r.bottom == 10);
}

TEST_CASE (CRectTest, GetSize)
{
	CRect r (20, 20, 22, 22);
	auto s = r.getSize ();
	EXPECT (s.x == 2 && s.y == 2);
}

TEST_CASE (CRectTest, IsEmpty)
{
	CRect r (1, 1, 1, 2);
	EXPECT (r.isEmpty ());
	r (1, 1, 2, 1);
	EXPECT (r.isEmpty ());
}

TEST_CASE (CRectTest, OperatorEqual)
{
	CRect r1 (0, 1, 2, 3);
	CRect r2 (0, 1, 2, 3);
	EXPECT (r1 == r2);
}

} // VSTGUI
