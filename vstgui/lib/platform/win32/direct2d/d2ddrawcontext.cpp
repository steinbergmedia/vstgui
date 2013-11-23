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

#include "d2ddrawcontext.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../win32support.h"
#include "d2dbitmap.h"
#include "d2dgraphicspath.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
D2DDrawContext::D2DApplyClip::D2DApplyClip (D2DDrawContext* drawContext)
: drawContext (drawContext)
{
	drawContext->getRenderTarget ()->PushAxisAlignedClip (makeD2DRect (drawContext->currentState.clipRect), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DApplyClip::~D2DApplyClip ()
{
	drawContext->getRenderTarget ()->PopAxisAlignedClip ();
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DDrawContext (HWND window, const CRect& drawSurface)
: COffscreenContext (drawSurface)
, window (window)
, renderTarget (0)
, fillBrush (0)
, strokeBrush (0)
, fontBrush (0)
, strokeStyle (0)
{
	createRenderTarget ();
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DDrawContext (D2DBitmap* inBitmap)
: COffscreenContext (new CBitmap (inBitmap))
, window (0)
, renderTarget (0)
, fillBrush (0)
, strokeBrush (0)
, fontBrush (0)
, strokeStyle (0)
{
	createRenderTarget ();
	bitmap->forget ();
}

//-----------------------------------------------------------------------------
D2DDrawContext::~D2DDrawContext ()
{
	releaseRenderTarget ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::createRenderTarget ()
{
	if (window)
	{
		RECT rc;
		GetClientRect (window, &rc);

		D2D1_SIZE_U size = D2D1::SizeU (rc.right - rc.left, rc.bottom - rc.top);
		ID2D1HwndRenderTarget* hwndRenderTarget = 0;
//		D2D1_RENDER_TARGET_TYPE targetType = D2D1_RENDER_TARGET_TYPE_DEFAULT;
		D2D1_RENDER_TARGET_TYPE targetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
		HRESULT hr = getD2DFactory ()->CreateHwndRenderTarget (D2D1::RenderTargetProperties (targetType, pixelFormat), D2D1::HwndRenderTargetProperties (window, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS), &hwndRenderTarget);
		if (SUCCEEDED (hr))
		{
			hwndRenderTarget->SetDpi (96.0, 96.0);
			renderTarget = hwndRenderTarget;
		}
	}
	else if (bitmap)
	{
		D2DBitmap* d2dBitmap = dynamic_cast<D2DBitmap*> (bitmap->getPlatformBitmap ());
		if (d2dBitmap)
		{
			D2D1_RENDER_TARGET_TYPE targetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
			D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
			HRESULT hr = getD2DFactory ()->CreateWicBitmapRenderTarget (d2dBitmap->getBitmap (), D2D1::RenderTargetProperties (targetType, pixelFormat), &renderTarget);
		}
	}
	init ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::releaseRenderTarget ()
{
	if (fillBrush)
	{
		fillBrush->Release ();
		fillBrush = 0;
	}
	if (strokeBrush)
	{
		strokeBrush->Release ();
		strokeBrush = 0;
	}
	if (fontBrush)
	{
		fontBrush->Release ();
		fontBrush = 0;
	}
	if (strokeStyle)
	{
		strokeStyle->Release ();
		strokeStyle = 0;
	}
	if (renderTarget)
	{
		D2DBitmapCache::instance ()->removeRenderTarget (renderTarget);
		renderTarget->Release ();
		renderTarget = 0;
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::beginDraw ()
{
	if (renderTarget)
	{
		renderTarget->BeginDraw ();
		renderTarget->SetTransform (D2D1::Matrix3x2F::Identity ());
 	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::endDraw ()
{
	if (renderTarget)
	{
		renderTarget->Flush ();
		HRESULT result = renderTarget->EndDraw ();
		if (result == D2DERR_RECREATE_TARGET)
		{
			releaseRenderTarget ();
			createRenderTarget ();
		}
		if (bitmap)
		{
			D2DBitmap* d2dBitmap = dynamic_cast<D2DBitmap*> (bitmap->getPlatformBitmap ());
			D2DBitmapCache::instance ()->removeBitmap (d2dBitmap);
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::init ()
{
	COffscreenContext::init ();
}

//-----------------------------------------------------------------------------
CGraphicsPath* D2DDrawContext::createGraphicsPath ()
{
	return new D2DGraphicsPath ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	if (renderTarget == 0)
		return;

	D2DGraphicsPath* d2dPath = dynamic_cast<D2DGraphicsPath*> (_path);
	if (d2dPath == 0)
		return;

	ID2D1PathGeometry* path = d2dPath->getPath (mode == kPathFilledEvenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
	if (path)
	{
		D2DApplyClip ac (this);

		ID2D1Geometry* geometry = 0;
		if (t)
		{
			ID2D1TransformedGeometry* tg = 0;
			D2D1_MATRIX_3X2_F matrix;
			matrix._11 = (FLOAT)t->m11;
			matrix._12 = (FLOAT)t->m12;
			matrix._21 = (FLOAT)t->m21;
			matrix._22 = (FLOAT)t->m22;
			matrix._31 = (FLOAT)t->dx;
			matrix._32 = (FLOAT)t->dy;
			getD2DFactory ()->CreateTransformedGeometry (path, matrix, &tg);
			geometry = tg;
		}
		else
		{
			geometry = path;
			geometry->AddRef ();
		}

		getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Translation ((FLOAT)getOffset ().x, (FLOAT)getOffset ().y));

		if (mode == kPathFilled || mode == kPathFilledEvenOdd)
			getRenderTarget ()->FillGeometry (geometry, getFillBrush ());
		else if (mode == kPathStroked)
			getRenderTarget ()->DrawGeometry (geometry, getStrokeBrush (), (FLOAT)getLineWidth (), getStrokeStyle ());

		getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Identity ());

		geometry->Release ();
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::fillLinearGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
	if (renderTarget == 0)
		return;

	D2DGraphicsPath* d2dPath = dynamic_cast<D2DGraphicsPath*> (_path);
	if (d2dPath == 0)
		return;

	ID2D1PathGeometry* path = d2dPath->getPath (evenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
	if (path)
	{
		D2DApplyClip ac (this);

		ID2D1Geometry* geometry = 0;
		if (t)
		{
			ID2D1TransformedGeometry* tg = 0;
			D2D1_MATRIX_3X2_F matrix;
			matrix._11 = (FLOAT)t->m11;
			matrix._12 = (FLOAT)t->m12;
			matrix._21 = (FLOAT)t->m21;
			matrix._22 = (FLOAT)t->m22;
			matrix._31 = (FLOAT)t->dx;
			matrix._32 = (FLOAT)t->dy;
			getD2DFactory ()->CreateTransformedGeometry (path, matrix, &tg);
			geometry = tg;
		}
		else
		{
			geometry = path;
			geometry->AddRef ();
		}

		ID2D1GradientStopCollection* collection = 0;
		D2D1_GRADIENT_STOP gradientStops[2];
		gradientStops[0].position = (FLOAT)gradient.getColor1Start ();
		gradientStops[1].position = (FLOAT)gradient.getColor2Start ();
		gradientStops[0].color = D2D1::ColorF (gradient.getColor1 ().red/255.f, gradient.getColor1 ().green/255.f, gradient.getColor1 ().blue/255.f, gradient.getColor1 ().alpha/255.f * currentState.globalAlpha);
		gradientStops[1].color = D2D1::ColorF (gradient.getColor2 ().red/255.f, gradient.getColor2 ().green/255.f, gradient.getColor2 ().blue/255.f, gradient.getColor2 ().alpha/255.f * currentState.globalAlpha);

		if (SUCCEEDED (getRenderTarget ()->CreateGradientStopCollection (gradientStops, 2, &collection)))
		{
			ID2D1LinearGradientBrush* brush = 0;
			D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties;
			properties.startPoint = makeD2DPoint (startPoint);
			properties.endPoint = makeD2DPoint (endPoint);
			if (SUCCEEDED (getRenderTarget ()->CreateLinearGradientBrush (properties, collection, &brush)))
			{
				getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Translation ((FLOAT)getOffset ().x, (FLOAT)getOffset ().y));
				getRenderTarget ()->FillGeometry (geometry, brush);
				getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Identity ());
				brush->Release ();
			}
			collection->Release ();
		}

		geometry->Release ();
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::clearRect (const CRect& rect)
{
	if (renderTarget)
	{
		CRect oldClip = currentState.clipRect;
		setClipRect (rect);
		D2DApplyClip ac (this);
		renderTarget->Clear (D2D1::ColorF (1.f, 1.f, 1.f, 0.f));
		setClipRect (oldClip);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset, float alpha)
{
	D2DBitmap* d2dBitmap = bitmap->getPlatformBitmap () ? dynamic_cast<D2DBitmap*> (bitmap->getPlatformBitmap ()) : 0;
	if (renderTarget && d2dBitmap)
	{
		if (d2dBitmap->getSource ())
		{
			ID2D1Bitmap* d2d1Bitmap = D2DBitmapCache::instance ()->getBitmap (d2dBitmap, renderTarget);
			if (d2d1Bitmap)
			{
				D2DApplyClip clip (this);
				CRect d (dest);
				d.offset (currentState.offset.x, currentState.offset.y);
				d.makeIntegral ();
				CRect source (dest);
				source.offset (-source.left, -source.top);
				source.offset (offset.x, offset.y);
				D2D1_RECT_F sourceRect = makeD2DRect (source);
				renderTarget->DrawBitmap (d2d1Bitmap, makeD2DRect (d), alpha * currentState.globalAlpha, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRect);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::moveTo (const CPoint &point)
{
	CPoint p (point);
	p.offset (currentState.offset.x, currentState.offset.y);
	COffscreenContext::moveTo (p);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::lineTo (const CPoint &point)
{
	CPoint penLocation (currentState.penLoc);
	CPoint p (point);
	p.offset (currentState.offset.x, currentState.offset.y);
	if (currentState.drawMode.integralMode ())
	{
		p.makeIntegral ();
		penLocation.makeIntegral ();
	}

	if (renderTarget)
	{
		D2DApplyClip clip (this);
		if ((((int32_t)currentState.frameWidth) % 2))
			renderTarget->SetTransform (D2D1::Matrix3x2F::Translation (0.5f, 0.5f));
		renderTarget->DrawLine (makeD2DPoint (penLocation), makeD2DPoint (p), strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
		renderTarget->SetTransform (D2D1::Matrix3x2F::Identity ());
	}
	currentState.penLoc = p;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLines (const CPoint* points, const int32_t& numberOfLines)
{
	for (int32_t i = 0; i < numberOfLines * 2; i+=2)
	{
		moveTo (points[i]);
		lineTo (points[i+1]);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawPolygon (const CPoint *pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle)
{
	if (renderTarget)
	{
		D2DApplyClip clip (this);
		D2DGraphicsPath path;
		path.beginSubpath (pPoints[0]);
		path.addLine (pPoints[1]);
		for (int32_t i = 2; i < numberOfPoints; i++)
		{
			path.addLine (pPoints[i]);
		}
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		{
			drawGraphicsPath (&path, kPathFilled);
		}
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		{
			drawGraphicsPath (&path, kPathStroked);
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	if (renderTarget)
	{
		CRect rect (_rect);
		rect.offset (currentState.offset.x, currentState.offset.y);
		rect.normalize ();
		if (currentState.drawMode.integralMode ())
		{
			rect.makeIntegral ();
		}
		D2DApplyClip clip (this);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		{
			renderTarget->FillRectangle (makeD2DRect (rect), fillBrush);
		}
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		{
			rect.left++;
			rect.bottom--;
			if ((((int32_t)currentState.frameWidth) % 2))
				renderTarget->SetTransform (D2D1::Matrix3x2F::Translation (0.f, 0.5f));
			renderTarget->DrawRectangle (makeD2DRect (rect), strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
			renderTarget->SetTransform (D2D1::Matrix3x2F::Identity ());
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawArc (const CRect& _rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle)
{
	CGraphicsPath* path = createGraphicsPath ();
	if (path)
	{
		path->addArc (_rect, _startAngle, _endAngle, false);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
			drawGraphicsPath (path, kPathFilled);
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
			drawGraphicsPath (path, kPathStroked);
		path->forget ();
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawEllipse (const CRect &_rect, const CDrawStyle drawStyle)
{
	if (renderTarget)
	{
		CRect rect (_rect);
		rect.offset (currentState.offset.x, currentState.offset.y);
		rect.normalize ();
		D2DApplyClip clip (this);
		CPoint center (rect.getTopLeft ());
		center.offset (rect.getWidth () / 2., rect.getHeight () / 2.);
		D2D1_ELLIPSE ellipse;
		ellipse.point = makeD2DPoint (center);
		ellipse.radiusX = (FLOAT)(rect.getWidth () / 2.);
		ellipse.radiusY = (FLOAT)(rect.getHeight () / 2.);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		{
			renderTarget->FillEllipse (ellipse, fillBrush);
		}
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		{
			renderTarget->DrawEllipse (ellipse, strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawPoint (const CPoint &point, const CColor& color)
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
void D2DDrawContext::setLineStyle (const CLineStyle& style)
{
	if (strokeStyle && currentState.lineStyle == style)
		return;
	if (strokeStyle)
	{
		strokeStyle->Release ();
		strokeStyle = 0;
	}
	D2D1_STROKE_STYLE_PROPERTIES properties;
	switch (style.getLineCap ())
	{
		case CLineStyle::kLineCapButt: properties.startCap = properties.endCap = properties.dashCap = D2D1_CAP_STYLE_FLAT; break;
		case CLineStyle::kLineCapRound: properties.startCap = properties.endCap = properties.dashCap = D2D1_CAP_STYLE_ROUND; break;
		case CLineStyle::kLineCapSquare: properties.startCap = properties.endCap = properties.dashCap = D2D1_CAP_STYLE_SQUARE; break;
	}
	switch (style.getLineJoin ())
	{
		case CLineStyle::kLineJoinMiter: properties.lineJoin = D2D1_LINE_JOIN_MITER; break;
		case CLineStyle::kLineJoinRound: properties.lineJoin = D2D1_LINE_JOIN_ROUND; break;
		case CLineStyle::kLineJoinBevel: properties.lineJoin = D2D1_LINE_JOIN_BEVEL; break;
	}
	properties.dashOffset = (FLOAT)style.getDashPhase ();
	properties.miterLimit = 10.f;
	if (style.getDashCount ())
	{
		properties.dashStyle = D2D1_DASH_STYLE_CUSTOM;
		FLOAT* lengths = new FLOAT[style.getDashCount ()];
		for (int32_t i = 0; i < style.getDashCount (); i++)
			lengths[i] = (FLOAT)style.getDashLengths ()[i];
		getD2DFactory ()->CreateStrokeStyle (properties, lengths, style.getDashCount (), &strokeStyle);
		delete [] lengths;
	}
	else
	{
		properties.dashStyle = D2D1_DASH_STYLE_SOLID;
		getD2DFactory ()->CreateStrokeStyle (properties, 0, 0, &strokeStyle);
	}
	COffscreenContext::setLineStyle (style);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setLineWidth (CCoord width)
{
	if (currentState.frameWidth == width)
		return;
	if (strokeStyle)
	{
		strokeStyle->Release ();
		strokeStyle = 0;
	}
	COffscreenContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setDrawMode (CDrawMode mode)
{
	if (currentState.drawMode != mode)
	{
		if (renderTarget)
		{
			if (mode == kAntiAliasing)
				renderTarget->SetAntialiasMode (D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			else
				renderTarget->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
		}
	}
	COffscreenContext::setDrawMode (mode);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setClipRect (const CRect &clip)
{
	COffscreenContext::setClipRect (clip);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::resetClipRect ()
{
	COffscreenContext::resetClipRect ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFillColor (const CColor& color)
{
	if (currentState.fillColor == color)
		return;
	if (fillBrush)
	{
		fillBrush->Release ();
		fillBrush = 0;
	}
	if (renderTarget)
	{
		D2D1_COLOR_F d2Color = {color.red/255.f, color.green/255.f, color.blue/255.f, (color.alpha/255.f) * currentState.globalAlpha};
		renderTarget->CreateSolidColorBrush (d2Color, &fillBrush);
	}
	COffscreenContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFrameColor (const CColor& color)
{
	if (currentState.frameColor == color)
		return;
	if (strokeBrush)
	{
		strokeBrush->Release ();
		strokeBrush = 0;
	}
	if (renderTarget)
	{
		D2D1_COLOR_F d2Color = {color.red/255.f, color.green/255.f, color.blue/255.f, (color.alpha/255.f) * currentState.globalAlpha};
		renderTarget->CreateSolidColorBrush (d2Color, &strokeBrush);
	}
	COffscreenContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFontColor (const CColor& color)
{
	if (currentState.fontColor == color)
		return;
	if (fontBrush)
	{
		fontBrush->Release ();
		fontBrush = 0;
	}
	if (renderTarget)
	{
		D2D1_COLOR_F d2Color = {color.red/255.f, color.green/255.f, color.blue/255.f, (color.alpha/255.f) * currentState.globalAlpha};
		renderTarget->CreateSolidColorBrush (d2Color, &fontBrush);
	}
	COffscreenContext::setFontColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setGlobalAlpha (float newAlpha)
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
void D2DDrawContext::saveGlobalState ()
{
	COffscreenContext::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::restoreGlobalState ()
{
	COffscreenContext::restoreGlobalState ();
}

} // namespace

#endif // WINDOWS
