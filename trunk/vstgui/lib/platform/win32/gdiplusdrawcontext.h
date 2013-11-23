//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __gdiplusdrawcontext__
#define __gdiplusdrawcontext__

#include "../../coffscreencontext.h"

#if WINDOWS

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

namespace VSTGUI {
class GdiplusBitmap;

//-----------------------------------------------------------------------------
class GdiplusDrawContext : public COffscreenContext
{
public:
	GdiplusDrawContext (HWND window, const CRect& drawSurface);
	GdiplusDrawContext (GdiplusBitmap* bitmap);
	~GdiplusDrawContext ();

	Gdiplus::Graphics* getGraphics () const { return pGraphics; }
	Gdiplus::Pen* getPen () const { return pPen; }
	Gdiplus::SolidBrush* getBrush () const { return pBrush; }
	Gdiplus::SolidBrush* getFontBrush () const { return pFontBrush; }

	// CDrawContext
	void moveTo (const CPoint &point) VSTGUI_OVERRIDE_VMETHOD;
	void lineTo (const CPoint &point) VSTGUI_OVERRIDE_VMETHOD;
	void drawLines (const CPoint* points, const int32_t& numberOfLines) VSTGUI_OVERRIDE_VMETHOD;
	void drawPolygon (const CPoint *pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
	void drawPoint (const CPoint &point, const CColor& color) VSTGUI_OVERRIDE_VMETHOD;
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) VSTGUI_OVERRIDE_VMETHOD;
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
	CGraphicsPath* createGraphicsPath () VSTGUI_OVERRIDE_VMETHOD;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;

	void endDraw () VSTGUI_OVERRIDE_VMETHOD;
//-----------------------------------------------------------------------------
protected:
	void init () VSTGUI_OVERRIDE_VMETHOD;

	HWND window;
	Gdiplus::Graphics	*pGraphics;
	Gdiplus::Pen		*pPen;
	Gdiplus::SolidBrush	*pBrush;
	Gdiplus::SolidBrush	*pFontBrush;
};

} // namespace

#endif // WINDOWS

#endif // __gdiplusdrawcontext__
