// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cairoutils.h"

#include "../../coffscreencontext.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

class Bitmap;

//------------------------------------------------------------------------
class Context : public COffscreenContext
{
public:
	using super = COffscreenContext;

	Context (const CRect& rect, const SurfaceHandle& surface);
	Context (CRect r, cairo_t* context);
	Context (Bitmap* bitmap);

	~Context ();

	bool valid () const { return cr != nullptr; }
	const SurfaceHandle& getSurface () const { return surface; }
	const ContextHandle& getCairo () const { return cr; }

	void drawLine (const LinePair& line) override;
	void drawLines (const LineList& lines) override;
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle) override;
	void drawRect (const CRect& rect, const CDrawStyle drawStyle) override;
	void drawArc (const CRect& rect, const float startAngle1, const float endAngle2,
	              const CDrawStyle drawStyle) override;
	void drawEllipse (const CRect& rect, const CDrawStyle drawStyle) override;
	void drawPoint (const CPoint& point, const CColor& color) override;
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset,
	                 float alpha) override;
	void clearRect (const CRect& rect) override;
	CGraphicsPath* createGraphicsPath () override;
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) override;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode,
	                       CGraphicsTransform* transformation) override;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient,
	                         const CPoint& startPoint, const CPoint& endPoint, bool evenOdd,
	                         CGraphicsTransform* transformation) override;
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center,
	                         CCoord radius, const CPoint& originOffset, bool evenOdd,
	                         CGraphicsTransform* transformation) override;

	void saveGlobalState () override;
	void restoreGlobalState () override;

	void beginDraw () override;
	void endDraw () override;

private:
	void init () override;
	void setSourceColor (CColor color);
	void setupCurrentStroke ();
	void draw (CDrawStyle drawstyle);

	SurfaceHandle surface;
	ContextHandle cr;
};

//------------------------------------------------------------------------
struct DrawBlock
{
	static DrawBlock begin (Context& context);

	~DrawBlock ();
	operator bool () { return !clipIsEmpty; }
private:
	explicit DrawBlock (Context& context);
	Context& context;
	bool clipIsEmpty {false};
};

//-----------------------------------------------------------------------------
CPoint pixelAlign (const ContextHandle& handle, const CPoint& point);
CRect pixelAlign (const ContextHandle& handle, const CRect& rect);

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
