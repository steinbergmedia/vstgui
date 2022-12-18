// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
enum class PlatformGraphicsDrawStyle : uint32_t
{
	Stroked,
	Filled,
	FilledAndStroked
};

//------------------------------------------------------------------------
enum class PlatformGraphicsPathDrawMode : uint32_t
{
	Filled,
	FilledEvenOdd,
	Stroked
};

//------------------------------------------------------------------------
using TransformMatrix = CGraphicsTransform;

//------------------------------------------------------------------------
struct ScreenInfo
{
	using Identifier = uint32_t;
	/*
		Identifier identifier;
		uint32_t width;
		uint32_t height;
	*/
};
static constexpr const ScreenInfo::Identifier DefaultScreenIdentifier = 0u;

//------------------------------------------------------------------------
class IPlatformGraphicsDeviceFactory
{
public:
	virtual ~IPlatformGraphicsDeviceFactory () noexcept = default;

	virtual PlatformGraphicsDevicePtr getDeviceForScreen (ScreenInfo::Identifier screen) const = 0;
};

//------------------------------------------------------------------------
class IPlatformGraphicsDevice
{
public:
	virtual ~IPlatformGraphicsDevice () noexcept = default;

	virtual PlatformGraphicsDeviceContextPtr
		createBitmapContext (const PlatformBitmapPtr& bitmap) const = 0;
};

//------------------------------------------------------------------------
class IPlatformGraphicsDeviceContext
{
public:
	virtual ~IPlatformGraphicsDeviceContext () noexcept = default;

	virtual const IPlatformGraphicsDevice& getDevice () const = 0;
	virtual PlatformGraphicsPathFactoryPtr getGraphicsPathFactory () const = 0;

	virtual bool beginDraw () const = 0;
	virtual bool endDraw () const = 0;
	// draw commands
	virtual bool drawLine (LinePair line) const = 0;
	virtual bool drawLines (const LineList& lines) const = 0;
	virtual bool drawPolygon (const PointList& polygonPointList,
							  PlatformGraphicsDrawStyle drawStyle) const = 0;
	virtual bool drawRect (CRect rect, PlatformGraphicsDrawStyle drawStyle) const = 0;
	virtual bool drawArc (CRect rect, double startAngle1, double endAngle2,
						  PlatformGraphicsDrawStyle drawStyle) const = 0;
	virtual bool drawEllipse (CRect rect, PlatformGraphicsDrawStyle drawStyle) const = 0;
	virtual bool drawPoint (CPoint point, CColor color) const = 0;
	virtual bool drawBitmap (IPlatformBitmap& bitmap, CRect dest, CPoint offset, double alpha,
							 BitmapInterpolationQuality quality) const = 0;
	virtual bool clearRect (CRect rect) const = 0;
	virtual bool drawGraphicsPath (IPlatformGraphicsPath& path, PlatformGraphicsPathDrawMode mode,
								   TransformMatrix* transformation) const = 0;
	virtual bool fillLinearGradient (IPlatformGraphicsPath& path, const IPlatformGradient& gradient,
									 CPoint startPoint, CPoint endPoint, bool evenOdd,
									 TransformMatrix* transformation) const = 0;
	virtual bool fillRadialGradient (IPlatformGraphicsPath& path, const IPlatformGradient& gradient,
									 CPoint center, CCoord radius, CPoint originOffset,
									 bool evenOdd, TransformMatrix* transformation) const = 0;
	// state
	virtual void saveGlobalState () const = 0;
	virtual void restoreGlobalState () const = 0;
	virtual void setLineStyle (const CLineStyle& style) const = 0;
	virtual void setLineWidth (CCoord width) const = 0;
	virtual void setDrawMode (CDrawMode mode) const = 0;
	virtual void setClipRect (CRect clip) const = 0;
	virtual void setFillColor (CColor color) const = 0;
	virtual void setFrameColor (CColor color) const = 0;
	virtual void setGlobalAlpha (double newAlpha) const = 0;
	virtual void setTransformMatrix (const TransformMatrix& tm) const = 0;

	// extension
	virtual const IPlatformGraphicsDeviceContextBitmapExt* asBitmapExt () const = 0;
};

//------------------------------------------------------------------------
class IPlatformGraphicsDeviceContextBitmapExt
{
public:
	virtual ~IPlatformGraphicsDeviceContextBitmapExt () noexcept = default;

	virtual bool drawBitmapNinePartTiled (IPlatformBitmap& bitmap, CRect dest,
										  const CNinePartTiledDescription& desc, double alpha,
										  BitmapInterpolationQuality quality) const = 0;
	virtual bool fillRectWithBitmap (IPlatformBitmap& bitmap, CRect srcRect, CRect dstRect,
									 double alpha, BitmapInterpolationQuality quality) const = 0;
};

//------------------------------------------------------------------------
} // VSTGUI
