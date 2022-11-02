// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include "iplatformbitmap.h"

//------------------------------------------------------------------------
namespace VSTGUI {

class IPlatformDrawDeviceBitmapExt;

//------------------------------------------------------------------------
enum class DrawStyle
{
	Stroked,
	Filled,
	FilledAndStroked
};

//------------------------------------------------------------------------
enum class PathDrawMode
{
	Filled,
	FilledEvenOdd,
	Stroked
};

//------------------------------------------------------------------------
class IPlatformDrawDevice
{
public:
	virtual ~IPlatformDrawDevice () noexcept = default;

	virtual bool beginDraw () = 0;
	virtual bool endDraw () = 0;
	// draw commands
	virtual bool drawLine (LinePair line) = 0;
	virtual bool drawLines (const LineList& lines) = 0;
	virtual bool drawPolygon (const PointList& polygonPointList, DrawStyle drawStyle) = 0;
	virtual bool drawRect (CRect rect, DrawStyle drawStyle) = 0;
	virtual bool drawArc (CRect rect, float startAngle1, float endAngle2, DrawStyle drawStyle) = 0;
	virtual bool drawEllipse (CRect rect, DrawStyle drawStyle) = 0;
	virtual bool drawPoint (CPoint point, CColor color) = 0;
	virtual bool drawBitmap (IPlatformBitmap& bitmap, CRect dest, CPoint offset, float alpha,
							 BitmapInterpolationQuality quality) = 0;
	virtual bool clearRect (CRect rect) = 0;
	virtual bool drawGraphicsPath (IPlatformGraphicsPath& path, PathDrawMode mode,
								   CGraphicsTransform* transformation) = 0;
	virtual bool fillLinearGradient (IPlatformGraphicsPath& path, const IPlatformGradient& gradient,
									 CPoint startPoint, CPoint endPoint, bool evenOdd,
									 CGraphicsTransform* transformation) = 0;
	virtual bool fillRadialGradient (IPlatformGraphicsPath& path, const IPlatformGradient& gradient,
									 CPoint center, CCoord radius, CPoint originOffset,
									 bool evenOdd, CGraphicsTransform* transformation) = 0;
	// state
	virtual bool saveGlobalState () = 0;
	virtual bool restoreGlobalState () = 0;
	virtual void setLineStyle (const CLineStyle& style) = 0;
	virtual void setLineWidth (CCoord width) = 0;
	virtual void setDrawMode (CDrawMode mode) = 0;
	virtual void setClipRect (CRect clip) = 0;
	virtual void setFillColor (CColor color) = 0;
	virtual void setFrameColor (CColor color) = 0;
	virtual void setGlobalAlpha (float newAlpha) = 0;
	virtual void setTransformMatrix (CGraphicsTransform tm);

	// extension
	virtual IPlatformDrawDeviceBitmapExt* asBitmapExt () const = 0;
};

//------------------------------------------------------------------------
class IPlatformDrawDeviceBitmapExt
{
public:
	virtual ~IPlatformDrawDeviceBitmapExt () noexcept = default;

	virtual bool drawBitmapNinePartTiled (IPlatformBitmap& bitmap, CRect dest,
										  const CNinePartTiledDescription& desc, float alpha,
										  BitmapInterpolationQuality quality) = 0;
	virtual bool fillRectWithBitmap (IPlatformBitmap& bitmap, CRect srcRect, CRect dstRect,
									 float alpha, BitmapInterpolationQuality quality) = 0;
};

//------------------------------------------------------------------------
} // VSTGUI
