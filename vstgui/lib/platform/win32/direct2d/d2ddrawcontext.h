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

#ifndef __d2ddrawcontext__
#define __d2ddrawcontext__

#include "../../../coffscreencontext.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "d2dbitmap.h"
#include <windows.h>
#include <d2d1.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DDrawContext : public COffscreenContext
{
public:
	D2DDrawContext (HWND window, const CRect& drawSurface);
	D2DDrawContext (D2DOffscreenBitmap* bitmap);
	~D2DDrawContext ();

	ID2D1RenderTarget* getRenderTarget () const { return renderTarget; }
	ID2D1SolidColorBrush* getFillBrush () const { return fillBrush; }
	ID2D1SolidColorBrush* getStrokeBrush () const { return strokeBrush; }
	ID2D1SolidColorBrush* getFontBrush () const { return fontBrush; }
	ID2D1StrokeStyle* getStrokeStyle () const { return strokeStyle; }

	// CDrawContext
	void moveTo (const CPoint &point);
	void lineTo (const CPoint &point);
	void drawLines (const CPoint* points, const long& numberOfLines);
	void drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle = kDrawStroked);
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked);
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);
	void drawPoint (const CPoint &point, CColor color);
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f);
	void clearRect (const CRect& rect);
	void setLineStyle (CLineStyle style);
	void setLineWidth (CCoord width);
	void setDrawMode (CDrawMode mode);
	void setClipRect (const CRect &clip);
	void resetClipRect ();
	void setFillColor  (const CColor color);
	void setFrameColor (const CColor color);
	void setFontColor (const CColor color);
	void setGlobalAlpha (float newAlpha);
	void saveGlobalState ();
	void restoreGlobalState ();

	void beginDraw ();
	void endDraw ();

	//-----------------------------------------------------------------------------
	class D2DApplyClip
	{
	public:
		D2DApplyClip (D2DDrawContext* drawContext);
		~D2DApplyClip ();
	protected:
		D2DDrawContext* drawContext;
	};

//-----------------------------------------------------------------------------
protected:
	void init ();
	void createRenderTarget ();
	void releaseRenderTarget ();

	HWND window;
	ID2D1RenderTarget* renderTarget;
	ID2D1SolidColorBrush* fillBrush;
	ID2D1SolidColorBrush* strokeBrush;
	ID2D1SolidColorBrush* fontBrush;
	ID2D1StrokeStyle* strokeStyle;
};

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

} // namespace

#endif // WINDOWS

#endif // __d2ddrawcontext__
