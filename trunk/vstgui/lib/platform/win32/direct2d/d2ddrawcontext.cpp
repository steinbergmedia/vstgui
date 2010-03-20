//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
D2DDrawContext::D2DDrawContext (D2DOffscreenBitmap* inBitmap)
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
			renderTarget = hwndRenderTarget;
		}
	}
	else if (bitmap)
	{
		D2DOffscreenBitmap* d2dBitmap = dynamic_cast<D2DOffscreenBitmap*> (bitmap->getPlatformBitmap ());
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
		HRESULT result = renderTarget->EndDraw ();
		if (result == D2DERR_RECREATE_TARGET)
		{
			releaseRenderTarget ();
			createRenderTarget ();
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::init ()
{
	COffscreenContext::init ();
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
	CPoint p (point);
	p.offset (currentState.offset.x, currentState.offset.y);
	if (renderTarget)
	{
		D2DApplyClip clip (this);
		if ((((int)currentState.frameWidth) % 2))
			renderTarget->SetTransform (D2D1::Matrix3x2F::Translation (0.f, 0.5f));
		renderTarget->DrawLine (makeD2DPoint (currentState.penLoc), makeD2DPoint (p), strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
		renderTarget->SetTransform (D2D1::Matrix3x2F::Identity ());
	}
	currentState.penLoc = p;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLines (const CPoint* points, const long& numberOfLines)
{
	for (long i = 0; i < numberOfLines * 2; i+=2)
	{
		moveTo (points[i]);
		lineTo (points[i+1]);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle)
{
	if (renderTarget)
	{
		D2DGraphicsPath path;
		path.addLine (pPoints[0], pPoints[1]);
		for (long i = 2; i < numberOfPoints; i++)
		{
			path.addLine (path.getCurrentPosition (), pPoints[i]);
		}
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		{
			path.draw (this, CGraphicsPath::kFilled);
		}
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		{
			path.draw (this, CGraphicsPath::kStroked);
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
		D2DApplyClip clip (this);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		{
			renderTarget->FillRectangle (makeD2DRect (rect), fillBrush);
		}
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		{
			renderTarget->DrawRectangle (makeD2DRect (rect), strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawArc (const CRect& _rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle)
{
	if (renderTarget)
	{
		CRect rect (_rect);
		rect.offset (currentState.offset.x, currentState.offset.y);

		// TODO: drawArc
#if 0
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
#endif
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
void D2DDrawContext::drawPoint (const CPoint &point, CColor color)
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
void D2DDrawContext::setLineStyle (CLineStyle style)
{
	if (currentState.lineStyle == style)
		return;
	if (strokeStyle)
	{
		strokeStyle->Release ();
		strokeStyle = 0;
	}
	D2D1_STROKE_STYLE_PROPERTIES properties;
	if (style != kLineSolid)
	{
		properties.startCap = D2D1_CAP_STYLE_FLAT;
		properties.endCap = D2D1_CAP_STYLE_FLAT;
		properties.dashCap = D2D1_CAP_STYLE_SQUARE;
		properties.lineJoin = D2D1_LINE_JOIN_MITER;
		properties.miterLimit = 10.f;
		properties.dashStyle = D2D1_DASH_STYLE_DOT;
		properties.dashOffset = 0.f;
		getD2DFactory ()->CreateStrokeStyle (properties, 0, 0, &strokeStyle);
	}
	COffscreenContext::setLineStyle (style);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setLineWidth (CCoord width)
{
	if (currentState.frameWidth == width)
		return;
	COffscreenContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setDrawMode (CDrawMode mode)
{
	if (currentState.drawMode == mode)
		return;
	if (renderTarget)
	{
		if (mode == kAntiAliasing)
			renderTarget->SetAntialiasMode (D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		else
			renderTarget->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
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
void D2DDrawContext::setFillColor (const CColor color)
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
void D2DDrawContext::setFrameColor (const CColor color)
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
void D2DDrawContext::setFontColor (const CColor color)
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
