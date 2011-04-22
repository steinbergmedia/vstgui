//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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

#include <ApplicationServices/ApplicationServices.h>

#if MAC_CARBON
#include <Carbon/Carbon.h>
#endif

namespace VSTGUI {
class CGBitmap;

//-----------------------------------------------------------------------------
class CGDrawContext : public COffscreenContext
{
public:
	CGDrawContext (CGContextRef cgContext, const CRect& rect);
	CGDrawContext (CGBitmap* bitmap);
	~CGDrawContext ();
	
	void lineTo (const CPoint &point);
	void drawLines (const CPoint* points, const int32_t& numberOfLines);
	void drawPolygon (const CPoint *pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle = kDrawStroked);
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked);
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);
	void drawPoint (const CPoint &point, const CColor& color);
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f);
	void clearRect (const CRect& rect);
	void setLineStyle (const CLineStyle& style);
	void setLineWidth (CCoord width);
	void setDrawMode (CDrawMode mode);
	void setClipRect (const CRect &clip);
	void resetClipRect ();
	void setFillColor  (const CColor& color);
	void setFrameColor (const CColor& color);
	void setFontColor (const CColor& color);
	void setGlobalAlpha (float newAlpha);
	void saveGlobalState ();
	void restoreGlobalState ();
	void endDraw ();
	CGraphicsPath* createGraphicsPath ();
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = 0);
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = 0);

	CGContextRef beginCGContext (bool swapYAxis = false, bool integralOffset = false);
	void releaseCGContext (CGContextRef context);

	CGContextRef getCGContext () const { return cgContext; }
	void applyLineStyle (CGContextRef context);

//------------------------------------------------------------------------------------
protected:
	void init ();

	CGContextRef cgContext;
};

} // namespace

#endif // MAC

#endif // __cgdrawcontext__

