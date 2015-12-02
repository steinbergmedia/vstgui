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

#include "d2ddrawcontext.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../win32support.h"
#include "../../../cgradient.h"
#include "d2dbitmap.h"
#include "d2dgraphicspath.h"
#include "d2dfont.h"
#include <cassert>

namespace VSTGUI {

static D2D1_RENDER_TARGET_TYPE gRenderTargetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;

//-----------------------------------------------------------------------------
void useD2DHardwareRenderer (bool state)
{
	if (state)
		gRenderTargetType = D2D1_RENDER_TARGET_TYPE_HARDWARE;
	else
		gRenderTargetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DApplyClip::D2DApplyClip (D2DDrawContext* drawContext, bool halfPointOffset)
: drawContext (drawContext)
{
	if (drawContext->currentClip != drawContext->currentState.clipRect)
	{
		CRect clip = drawContext->currentState.clipRect;
		if (drawContext->getDrawMode ().integralMode ())
			clip.makeIntegral ();
		if (drawContext->currentClip.isEmpty () == false)
			drawContext->getRenderTarget ()->PopAxisAlignedClip ();
		if (clip.isEmpty () == false)
			drawContext->getRenderTarget ()->PushAxisAlignedClip (makeD2DRect (clip), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		drawContext->currentClip = applyClip = clip;
	}
	else
	{
		applyClip = drawContext->currentClip;
	}

	if (drawContext->getCurrentTransform ().isInvariant () == false)
	{
		CGraphicsTransform transform = drawContext->getCurrentTransform ();
		if (halfPointOffset)
			transform.translate (0.5, 0.5);
		drawContext->getRenderTarget ()->SetTransform (convert (transform));
	}
	else if (halfPointOffset)
		drawContext->getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Translation (0.5f, 0.5f));
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DApplyClip::~D2DApplyClip ()
{
	drawContext->getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Identity ());
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
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
		HRESULT hr = getD2DFactory ()->CreateHwndRenderTarget (D2D1::RenderTargetProperties (gRenderTargetType, pixelFormat), D2D1::HwndRenderTargetProperties (window, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS), &hwndRenderTarget);
		if (SUCCEEDED (hr))
		{
			UINT dpix = 96;
			UINT dpiy = 96;
			hwndRenderTarget->SetDpi (static_cast<FLOAT> (dpix), static_cast<FLOAT> (dpiy));
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
			getD2DFactory ()->CreateWicBitmapRenderTarget (d2dBitmap->getBitmap (), D2D1::RenderTargetProperties (targetType, pixelFormat), &renderTarget);
		}
	}
	vstgui_assert (renderTarget);
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
		if (currentClip.isEmpty () == false)
		{
			getRenderTarget ()->PopAxisAlignedClip ();
			currentClip = CRect ();
		}
		renderTarget->Flush ();
		HRESULT result = renderTarget->EndDraw ();
		if (result == D2DERR_RECREATE_TARGET)
		{
			releaseRenderTarget ();
			createRenderTarget ();
		}
		else
		{
			vstgui_assert (result == S_OK);
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
CGraphicsPath* D2DDrawContext::createTextPath (const CFontRef font, UTF8StringPtr text)
{
 	const D2DFont* ctFont = dynamic_cast<const D2DFont*>(font->getPlatformFont ());
 	return ctFont ? new D2DGraphicsPath (ctFont, text) : 0;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	if (renderTarget == 0)
		return;

	bool halfPointOffset = (mode == kPathStroked || currentState.drawMode.integralMode ()) ? ((((int32_t)currentState.frameWidth) % 2) != 0) : false;
	D2DApplyClip ac (this, halfPointOffset);
	if (ac.isEmpty ())
		return;

	D2DGraphicsPath* d2dPath = dynamic_cast<D2DGraphicsPath*> (_path);
	if (d2dPath == 0)
		return;

	ID2D1Geometry* path = d2dPath->createPath (mode == kPathFilledEvenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING, currentState.drawMode.integralMode () ? this : 0, t);
	if (path)
	{
		if (mode == kPathFilled || mode == kPathFilledEvenOdd)
			getRenderTarget ()->FillGeometry (path, getFillBrush ());
		else if (mode == kPathStroked)
			getRenderTarget ()->DrawGeometry (path, getStrokeBrush (), (FLOAT)getLineWidth (), getStrokeStyle ());
		path->Release ();
	}
}

//-----------------------------------------------------------------------------
ID2D1GradientStopCollection* D2DDrawContext::createGradientStopCollection (const CGradient& d2dGradient) const
{
	ID2D1GradientStopCollection* collection = 0;
	D2D1_GRADIENT_STOP* gradientStops = new D2D1_GRADIENT_STOP [d2dGradient.getColorStops ().size ()];
	uint32_t index = 0;
	for (CGradient::ColorStopMap::const_iterator it = d2dGradient.getColorStops ().begin (); it != d2dGradient.getColorStops ().end (); ++it, ++index)
	{
		gradientStops[index].position = (FLOAT)it->first;
		gradientStops[index].color = D2D1::ColorF (it->second.red/255.f, it->second.green/255.f, it->second.blue/255.f, it->second.alpha/255.f * currentState.globalAlpha);
	}
	getRenderTarget ()->CreateGradientStopCollection (gradientStops, static_cast<UINT32> (d2dGradient.getColorStops ().size ()), &collection);
	delete [] gradientStops;
	return collection;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::fillLinearGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
	if (renderTarget == 0)
		return;

	bool halfPointOffset = currentState.drawMode.integralMode () ? ((((int32_t)currentState.frameWidth) % 2) != 0) : false;
	D2DApplyClip ac (this, halfPointOffset);
	if (ac.isEmpty ())
		return;
	
	D2DGraphicsPath* d2dPath = dynamic_cast<D2DGraphicsPath*> (_path);
	if (d2dPath == 0)
		return;

	ID2D1Geometry* path = d2dPath->createPath (evenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING, this, t);
	if (path)
	{

		ID2D1GradientStopCollection* collection = createGradientStopCollection (gradient);
		if (collection)
		{
			ID2D1LinearGradientBrush* brush = 0;
			D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties;
			properties.startPoint = makeD2DPoint (startPoint);
			properties.endPoint = makeD2DPoint (endPoint);
			if (SUCCEEDED (getRenderTarget ()->CreateLinearGradientBrush (properties, collection, &brush)))
			{
				getRenderTarget ()->FillGeometry (path, brush);
				brush->Release ();
			}
			collection->Release ();
		}
		path->Release ();
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::fillRadialGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset, bool evenOdd, CGraphicsTransform* t)
{
	if (renderTarget == 0)
		return;

	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	
	D2DGraphicsPath* d2dPath = dynamic_cast<D2DGraphicsPath*> (_path);
	if (d2dPath == 0)
		return;

	ID2D1Geometry* path = d2dPath->createPath (evenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
	if (path)
	{
		ID2D1Geometry* geometry = 0;
		if (t)
		{
			ID2D1TransformedGeometry* tg = 0;
			getD2DFactory ()->CreateTransformedGeometry (path, convert (*t), &tg);
			geometry = tg;
		}
		else
		{
			geometry = path;
			geometry->AddRef ();
		}
		ID2D1GradientStopCollection* collection = createGradientStopCollection (gradient);
		if (collection)
		{
			// brush properties
			ID2D1RadialGradientBrush* brush = 0;
			D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES properties;
			properties.center = makeD2DPoint (center);
			properties.gradientOriginOffset = makeD2DPoint (originOffset);
			properties.radiusX = (FLOAT)radius;
			properties.radiusY = (FLOAT)radius;

			if (SUCCEEDED (getRenderTarget ()->CreateRadialGradientBrush (properties, collection, &brush)))
			{
				getRenderTarget ()->FillGeometry (geometry, brush);
				brush->Release ();
			}
			collection->Release ();
		}
		geometry->Release ();
		path->Release ();
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
	if (renderTarget == 0)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	
	double transformedScaleFactor = getScaleFactor ();
	CGraphicsTransform t = getCurrentTransform ();
	if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
		transformedScaleFactor *= t.m11;
	IPlatformBitmap* platformBitmap = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor);
	D2DBitmap* d2dBitmap = platformBitmap ? dynamic_cast<D2DBitmap*> (platformBitmap) : 0;
	if (d2dBitmap)
	{
		if (d2dBitmap->getSource ())
		{
			ID2D1Bitmap* d2d1Bitmap = D2DBitmapCache::instance ()->getBitmap (d2dBitmap, renderTarget);
			if (d2d1Bitmap)
			{
				double bitmapScaleFactor = platformBitmap->getScaleFactor ();
				CGraphicsTransform bitmapTransform;
				bitmapTransform.scale (bitmapScaleFactor, bitmapScaleFactor);
				Transform transform (*this, bitmapTransform.inverse ());

				CRect d (dest);
				d.makeIntegral ();
				CRect source (dest);
				source.offset (-source.left, -source.top);
				source.offset (offset.x, offset.y);

				bitmapTransform.transform (source);

				D2D1_RECT_F sourceRect = makeD2DRect (source);
				renderTarget->DrawBitmap (d2d1Bitmap, makeD2DRect (d), alpha * currentState.globalAlpha, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRect);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLine (const LinePair& line)
{
	if (renderTarget == 0)
		return;
	D2DApplyClip ac (this, (((int32_t)currentState.frameWidth) % 2) != 0);
	if (ac.isEmpty ())
		return;
	
	if (renderTarget)
	{
		CPoint start (line.first);
		CPoint end (line.second);
		if (currentState.drawMode.integralMode ())
		{
			pixelAllign (start);
			pixelAllign (end);
		}

		renderTarget->DrawLine (makeD2DPoint (start), makeD2DPoint (end), strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLines (const LineList& lines)
{
	if (lines.size () == 0)
		return;
	VSTGUI_RANGE_BASED_FOR_LOOP(LineList, lines, LinePair, line)
		drawLine (line);
	VSTGUI_RANGE_BASED_FOR_LOOP_END
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
	if (renderTarget == 0 || polygonPointList.size () == 0)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;

	D2DGraphicsPath path;
	path.beginSubpath (polygonPointList[0]);
	for (uint32_t i = 1; i < polygonPointList.size (); ++i)
	{
		path.addLine (polygonPointList[i]);
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

//-----------------------------------------------------------------------------
void D2DDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	if (renderTarget == 0)
		return;
	CRect rect (_rect);
	if (currentState.drawMode.integralMode ())
		pixelAllign (rect);
	bool halfPointOffset = (((int32_t)currentState.frameWidth) % 2) != 0;
	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		D2DApplyClip ac (this);
		if (ac.isEmpty ())
			return;
		renderTarget->FillRectangle (makeD2DRect (rect), fillBrush);
	}
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
	{
		D2DApplyClip ac (this, halfPointOffset);
		if (ac.isEmpty ())
			return;
		rect.right -= 1.;
		rect.bottom -= 1.;
		renderTarget->DrawRectangle (makeD2DRect (rect), strokeBrush, (FLOAT)currentState.frameWidth, strokeStyle);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawArc (const CRect& _rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle)
{
	CGraphicsPath* path = createGraphicsPath ();
	if (path)
	{
		path->addArc (_rect, _startAngle, _endAngle, true);
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
	if (renderTarget == 0)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	CRect rect (_rect);
	if (currentState.drawMode.integralMode ())
		pixelAllign (rect);
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

//-----------------------------------------------------------------------------
void D2DDrawContext::drawPoint (const CPoint &point, const CColor& color)
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
void D2DDrawContext::setLineStyle (const CLineStyle& style)
{
	if (strokeStyle && currentState.lineStyle == style)
		return;
	setLineStyleInternal (style);
	COffscreenContext::setLineStyle (style);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setLineStyleInternal (const CLineStyle& style)
{
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
		for (uint32_t i = 0; i < style.getDashCount (); i++)
			lengths[i] = (FLOAT)style.getDashLengths ()[i];
		getD2DFactory ()->CreateStrokeStyle (properties, lengths, style.getDashCount (), &strokeStyle);
		delete [] lengths;
	}
	else
	{
		properties.dashStyle = D2D1_DASH_STYLE_SOLID;
		getD2DFactory ()->CreateStrokeStyle (properties, 0, 0, &strokeStyle);
	}
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
	if (currentState.drawMode == mode && currentState.drawMode.integralMode () == mode.integralMode ())
		return;
	setDrawModeInternal (mode);
	COffscreenContext::setDrawMode (mode);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setDrawModeInternal (CDrawMode mode)
{
	if (renderTarget)
	{
		if (mode == kAntiAliasing)
			renderTarget->SetAntialiasMode (D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		else
			renderTarget->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
	}
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
	setFillColorInternal (color);
	COffscreenContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFrameColor (const CColor& color)
{
	if (currentState.frameColor == color)
		return;
	setFrameColorInternal (color);
	COffscreenContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFontColor (const CColor& color)
{
	if (currentState.fontColor == color)
		return;
	setFontColorInternal (color);
	COffscreenContext::setFontColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFillColorInternal (const CColor& color)
{
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
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFrameColorInternal (const CColor& color)
{
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
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFontColorInternal (const CColor& color)
{
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
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setGlobalAlpha (float newAlpha)
{
	if (currentState.globalAlpha == newAlpha)
		return;
	COffscreenContext::setGlobalAlpha (newAlpha);
	setFrameColorInternal (currentState.frameColor);
	setFillColorInternal (currentState.fillColor);
	setFontColorInternal (currentState.fontColor);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::saveGlobalState ()
{
	COffscreenContext::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::restoreGlobalState ()
{
	CColor prevFillColor = currentState.fillColor;
	CColor prevFrameColor = currentState.frameColor;
	CColor prevFontColor = currentState.fontColor;
	CLineStyle prevLineStye = currentState.lineStyle;
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
	if (prevDrawMode != currentState.drawMode)
	{
		setDrawModeInternal (currentState.drawMode);
	}
}

} // namespace

#endif // WINDOWS
