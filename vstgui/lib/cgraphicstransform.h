// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cgraphicstransform__
#define __cgraphicstransform__

#include "cpoint.h"
#include "crect.h"

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288
#endif

namespace VSTGUI {

static inline double radians (double degrees) { return degrees * M_PI / 180; }

//-----------------------------------------------------------------------------
/// @brief Graphics Transform Matrix
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
struct CGraphicsTransform
{
	double m11 {1.};
	double m12 {0.};
	double m21 {0.};
	double m22 {1.};
	double dx {0.};
	double dy {0.};
	
	CGraphicsTransform () = default;
	CGraphicsTransform (double _m11, double _m12, double _m21, double _m22, double _dx, double _dy)
	: m11 (_m11), m12 (_m12), m21 (_m21), m22 (_m22), dx (_dx), dy (_dy)
	{}
	
	CGraphicsTransform& translate (double x, double y)
	{
		*this = CGraphicsTransform (1, 0, 0, 1, x, y) * this;
		return *this;
	}
	
	CGraphicsTransform& translate (const CPoint& p)
	{
		return translate (p.x, p.y);
	}
	
	CGraphicsTransform& scale (double x, double y)
	{
		*this = CGraphicsTransform (x, 0., 0., y, 0., 0.) * this;
		return *this;
	}

	CGraphicsTransform& scale (const CPoint& p)
	{
		return scale (p.x, p.y);
	}
	
	CGraphicsTransform& rotate (double angle)
	{
		angle = radians (angle);
		*this = CGraphicsTransform (cos (angle), -sin (angle), sin (angle), cos (angle), 0, 0) * this;
		return *this;
	}

	CGraphicsTransform& rotate (double angle, const CPoint& center)
	{
		return translate (-center.x, -center.y).rotate (angle).translate (center.x, center.y);
	}

	CGraphicsTransform& skewX (double angle)
	{
		*this = CGraphicsTransform (1, std::tan (radians (angle)), 0, 1, 0, 0) * *this;
		return *this;
	}

	CGraphicsTransform& skewY (double angle)
	{
		*this = CGraphicsTransform (1, 0, std::tan (radians (angle)), 1, 0, 0) * *this;
		return *this;
	}
	
	bool isInvariant () const
	{
		return *this == CGraphicsTransform ();
	}

	void transform (CCoord& x, CCoord& y) const
	{
		CCoord x2 = m11*x + m12*y + dx;
		CCoord y2 = m21*x + m22*y + dy;
		x = x2;
		y = y2;
	}
		
	void transform (CCoord& left, CCoord& right, CCoord& top, CCoord& bottom) const
	{
		transform (left, top);
		transform (right, bottom);
	}
	
	CPoint& transform (CPoint& p) const
	{
		transform (p.x, p.y);
		return p;
	}

	CRect& transform (CRect& r) const
	{
		transform (r.left, r.right, r.top, r.bottom);
		return r;
	}

	CGraphicsTransform inverse () const
	{
		CGraphicsTransform result;
		const double denominator = m11 * m22 - m12 * m21;
		if (denominator != 0)
		{
			result.m11 = m22 / denominator;
			result.m12 = -m12 / denominator;
			result.m21 = -m21 / denominator;
			result.m22 = m11 / denominator;
			result.dx = ((m12 * dy) - (m22 * dx)) / denominator;
			result.dy = ((m21 * dx) - (m11 * dy)) / denominator;
		}
		return result;
	}

	CGraphicsTransform operator* (const CGraphicsTransform& t) const
	{
		CGraphicsTransform result;
		result.m11 = (m11 * t.m11) + (m12 * t.m21);
		result.m21 = (m21 * t.m11) + (m22 * t.m21);
		result.dx = (m11 * t.dx) + (m12 * t.dy) + dx;
		result.m12 = (m11 * t.m12) + (m12 * t.m22);
		result.m22 = (m21 * t.m12) + (m22 * t.m22);
		result.dy = (m21 * t.dx) + (m22 * t.dy) + dy;
		return result;
	}
	
	CGraphicsTransform operator* (const CGraphicsTransform* t) const { return *this * *t; }

	bool operator== (const CGraphicsTransform& t) const
	{
		return m11 == t.m11 && m12 == t.m12 && m21 == t.m21 && m22 == t.m22 && dx == t.dx && dy == t.dy;
	}

	bool operator!= (const CGraphicsTransform& t) const
	{
		return m11 != t.m11 || m12 != t.m12 || m21 != t.m21 || m22 != t.m22 || dx != t.dx || dy != t.dy;
	}
};


}

#endif // __cgraphicstransform__
