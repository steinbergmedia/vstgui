// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __crect__
#define __crect__

#include "vstguibase.h"
#include "cpoint.h"
#include <cmath>

namespace VSTGUI {

//-----------------------------------------------------------------------------
//! @brief Rect structure
//-----------------------------------------------------------------------------
struct CRect
{
	CRect () = default;
	inline CRect (CCoord left, CCoord top, CCoord right, CCoord bottom);
	inline CRect (const CRect& r);
	inline CRect (const CPoint& origin, const CPoint& size);

	inline CRect& operator () (CCoord left, CCoord top, CCoord right, CCoord bottom);
	inline bool operator != (const CRect& other) const;
	inline bool operator == (const CRect& other) const;
	
	inline CCoord getWidth () const;
	inline CCoord getHeight () const;

	inline CRect& setWidth (CCoord width);
	inline CRect& setHeight (CCoord height);

	inline CPoint getTopLeft () const;
	inline CPoint getTopRight () const;
	inline CPoint getBottomLeft () const;
	inline CPoint getBottomRight () const;
	inline CRect& setTopLeft (const CPoint& inPoint);
	inline CRect& setTopRight (const CPoint& inPoint);
	inline CRect& setBottomLeft (const CPoint& inPoint);
	inline CRect& setBottomRight (const CPoint& inPoint);

	inline CPoint getCenter () const;

	inline CPoint getSize () const;
	inline CRect& setSize (const CPoint& size);

	inline CRect& offset (CCoord x, CCoord y);
	inline CRect& inset (CCoord deltaX, CCoord deltaY);
	inline CRect& extend (CCoord deltaX, CCoord deltaY);
	inline CRect& moveTo (CCoord x, CCoord y);

	inline CRect& inset (const CPoint& p);
	inline CRect& extend (const CPoint& p);
	inline CRect& moveTo (const CPoint& p);
	inline CRect& offset (const CPoint& p);
	inline CRect& offsetInverse (const CPoint& p);

	inline bool pointInside (const CPoint& where) const;	///< Checks if point is inside this rect
	inline bool isEmpty () const;
	inline bool rectOverlap (const CRect& rect) const;
	inline CRect& bound (const CRect& rect);
	inline CRect& unite (const CRect& rect);
	inline CRect& normalize ();
	inline CRect& originize ();
	inline CRect& centerInside (const CRect& r); ///< moves this rect to the center of r
	inline CRect& makeIntegral ();

