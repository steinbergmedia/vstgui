//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
	CGDrawContext (CGBitmap* bitmap);
	~CGDrawContext ();
	
	void drawLine (const LinePair& line) VSTGUI_OVERRIDE_VMETHOD;
	void drawLines (const LineList& lines) VSTGUI_OVERRIDE_VMETHOD;
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawPoint (const CPoint &point, const CColor& color) VSTGUI_OVERRIDE_VMETHOD;
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) VSTGUI_OVERRIDE_VMETHOD;
	void drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& dest, const CNinePartTiledDescription& desc, float alpha = 1.f) VSTGUI_OVERRIDE_VMETHOD;
	void fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect, float alpha) VSTGUI_OVERRIDE_VMETHOD;
	void clearRect (const CRect& rect) VSTGUI_OVERRIDE_VMETHOD;
	void setLineStyle (const CLineStyle& style) VSTGUI_OVERRIDE_VMETHOD;
	void setLineWidth (CCoord width) VSTGUI_OVERRIDE_VMETHOD;
	void setDrawMode (CDrawMode mode) VSTGUI_OVERRIDE_VMETHOD;
	void setClipRect (const CRect &clip) VSTGUI_OVERRIDE_VMETHOD;
	void resetClipRect () VSTGUI_OVERRIDE_VMETHOD;
	void setFillColor  (const CColor& color) VSTGUI_OVERRIDE_VMETHOD;
	void setFrameColor (const CColor& color) VSTGUI_OVERRIDE_VMETHOD;
	void setFontColor (const CColor& color) VSTGUI_OVERRIDE_VMETHOD;
	void setGlobalAlpha (float newAlpha) VSTGUI_OVERRIDE_VMETHOD;
	void saveGlobalState () VSTGUI_OVERRIDE_VMETHOD;
	void restoreGlobalState () VSTGUI_OVERRIDE_VMETHOD;
	void endDraw () VSTGUI_OVERRIDE_VMETHOD;
	CGraphicsPath* createGraphicsPath () VSTGUI_OVERRIDE_VMETHOD;
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) VSTGUI_OVERRIDE_VMETHOD;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset = CPoint (0, 0), bool evenOdd = false, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;
	double getScaleFactor () const VSTGUI_OVERRIDE_VMETHOD { return scaleFactor; }
	
	CGContextRef beginCGContext (bool swapYAxis = false, bool integralOffset = false);
	void releaseCGContext (CGContextRef context);

	CGContextRef getCGContext () const { return cgContext; }
	void applyLineStyle (CGContextRef context);
	void applyLineWidthCTM (CGContextRef context) const;

	CGRect pixelAlligned (const CGRect& r) const;
	CGPoint pixelAlligned (const CGPoint& p) const;
	
//------------------------------------------------------------------------------------
protected:
	void init () VSTGUI_OVERRIDE_VMETHOD;
	void drawCGImageRef (CGContextRef context, CGImageRef image, CGLayerRef layer, double imageScaleFactor, const CRect& inRect, const CPoint& inOffset, float alpha, CBitmap* bitmap);

	CGContextRef cgContext;

	typedef std::map<CGBitmap*, int32_t> BitmapDrawCountMap;
	BitmapDrawCountMap bitmapDrawCount;
	
	double scaleFactor;
};

} // namespace

#endif // MAC

#endif // __cgdrawcontext__

