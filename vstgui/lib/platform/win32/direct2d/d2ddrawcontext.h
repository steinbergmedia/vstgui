// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../coffscreencontext.h"

#if WINDOWS
struct IUnknown;
struct ID2D1DeviceContext;

#include "d2dbitmap.h"
#include <windows.h>
#include <d2d1_1.h>
#include <stack>

namespace VSTGUI {
class CGradient;

//-----------------------------------------------------------------------------
class D2DDrawContext final : public COffscreenContext
{
public:
	D2DDrawContext (HWND window, const CRect& drawSurface);
	D2DDrawContext (ID2D1DeviceContext* deviceContext, const CRect& drawSurface,
					ID2D1Device* device = nullptr);
	D2DDrawContext (D2DBitmap* bitmap);
	~D2DDrawContext ();

	bool usable () const { return getRenderTarget () != nullptr; }

	ID2D1RenderTarget* getRenderTarget () const { return renderTarget; }
	ID2D1SolidColorBrush* getFillBrush () const { return fillBrush; }
	ID2D1SolidColorBrush* getStrokeBrush () const { return strokeBrush; }
	ID2D1SolidColorBrush* getFontBrush () const { return fontBrush; }
	ID2D1StrokeStyle* getStrokeStyle () const { return strokeStyle; }

	// CDrawContext
	void drawLine (const LinePair& line) override;
	void drawLines (const LineList& lines) override;
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) override;
	void drawPoint (const CPoint &point, const CColor& color) override;
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) override;
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
	CGraphicsPath* createGraphicsPath () override;
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) override;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = nullptr) override;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = nullptr) override;
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset = CPoint (0, 0), bool evenOdd = false, CGraphicsTransform* transformation = nullptr) override;

	void beginDraw () override;
	void endDraw () override;

	double getScaleFactor () const override { return scaleFactor; }

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
		bool layerIsUsed {false};
	};

	template<typename T> void pixelAllign (T& rect) const;
	
//-----------------------------------------------------------------------------
protected:
	void init () override;
	void createRenderTarget ();
	void releaseRenderTarget ();
	ID2D1GradientStopCollection* createGradientStopCollection (const CGradient& gradient) const;

	void setFillColorInternal (const CColor& color);
	void setFrameColorInternal (const CColor& color);
	void setFontColorInternal (const CColor& color);
	void setLineStyleInternal (const CLineStyle& style);
	void setDrawModeInternal (CDrawMode mode);
	void drawLineInternal (CPoint start, CPoint end);

	bool needsHalfPointOffset () const;

	HWND window;
	ID2D1Device* device {nullptr};
	ID2D1RenderTarget* renderTarget;
	ID2D1SolidColorBrush* fillBrush;
	ID2D1SolidColorBrush* strokeBrush;
	ID2D1SolidColorBrush* fontBrush;
	ID2D1StrokeStyle* strokeStyle;
	CRect currentClip;
	double scaleFactor {1.};
};

//-----------------------------------------------------------------------------
template<typename T> void D2DDrawContext::pixelAllign (T& obj) const
{
	const CGraphicsTransform& t = getCurrentTransform ();
	CGraphicsTransform tInv = t.inverse ();
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

//-----------------------------------------------------------------------------
static inline D2D1::ColorF toColorF (CColor c, float alpha)
{
	return D2D1::ColorF (c.normRed<float> (), c.normGreen<float> (), c.normBlue<float> (),
	                     c.normAlpha<float> () * alpha);
}


} // VSTGUI

#endif // WINDOWS
