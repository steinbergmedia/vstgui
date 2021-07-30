// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../ccolor.h"
#include "../cgraphicstransform.h"
#include "../cpoint.h"
#include "../crect.h"

namespace VSTGUI {

//------------------------------------------------------------------------
enum class PlatformGraphicsPathFillMode : int32_t
{
	Winding,
	Alternate,
	Ignored
};

//------------------------------------------------------------------------
class IPlatformGraphicsPathFactory
{
public:
	virtual PlatformGraphicsPathPtr createPath (
		PlatformGraphicsPathFillMode fillMode = PlatformGraphicsPathFillMode::Winding) = 0;
	virtual PlatformGraphicsPathPtr createTextPath (const PlatformFontPtr& font,
													UTF8StringPtr text) = 0;

	virtual ~IPlatformGraphicsPathFactory () noexcept = default;
};

//-----------------------------------------------------------------------------
class IPlatformGraphicsPath
{
public:
	/** add an arc to the path. Begins a new subpath if no elements were added before. */
	virtual void addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise) = 0;
	/** add an ellipse to the path. Begins a new subpath if no elements were added before. */
	virtual void addEllipse (const CRect& rect) = 0;
	/** add a rectangle to the path. Begins a new subpath if no elements were added before. */
	virtual void addRect (const CRect& rect) = 0;
	/** add a line to the path. A subpath must begin before */
	virtual void addLine (const CPoint& to) = 0;
	/** add a bezier curve to the path. A subpath must begin before */
	virtual void addBezierCurve (const CPoint& control1, const CPoint& control2,
	                             const CPoint& end) = 0;
	/** begin a new subpath. */
	virtual void beginSubpath (const CPoint& start) = 0;
	/** close a subpath. A straight line will be added from the current point to the start point. */
	virtual void closeSubpath () = 0;
	virtual void finishBuilding () = 0;

	virtual bool hitTest (const CPoint& p, bool evenOddFilled = false,
	                      CGraphicsTransform* transform = nullptr) const = 0;
	virtual CRect getBoundingBox () const = 0;

	virtual PlatformGraphicsPathFillMode getFillMode () const = 0;

	virtual ~IPlatformGraphicsPath () noexcept = default;
};

} // VSTGUI
