
#ifndef __cgdrawcontext__
#define __cgdrawcontext__

#include "../../coffscreencontext.h"

#if MAC

#include <ApplicationServices/ApplicationServices.h>

#if MAC_CARBON
#include <Carbon/Carbon.h>
#endif

BEGIN_NAMESPACE_VSTGUI
class CGOffscreenBitmap;

//-----------------------------------------------------------------------------
class CGDrawContext : public COffscreenContext
{
public:
	CGDrawContext (CGContextRef cgContext, const CRect& rect);
	CGDrawContext (CGOffscreenBitmap* bitmap);
	~CGDrawContext ();
	
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

	CGContextRef beginCGContext (bool swapYAxis = false);
	void releaseCGContext (CGContextRef context);

	CGContextRef getCGContext () const { return cgContext; }

protected:
	void init ();
	void applyLineDash ();

	CGContextRef cgContext;
};

END_NAMESPACE_VSTGUI

#endif // MAC

#endif // __cgdrawcontext__

