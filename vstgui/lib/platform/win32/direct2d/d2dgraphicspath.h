// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../cgraphicspath.h"
#include "../../iplatformgraphicspath.h"

#if WINDOWS

#include <list>

struct ID2D1PathGeometry;
struct ID2D1Geometry;
struct ID2D1GeometrySink;
struct ID2D1Factory;
struct D2D1_GRADIENT_STOP;

namespace VSTGUI {
class D2DFont;
class D2DDrawContext;

//-----------------------------------------------------------------------------
class D2DGraphicsPathFactory : public IPlatformGraphicsPathFactory
{
public:
	static PlatformGraphicsPathFactoryPtr instance ();

	PlatformGraphicsPathPtr createPath (PlatformGraphicsPathFillMode fillMode) override;
	PlatformGraphicsPathPtr createTextPath (const PlatformFontPtr& font,
											UTF8StringPtr text) override;
};

//-----------------------------------------------------------------------------
class D2DGraphicsPath : public IPlatformGraphicsPath
{
public:
	D2DGraphicsPath (ID2D1PathGeometry* path, PlatformGraphicsPathFillMode fillMode);
	~D2DGraphicsPath () noexcept override;

	ID2D1PathGeometry* getPathGeometry () const { return path; }
	ID2D1Geometry* createTransformedGeometry (ID2D1Factory* factory,
											  const CGraphicsTransform& tm) const;
	ID2D1Geometry* createPixelAlignedGeometry (ID2D1Factory* factory, D2DDrawContext& context,
											   const CGraphicsTransform* tm = nullptr) const;

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
	PlatformGraphicsPathFillMode getFillMode () const override { return fillMode; }

private:
	ID2D1GeometrySink* getSink ();

	ID2D1PathGeometry* path {nullptr};
	ID2D1GeometrySink* sinkInternal {nullptr};
	PlatformGraphicsPathFillMode fillMode {PlatformGraphicsPathFillMode::Winding};
	bool figureOpen {false};
	CPoint lastPos;
};

} // VSTGUI

#endif // WINDOWS
