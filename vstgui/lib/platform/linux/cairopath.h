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

	IPlatformGraphicsPathPtr createPath () override;
	IPlatformGraphicsPathPtr createTextPath (const PlatformFontPtr& font, UTF8StringPtr text) override;

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
	std::shared_ptr<GraphicsPath> copyPixelAlign (const CGraphicsTransform& tm);

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

private:
	ContextHandle context;
	cairo_path_t* path {nullptr};
};

//------------------------------------------------------------------------
class Path : public CGraphicsPath
{
public:
	Path (const IPlatformGraphicsPathFactoryPtr& factory,
	      const IPlatformGraphicsPathPtr& path = nullptr) noexcept;
	~Path () noexcept;

	const IPlatformGraphicsPathPtr& getPlatformPath ();

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1,
							   const CColor& color2) override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false,
				  CGraphicsTransform* transform = 0) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;

	void dirty () override;

//------------------------------------------------------------------------
private:
	bool ensurePathValid ();
	void makeGraphicsPath ();

	IPlatformGraphicsPathFactoryPtr factory;
	IPlatformGraphicsPathPtr path;
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
