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

#ifndef __d2ddrawcontext__
#define __d2ddrawcontext__

#include "../../../coffscreencontext.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "d2dbitmap.h"
#include <windows.h>
#include <d2d1.h>
#include <stack>

namespace VSTGUI {
class CGradient;

//-----------------------------------------------------------------------------
class D2DDrawContext : public COffscreenContext
{
public:
	D2DDrawContext (HWND window, const CRect& drawSurface);
	D2DDrawContext (D2DBitmap* bitmap);
	~D2DDrawContext ();

	ID2D1RenderTarget* getRenderTarget () const { return renderTarget; }
	ID2D1SolidColorBrush* getFillBrush () const { return fillBrush; }
	ID2D1SolidColorBrush* getStrokeBrush () const { return strokeBrush; }
	ID2D1SolidColorBrush* getFontBrush () const { return fontBrush; }
	ID2D1StrokeStyle* getStrokeStyle () const { return strokeStyle; }

	// CDrawContext
	void drawLine (const LinePair& line) VSTGUI_OVERRIDE_VMETHOD;
	void drawLines (const LineList& lines) VSTGUI_OVERRIDE_VMETHOD;
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked) VSTGUI_OVERRIDE_VMETHOD;
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
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) VSTGUI_OVERRIDE_VMETHOD;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset = CPoint (0, 0), bool evenOdd = false, CGraphicsTransform* transformation = 0) VSTGUI_OVERRIDE_VMETHOD;

	void beginDraw () VSTGUI_OVERRIDE_VMETHOD;
	void endDraw () VSTGUI_OVERRIDE_VMETHOD;

	//-----------------------------------------------------------------------------
	class D2DApplyClip
	{
	public:
		D2DApplyClip (D2DDrawContext* drawContext, bool halfPointOffset = false);
		~D2DApplyClip ();
		bool isEmpty () const { return applyClip.isEmpty (); }
	protected:
		D2DDrawContext* drawContext;
		CRect applyClip;
	};

	template<typename T> void pixelAllign (T& rect) const;
	
//-----------------------------------------------------------------------------
protected:
	void init () VSTGUI_OVERRIDE_VMETHOD;
	void createRenderTarget ();
	void releaseRenderTarget ();
	ID2D1GradientStopCollection* createGradientStopCollection (const CGradient& gradient) const;

	void setFillColorInternal (const CColor& color);
	void setFrameColorInternal (const CColor& color);
	void setFontColorInternal (const CColor& color);
	void setLineStyleInternal (const CLineStyle& style);
	void setDrawModeInternal (CDrawMode mode);

	HWND window;
	ID2D1RenderTarget* renderTarget;
	ID2D1SolidColorBrush* fillBrush;
	ID2D1SolidColorBrush* strokeBrush;
	ID2D1SolidColorBrush* fontBrush;
	ID2D1StrokeStyle* strokeStyle;
	CRect currentClip;
};

//-----------------------------------------------------------------------------
template<typename T> void D2DDrawContext::pixelAllign (T& obj) const
{
	const CGraphicsTransform& t = getCurrentTransform ();
	CGraphicsTransform tInv = t.inverse ();
	if (currentState.drawMode.integralMode ())
		obj.offset (-0.5, -0.5);
	t.transform (obj);
	obj.makeIntegral ();
	tInv.transform (obj);
}

//-----------------------------------------------------------------------------
static inline D2D1_RECT_F makeD2DRect (const CRect& r)
{
	D2D1_RECT_F dr = {(FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom};
	return dr;
}

//-----------------------------------------------------------------------------
static inline D2D1_POINT_2F makeD2DPoint (const CPoint& p)
{
	D2D1_POINT_2F dp = {(FLOAT)p.x, (FLOAT)p.y};
	return dp;
}

static inline D2D1_SIZE_F makeD2DSize (CCoord width, CCoord height)
{
	D2D1_SIZE_F ds = {(FLOAT)width, (FLOAT)height};
	return ds;
}

//-----------------------------------------------------------------------------
static inline D2D1_MATRIX_3X2_F convert (const CGraphicsTransform& t)
{
	D2D1_MATRIX_3X2_F matrix;
	matrix._11 = static_cast<FLOAT> (t.m11);
	matrix._12 = static_cast<FLOAT> (t.m21);
	matrix._21 = static_cast<FLOAT> (t.m12);
	matrix._22 = static_cast<FLOAT> (t.m22);
	matrix._31 = static_cast<FLOAT> (t.dx);
	matrix._32 = static_cast<FLOAT> (t.dy);
	return matrix;
}

} // namespace

#endif // WINDOWS

#endif // __d2ddrawcontext__
