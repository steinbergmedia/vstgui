// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cgdrawcontext__
#define __cgdrawcontext__

#include "../../coffscreencontext.h"

#if MAC

#if TARGET_OS_IPHONE
	#include <CoreGraphics/CoreGraphics.h>
	#include <ImageIO/ImageIO.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

#if MAC_CARBON
#include <Carbon/Carbon.h>
#endif

#include <map>

namespace VSTGUI {
class CGBitmap;

//-----------------------------------------------------------------------------
class CGDrawContext : public COffscreenContext
{
public:
	CGDrawContext (CGContextRef cgContext, const CRect& rect);
	explicit CGDrawContext (CGBitmap* bitmap);
	~CGDrawContext () noexcept override;
	
	void drawLine (const LinePair& line) override;
	void drawLines (const LineList& lines) override;
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawPoint (const CPoint &point, const CColor& color) override;
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) override;
	void drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& dest, const CNinePartTiledDescription& desc, float alpha = 1.f) override;
	void fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect, float alpha) override;
	void clearRect (const CRect& rect) override;
	void setLineStyle (const CLineStyle& style) override;
	void setLineWidth (CCoord width) override;
	void setDrawMode (CDrawMode mode) override;
	void setClipRect (const CRect &clip) override;
	void resetClipRect () override;
	void setFillColor  (const CColor& color) override;
	void setFrameColor (const CColor& color) override;
	void setFontColor (const CColor& color) override;
	void setGlobalAlpha (float newAlpha) override;
	void saveGlobalState () override;
	void restoreGlobalState () override;
	void endDraw () override;
	CGraphicsPath* createGraphicsPath () override;
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) override;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = nullptr) override;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = nullptr) override;
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset = CPoint (0, 0), bool evenOdd = false, CGraphicsTransform* transformation = nullptr) override;
	double getScaleFactor () const override { return scaleFactor; }
	
	CGContextRef beginCGContext (bool swapYAxis = false, bool integralOffset = false);
	void releaseCGContext (CGContextRef context);

	CGContextRef getCGContext () const { return cgContext; }
	void applyLineStyle (CGContextRef context);
	void applyLineWidthCTM (CGContextRef context) const;

	CGRect pixelAlligned (const CGRect& r) const;
	CGPoint pixelAlligned (const CGPoint& p) const;
	
//------------------------------------------------------------------------------------
protected:
	void init () override;
	void drawCGImageRef (CGContextRef context, CGImageRef image, CGLayerRef layer, double imageScaleFactor, const CRect& inRect, const CPoint& inOffset, float alpha, CBitmap* bitmap);

	CGContextRef cgContext;

	using BitmapDrawCountMap = std::map<CGBitmap*, int32_t>;
	BitmapDrawCountMap bitmapDrawCount;
	
	double scaleFactor;
};

} // namespace

#endif // MAC

#endif // __cgdrawcontext__

