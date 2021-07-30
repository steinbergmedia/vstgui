// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../cgraphicspath.h"
#include "../iplatformgraphicspath.h"
#include "cairoutils.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
class GraphicsPathFactory : public IPlatformGraphicsPathFactory
{
public:
	GraphicsPathFactory (const ContextHandle& cr);

	PlatformGraphicsPathPtr createPath (PlatformGraphicsPathFillMode fillMode) override;
	PlatformGraphicsPathPtr createTextPath (const PlatformFontPtr& font,
											UTF8StringPtr text) override;

private:
	ContextHandle context;
};

//-----------------------------------------------------------------------------
class GraphicsPath : public IPlatformGraphicsPath
{
public:
	GraphicsPath (const ContextHandle& c);
	~GraphicsPath () noexcept;

	cairo_path_t* getCairoPath () const { return path; }
	std::unique_ptr<GraphicsPath> copyPixelAlign (const CGraphicsTransform& tm);

	// IPlatformGraphicsPath
	void addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise) override;
	void addEllipse (const CRect& rect) override;
	void addRect (const CRect& rect) override;
	void addLine (const CPoint& to) override;
	void addBezierCurve (const CPoint& control1, const CPoint& control2,
	                     const CPoint& end) override;
	void beginSubpath (const CPoint& start) override;
	void closeSubpath () override;
	void finishBuilding () override;
	bool hitTest (const CPoint& p, bool evenOddFilled = false,
	              CGraphicsTransform* transform = nullptr) const override;
	CRect getBoundingBox () const override;
	PlatformGraphicsPathFillMode getFillMode () const override
	{
		return PlatformGraphicsPathFillMode::Ignored;
	}

private:
	ContextHandle context;
	cairo_path_t* path {nullptr};
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
