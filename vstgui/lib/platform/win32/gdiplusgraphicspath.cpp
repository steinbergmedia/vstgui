// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdiplusgraphicspath.h"

#if WINDOWS

#include "gdiplusdrawcontext.h"
#include "../../cgradient.h"
#include "cfontwin32.h"
#include "win32support.h"
#include <cmath>

namespace VSTGUI {

//-----------------------------------------------------------------------------
GdiplusGraphicsPath::GdiplusGraphicsPath ()
: platformPath (0)
{
}

//-----------------------------------------------------------------------------
GdiplusGraphicsPath::GdiplusGraphicsPath (const GdiPlusFont* font, UTF8StringPtr text)
{
	platformPath = ::new Gdiplus::GraphicsPath ();

	Gdiplus::Font* gdiFont = font->getFont ();
	Gdiplus::FontFamily fontFamily;
	gdiFont->GetFamily (&fontFamily);
	UTF8StringHelper stringHelper (text);
	const WCHAR* wchar = stringHelper.getWideString ();
	platformPath->AddString (wchar, -1, &fontFamily, gdiFont->GetStyle (), gdiFont->GetSize (), Gdiplus::PointF (0.0f, 0.0f), NULL);
}

//-----------------------------------------------------------------------------
GdiplusGraphicsPath::~GdiplusGraphicsPath () noexcept
{
	if (platformPath)
		::delete platformPath;
}

//-----------------------------------------------------------------------------
CGradient* GdiplusGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	return CGradient::create (color1Start, color2Start, color1, color2);
}

//-----------------------------------------------------------------------------
void GdiplusGraphicsPath::dirty ()
{
	if (platformPath)
	{
		::delete platformPath;
		platformPath = 0;
	}
}

//-----------------------------------------------------------------------------
Gdiplus::GraphicsPath* GdiplusGraphicsPath::getGraphicsPath ()
{
	if (platformPath == 0)
	{
		platformPath = ::new Gdiplus::GraphicsPath ();
		Gdiplus::PointF pos;
		for (ElementList::const_iterator it = elements.begin (); it != elements.end (); it++)
		{
			const Element& e = (*it);
			switch (e.type)
			{
				case Element::kArc:
				{
					CCoord width = e.instruction.arc.rect.right - e.instruction.arc.rect.left;
					CCoord height = e.instruction.arc.rect.bottom - e.instruction.arc.rect.top;

					double sweepangle = e.instruction.arc.endAngle - e.instruction.arc.startAngle;
					if (e.instruction.arc.clockwise) {
						// sweepangle positive
						while (sweepangle < 0.0)
							sweepangle += 360.0;
						while (sweepangle > 360.0)
							sweepangle -= 360.0;
					} else {
						// sweepangle negative
						while (sweepangle > 0.0)
							sweepangle -= 360.0;
						while (sweepangle < -360.0)
							sweepangle += 360.0;
					}
					platformPath->AddArc ((Gdiplus::REAL)e.instruction.arc.rect.left, (Gdiplus::REAL)e.instruction.arc.rect.top, (Gdiplus::REAL)width, (Gdiplus::REAL)height, (Gdiplus::REAL)e.instruction.arc.startAngle, (Gdiplus::REAL)sweepangle);
					break;
				}
				case Element::kEllipse:
				{
					CCoord width = e.instruction.rect.right - e.instruction.rect.left;
					CCoord height = e.instruction.rect.bottom - e.instruction.rect.top;
					platformPath->AddEllipse ((Gdiplus::REAL)e.instruction.rect.left, (Gdiplus::REAL)e.instruction.rect.top, (Gdiplus::REAL)width, (Gdiplus::REAL)height);
					break;
				}
				case Element::kRect:
				{
					CCoord width = e.instruction.rect.right - e.instruction.rect.left;
					CCoord height = e.instruction.rect.bottom - e.instruction.rect.top;
					platformPath->AddRectangle (Gdiplus::RectF ((Gdiplus::REAL)e.instruction.rect.left, (Gdiplus::REAL)e.instruction.rect.top, (Gdiplus::REAL)width, (Gdiplus::REAL)height));
					break;
				}
				case Element::kLine:
				{
					platformPath->AddLine (pos.X, pos.Y, (Gdiplus::REAL)e.instruction.point.x, (Gdiplus::REAL)e.instruction.point.y);
					break;
				}
				case Element::kBezierCurve:
				{
					platformPath->AddBezier (pos.X, pos.Y, (Gdiplus::REAL)e.instruction.curve.control1.x, (Gdiplus::REAL)e.instruction.curve.control1.y, (Gdiplus::REAL)e.instruction.curve.control2.x, (Gdiplus::REAL)e.instruction.curve.control2.y, (Gdiplus::REAL)e.instruction.curve.end.x, (Gdiplus::REAL)e.instruction.curve.end.y);
					break;
				}
				case Element::kBeginSubpath:
				{
					platformPath->StartFigure ();
					pos.X = (Gdiplus::REAL)e.instruction.point.x;
					pos.Y = (Gdiplus::REAL)e.instruction.point.y;
					continue;
				}
				case Element::kCloseSubpath:
				{
					platformPath->CloseFigure ();
					break;
				}
			}
			platformPath->GetLastPoint (&pos);
		}
	}
	return platformPath;
}

//-----------------------------------------------------------------------------
bool GdiplusGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled, CGraphicsTransform* transform)
{
	Gdiplus::GraphicsPath* path = getGraphicsPath ();
	if (path)
	{
		if (transform)
		{
			Gdiplus::Matrix matrix;
			convert (matrix, *transform);
			path = path->Clone ();
			path->Transform (&matrix);
			bool result = path->IsVisible ((Gdiplus::REAL)p.x, (Gdiplus::REAL)p.y, NULL) != 0;
			delete path;
			return result;
		}
		return path->IsVisible ((Gdiplus::REAL)p.x, (Gdiplus::REAL)p.y, NULL) != 0;
	}
	return false;
}

//-----------------------------------------------------------------------------
CPoint GdiplusGraphicsPath::getCurrentPosition ()
{
	CPoint p (0, 0);

	Gdiplus::GraphicsPath* path = getGraphicsPath ();
	if (path)
	{
		Gdiplus::PointF gdiPoint;
		path->GetLastPoint (&gdiPoint);
		p.x = gdiPoint.X;
		p.y = gdiPoint.Y;
	}
	return p;
}

//-----------------------------------------------------------------------------
CRect GdiplusGraphicsPath::getBoundingBox ()
{
	CRect r;

	Gdiplus::GraphicsPath* path = getGraphicsPath ();
	if (path)
	{
		Gdiplus::RectF gdiRect;
		path->GetBounds (&gdiRect);
		r.left = gdiRect.X;
		r.top = gdiRect.Y;
		r.setWidth (gdiRect.Width);
		r.setHeight (gdiRect.Height);
	}
	return r;
}

} // namespace

#endif
