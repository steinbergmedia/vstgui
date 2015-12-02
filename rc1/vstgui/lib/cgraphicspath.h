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

#ifndef __cgraphicspath__
#define __cgraphicspath__

#include "vstguifwd.h"
#include "ccolor.h"
#include "crect.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief Graphics Path Object
///	@ingroup new_in_4_0
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
	/** add an arc to the path. Begins a new subpath if no elements were added before. */
	virtual void addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise);
	/** add an ellipse to the path. Begins a new subpath if no elements were added before. */
	virtual void addEllipse (const CRect& rect);
	/** add a rectangle to the path. Begins a new subpath if no elements were added before. */
	virtual void addRect (const CRect& rect);
	/** add another path to the path. Begins a new subpath if no elements were added before. */
	virtual void addPath (const CGraphicsPath& path, CGraphicsTransform* transformation = 0);
	/** add a line to the path. A subpath must begin before */
	virtual void addLine (const CPoint& to);
	/** add a bezier curve to the path. A subpath must begin before */
	virtual void addBezierCurve (const CPoint& control1, const CPoint& control2, const CPoint& end);
	/** begin a new subpath. */
	virtual void beginSubpath (const CPoint& start);
	/** close a subpath. A straight line will be added from the current point to the start point. */
	virtual void closeSubpath ();

	inline void beginSubpath (CCoord x, CCoord y)
	{
		beginSubpath (CPoint (x, y));
	}
	inline void addLine (CCoord x, CCoord y)
	{
		addLine (CPoint(x, y));
	}
	inline void addBezierCurve (CCoord cp1x, CCoord cp1y, CCoord cp2x, CCoord cp2y, CCoord x, CCoord y)
	{
		addBezierCurve (CPoint (cp1x, cp1y), CPoint (cp2x, cp2y), CPoint (x, y));
	}
	//@}

	//-----------------------------------------------------------------------------
	/// @name Helpers
	//-----------------------------------------------------------------------------
	//@{
	void addRoundRect (const CRect& size, CCoord radius);
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Hit Testing
	//-----------------------------------------------------------------------------
	//@{
	virtual bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = 0) = 0;
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name States
	//-----------------------------------------------------------------------------
	//@{
	virtual CPoint getCurrentPosition () = 0;
	virtual CRect getBoundingBox () = 0;
	//@}
	
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGraphicsPath, CBaseObject)
protected:
	CGraphicsPath () {}

	virtual void dirty () = 0; ///< platform object should be released

	/// @cond ignore

	struct Rect {
		CCoord left;
		CCoord top;
		CCoord right;
		CCoord bottom;
	};

	struct Point {
		CCoord x;
		CCoord y;
	};
	
	struct Arc {
		Rect rect;
		double startAngle;
		double endAngle;
		bool clockwise;
	};

	struct BezierCurve {
		Point control1;
		Point control2;
		Point end;
	};

	struct Element {
		enum Type {
			kArc = 0,
			kEllipse,
			kRect,
			kLine,
			kBezierCurve,
			kBeginSubpath,
			kCloseSubpath
		};
		
		Type type;
		union Instruction {
			Arc arc;
			Rect rect;
			BezierCurve curve;
			Point point;
		} instruction;
	};

	inline void CRect2Rect (const CRect& rect, CGraphicsPath::Rect& r) const {r.left = rect.left;r.right = rect.right;r.top = rect.top;r.bottom = rect.bottom;}
	inline void CPoint2Point (const CPoint& point, CGraphicsPath::Point& p) const {p.x = point.x;p.y = point.y;}
	/// @endcond

	typedef std::vector<Element> ElementList;
	ElementList elements;
};

} // namespace

#endif
