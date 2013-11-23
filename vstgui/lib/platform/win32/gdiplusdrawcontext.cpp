//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "gdiplusdrawcontext.h"

#if WINDOWS

#include <cmath>
#include "gdiplusbitmap.h"
#include "gdiplusgraphicspath.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
inline Gdiplus::Color createGdiPlusColor (const CColor& color)
{
	return Gdiplus::Color (color.alpha, color.red, color.green, color.blue);
}

//-----------------------------------------------------------------------------
GdiplusDrawContext::GdiplusDrawContext (HWND window, const CRect& drawSurface)
: COffscreenContext (drawSurface)
, window (window)
, pGraphics (0)
, pPen (0)
, pBrush (0)
, pFontBrush (0)
{
	pGraphics = new Gdiplus::Graphics (window);

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

	bitmap->forget ();
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
	pGraphics->SetPixelOffsetMode (Gdiplus::PixelOffsetModeHalf);
	pPen = new Gdiplus::Pen (Gdiplus::Color (0, 0, 0), 1);
	pBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	pFontBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));

	COffscreenContext::init ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::endDraw ()
{
//	if (pGraphics)
//		pGraphics->Flush ();
}

//-----------------------------------------------------------------------------
CGraphicsPath* GdiplusDrawContext::createGraphicsPath ()
{
	return new GdiplusGraphicsPath ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	GdiplusGraphicsPath* gdiPlusPath = dynamic_cast<GdiplusGraphicsPath*> (_path);
	if (gdiPlusPath && pGraphics)
	{
		Gdiplus::GraphicsState state = pGraphics->Save ();
		pGraphics->TranslateTransform ((Gdiplus::REAL)getOffset ().x, (Gdiplus::REAL)getOffset ().y);

		Gdiplus::GraphicsPath* path = gdiPlusPath->getGraphicsPath ();

		if (t)
		{
			Gdiplus::Matrix matrix ((Gdiplus::REAL)t->m11, (Gdiplus::REAL)t->m12, (Gdiplus::REAL)t->m21, (Gdiplus::REAL)t->m22, (Gdiplus::REAL)t->dx, (Gdiplus::REAL)t->dy);
			path = path->Clone ();
			path->Transform (&matrix);
		}

		if (mode == kPathStroked)
		{
			pGraphics->DrawPath (getPen (), path);
		}
		else
		{
			path->SetFillMode (mode == kPathFilledEvenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);
			pGraphics->FillPath (getBrush (), path);
		}
		pGraphics->Restore (state);
		if (path != gdiPlusPath->getGraphicsPath ())
			delete path;
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::fillLinearGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
	GdiplusGraphicsPath* gdiPlusPath = dynamic_cast<GdiplusGraphicsPath*> (_path);
	if (gdiPlusPath && pGraphics)
	{
		Gdiplus::GraphicsState state = pGraphics->Save ();
		pGraphics->TranslateTransform ((Gdiplus::REAL)getOffset ().x, (Gdiplus::REAL)getOffset ().y);

		Gdiplus::GraphicsPath* path = gdiPlusPath->getGraphicsPath ();

		if (t)
		{
			Gdiplus::Matrix matrix ((Gdiplus::REAL)t->m11, (Gdiplus::REAL)t->m12, (Gdiplus::REAL)t->m21, (Gdiplus::REAL)t->m22, (Gdiplus::REAL)t->dx, (Gdiplus::REAL)t->dy);
			path = path->Clone ();
			path->Transform (&matrix);
		}

		CColor startColor (gradient.getColor1 ());
		startColor.alpha = (int8_t)(float)(startColor.alpha * currentState.globalAlpha);
		CColor endColor (gradient.getColor2 ());
		endColor.alpha = (int8_t)(float)(endColor.alpha * currentState.globalAlpha);
		Gdiplus::PointF c1p ((Gdiplus::REAL)(startPoint.x-getOffset ().x), (Gdiplus::REAL)(startPoint.y-getOffset ().y));
		Gdiplus::PointF c2p ((Gdiplus::REAL)(endPoint.x-getOffset ().x), (Gdiplus::REAL)(endPoint.y-getOffset ().y));
		Gdiplus::LinearGradientBrush brush (c1p, c2p, createGdiPlusColor (startColor), createGdiPlusColor (endColor));
		Gdiplus::REAL blendFactors[] = { 0.f, 0.f, 1.f, 1.f };
		Gdiplus::REAL blendPositions [] = { 0.f, (Gdiplus::REAL)gradient.getColor1Start (), (Gdiplus::REAL)gradient.getColor2Start (), 1.f };
		brush.SetBlend (blendFactors, blendPositions, 4);
		path->SetFillMode (evenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);

		pGraphics->FillPath (&brush, path);
		pGraphics->Restore (state);
		if (path != gdiPlusPath->getGraphicsPath ())
			delete path;
	}
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
	{
		pGraphics->TranslateTransform (0.5f, 0.5f);
		pGraphics->DrawLine (pPen, (Gdiplus::REAL)currentState.penLoc.h, (Gdiplus::REAL)currentState.penLoc.v, (Gdiplus::REAL)p.h, (Gdiplus::REAL)p.v);
		pGraphics->TranslateTransform (-0.5f, -0.5f);
	}
	currentState.penLoc = p;
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawLines (const CPoint* points, const int32_t& numberOfLines)
{
	for (int32_t i = 0; i < numberOfLines * 2; i+=2)
	{
		moveTo (points[i]);
		lineTo (points[i+1]);
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawPolygon (const CPoint *pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle)
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
	
	for (int32_t i = 0; i < numberOfPoints; i++)
	{
		polyPoints[i].X = (Gdiplus::REAL)(pPoints[i].h + currentState.offset.h);
		polyPoints[i].Y = (Gdiplus::REAL)(pPoints[i].v + currentState.offset.v);
	}

	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		pGraphics->FillPolygon (pBrush, polyPoints, numberOfPoints);
	if (drawStyle == kDrawFilledAndStroked || drawStyle == kDrawStroked)
	{
		pGraphics->TranslateTransform (0.5f, 0.5f);
		pGraphics->DrawPolygon (pPen, polyPoints, numberOfPoints);
		pGraphics->TranslateTransform (-0.5f, -0.5f);
	}

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
			Gdiplus::RectF r ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth () - 1, (Gdiplus::REAL)rect.getHeight () - 1);
			pGraphics->TranslateTransform (0.5f, 0.5f);
			pGraphics->DrawRectangle (pPen, r);
			pGraphics->TranslateTransform (-0.5f, -0.5f);
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
void GdiplusDrawContext::drawPoint (const CPoint &point, const CColor& color)
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
	alpha *= currentState.globalAlpha;
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
#if 1
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
				&imageAtt);
#else
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
#endif
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
void GdiplusDrawContext::clearRect (const CRect& rect)
{
	// TODO:
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setLineStyle (const CLineStyle& style)
{
	if (currentState.lineStyle == style)
		return;
	if (pPen)
	{
		Gdiplus::LineCap lineCap;
		switch (style.getLineCap ())
		{
			case CLineStyle::kLineCapButt: lineCap = Gdiplus::LineCapFlat; break;
			case CLineStyle::kLineCapRound: lineCap = Gdiplus::LineCapRound; break;
			case CLineStyle::kLineCapSquare: lineCap = Gdiplus::LineCapSquare; break;
		}
		pPen->SetLineCap (lineCap, lineCap, Gdiplus::DashCapFlat); // DashCapFlat correct ?

		Gdiplus::LineJoin lineJoin;
		switch (style.getLineJoin ())
		{
			case CLineStyle::kLineJoinMiter: lineJoin = Gdiplus::LineJoinMiter; break;
			case CLineStyle::kLineJoinRound: lineJoin = Gdiplus::LineJoinRound; break;
			case CLineStyle::kLineJoinBevel: lineJoin = Gdiplus::LineJoinBevel; break;
		}
		pPen->SetLineJoin (lineJoin);

		pPen->SetDashOffset ((Gdiplus::REAL)style.getDashPhase ());
		
		if (style.getDashCount () == 0)
		{
			pPen->SetDashStyle (Gdiplus::DashStyleSolid);
		}
		else
		{
			Gdiplus::REAL* dashes = new Gdiplus::REAL [style.getDashCount ()];
			for (int32_t i = 0; i < style.getDashCount (); i++)
				dashes[i] = (Gdiplus::REAL)style.getDashLengths ()[i];
			pPen->SetDashPattern (dashes, style.getDashCount ());
			delete [] dashes; 
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
		if (mode == kAntiAliasing)
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
void GdiplusDrawContext::setFillColor (const CColor& color)
{
	if (currentState.fillColor == color)
		return;
	if (pBrush)
		pBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
	COffscreenContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFrameColor (const CColor& color)
{
	if (currentState.frameColor == color)
		return;
	if (pPen)
		pPen->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
	COffscreenContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFontColor (const CColor& color)
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
	CColor color (currentState.frameColor);
	currentState.frameColor = kTransparentCColor;
	setFrameColor (color);
	color = currentState.fillColor;
	currentState.fillColor = kTransparentCColor;
	setFillColor (color);
	color = currentState.fontColor;
	currentState.fontColor = kTransparentCColor;
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
