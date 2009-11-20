
#ifndef __gdiplusgraphicspath__
#define __gdiplusgraphicspath__

#include "../../cgraphicspath.h"

#if WINDOWS

namespace Gdiplus {
class GraphicsPath;
}

namespace VSTGUI {

class GdiplusGraphicsPath : public CGraphicsPath
{
public:
	GdiplusGraphicsPath ();
	~GdiplusGraphicsPath ();

	Gdiplus::GraphicsPath* getGraphicsPath () const { return platformPath; }

	// CGraphicsPath
	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2);
	void addArc (const CRect& rect, double startAngle, double endAngle);
	void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end);
	void addEllipse (const CRect& rect);
	void addLine (const CPoint& start, const CPoint& end);
	void addRect (const CRect& rect);
	void addPath (const CGraphicsPath& path, CGraphicsTransformation* transformation = 0);
	void addString (const char* utf8String, CFontRef font, const CPoint& position);
	void closeSubpath ();
	void draw (CDrawContext* context, PathDrawMode mode = kFilled, CGraphicsTransformation* transformation = 0);
	void fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransformation* transformation = 0);
	CPoint getCurrentPosition () const;
	CRect getBoundingBox () const;
protected:
	Gdiplus::GraphicsPath* platformPath;
};

} // namespace

#endif // WINDOWS

#endif // __gdiplusgraphicspath__
