//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __cgraphicspath__
#define __cgraphicspath__

#include "crect.h"
#include "cpoint.h"
#include "ccolor.h"

namespace VSTGUI {
class CGradient;

//-----------------------------------------------------------------------------
/// @brief Graphics Transform Matrix [new in 4.0]
//!
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
struct CGraphicsTransform
{
	double m11;
	double m12;
	double m21;
	double m22;
	double dx;
	double dy;
	
	CGraphicsTransform (double _m11 = 1., double _m12 = 0., double _m21 = 0., double _m22 = 1., double _dx = 0., double _dy = 0.)
	: m11 (_m11), m12 (_m12), m21 (_m21), m22 (_m22), dx (_dx), dy (_dy)
	{}
	
	void translate (double x, double y)
	{
		*this = CGraphicsTransform (1, 0, 0, 1, x, y) * this;
	}
	
	void scale (double x, double y)
	{
		*this = CGraphicsTransform (x, 0., 0., y, 0., 0.) * this;
	}
	
	void rotate (double angle);
	
	CPoint& transform (CPoint& p)
	{
		CCoord x = m11*p.x + m12*p.y + dx;
		CCoord y = m21*p.x + m22*p.y + dy;
		p.x = x;
		p.y = y;
		return p;
	}

	CRect& transform (CRect& r)
	{
		CPoint p (r.getTopLeft ());
		r.setTopLeft (transform (p));
		p = r.getBottomRight ();
		r.setBottomRight (transform (p));
		return r;
	}

	CGraphicsTransform operator* (const CGraphicsTransform& t) const
	{
		CGraphicsTransform result;
		result.m11 = (m11 * t.m11) + (m12 * t.m21);
		result.m21 = (m21 * t.m11) + (m22 * t.m21);
		result.dx = (dx * t.m11) + (dy * t.m21) + t.dx;
		result.m12 = (m11 * t.m12) + (m12 * t.m22);
		result.m22 = (m21 * t.m12) + (m22 * t.m22);
		result.dy = (dx * t.m12) + (dy * t.m22) + t.dy;
		return result;
	}
	CGraphicsTransform operator* (const CGraphicsTransform* t) const { return *this * *t; }
};

//-----------------------------------------------------------------------------
/*! @class CGraphicsPath
	@brief Graphics Path Object [new in 4.0]

	@ingroup new_in_4_0
*/
//-----------------------------------------------------------------------------
class CGraphicsPath : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// @name Creating gradients
	//-----------------------------------------------------------------------------
	//@{
	/**
	 * @brief creates a new gradient object, you must release it with forget() when you're done with it
	 * @param color1Start value between zero and one which defines the normalized start offset for color1
	 * @param color2Start value between zero and one which defines the normalized start offset for color2
	 * @param color1 the first color of the gradient
	 * @param color2 the second color of the gradient
	 * @return a new gradient object
	*/
	virtual CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) = 0;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Adding Elements
	//-----------------------------------------------------------------------------
	//@{
	virtual void addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise) = 0;
	virtual void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end) = 0;
	virtual void addEllipse (const CRect& rect) = 0;
	virtual void addLine (const CPoint& start, const CPoint& end) = 0;
	virtual void addRect (const CRect& rect) = 0;
	virtual void addPath (const CGraphicsPath& path, CGraphicsTransform* transformation = 0) = 0;
	virtual void closeSubpath () = 0;

	void addRoundRect (const CRect& size, CCoord radius);
	//@}

	//-----------------------------------------------------------------------------
	/// @name States
	//-----------------------------------------------------------------------------
	//@{
	virtual CPoint getCurrentPosition () const = 0;
	virtual CRect getBoundingBox () const = 0;
	//@}
	
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGraphicsPath, CBaseObject)
protected:
	CGraphicsPath () {}
};

//-----------------------------------------------------------------------------
/*! @class CGradient
	@brief Gradient Object [new in 4.0]

	@ingroup new_in_4_0
*/
class CGradient : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// @name Member Access
	//-----------------------------------------------------------------------------
	//@{
	double getColor1Start () const { return color1Start; }
	double getColor2Start () const { return color2Start; }
	const CColor& getColor1 () const { return color1; }
	const CColor& getColor2 () const { return color2; }
	//@}
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGradient, CBaseObject)
protected:
	CGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	: color1Start (color1Start), color2Start (color2Start), color1 (color1), color2 (color2) {}
	
	double color1Start;
	double color2Start;
	CColor color1;
	CColor color2;
};

} // namespace

#endif
