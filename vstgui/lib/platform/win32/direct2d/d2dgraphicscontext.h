// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../iplatformgraphicsdevice.h"

struct ID2D1DeviceContext;
struct ID2D1Device;
struct ID2D1SolidColorBrush;
struct IDWriteTextLayout;

//------------------------------------------------------------------------
namespace VSTGUI {

class D2DGraphicsDevice;

//------------------------------------------------------------------------
class D2DGraphicsDeviceContext : public IPlatformGraphicsDeviceContext
{
public:
	D2DGraphicsDeviceContext (const D2DGraphicsDevice& device, ID2D1DeviceContext* deviceContext);
	~D2DGraphicsDeviceContext () noexcept;

	const IPlatformGraphicsDevice& getDevice () const override;
	PlatformGraphicsPathFactoryPtr getGraphicsPathFactory () const override;

	bool beginDraw () const override;
	bool endDraw () const override;
	// draw commands
	bool drawLine (LinePair line) const override;
	bool drawLines (const LineList& lines) const override;
	bool drawPolygon (const PointList& polygonPointList,
					  PlatformGraphicsDrawStyle drawStyle) const override;
	bool drawRect (CRect rect, PlatformGraphicsDrawStyle drawStyle) const override;
	bool drawArc (CRect rect, double startAngle1, double endAngle2,
				  PlatformGraphicsDrawStyle drawStyle) const override;
	bool drawEllipse (CRect rect, PlatformGraphicsDrawStyle drawStyle) const override;
	bool drawPoint (CPoint point, CColor color) const override;
	bool drawBitmap (IPlatformBitmap& bitmap, CRect dest, CPoint offset, double alpha,
					 BitmapInterpolationQuality quality) const override;
	bool clearRect (CRect rect) const override;
	bool drawGraphicsPath (IPlatformGraphicsPath& path, PlatformGraphicsPathDrawMode mode,
						   TransformMatrix* transformation) const override;
	bool fillLinearGradient (IPlatformGraphicsPath& path, const IPlatformGradient& gradient,
							 CPoint startPoint, CPoint endPoint, bool evenOdd,
							 TransformMatrix* transformation) const override;
	bool fillRadialGradient (IPlatformGraphicsPath& path, const IPlatformGradient& gradient,
							 CPoint center, CCoord radius, CPoint originOffset, bool evenOdd,
							 TransformMatrix* transformation) const override;
	// state
	void saveGlobalState () const override;
	void restoreGlobalState () const override;
	void setLineStyle (const CLineStyle& style) const override;
	void setLineWidth (CCoord width) const override;
	void setDrawMode (CDrawMode mode) const override;
	void setClipRect (CRect clip) const override;
	void setFillColor (CColor color) const override;
	void setFrameColor (CColor color) const override;
	void setFontColor (CColor color) const override;
	void setGlobalAlpha (double newAlpha) const override;
	void setTransformMatrix (const TransformMatrix& tm) const override;

	// extension
	const IPlatformGraphicsDeviceContextBitmapExt* asBitmapExt () const override;

	// private
	void drawTextLayout (IDWriteTextLayout* textLayout, CPoint pos, bool antialias);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class D2DGraphicsDevice : public IPlatformGraphicsDevice
{
public:
	D2DGraphicsDevice (ID2D1Device* device);
	~D2DGraphicsDevice () noexcept;

	PlatformGraphicsDeviceContextPtr
		createBitmapContext (const PlatformBitmapPtr& bitmap) const override;

	ID2D1Device* get () const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI