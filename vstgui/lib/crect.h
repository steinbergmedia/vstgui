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

#ifndef __crect__
#define __crect__

#include "vstguibase.h"

namespace VSTGUI {

struct CPoint;

//-----------------------------------------------------------------------------
//! @brief Rect structure
//-----------------------------------------------------------------------------
struct CRect
{
	CRect (CCoord left = 0, CCoord top = 0, CCoord right = 0, CCoord bottom = 0)
	: left (left), top (top), right (right), bottom (bottom) {}

	CRect (const CRect& r)
	: left (r.left), top (r.top), right (r.right), bottom (r.bottom) {}

	CRect (const CPoint& origin, const CPoint& size) { setTopLeft (origin); setSize (size); }

	CRect& operator () (CCoord left, CCoord top, CCoord right, CCoord bottom)
	{
		if (left < right)
			this->left = left, this->right = right;
		else
			this->left = right, this->right = left;
		if (top < bottom)
			this->top = top, this->bottom = bottom;
		else
			this->top = bottom, this->bottom = top;
		return *this;
	}

	bool operator != (const CRect& other) const
		{ return (left != other.left || right != other.right ||
					top != other.top || bottom != other.bottom); }

	bool operator == (const CRect& other) const
		{ return (left == other.left && right == other.right &&
					top == other.top && bottom == other.bottom); }
	
	inline CCoord width () const  { return getWidth (); }
	inline CCoord height () const { return getHeight (); }

	inline CCoord getWidth () const  { return right - left; }
	inline CCoord getHeight () const { return bottom - top; }

	inline void setWidth (CCoord width) { right = left + width; }
	inline void setHeight (CCoord height) { bottom = top + height; }

	CPoint getTopLeft () const;
	CPoint getTopRight () const;
	CPoint getBottomLeft () const;
	CPoint getBottomRight () const;
	void setTopLeft (const CPoint& inPoint);
	void setTopRight (const CPoint& inPoint);
	void setBottomLeft (const CPoint& inPoint);
	void setBottomRight (const CPoint& inPoint);

	CPoint getCenter () const;

	CPoint getSize () const;
	void setSize (const CPoint& size);

	CRect& offset (CCoord x, CCoord y)
		{ left += x; right += x; top += y; bottom += y; return *this; }

	CRect& inset (CCoord deltaX, CCoord deltaY)
		{ left += deltaX; right -= deltaX; top += deltaY; bottom -= deltaY;
    	return *this; }

	CRect& moveTo (CCoord x, CCoord y)
		{ CCoord vDiff = y - top; CCoord hDiff = x - left; 
		top += vDiff; bottom += vDiff; left += hDiff; right += hDiff;
		return *this; }

	bool pointInside (const CPoint& where) const;	///< Checks if point is inside this rect

	bool isEmpty () const;

	bool rectOverlap (const CRect& rect) const
	{
		if (right < rect.left) return false;
		if (left > rect.right) return false;
		if (bottom < rect.top) return false;
		if (top > rect.bottom) return false;
		return true;
	}

	void bound (const CRect& rect);
	void unite (const CRect& rect);

	void normalize ()
	{
		if (left > right)
		{
			CCoord tmp = left;
			left = right;
			right = tmp;
		}
		if (top > bottom)
		{
			CCoord tmp = top;
			top = bottom;
			bottom = tmp;
		}
	}

	void originize ()
	{
		offset (-left, -top);
	}

	void centerInside (const CRect& r); ///< moves this rect to the center of r
	void makeIntegral ();

	CCoord left;
	CCoord top;
	CCoord right;
	CCoord bottom;
};

} // namespace

#endif
