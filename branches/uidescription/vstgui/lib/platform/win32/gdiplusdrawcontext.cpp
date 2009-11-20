
#include "gdiplusdrawcontext.h"

#if WINDOWS

#include <cmath>
#include "gdiplusbitmap.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
GdiplusDrawContext::GdiplusDrawContext (HDC deviceContext, const CRect& drawSurface)
: COffscreenContext (drawSurface)
, pGraphics (0)
, pPen (0)
, pBrush (0)
, pFontBrush (0)
{
	pGraphics = new Gdiplus::Graphics (deviceContext);

	init ();
}

//-----------------------------------------------------------------------------
GdiplusDrawContext::GdiplusDrawContext (GdiplusBitmap* inBitmap)
: COffscreenContext (new CBitmap (inBitmap))
, pGraphics (0)
, pPen (0)
, pBrush (0)
, pFontBrush (0)
{
	pGraphics = new Gdiplus::Graphics (inBitmap->getBitmap ());

	init ();
}

//-----------------------------------------------------------------------------
GdiplusDrawContext::~GdiplusDrawContext ()
{
	if (pFontBrush)
		delete pFontBrush;
	if (pBrush)
		delete pBrush;
	if (pPen)
		delete pPen;
	if (pGraphics)
		delete pGraphics;
}

