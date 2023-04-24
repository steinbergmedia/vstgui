// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformgraphicsdevice.h"

#if TARGET_OS_IPHONE
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#else
#include <ApplicationServices/ApplicationServices.h>
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

class CoreGraphicsDevice;

//------------------------------------------------------------------------
class CoreGraphicsDeviceContext : public IPlatformGraphicsDeviceContext,
								  public IPlatformGraphicsDeviceContextBitmapExt
{
public:
	CoreGraphicsDeviceContext (const CoreGraphicsDevice& device, void* cgContext);
	~CoreGraphicsDeviceContext () noexcept override;

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

	// IPlatformGraphicsDeviceContextBitmapExt
	bool drawBitmapNinePartTiled (IPlatformBitmap& bitmap, CRect dest,
								  const CNinePartTiledDescription& desc, double alpha,
								  BitmapInterpolationQuality quality) const override;
	bool fillRectWithBitmap (IPlatformBitmap& bitmap, CRect srcRect, CRect dstRect, double alpha,
							 BitmapInterpolationQuality quality) const override;

	// private
	void drawCTLine (CTLineRef line, CGPoint cgPoint, CTFontRef fontRef, CColor color,
					 bool underline, bool strikeThrough, bool antialias) const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class CoreGraphicsBitmapContext : public CoreGraphicsDeviceContext
{
public:
	using EndDrawFunc = std::function<void ()>;
	CoreGraphicsBitmapContext (const CoreGraphicsDevice& device, void* cgContext, EndDrawFunc&& f);

	bool endDraw () const override;

private:
	EndDrawFunc endDrawFunc;
};

//------------------------------------------------------------------------
class CoreGraphicsDevice : public IPlatformGraphicsDevice
{
public:
	PlatformGraphicsDeviceContextPtr
		createBitmapContext (const PlatformBitmapPtr& bitmap) const override;
};

//------------------------------------------------------------------------
class CoreGraphicsDeviceFactory : public IPlatformGraphicsDeviceFactory
{
public:
	PlatformGraphicsDevicePtr getDeviceForScreen (ScreenInfo::Identifier screen) const override;
};

//------------------------------------------------------------------------
} // VSTGUI
