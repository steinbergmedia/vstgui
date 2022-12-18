// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformgraphicsdevice.h"

#include "cairoutils.h"

//------------------------------------------------------------------------
namespace VSTGUI {

class CairoGraphicsDevice;

//------------------------------------------------------------------------
class CairoGraphicsDeviceContext : public IPlatformGraphicsDeviceContext
{
public:
	CairoGraphicsDeviceContext (const CairoGraphicsDevice& device,
								const Cairo::SurfaceHandle& handle);
	~CairoGraphicsDeviceContext () noexcept;

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
	void setGlobalAlpha (double newAlpha) const override;
	void setTransformMatrix (const TransformMatrix& tm) const override;

	// extension
	const IPlatformGraphicsDeviceContextBitmapExt* asBitmapExt () const override;

	// private
	void drawPangoLayout (void* layout, CPoint pos, CColor color) const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class CairoGraphicsDevice : public IPlatformGraphicsDevice
{
public:
	CairoGraphicsDevice (cairo_device_t* device);
	~CairoGraphicsDevice () noexcept;

	PlatformGraphicsDeviceContextPtr
		createBitmapContext (const PlatformBitmapPtr& bitmap) const override;

	cairo_device_t* get () const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class CairoGraphicsDeviceFactory : public IPlatformGraphicsDeviceFactory
{
public:
	CairoGraphicsDeviceFactory ();
	~CairoGraphicsDeviceFactory () noexcept;

	PlatformGraphicsDevicePtr getDeviceForScreen (ScreenInfo::Identifier screen) const override;

	PlatformGraphicsDevicePtr addDevice (cairo_device_t* device);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
