// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cpoint__
#define __cpoint__

#include "vstguibase.h"
#include <cmath>

namespace VSTGUI {
struct CRect;

//-----------------------------------------------------------------------------
//! @brief Point structure
//-----------------------------------------------------------------------------
struct CPoint
{
	CPoint () = default;
	CPoint (CCoord x, CCoord y) : x (x), y (y) {}
	CPoint& operator () (CCoord _x, CCoord _y) { x = _x; y = _y; return *this; }

	bool operator!= (const CPoint &other) const { return (x != other.x || y != other.y); }
	bool operator== (const CPoint &other) const { return (x == other.x && y == other.y); }

	CPoint& operator+= (const CPoint& other) { x += other.x; y += other.y; return *this; }
	CPoint& operator-= (const CPoint& other) { x -= other.x; y -= other.y; return *this; }
	CPoint operator+ (const CPoint& other) const { return CPoint (x + other.x, y + other.y); }
	CPoint operator- (const CPoint& other) const { return CPoint (x - other.x, y - other.y); }
	CPoint operator- () const { return CPoint (-x, -y); }
	
	CPoint& offset (const CCoord _x, const CCoord _y) { *this += CPoint (_x, _y); return *this; }
	CPoint& offset (const CPoint& other) { *this += other; return *this; }
	CPoint& offsetInverse (const CPoint& other) { *this -= other; return *this; }

	inline CPoint& makeIntegral ();

	CCoord x {0.};
	CCoord y {0.};
};

//-----------------------------------------------------------------------------
inline CPoint& CPoint::makeIntegral ()
{
	x = std::floor (x + 0.5);
	y = std::floor (y + 0.5);
	return *this;
}


} // namespace

#endif
