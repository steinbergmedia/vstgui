// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "ccolor.h"
#include "crect.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief Graphics Path Object
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CGraphicsPath : public AtomicReferenceCounted
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
	virtual void addPath (const CGraphicsPath& path, CGraphicsTransform* transformation = nullptr);
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
	virtual bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = nullptr) = 0;
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name States
	//-----------------------------------------------------------------------------
	//@{
	virtual CPoint getCurrentPosition () = 0;
	virtual CRect getBoundingBox () = 0;
	//@}
	
protected:
	CGraphicsPath () {}

	virtual void dirty () = 0; // platform object should be released

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

	using ElementList = std::vector<Element>;
	ElementList elements;
};

} // VSTGUI
