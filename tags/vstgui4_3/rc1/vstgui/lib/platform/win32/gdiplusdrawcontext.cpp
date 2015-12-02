//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#include "gdiplusbitmap.h"
#include "gdiplusgraphicspath.h"
#include "cfontwin32.h"
#include "../../cbitmap.h"
#include "../../cgradient.h"
#include <cmath>

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
	pGraphics = ::new Gdiplus::Graphics (window);

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
	pGraphics = ::new Gdiplus::Graphics (inBitmap->getBitmap ());

	init ();

	bitmap->forget ();
}

//-----------------------------------------------------------------------------
GdiplusDrawContext::~GdiplusDrawContext ()
{
	if (pFontBrush)
		::delete pFontBrush;
	if (pBrush)
		::delete pBrush;
	if (pPen)
		::delete pPen;
	if (pGraphics)
		::delete pGraphics;
}

// CDrawContext

//-----------------------------------------------------------------------------
void GdiplusDrawContext::init ()
{
	pGraphics->SetInterpolationMode (Gdiplus::InterpolationModeLowQuality);
	pGraphics->SetPageUnit (Gdiplus::UnitPixel);
	pGraphics->SetPixelOffsetMode (Gdiplus::PixelOffsetModeHalf);
	pPen = ::new Gdiplus::Pen (Gdiplus::Color (0, 0, 0), 1);
	pBrush = ::new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	pFontBrush = ::new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));

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
CGraphicsPath* GdiplusDrawContext::createTextPath (const CFontRef font, UTF8StringPtr text)
{
	const GdiPlusFont* ctFont = dynamic_cast<const GdiPlusFont*>(font->getPlatformFont ());
	return ctFont ? new GdiplusGraphicsPath (ctFont, text) : 0;
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	GdiplusGraphicsPath* gdiPlusPath = dynamic_cast<GdiplusGraphicsPath*> (_path);
	if (gdiPlusPath && pGraphics)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

		Gdiplus::GraphicsPath* path = gdiPlusPath->getGraphicsPath ();

		if (t)
		{
			Gdiplus::Matrix matrix;
			convert (matrix, *t);
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
		if (path != gdiPlusPath->getGraphicsPath ())
			delete path;
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::fillLinearGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
#if DEBUG
	DebugPrint ("WARNING: GdiplusDrawContext::fillLinearGradient is not working as expected ! FIXME");
#endif
	GdiplusGraphicsPath* gdiPlusPath = dynamic_cast<GdiplusGraphicsPath*> (_path);
	if (gdiPlusPath && pGraphics)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

		Gdiplus::GraphicsPath* path = gdiPlusPath->getGraphicsPath ();

		if (t)
		{
			Gdiplus::Matrix matrix;
			convert (matrix, *t);
			path = path->Clone ();
			path->Transform (&matrix);
		}

		Gdiplus::Color* colors = new Gdiplus::Color [gradient.getColorStops ().size ()];
		Gdiplus::REAL* positions = new Gdiplus::REAL [gradient.getColorStops ().size ()];
		uint32_t index = 0;
		for (CGradient::ColorStopMap::const_iterator it = gradient.getColorStops ().begin (); it != gradient.getColorStops ().end (); ++it, ++index)
		{
			CColor color = it->second;
			color.alpha = (int8_t)((float)color.alpha * currentState.globalAlpha);
			colors[index] = createGdiPlusColor (color);
			positions[index] = (Gdiplus::REAL)it->first;
		}

		Gdiplus::PointF c1p ((Gdiplus::REAL)(startPoint.x), (Gdiplus::REAL)(startPoint.y));
		Gdiplus::PointF c2p ((Gdiplus::REAL)(endPoint.x), (Gdiplus::REAL)(endPoint.y));
		Gdiplus::LinearGradientBrush brush (c1p, c2p, colors[0], colors[gradient.getColorStops ().size () - 1]);
		brush.SetInterpolationColors (colors, positions, static_cast<INT> (gradient.getColorStops ().size ()));
		path->SetFillMode (evenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);

		pGraphics->FillPath (&brush, path);
		if (path != gdiPlusPath->getGraphicsPath ())
			delete path;
		delete [] colors;
		delete [] positions;
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::fillRadialGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset, bool evenOdd, CGraphicsTransform* t)
{
#if DEBUG
	DebugPrint ("WARNING: GdiplusDrawContext::fillRadialGradient is not working as expected ! FIXME\n");
#endif
	GdiplusGraphicsPath* gdiPlusPath = dynamic_cast<GdiplusGraphicsPath*> (_path);
	if (gdiPlusPath && pGraphics)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

		Gdiplus::GraphicsPath* path = gdiPlusPath->getGraphicsPath ();

		if (t)
		{
			Gdiplus::Matrix matrix;
			convert (matrix, *t);
			path = path->Clone ();
			path->Transform (&matrix);
		}

		path->SetFillMode (evenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);
		Gdiplus::PointF c1p ((Gdiplus::REAL)(center.x + originOffset.x), (Gdiplus::REAL)(center.y + originOffset.y));

		CRect boundingBox = gdiPlusPath->getBoundingBox ();
		Gdiplus::GraphicsPath brushPath;
		brushPath.AddEllipse ((Gdiplus::REAL)boundingBox.left, (Gdiplus::REAL)boundingBox.top, (Gdiplus::REAL)boundingBox.getWidth (), (Gdiplus::REAL)boundingBox.getHeight ());
		Gdiplus::Matrix graphicsMatrix;
		pGraphics->GetTransform (&graphicsMatrix);
		brushPath.Transform (&graphicsMatrix);

		Gdiplus::PathGradientBrush brush (&brushPath);
		// set center
		brush.SetCenterPoint (c1p);
		// set the colors
		Gdiplus::Color* colors = new Gdiplus::Color [gradient.getColorStops ().size ()];
		Gdiplus::REAL* positions = new Gdiplus::REAL [gradient.getColorStops ().size ()];
		uint32_t index = 0;
		for (CGradient::ColorStopMap::const_iterator it = gradient.getColorStops ().begin (); it != gradient.getColorStops ().end (); ++it, ++index)
		{
			CColor color = it->second;
			color.alpha = (int8_t)((float)color.alpha * currentState.globalAlpha);
			colors[index] = createGdiPlusColor (color);
			positions[index] = (Gdiplus::REAL)it->first;
		}
		brush.SetCenterColor (colors[0]);
		INT count = static_cast<INT> (gradient.getColorStops ().size ()) - 1;
		brush.SetSurroundColors (colors+1, &count);

		pGraphics->FillPath (&brush, path);
		if (path != gdiPlusPath->getGraphicsPath ())
			delete path;
		delete [] colors;
		delete [] positions;
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawLine (const LinePair& line)
{
	if (pGraphics && pPen)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

		CPoint p1 (line.first);
		CPoint p2 (line.second);

		pGraphics->TranslateTransform (0.5f, 0.5f);
		pGraphics->DrawLine (pPen, (Gdiplus::REAL)p1.x, (Gdiplus::REAL)p1.y, (Gdiplus::REAL)p2.x, (Gdiplus::REAL)p2.y);
		pGraphics->TranslateTransform (-0.5f, -0.5f);
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawLines (const LineList& lines)
{
	if (lines.size () == 0)
		return;
	VSTGUI_RANGE_BASED_FOR_LOOP(LineList, lines, LinePair, line)
		drawLine (line);
	VSTGUI_RANGE_BASED_FOR_LOOP_END
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
	if (polygonPointList.size () == 0)
		return;

	GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

	Gdiplus::PointF points[30];
	Gdiplus::PointF* polyPoints;
	bool allocated = false;
	if (polygonPointList.size () > 30)
	{
		polyPoints = new Gdiplus::PointF[polygonPointList.size ()];
		allocated = true;
	}
	else
		polyPoints = points;
	
	for (uint32_t i = 0; i < polygonPointList.size (); i++)
	{
		polyPoints[i].X = (Gdiplus::REAL)(polygonPointList[i].x);
		polyPoints[i].Y = (Gdiplus::REAL)(polygonPointList[i].y);
	}

	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		pGraphics->FillPolygon (pBrush, polyPoints, static_cast<INT> (polygonPointList.size ()));
	if (drawStyle == kDrawFilledAndStroked || drawStyle == kDrawStroked)
	{
		pGraphics->TranslateTransform (0.5f, 0.5f);
		pGraphics->DrawPolygon (pPen, polyPoints, static_cast<INT> (polygonPointList.size ()));
		pGraphics->TranslateTransform (-0.5f, -0.5f);
	}

	if (allocated)
		delete[] polyPoints;
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	if (pGraphics)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

		CRect rect (_rect);
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
void GdiplusDrawContext::drawArc (const CRect& rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle)
{
	if (pGraphics)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

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
	if (pGraphics)
	{
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

		CRect rect (_rect);
		rect.normalize ();
		if (pBrush && (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked))
		{
			pGraphics->FillEllipse (pBrush, (Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());
		}
		if (pPen && (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked))
		{
			pGraphics->DrawEllipse (pPen, (Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());
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
	point2.x++;
	COffscreenContext::drawLine (point, point2);
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
		GdiplusDrawScope drawScope (pGraphics, currentState.clipRect, getCurrentTransform ());

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
				(INT)dest.left,
				(INT)dest.top,
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
				(INT)dest.left,
				(INT)dest.top,
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
				(INT)dest.left,
				(INT)dest.top,
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
	setLineStyleInternal (style);
	COffscreenContext::setLineStyle (style);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setLineStyleInternal (const CLineStyle& style)
{
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
			for (uint32_t i = 0; i < style.getDashCount (); i++)
				dashes[i] = (Gdiplus::REAL)style.getDashLengths ()[i];
			pPen->SetDashPattern (dashes, style.getDashCount ());
			delete [] dashes; 
		}
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setLineWidth (CCoord width)
{
	if (currentState.frameWidth == width)
		return;
	setLineWidthInternal (width);
	COffscreenContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setLineWidthInternal (CCoord width)
{
	if (pPen)
		pPen->SetWidth ((Gdiplus::REAL)width);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setDrawMode (CDrawMode mode)
{
	if (currentState.drawMode == mode)
		return;
	setDrawModeInternal (mode);
	COffscreenContext::setDrawMode (mode);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setDrawModeInternal (CDrawMode mode)
{
	if (pGraphics)
	{
		if (mode == kAntiAliasing)
			pGraphics->SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
		else
			pGraphics->SetSmoothingMode (Gdiplus::SmoothingModeNone);
	}
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setClipRect (const CRect &clip)
{
	COffscreenContext::setClipRect (clip);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::resetClipRect ()
{
	COffscreenContext::resetClipRect ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFillColor (const CColor& color)
{
	if (currentState.fillColor == color)
		return;
	setFillColorInternal (color);
	COffscreenContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFillColorInternal (const CColor& color)
{
	if (pBrush)
		pBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFrameColor (const CColor& color)
{
	if (currentState.frameColor == color)
		return;
	setFrameColorInternal (color);
	COffscreenContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFrameColorInternal (const CColor& color)
{
	if (pPen)
		pPen->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFontColor (const CColor& color)
{
	if (currentState.fontColor == color)
		return;
	setFontColorInternal (color);
	COffscreenContext::setFontColor (color);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setFontColorInternal (const CColor& color)
{
	if (pFontBrush)
		pFontBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * currentState.globalAlpha), color.red, color.green, color.blue));
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::setGlobalAlpha (float newAlpha)
{
	if (currentState.globalAlpha == newAlpha)
		return;
	COffscreenContext::setGlobalAlpha (newAlpha);
	setFrameColorInternal (currentState.frameColor);
	setFillColorInternal (currentState.fillColor);
	setFontColorInternal (currentState.fontColor);
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::saveGlobalState ()
{
	COffscreenContext::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void GdiplusDrawContext::restoreGlobalState ()
{
	CColor prevFillColor = currentState.fillColor;
	CColor prevFrameColor = currentState.frameColor;
	CColor prevFontColor = currentState.fontColor;
	CLineStyle prevLineStye = currentState.lineStyle;
	CCoord prevFrameWidth = currentState.frameWidth;
	CDrawMode prevDrawMode = currentState.drawMode;
	float prevAlpha = currentState.globalAlpha;
	COffscreenContext::restoreGlobalState ();
	if (prevAlpha != currentState.globalAlpha)
	{
		float prevAlpha = currentState.globalAlpha;
		currentState.globalAlpha = -1.f;
		setGlobalAlpha (prevAlpha);
	}
	else
	{
		if (prevFillColor != currentState.fillColor)
		{
			setFillColorInternal (currentState.fillColor);
		}
		if (prevFrameColor != currentState.frameColor)
		{
			setFrameColorInternal (currentState.frameColor);
		}
		if (prevFontColor != currentState.fontColor)
		{
			setFontColorInternal (currentState.fontColor);
		}
	}
	if (prevLineStye != currentState.lineStyle)
	{
		setLineStyleInternal (currentState.lineStyle);
	}
	if (prevFrameWidth != currentState.frameWidth)
	{
		setLineWidthInternal (currentState.frameWidth);
	}
	if (prevDrawMode != currentState.drawMode)
	{
		setDrawModeInternal (currentState.drawMode);
	}
}

} // namespace

#endif // WINDOWS
