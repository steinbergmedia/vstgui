
#include "gdiplusgraphicspath.h"

#if WINDOWS

#include "gdiplusdrawcontext.h"
#include "cfontwin32.h"
#include "win32support.h"
#include <cmath>

namespace VSTGUI {

//-----------------------------------------------------------------------------
inline Gdiplus::Color createGdiPlusColor (const CColor& color)
{
	return Gdiplus::Color (color.alpha, color.red, color.green, color.blue);
}

//-----------------------------------------------------------------------------
inline Gdiplus::Graphics* getGraphics (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getGraphics () : 0;
}

//-----------------------------------------------------------------------------
inline Gdiplus::Pen* getPen (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getPen () : 0;
}

//-----------------------------------------------------------------------------
inline Gdiplus::Brush* getBrush (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getBrush () : 0;
}

//-----------------------------------------------------------------------------
class GdiplusGradient : public CGradient
{
public:
	GdiplusGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	: CGradient (color1Start, color2Start, color1, color2) {}
};

//-----------------------------------------------------------------------------
GdiplusGraphicsPath::GdiplusGraphicsPath ()
: platformPath (new Gdiplus::GraphicsPath ())
{
}

//-----------------------------------------------------------------------------
GdiplusGraphicsPath::~GdiplusGraphicsPath ()
{
	delete platformPath;
}

//-----------------------------------------------------------------------------
CGradient* GdiplusGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	return new GdiplusGradient (color1Start, color2Start, color1, color2);
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::draw (CDrawContext* context, PathDrawMode mode, CGraphicsTransformation* transformation)
{
	Gdiplus::Graphics* graphics = getGraphics (context);
	if (graphics)
	{
		Gdiplus::GraphicsState state = graphics->Save ();
		graphics->TranslateTransform ((Gdiplus::REAL)context->getOffset ().x, (Gdiplus::REAL)context->getOffset ().y);

		Gdiplus::GraphicsPath* path = platformPath;

		if (transformation)
		{
			Gdiplus::Matrix matrix;
			matrix.Translate ((Gdiplus::REAL)transformation->offset.x, (Gdiplus::REAL)transformation->offset.y);
			matrix.Scale ((Gdiplus::REAL)transformation->scaleX, (Gdiplus::REAL)transformation->scaleY);
			matrix.Rotate ((Gdiplus::REAL)transformation->rotation);
			path = platformPath->Clone ();
			path->Transform (&matrix);
		}

		if (mode == kStroked)
		{
			graphics->DrawPath (getPen (context), path);
		}
		else
		{
			path->SetFillMode (mode == kFilledEvenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);
			graphics->FillPath (getBrush (context), path);
		}
		graphics->Restore (state);
		if (path != platformPath)
			delete path;
	}
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransformation* transformation)
{
	Gdiplus::Graphics* graphics = getGraphics (context);
	if (graphics)
	{
		Gdiplus::GraphicsState state = graphics->Save ();
		graphics->TranslateTransform ((Gdiplus::REAL)context->getOffset ().x, (Gdiplus::REAL)context->getOffset ().y);

		Gdiplus::GraphicsPath* path = platformPath;

		if (transformation)
		{
			Gdiplus::Matrix matrix;
			matrix.Translate ((Gdiplus::REAL)transformation->offset.x, (Gdiplus::REAL)transformation->offset.y);
			matrix.Scale ((Gdiplus::REAL)transformation->scaleX, (Gdiplus::REAL)transformation->scaleY);
			matrix.Rotate ((Gdiplus::REAL)transformation->rotation);
			path = platformPath->Clone ();
			path->Transform (&matrix);
		}

		Gdiplus::PointF c1p ((Gdiplus::REAL)(startPoint.x-context->getOffset ().x), (Gdiplus::REAL)(startPoint.y-context->getOffset ().y));
		Gdiplus::PointF c2p ((Gdiplus::REAL)(endPoint.x-context->getOffset ().x), (Gdiplus::REAL)(endPoint.y-context->getOffset ().y));
		Gdiplus::LinearGradientBrush brush (c1p, c2p, createGdiPlusColor (gradient.getColor1 ()), createGdiPlusColor (gradient.getColor2 ()));
		Gdiplus::REAL blendFactors[] = { 0.f, 0.f, 1.f, 1.f };
		Gdiplus::REAL blendPositions [] = { 0.f, (Gdiplus::REAL)gradient.getColor1Start (), (Gdiplus::REAL)gradient.getColor2Start (), 1.f };
		brush.SetBlend (blendFactors, blendPositions, 4);
		path->SetFillMode (evenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);

		graphics->FillPath (&brush, path);
		graphics->Restore (state);
		if (path != platformPath)
			delete path;
	}
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::closeSubpath ()
{
	platformPath->CloseFigure ();
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle)
{
	if (endAngle < startAngle)
		endAngle += 360.f;
	endAngle = fabs (endAngle - (Gdiplus::REAL)startAngle);
	platformPath->AddArc ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight (), (Gdiplus::REAL)startAngle, (Gdiplus::REAL)endAngle);
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end)
{
	platformPath->AddBezier ((Gdiplus::REAL)start.x, (Gdiplus::REAL)start.y, (Gdiplus::REAL)control1.x, (Gdiplus::REAL)control1.y, (Gdiplus::REAL)control2.x, (Gdiplus::REAL)control2.y, (Gdiplus::REAL)end.x, (Gdiplus::REAL)end.y);
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addEllipse (const CRect& rect)
{
	platformPath->AddEllipse ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addLine (const CPoint& start, const CPoint& end)
{
	if (start != getCurrentPosition ())
		platformPath->StartFigure ();
	platformPath->AddLine ((Gdiplus::REAL)start.x, (Gdiplus::REAL)start.y, (Gdiplus::REAL)end.x, (Gdiplus::REAL)end.y);
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addRect (const CRect& rect)
{
	platformPath->AddRectangle (Gdiplus::RectF ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ()));
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addPath (const CGraphicsPath& inPath, CGraphicsTransformation* transformation)
{
	const GdiplusGraphicsPath* path = dynamic_cast<const GdiplusGraphicsPath*> (&inPath);
	if (path)
	{
		Gdiplus::GraphicsPath* gdiPath = path->getGraphicsPath ()->Clone ();
		if (transformation)
		{
			Gdiplus::Matrix matrix;
			matrix.Translate ((Gdiplus::REAL)transformation->offset.x, (Gdiplus::REAL)transformation->offset.y);
			matrix.Scale ((Gdiplus::REAL)transformation->scaleX, (Gdiplus::REAL)transformation->scaleY);
			matrix.Rotate ((Gdiplus::REAL)transformation->rotation);
			gdiPath->Transform (&matrix);
		}
		platformPath->AddPath (gdiPath, true); // TODO: maybe the second parameter must be false
		delete gdiPath;
	}
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::addString (const char* utf8String, CFontRef font, const CPoint& position)
{
	GdiPlusFont* gf = dynamic_cast<GdiPlusFont*> (font->getPlatformFont ());
	if (gf == 0)
		return;

	Gdiplus::Font* gdiFont = gf->getFont ();
	if (gdiFont == 0)
		return;

	Gdiplus::FontFamily family;
	gdiFont->GetFamily (&family);
	INT fontStyle = 0;
	if (font->getStyle () & kBoldFace)
		fontStyle |= Gdiplus::FontStyleBold;
	if (font->getStyle () & kItalicFace)
		fontStyle |= Gdiplus::FontStyleItalic;
	// kUnderlineFace is not supported on Mac OS X, so we neither support it with GDIPlus
	//if (font->getStyle () & kUnderlineFace)
	//	fontStyle |= Gdiplus::FontStyleUnderline;

	UTF8StringHelper str (utf8String);
	platformPath->AddString (str, -1, &family, fontStyle, (Gdiplus::REAL)font->getSize (), Gdiplus::PointF ((Gdiplus::REAL)position.x, (Gdiplus::REAL)position.y - (Gdiplus::REAL)font->getSize ()), 0);
}

//-----------------------------------------------------------------------------
CPoint GdiplusGraphicsPath::getCurrentPosition () const
{
	CPoint p (0, 0);

	Gdiplus::PointF gdiPoint;
	platformPath->GetLastPoint (&gdiPoint);
	p.x = gdiPoint.X;
	p.y = gdiPoint.Y;

	return p;
}

//-----------------------------------------------------------------------------
CRect GdiplusGraphicsPath::getBoundingBox () const
{
	CRect r;

	Gdiplus::RectF gdiRect;
	platformPath->GetBounds (&gdiRect);
	r.left = gdiRect.X;
	r.top = gdiRect.Y;
	r.setWidth (gdiRect.Width);
	r.setHeight (gdiRect.Height);
	
	return r;
}

} // namespace

#endif
