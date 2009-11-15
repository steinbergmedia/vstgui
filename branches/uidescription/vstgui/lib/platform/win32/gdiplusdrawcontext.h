
#ifndef __gdiplusdrawcontext__
#define __gdiplusdrawcontext__

#include "../../coffscreencontext.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

namespace VSTGUI {
class GdiplusBitmap;

//-----------------------------------------------------------------------------
class GdiplusDrawContext : public COffscreenContext
{
public:
	GdiplusDrawContext (HDC deviceContext, const CRect& drawSurface);
	GdiplusDrawContext (GdiplusBitmap* bitmap);
	~GdiplusDrawContext ();

	Gdiplus::Graphics* getGraphics () const { return pGraphics; }
	Gdiplus::Pen* getPen () const { return pPen; }
	Gdiplus::SolidBrush* getBrush () const { return pBrush; }
	Gdiplus::SolidBrush* getFontBrush () const { return pFontBrush; }

	// CDrawContext
	void lineTo (const CPoint &point);
	void drawLines (const CPoint* points, const long& numberOfLines);
	void drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle = kDrawStroked);
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked);
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);
	void drawPoint (const CPoint &point, CColor color);
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f);
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

//-----------------------------------------------------------------------------
protected:
	Gdiplus::Graphics	*pGraphics;
	Gdiplus::Pen		*pPen;
	Gdiplus::SolidBrush	*pBrush;
	Gdiplus::SolidBrush	*pFontBrush;
};

} // namespace

#endif // WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#endif // __gdiplusdrawcontext__
