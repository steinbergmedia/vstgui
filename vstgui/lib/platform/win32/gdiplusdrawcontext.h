// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __gdiplusdrawcontext__
#define __gdiplusdrawcontext__

#include "../../coffscreencontext.h"

#if WINDOWS

#include "win32support.h"

namespace VSTGUI {
class GdiplusBitmap;

//-----------------------------------------------------------------------------
class GdiplusDrawContext : public COffscreenContext
{
public:
	GdiplusDrawContext (HWND window, const CRect& drawSurface);
	GdiplusDrawContext (GdiplusBitmap* bitmap);
	~GdiplusDrawContext () noexcept;

	Gdiplus::Graphics* getGraphics () const { return pGraphics; }
	Gdiplus::Pen* getPen () const { return pPen; }
	Gdiplus::SolidBrush* getBrush () const { return pBrush; }
	Gdiplus::SolidBrush* getFontBrush () const { return pFontBrush; }

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

	void endDraw () override;
//-----------------------------------------------------------------------------
protected:
	void init () override;

	void setFillColorInternal (const CColor& color);
	void setFrameColorInternal (const CColor& color);
	void setFontColorInternal (const CColor& color);
	void setLineStyleInternal (const CLineStyle& style);
	void setLineWidthInternal (CCoord width);
	void setDrawModeInternal (CDrawMode mode);

	HWND window;
	Gdiplus::Graphics	*pGraphics;
	Gdiplus::Pen		*pPen;
	Gdiplus::SolidBrush	*pBrush;
	Gdiplus::SolidBrush	*pFontBrush;
};

//-----------------------------------------------------------------------------
struct GdiplusDrawScope
{
	GdiplusDrawScope (Gdiplus::Graphics* graphics, const CRect& clipRect, const CGraphicsTransform& transform)
	: graphics (graphics)
	{
		graphics->SetClip (Gdiplus::RectF ((Gdiplus::REAL)clipRect.left, (Gdiplus::REAL)clipRect.top, (Gdiplus::REAL)clipRect.getWidth (), (Gdiplus::REAL)clipRect.getHeight ()), Gdiplus::CombineModeReplace);
		if (transform.isInvariant () == false)
		{
			Gdiplus::Matrix matrix ((Gdiplus::REAL)transform.m11, (Gdiplus::REAL)transform.m12, (Gdiplus::REAL)transform.m21, (Gdiplus::REAL)transform.m22, (Gdiplus::REAL)transform.dx, (Gdiplus::REAL)transform.dy);
			graphics->SetTransform (&matrix);
		}
	}

	~GdiplusDrawScope () noexcept
	{
		Gdiplus::Matrix matrix;
		graphics->SetTransform (&matrix);
	}

	Gdiplus::Graphics* graphics;
};

//-----------------------------------------------------------------------------
inline void convert (Gdiplus::Matrix& matrix, const CGraphicsTransform& t)
{
	matrix.SetElements ((Gdiplus::REAL)t.m11, (Gdiplus::REAL)t.m21, (Gdiplus::REAL)t.m12, (Gdiplus::REAL)t.m22, (Gdiplus::REAL)t.dx, (Gdiplus::REAL)t.dy);
}


} // namespace

#endif // WINDOWS

#endif // __gdiplusdrawcontext__
