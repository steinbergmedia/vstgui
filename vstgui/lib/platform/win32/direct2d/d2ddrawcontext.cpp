// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2ddrawcontext.h"

#if WINDOWS

#include "../win32support.h"
#include "../win32factory.h"
#include "../../../cgradient.h"
#include "d2dbitmap.h"
#include "d2dbitmapcache.h"
#include "d2dgraphicspath.h"
#include "d2dfont.h"
#include "d2dgradient.h"
#include <cassert>

namespace VSTGUI {

//-----------------------------------------------------------------------------
D2DDrawContext::D2DApplyClip::D2DApplyClip (D2DDrawContext* drawContext, bool halfPointOffset)
: drawContext (drawContext)
{
	CGraphicsTransform transform = drawContext->getCurrentTransform ();
	auto scale = drawContext->getScaleFactor ();
	transform.scale (scale, scale);
	if (halfPointOffset)
	{
		CPoint offset (0.5, 0.5);
		transform.translate (offset);
	}
	if (transform.m12 != 0. || transform.m21 != 0.)
	{ // we have a rotated matrix, we need to use a layer
		layerIsUsed = true;
		if (drawContext->currentClip.isEmpty () == false)
			drawContext->getRenderTarget ()->PopAxisAlignedClip ();

		ID2D1RectangleGeometry* geometry;
		if (FAILED (getD2DFactory ()->CreateRectangleGeometry (makeD2DRect (drawContext->getCurrentState ().clipRect), &geometry)))
			return;
		auto d2dMatrix = convert (transform);
		drawContext->getRenderTarget ()->PushLayer (
		    D2D1::LayerParameters (D2D1::InfiniteRect (), geometry,
		                           D2D1_ANTIALIAS_MODE_ALIASED),
		    nullptr);
		drawContext->getRenderTarget ()->SetTransform (d2dMatrix);
		geometry->Release ();
		applyClip = drawContext->getCurrentState ().clipRect;
		drawContext->currentClip = {};
	}
	else
	{
		if (drawContext->currentClip != drawContext->getCurrentState ().clipRect)
		{
			CRect clip = drawContext->getCurrentState ().clipRect;
			if (drawContext->currentClip.isEmpty () == false)
				drawContext->getRenderTarget ()->PopAxisAlignedClip ();
			if (clip.isEmpty () == false)
				drawContext->getRenderTarget ()->PushAxisAlignedClip (makeD2DRect (clip), D2D1_ANTIALIAS_MODE_ALIASED);
			drawContext->currentClip = applyClip = clip;
		}
		else
		{
			applyClip = drawContext->currentClip;
		}
		drawContext->getRenderTarget ()->SetTransform (convert (transform));
	}
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DApplyClip::~D2DApplyClip ()
{
	if (layerIsUsed)
	{
		drawContext->getRenderTarget ()->PopLayer ();
	}
	auto scale = drawContext->getScaleFactor ();
	CGraphicsTransform transform;
	transform.scale (scale, scale);
	drawContext->getRenderTarget ()->SetTransform (convert (transform));
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DDrawContext (HWND window, const CRect& drawSurface)
: COffscreenContext (drawSurface)
, window (window)
, renderTarget (nullptr)
, fillBrush (nullptr)
, strokeBrush (nullptr)
, fontBrush (nullptr)
, strokeStyle (nullptr)
{
	createRenderTarget ();
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DDrawContext (D2DBitmap* inBitmap)
: COffscreenContext (new CBitmap (inBitmap))
, window (nullptr)
, renderTarget (nullptr)
, fillBrush (nullptr)
, strokeBrush (nullptr)
, fontBrush (nullptr)
, strokeStyle (nullptr)
, scaleFactor (inBitmap->getScaleFactor ())
{
	createRenderTarget ();
	bitmap->forget ();
}

//-----------------------------------------------------------------------------
D2DDrawContext::D2DDrawContext (ID2D1DeviceContext* deviceContext, const CRect& drawSurface,
								ID2D1Device* device)
: COffscreenContext (drawSurface)
, window (nullptr)
, device (device)
, renderTarget (nullptr)
, fillBrush (nullptr)
, strokeBrush (nullptr)
, fontBrush (nullptr)
, strokeStyle (nullptr)
{
	renderTarget = deviceContext;
	renderTarget->AddRef ();
	init ();
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
		D2D1_RENDER_TARGET_TYPE renderTargetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
		if (auto pf = getPlatformFactory ().asWin32Factory ())
		{
			renderTargetType = pf->useD2DHardwareRenderer () ? D2D1_RENDER_TARGET_TYPE_HARDWARE :
			                                                   D2D1_RENDER_TARGET_TYPE_SOFTWARE;
		}
		RECT rc;
		GetClientRect (window, &rc);

		D2D1_SIZE_U size = D2D1::SizeU (static_cast<UINT32> (rc.right - rc.left), static_cast<UINT32> (rc.bottom - rc.top));
		ID2D1HwndRenderTarget* hwndRenderTarget = nullptr;
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
		HRESULT hr = getD2DFactory ()->CreateHwndRenderTarget (D2D1::RenderTargetProperties (renderTargetType, pixelFormat), D2D1::HwndRenderTargetProperties (window, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS), &hwndRenderTarget);
		if (SUCCEEDED (hr))
		{
			renderTarget = hwndRenderTarget;
			renderTarget->SetDpi (96, 96);
		}
	}
	else if (bitmap)
	{
		D2DBitmap* d2dBitmap = dynamic_cast<D2DBitmap*> (bitmap->getPlatformBitmap ().get ());
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
		fillBrush = nullptr;
	}
	if (strokeBrush)
	{
		strokeBrush->Release ();
		strokeBrush = nullptr;
	}
	if (fontBrush)
	{
		fontBrush->Release ();
		fontBrush = nullptr;
	}
	if (strokeStyle)
	{
		strokeStyle->Release ();
		strokeStyle = nullptr;
	}
	if (renderTarget)
	{
		if (!device)
			D2DBitmapCache::removeRenderTarget (renderTarget);
		renderTarget->Release ();
		renderTarget = nullptr;
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::beginDraw ()
{
	if (renderTarget)
	{
		auto scale = getScaleFactor ();
		CGraphicsTransform transform;
		transform.scale (scale, scale);
		renderTarget->BeginDraw ();
		renderTarget->SetTransform (convert (transform));
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
		if (result == (HRESULT)D2DERR_RECREATE_TARGET)
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
			D2DBitmap* d2dBitmap = dynamic_cast<D2DBitmap*> (bitmap->getPlatformBitmap ().get ());
			D2DBitmapCache::removeBitmap (d2dBitmap);
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
	return new CGraphicsPath (D2DGraphicsPathFactory::instance ());
}

//-----------------------------------------------------------------------------
CGraphicsPath* D2DDrawContext::createTextPath (const CFontRef font, UTF8StringPtr text)
{
	auto factory = D2DGraphicsPathFactory::instance ();
	if (auto path = factory->createTextPath (font->getPlatformFont (), text))
	{
		return new CGraphicsPath (D2DGraphicsPathFactory::instance (), std::move (path));
	}
 	return nullptr;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawGraphicsPath (CGraphicsPath* graphicsPath, PathDrawMode mode,
									   CGraphicsTransform* t)
{
	if (renderTarget == nullptr || graphicsPath == nullptr)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;

	auto d2dPath = dynamic_cast<D2DGraphicsPath*> (
		graphicsPath
			->getPlatformPath (mode == kPathFilledEvenOdd ? PlatformGraphicsPathFillMode::Alternate
														  : PlatformGraphicsPathFillMode::Winding)
			.get ());
	if (d2dPath == nullptr)
		return;

	ID2D1Geometry* path = nullptr;
	if (t)
		path = d2dPath->createTransformedGeometry (getD2DFactory (), *t);
	else
	{
		path = d2dPath->getPathGeometry ();
		path->AddRef ();
	}
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
ID2D1GradientStopCollection* D2DDrawContext::createGradientStopCollection (const CGradient& gradient) const
{
	if (auto d2dGradient = dynamic_cast<D2DGradient*> (gradient.getPlatformGradient ().get ()))
		return d2dGradient->create (getRenderTarget (), getCurrentState ().globalAlpha);
	return nullptr;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::fillLinearGradient (CGraphicsPath* graphicsPath, const CGradient& gradient,
										 const CPoint& startPoint, const CPoint& endPoint,
										 bool evenOdd, CGraphicsTransform* t)
{
	if (renderTarget == nullptr || graphicsPath == nullptr)
		return;

	D2DApplyClip ac (this, true);
	if (ac.isEmpty ())
		return;

	auto d2dPath = dynamic_cast<D2DGraphicsPath*> (
		graphicsPath
			->getPlatformPath (evenOdd ? PlatformGraphicsPathFillMode::Alternate
									   : PlatformGraphicsPathFillMode::Winding)
			.get ());
	if (d2dPath == nullptr)
		return;
	ID2D1Geometry* path = nullptr;
	if (t)
		path = d2dPath->createTransformedGeometry (getD2DFactory (), *t);
	else
	{
		path = d2dPath->getPathGeometry ();
		path->AddRef ();
	}

	if (path)
	{
		ID2D1GradientStopCollection* collection = createGradientStopCollection (gradient);
		if (collection)
		{
			ID2D1LinearGradientBrush* brush = nullptr;
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
void D2DDrawContext::fillRadialGradient (CGraphicsPath* graphicsPath, const CGradient& gradient,
										 const CPoint& center, CCoord radius,
										 const CPoint& originOffset, bool evenOdd,
										 CGraphicsTransform* t)
{
	if (renderTarget == nullptr || graphicsPath == nullptr)
		return;

	D2DApplyClip ac (this, true);
	if (ac.isEmpty ())
		return;

	auto d2dPath = dynamic_cast<D2DGraphicsPath*> (
		graphicsPath
			->getPlatformPath (evenOdd ? PlatformGraphicsPathFillMode::Alternate
									   : PlatformGraphicsPathFillMode::Winding)
			.get ());
	if (d2dPath == nullptr)
		return;

	ID2D1Geometry* path = nullptr;
	if (t)
		path = d2dPath->createTransformedGeometry (getD2DFactory (), *t);
	else
	{
		path = d2dPath->getPathGeometry ();
		path->AddRef ();
	}

	if (path)
	{
		if (ID2D1GradientStopCollection* collection = createGradientStopCollection (gradient))
		{
			// brush properties
			ID2D1RadialGradientBrush* brush = nullptr;
			D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES properties;
			properties.center = makeD2DPoint (center);
			properties.gradientOriginOffset = makeD2DPoint (originOffset);
			properties.radiusX = (FLOAT)radius;
			properties.radiusY = (FLOAT)radius;

			if (SUCCEEDED (getRenderTarget ()->CreateRadialGradientBrush (properties,
																			collection, &brush)))
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
void D2DDrawContext::clearRect (const CRect& rect)
{
	if (renderTarget)
	{
		CRect oldClip = getCurrentState ().clipRect;
		setClipRect (rect);
		D2DApplyClip ac (this);
		renderTarget->Clear (D2D1::ColorF (1.f, 1.f, 1.f, 0.f));
		setClipRect (oldClip);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset, float alpha)
{
	if (renderTarget == nullptr)
		return;
	ConcatClip concatClip (*this, dest);
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	
	double transformedScaleFactor = getScaleFactor ();
	CGraphicsTransform t = getCurrentTransform ();
	if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
		transformedScaleFactor *= t.m11;
	IPlatformBitmap* platformBitmap = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor);
	D2DBitmap* d2dBitmap = platformBitmap ? dynamic_cast<D2DBitmap*> (platformBitmap) : nullptr;
	if (d2dBitmap && d2dBitmap->getSource ())
	{
		if (auto d2d1Bitmap = D2DBitmapCache::getBitmap (d2dBitmap, renderTarget, device))
		{
			double bitmapScaleFactor = platformBitmap->getScaleFactor ();
			CGraphicsTransform bitmapTransform;
			bitmapTransform.scale (1./bitmapScaleFactor, 1./bitmapScaleFactor);
			Transform transform (*this, bitmapTransform);

			CRect d (dest);
			d.setWidth (bitmap->getWidth ());
			d.setHeight (bitmap->getHeight ());
			d.offset (-offset.x, -offset.y);
			d.makeIntegral ();
			CRect source;
			source.setWidth (d2d1Bitmap->GetSize ().width);
			source.setHeight (d2d1Bitmap->GetSize ().height);

			D2D1_BITMAP_INTERPOLATION_MODE mode;
			switch (getCurrentState ().bitmapQuality)
			{
				case BitmapInterpolationQuality::kLow:
					mode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
					break;

				case BitmapInterpolationQuality::kMedium:
				case BitmapInterpolationQuality::kHigh:
				default:
					mode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
					break;
			}

			D2D1_RECT_F sourceRect = makeD2DRect (source);
			renderTarget->DrawBitmap (d2d1Bitmap, makeD2DRect (d), alpha * getCurrentState ().globalAlpha, mode, &sourceRect);
		}
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLineInternal (CPoint start, CPoint end)
{
	renderTarget->DrawLine (makeD2DPoint (start), makeD2DPoint (end), strokeBrush, (FLOAT)getCurrentState ().frameWidth, strokeStyle);
}

//-----------------------------------------------------------------------------
bool D2DDrawContext::needsHalfPointOffset () const
{
	return static_cast<int32_t> (getCurrentState ().frameWidth) % 2 != 0;
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLine (const LinePair& line)
{
	if (renderTarget == nullptr)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	
	CPoint start (line.first);
	CPoint end (line.second);
	if (getDrawMode ().integralMode ())
	{
		pixelAllign (start);
		pixelAllign (end);
	}
	if (needsHalfPointOffset ())
	{
		start.offset (0.5, 0.5);
		end.offset (0.5, 0.5);
	}
	drawLineInternal (start, end);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawLines (const LineList& lines)
{
	if (lines.empty () || renderTarget == nullptr)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;

	bool needsOffset = needsHalfPointOffset ();
	bool integralMode = getDrawMode ().integralMode ();
	for (const auto& line : lines)
	{
		CPoint start (line.first);
		CPoint end (line.second);
		if (integralMode)
		{
			pixelAllign (start);
			pixelAllign (end);
		}
		if (needsOffset)
		{
			start.offset (0.5, 0.5);
			end.offset (0.5, 0.5);
		}
		drawLineInternal (start, end);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
	if (renderTarget == nullptr || polygonPointList.empty ())
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;

	auto path = owned (createGraphicsPath ());
	path->beginSubpath (polygonPointList[0]);
	for (uint32_t i = 1; i < polygonPointList.size (); ++i)
	{
		path->addLine (polygonPointList[i]);
	}
	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		drawGraphicsPath (path, kPathFilled);
	}
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
	{
		drawGraphicsPath (path, kPathStroked);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	if (renderTarget == nullptr)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	CRect rect (_rect);
	if (drawStyle != kDrawFilled)
	{
		rect.right -= 1.;
		rect.bottom -= 1.;
	}
	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		renderTarget->FillRectangle (makeD2DRect (rect), fillBrush);
	}
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
	{
		if (needsHalfPointOffset ())
		{
			rect.offset (0.5, 0.5);
		}
		renderTarget->DrawRectangle (makeD2DRect (rect), strokeBrush, (FLOAT)getCurrentState ().frameWidth, strokeStyle);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawArc (const CRect& _rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle)
{
	if (auto path = owned (createGraphicsPath ()))
	{
		CRect rect (_rect);
		if (getDrawMode ().integralMode ())
			pixelAllign (rect);
		path->addArc (rect, _startAngle, _endAngle, true);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
			drawGraphicsPath (path, kPathFilled);
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
			drawGraphicsPath (path, kPathStroked);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::drawEllipse (const CRect &_rect, const CDrawStyle drawStyle)
{
	if (renderTarget == nullptr)
		return;
	D2DApplyClip ac (this);
	if (ac.isEmpty ())
		return;
	CRect rect (_rect);
	if (getDrawMode ().integralMode ())
		pixelAllign (rect);
	if (drawStyle == kDrawStroked)
		rect.inset (0.5, 0.5);
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
		renderTarget->DrawEllipse (ellipse, strokeBrush, (FLOAT)getCurrentState ().frameWidth, strokeStyle);
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
	if (strokeStyle && getCurrentState ().lineStyle == style)
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
		strokeStyle = nullptr;
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
		getD2DFactory ()->CreateStrokeStyle (properties, nullptr, 0, &strokeStyle);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setLineWidth (CCoord width)
{
	if (getCurrentState ().frameWidth == width)
		return;
	COffscreenContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setDrawMode (CDrawMode mode)
{
	if (getCurrentState ().drawMode == mode && getCurrentState ().drawMode.integralMode () == mode.integralMode ())
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
	if (getCurrentState ().fillColor == color)
		return;
	setFillColorInternal (color);
	COffscreenContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFrameColor (const CColor& color)
{
	if (getCurrentState ().frameColor == color)
		return;
	setFrameColorInternal (color);
	COffscreenContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFontColor (const CColor& color)
{
	if (getCurrentState ().fontColor == color)
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
		fillBrush = nullptr;
	}
	if (renderTarget)
	{
		renderTarget->CreateSolidColorBrush (toColorF (color, getCurrentState ().globalAlpha),
		                                     &fillBrush);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFrameColorInternal (const CColor& color)
{
	if (strokeBrush)
	{
		strokeBrush->Release ();
		strokeBrush = nullptr;
	}
	if (renderTarget)
	{
		renderTarget->CreateSolidColorBrush (toColorF (color, getCurrentState ().globalAlpha),
		                                     &strokeBrush);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setFontColorInternal (const CColor& color)
{
	if (fontBrush)
	{
		fontBrush->Release ();
		fontBrush = nullptr;
	}
	if (renderTarget)
	{
		renderTarget->CreateSolidColorBrush (toColorF (color, getCurrentState ().globalAlpha),
		                                     &fontBrush);
	}
}

//-----------------------------------------------------------------------------
void D2DDrawContext::setGlobalAlpha (float newAlpha)
{
	if (getCurrentState ().globalAlpha == newAlpha)
		return;
	COffscreenContext::setGlobalAlpha (newAlpha);
	setFrameColorInternal (getCurrentState ().frameColor);
	setFillColorInternal (getCurrentState ().fillColor);
	setFontColorInternal (getCurrentState ().fontColor);
}

//-----------------------------------------------------------------------------
void D2DDrawContext::saveGlobalState ()
{
	COffscreenContext::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void D2DDrawContext::restoreGlobalState ()
{
	CColor prevFillColor = getCurrentState ().fillColor;
	CColor prevFrameColor = getCurrentState ().frameColor;
	CColor prevFontColor = getCurrentState ().fontColor;
	CLineStyle prevLineStye = getCurrentState ().lineStyle;
	CDrawMode prevDrawMode = getCurrentState ().drawMode;
	float prevAlpha = getCurrentState ().globalAlpha;
	COffscreenContext::restoreGlobalState ();
	if (prevAlpha != getCurrentState ().globalAlpha)
	{
		float _prevAlpha = getCurrentState ().globalAlpha;
		getCurrentState ().globalAlpha = -1.f;
		setGlobalAlpha (_prevAlpha);
	}
	else
	{
		if (prevFillColor != getCurrentState ().fillColor)
		{
			setFillColorInternal (getCurrentState ().fillColor);
		}
		if (prevFrameColor != getCurrentState ().frameColor)
		{
			setFrameColorInternal (getCurrentState ().frameColor);
		}
		if (prevFontColor != getCurrentState ().fontColor)
		{
			setFontColorInternal (getCurrentState ().fontColor);
		}
	}
	if (prevLineStye != getCurrentState ().lineStyle)
	{
		setLineStyleInternal (getCurrentState ().lineStyle);
	}
	if (prevDrawMode != getCurrentState ().drawMode)
	{
		setDrawModeInternal (getCurrentState ().drawMode);
	}
}

} // VSTGUI

#endif // WINDOWS