// CDrawContext
//-----------------------------------------------------------------------------
void GdiplusDrawContext::init ()
{
	pGraphics->SetInterpolationMode (Gdiplus::InterpolationModeLowQuality);
	pGraphics->SetPageUnit (Gdiplus::UnitPixel);
	pGraphics->SetPixelOffsetMode (Gdiplus::PixelOffsetModeNone);
	pPen = new Gdiplus::Pen (Gdiplus::Color (0, 0, 0), 1);
	pBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	pFontBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));

	COffscreenContext::init ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::moveTo (const CPoint &point)
{
	CPoint p (point);
	p.offset (currentState.offset.x, currentState.offset.y);
	COffscreenContext::moveTo (p);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::lineTo (const CPoint &point)
{
	CPoint p (point);
	p.offset (currentState.offset.x, currentState.offset.y);
	if (pGraphics && pPen)
		pGraphics->DrawLine (pPen, (Gdiplus::REAL)currentState.penLoc.h, (Gdiplus::REAL)currentState.penLoc.v, (Gdiplus::REAL)p.h, (Gdiplus::REAL)p.v);
	currentState.penLoc = p;
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawLines (const CPoint* points, const long& numberOfLines)
{
	for (long i = 0; i < numberOfLines * 2; i+=2)
	{
		moveTo (points[i]);
		lineTo (points[i+1]);
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle)
{
	Gdiplus::PointF points[30];
	Gdiplus::PointF* polyPoints;
	bool allocated = false;
	if (numberOfPoints > 30)
	{
		polyPoints = new Gdiplus::PointF[numberOfPoints];
		allocated = true;
	}
	else
		polyPoints = points;
	
	for (long i = 0; i < numberOfPoints; i++)
	{
		polyPoints[i].X = (Gdiplus::REAL)(pPoints[i].h + currentState.offset.h);
		polyPoints[i].Y = (Gdiplus::REAL)(pPoints[i].v + currentState.offset.v);
	}

	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		pGraphics->FillPolygon (pBrush, polyPoints, numberOfPoints);
	if (drawStyle == kDrawFilledAndStroked || drawStyle == kDrawStroked)
		pGraphics->DrawPolygon (pPen, polyPoints, numberOfPoints);

	if (allocated)
		delete[] polyPoints;
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	CRect rect (_rect);
	rect.offset (currentState.offset.x, currentState.offset.y);
	if (pGraphics)
	{
		rect.normalize ();
		if (pBrush && (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked))
		{
			Gdiplus::RectF r ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());
			pGraphics->FillRectangle (pBrush, r);
		}
		if (pPen && (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked))
		{
			Gdiplus::RectF r ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth ()-1, (Gdiplus::REAL)rect.getHeight ()-1);
			pGraphics->DrawRectangle (pPen, r);
		}
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawArc (const CRect& _rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle)
{
	CRect rect (_rect);
	rect.offset (currentState.offset.x, currentState.offset.y);
	if (pGraphics)
	{
		float endAngle = _endAngle;
		if (endAngle < _startAngle)
			endAngle += 360.f;
		endAngle = fabs (endAngle - _startAngle);
		Gdiplus::RectF r ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());
		Gdiplus::GraphicsPath path;
		path.AddArc (r, _startAngle, endAngle);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
			pGraphics->FillPath (pBrush, &path);
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
			pGraphics->DrawPath (pPen, &path);
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawEllipse (const CRect &_rect, const CDrawStyle drawStyle)
{
	CRect rect (_rect);
	rect.offset (currentState.offset.x, currentState.offset.y);
	if (pGraphics)
	{
		rect.normalize ();
		if (pBrush && (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked))
		{
			pGraphics->FillEllipse (pBrush, (Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth ()-1, (Gdiplus::REAL)rect.getHeight ()-1);
		}
		if (pPen && (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked))
		{
			pGraphics->DrawEllipse (pPen, (Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth ()-1, (Gdiplus::REAL)rect.getHeight ()-1);
		}
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawPoint (const CPoint &point, CColor color)
{
	saveGlobalState ();
	setLineWidth (1);
	setFrameColor (color);
	CPoint point2 (point);
	point2.h++;
	moveTo (point);
	lineTo (point2);
	restoreGlobalState ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawBitmap (CBitmap* cbitmap, const CRect& dest, const CPoint& offset, float alpha)
{
	if (alpha == 0.f || pGraphics == 0)
		return;
	IPlatformBitmap* platformBitmap = cbitmap ? cbitmap->getPlatformBitmap () : 0;
	GdiplusBitmap* gpb = platformBitmap ? dynamic_cast<GdiplusBitmap*> (platformBitmap) : 0;
	Gdiplus::Bitmap* bitmap = gpb ? gpb->getBitmap () : 0;
	if (bitmap)
	{
		Gdiplus::ImageAttributes imageAtt;
		if (alpha != 1.f)
		{
			// introducing the alpha blend matrix
			Gdiplus::ColorMatrix colorMatrix =
			{
				1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, alpha, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			};
			// create the imageattribute modifier
			imageAtt.SetColorMatrix (&colorMatrix, Gdiplus::ColorMatrixFlagsDefault,
				Gdiplus::ColorAdjustTypeBitmap);
			// create a temporary bitmap to prevent OutOfMemory errors
			Gdiplus::Bitmap myBitmapBuffer ((INT)dest.getWidth (), (INT)dest.getHeight (),PixelFormat32bppARGB);
			// create a graphics context for the temporary bitmap
			Gdiplus::Graphics* myGraphicsBuffer = Gdiplus::Graphics::FromImage (&myBitmapBuffer);
			// copy the rectangle we want to paint to the temporary bitmap
			Gdiplus::Rect myTransRect(
				0,
				0,
				(INT)dest.getWidth (),
				(INT)dest.getHeight ());
			// transfer the bitmap (without modification by imageattr!)
			myGraphicsBuffer->DrawImage (
				bitmap,myTransRect,
				(INT)offset.x,
				(INT)offset.y,
				(INT)dest.getWidth (),
				(INT)dest.getHeight (),
				Gdiplus::UnitPixel,
				0);
			// now transfer the temporary to the real context at the advised location
			Gdiplus::Rect myDestRect (
				(INT)dest.left + (INT)currentState.offset.h,
				(INT)dest.top + (INT)currentState.offset.v,
				(INT)dest.getWidth (),
				(INT)dest.getHeight ());
			// transfer from temporary bitmap to real context (with imageattr)
			pGraphics->DrawImage (
				&myBitmapBuffer,
				myDestRect,
				(INT)0,
				(INT)0,
				(INT)dest.getWidth (),
				(INT)dest.getHeight (),
				Gdiplus::UnitPixel,
				&imageAtt);
			// delete the temporary context of the temporary bitmap
			delete myGraphicsBuffer;
		}
		else
		{
			Gdiplus::Rect	myDestRect(
				(INT)dest.left + (INT)currentState.offset.h,
				(INT)dest.top + (INT)currentState.offset.v,
				(INT)dest.getWidth (),
				(INT)dest.getHeight ());
			pGraphics->DrawImage (
				bitmap,
				myDestRect,
				(INT)offset.x,
				(INT)offset.y,
				(INT)dest.getWidth (),
				(INT)dest.getHeight (),
				Gdiplus::UnitPixel,
				0);
		}
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setLineStyle (CLineStyle style)
{
	if (currentState.lineStyle == style)
		return;
	if (pPen)
	{
		switch (style) 
		{
			case kLineOnOffDash: 
				pPen->SetDashStyle (Gdiplus::DashStyleDot);
				break;
			default:
				pPen->SetDashStyle (Gdiplus::DashStyleSolid);
				break;
		}
	}
	COffscreenContext::setLineStyle (style);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setLineWidth (CCoord width)
{
	if (currentState.frameWidth == width)
		return;
	if (pPen)
		pPen->SetWidth ((Gdiplus::REAL)width);
	COffscreenContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setDrawMode (CDrawMode mode)
{
	if (currentState.drawMode == mode)
		return;
	if (pGraphics)
	{
		if (mode == kAntialias)
			pGraphics->SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
		else
			pGraphics->SetSmoothingMode (Gdiplus::SmoothingModeNone);
	}
	COffscreenContext::setDrawMode (mode);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setClipRect (const CRect &clip)
{
	COffscreenContext::setClipRect (clip);
	if (pGraphics)
		pGraphics->SetClip (Gdiplus::RectF ((Gdiplus::REAL)currentState.clipRect.left, (Gdiplus::REAL)currentState.clipRect.top, (Gdiplus::REAL)currentState.clipRect.getWidth (), (Gdiplus::REAL)currentState.clipRect.getHeight ()), Gdiplus::CombineModeReplace);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::resetClipRect ()
{
	COffscreenContext::resetClipRect ();
	if (pGraphics)
		pGraphics->SetClip (Gdiplus::RectF ((Gdiplus::REAL)currentState.clipRect.left, (Gdiplus::REAL)currentState.clipRect.top, (Gdiplus::REAL)currentState.clipRect.getWidth (), (Gdiplus::REAL)currentState.clipRect.getHeight ()), Gdiplus::CombineModeReplace);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFillColor (const CColor color)
{
	if (currentState.fillColor == color)
		return;
	if (pBrush)
		pBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
	COffscreenContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFrameColor (const CColor color)
{
	if (currentState.frameColor == color)
		return;
	if (pPen)
		pPen->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
	COffscreenContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFontColor (const CColor color)
{
	if (currentState.fontColor == color)
		return;
	if (pFontBrush)
		pFontBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
	COffscreenContext::setFontColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setGlobalAlpha (float newAlpha)
{
	if (currentState.globalAlpha == newAlpha)
		return;
	COffscreenContext::setGlobalAlpha (newAlpha);
	const CColor notInitalized = {0, 0, 0, 0};
	CColor color (currentState.frameColor);
	currentState.frameColor = notInitalized;
	setFrameColor (color);
	color = currentState.fillColor;
	currentState.fillColor = notInitalized;
	setFillColor (color);
	color = currentState.fontColor;
	currentState.fontColor = notInitalized;
	setFontColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::saveGlobalState ()
{
	COffscreenContext::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::restoreGlobalState ()
{
	COffscreenContext::restoreGlobalState ();
}

} // namespace

#endif // WINDOWS