	CCoord left {0.};
	CCoord top {0.};
	CCoord right {0.};
	CCoord bottom {0.};
};

//------------------------------------------------------------------------
inline CRect::CRect (CCoord left, CCoord top, CCoord right, CCoord bottom)
: left (left), top (top), right (right), bottom (bottom)
{}

//------------------------------------------------------------------------
inline CRect::CRect (const CRect& r)
: left (r.left), top (r.top), right (r.right), bottom (r.bottom)
{}

//------------------------------------------------------------------------
inline CRect::CRect (const CPoint& origin, const CPoint& size)
{
	setTopLeft (origin);
	setSize (size);
}

//------------------------------------------------------------------------
CRect& CRect::operator () (CCoord _left, CCoord _top, CCoord _right, CCoord _bottom)
{
	if (_left < _right)
	{
		left = _left;
		right = _right;
	}
	else
	{
		left = _right;
		right = _left;
	}
	if (_top < _bottom)
	{
		top = _top;
		bottom = _bottom;
	}
	else
	{
		top = _bottom;
		bottom = _top;
	}
	return *this;
}

//------------------------------------------------------------------------
bool CRect::operator != (const CRect& other) const
{
	return (left != other.left || right != other.right ||
			top != other.top || bottom != other.bottom);
}

//------------------------------------------------------------------------
bool CRect::operator == (const CRect& other) const
{
	return (left == other.left && right == other.right &&
			top == other.top && bottom == other.bottom);
}

//------------------------------------------------------------------------
inline CCoord CRect::getWidth () const
{
	return right - left;
}

//------------------------------------------------------------------------
inline CCoord CRect::getHeight () const
{
	return bottom - top;
}

//------------------------------------------------------------------------
inline CRect& CRect::setWidth (CCoord width)
{
	right = left + width;
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::setHeight (CCoord height)
{
	bottom = top + height;
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::offset (CCoord x, CCoord y)
{
	left += x;
	right += x;
	top += y;
	bottom += y;
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::inset (CCoord deltaX, CCoord deltaY)
{
	left += deltaX;
	right -= deltaX;
	top += deltaY;
	bottom -= deltaY;
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::extend (CCoord deltaX, CCoord deltaY)
{
	return inset (-deltaX, -deltaY);
}

//------------------------------------------------------------------------
inline CRect& CRect::moveTo (CCoord x, CCoord y)
{
	CCoord vDiff = y - top;
	CCoord hDiff = x - left;
	top += vDiff;
	bottom += vDiff;
	left += hDiff;
	right += hDiff;
	return *this;
}

//------------------------------------------------------------------------
inline bool CRect::isEmpty () const
{
	if (right <= left)
		return true;
	if (bottom <= top)
		return true;
	return false;
}

//------------------------------------------------------------------------
inline bool CRect::rectOverlap (const CRect& rect) const
{
	if (right < rect.left)
		return false;
	if (left > rect.right)
		return false;
	if (bottom < rect.top)
		return false;
	if (top > rect.bottom)
		return false;
	return true;
}

//------------------------------------------------------------------------
inline CRect& CRect::bound (const CRect& rect)
{
	if (left < rect.left)
		left = rect.left;
	if (top < rect.top)
		top = rect.top;
	if (right > rect.right)
		right = rect.right;
	if (bottom > rect.bottom)
		bottom = rect.bottom;
	if (bottom < top)
		bottom = top;
	if (right < left)
		right = left;
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::normalize ()
{
	if (left > right)
		std::swap (left, right);
	if (top > bottom)
		std::swap (top, bottom);
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::originize ()
{
	return offset (-left, -top);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::unite (const CRect& rect)
{
	if (left > rect.left)
		left = rect.left;
	if (right < rect.right)
		right = rect.right;
	if (top > rect.top)
		top = rect.top;
	if (bottom < rect.bottom)
		bottom = rect.bottom;
	return *this;
}

//------------------------------------------------------------------------
inline CRect& CRect::makeIntegral ()
{
	left = std::floor (left + 0.5);
	right = std::floor (right + 0.5);
	top = std::floor (top + 0.5);
	bottom = std::floor (bottom + 0.5);
	return *this;
}

//-----------------------------------------------------------------------------
inline bool CRect::pointInside (const CPoint& where) const
{
	return where.x >= left && where.x < right && where.y >= top && where.y < bottom;
}

//-----------------------------------------------------------------------------
inline CPoint CRect::getTopLeft () const
{
	CPoint myPoint (left, top);
	return myPoint;
}

//-----------------------------------------------------------------------------
inline CPoint CRect::getTopRight () const
{
	CPoint myPoint (right, top);
	return myPoint;
}

//-----------------------------------------------------------------------------
inline CPoint CRect::getBottomLeft () const
{
	CPoint myPoint (left, bottom);
	return myPoint;
}

//-----------------------------------------------------------------------------
inline CPoint CRect::getBottomRight () const
{
	CPoint myPoint (right, bottom);
	return myPoint;
}

//-----------------------------------------------------------------------------
inline CRect& CRect::setTopLeft (const CPoint& inPoint)
{
	left = inPoint.x;
	top = inPoint.y;
	return *this;
}

//-----------------------------------------------------------------------------
inline CRect& CRect::setTopRight (const CPoint& inPoint)
{
	right = inPoint.x;
	top = inPoint.y;
	return *this;
}

//-----------------------------------------------------------------------------
inline CRect& CRect::setBottomLeft (const CPoint& inPoint)
{
	left = inPoint.x;
	bottom = inPoint.y;
	return *this;
}

//-----------------------------------------------------------------------------
inline CRect& CRect::setBottomRight (const CPoint& inPoint)
{
	right = inPoint.x;
	bottom = inPoint.y;
	return *this;
}

//-----------------------------------------------------------------------------
inline CPoint CRect::getCenter () const
{
	CPoint myPoint (left + getWidth () / 2., top + getHeight () / 2.);
	return myPoint;
}

//-----------------------------------------------------------------------------
inline CPoint CRect::getSize () const
{
	CPoint myPoint (getWidth (), getHeight ());
	return myPoint;
}

//-----------------------------------------------------------------------------
inline CRect& CRect::setSize (const CPoint& size)
{
	setWidth (size.x);
	return setHeight (size.y);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::centerInside (const CRect& r)
{
	CPoint cp = r.getCenter ();
	CPoint cp2 = getCenter ();
	return offset (cp.x - cp2.x, cp.y - cp2.y);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::offset (const CPoint& p)
{
	return offset (p.x, p.y);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::inset (const CPoint& p)
{
	return inset (p.x, p.y);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::extend (const CPoint& p)
{
	return extend (p.x, p.y);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::moveTo (const CPoint& p)
{
	return moveTo (p.x, p.y);
}

//-----------------------------------------------------------------------------
inline CRect& CRect::offsetInverse (const CPoint& p)
{
	return offset (-p.x, -p.y);
}

} // namespace

#endif
