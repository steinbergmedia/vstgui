// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dgraphicscontext.h"
#include "d2dgradient.h"
#include "d2dgraphicspath.h"
#include "d2dbitmap.h"
#include "d2dbitmapcache.h"
#include "d2dgradient.h"
#include "../comptr.h"
#include "../../../crect.h"
#include "../../../cgraphicstransform.h"
#include "../../../ccolor.h"
#include "../../../cdrawdefs.h"
#include "../../../clinestyle.h"
#include <stack>

#include <d2d1_1.h>
#include <dwrite.h>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct D2DGraphicsDevice::Impl
{
	COM::Ptr<ID2D1Device> device;
};

//------------------------------------------------------------------------
D2DGraphicsDevice::D2DGraphicsDevice (ID2D1Device* device)
{
	impl = std::make_unique<Impl> ();
	impl->device = COM::share<ID2D1Device> (device);
}

//------------------------------------------------------------------------
D2DGraphicsDevice::~D2DGraphicsDevice () noexcept {}

//------------------------------------------------------------------------
PlatformGraphicsDeviceContextPtr
	D2DGraphicsDevice::createBitmapContext (const PlatformBitmapPtr& bitmap) const
{
	return nullptr;
}

//------------------------------------------------------------------------
ID2D1Device* D2DGraphicsDevice::get () const { return impl->device.get (); }

//-----------------------------------------------------------------------------
template<typename T>
void pixelAlign (const TransformMatrix& tm, T& obj)
{
	CGraphicsTransform tInv = tm.inverse ();
	tm.transform (obj);
	obj.makeIntegral ();
	tInv.transform (obj);
}

//------------------------------------------------------------------------
inline D2D1_POINT_2F convert (const CPoint& p)
{
	D2D1_POINT_2F dp = {(FLOAT)p.x, (FLOAT)p.y};
	return dp;
}

//------------------------------------------------------------------------
inline D2D1::ColorF convert (CColor c, double alpha)
{
	return D2D1::ColorF (c.normRed<float> (), c.normGreen<float> (), c.normBlue<float> (),
						 static_cast<FLOAT> (c.normAlpha<double> () * alpha));
}

//------------------------------------------------------------------------
inline D2D1_RECT_F convert (const CRect& r)
{
	D2D1_RECT_F dr = {(FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom};
	return dr;
}

//------------------------------------------------------------------------
inline D2D1_MATRIX_3X2_F convert (const TransformMatrix& t)
{
	D2D1_MATRIX_3X2_F matrix;
	matrix._11 = static_cast<FLOAT> (t.m11);
	matrix._12 = static_cast<FLOAT> (t.m21);
	matrix._21 = static_cast<FLOAT> (t.m12);
	matrix._22 = static_cast<FLOAT> (t.m22);
	matrix._31 = static_cast<FLOAT> (t.dx);
	matrix._32 = static_cast<FLOAT> (t.dy);
	return matrix;
}

//------------------------------------------------------------------------
inline TransformMatrix convert (const D2D1_MATRIX_3X2_F& t)
{
	TransformMatrix matrix;
	matrix.m11 = static_cast<FLOAT> (t._11);
	matrix.m21 = static_cast<FLOAT> (t._12);
	matrix.m12 = static_cast<FLOAT> (t._21);
	matrix.m22 = static_cast<FLOAT> (t._22);
	matrix.dx = static_cast<FLOAT> (t._31);
	matrix.dy = static_cast<FLOAT> (t._32);
	return matrix;
}

//------------------------------------------------------------------------
struct TransformGuard
{
	TransformGuard (ID2D1DeviceContext* context) : context (context)
	{
		context->GetTransform (&matrix);
	}
	~TransformGuard () noexcept { context->SetTransform (matrix); }

	D2D1_MATRIX_3X2_F matrix;
	ID2D1DeviceContext* context;
};

//------------------------------------------------------------------------
struct D2DGraphicsDeviceContext::Impl
{
	Impl (const D2DGraphicsDevice& device, ID2D1DeviceContext* deviceContext)
	: device (device), deviceContext (COM::share (deviceContext))
	{
	}

	template<typename Proc>
	void doInContext (Proc p, CPoint transformOffset = {})
	{
		if (state.clip.isEmpty ())
			return;

		TransformGuard tmGuard (deviceContext.get ());

		auto transform = convert (tmGuard.matrix) * state.tm;
		transform.scale (scaleFactor, scaleFactor);
		transform.translate (transformOffset);
		bool useLayer = transform.m12 != 0. || transform.m21 != 0.;
		if (useLayer)
		{ // we have a rotated matrix, we need to use a layer
			ID2D1Factory* factory {};
			deviceContext->GetFactory (&factory);
			COM::Ptr<ID2D1RectangleGeometry> geometry;
			if (SUCCEEDED (
					factory->CreateRectangleGeometry (convert (state.clip), geometry.adoptPtr ())))
			{
				if (applyClip.isEmpty () == false)
					deviceContext->PopAxisAlignedClip ();
				deviceContext->PushLayer (D2D1::LayerParameters (D2D1::InfiniteRect (),
																 geometry.get (),
																 D2D1_ANTIALIAS_MODE_ALIASED),
										  nullptr);
				geometry->Release ();
				applyClip = state.clip;
			}
			else
			{
				useLayer = false;
			}
		}
		if (!useLayer)
		{
			auto newClip = state.clip;
			if (applyClip != newClip)
			{
				if (applyClip.isEmpty () == false)
					deviceContext->PopAxisAlignedClip ();
				if (newClip.isEmpty () == false)
					deviceContext->PushAxisAlignedClip (convert (newClip),
														D2D1_ANTIALIAS_MODE_ALIASED);
				applyClip = newClip;
			}
		}
		deviceContext->SetTransform (convert (transform));

		p (deviceContext.get ());

		if (useLayer)
			deviceContext->PopLayer ();
	}

	//-----------------------------------------------------------------------------
	void applyFrameColor ()
	{
		if (state.frameBrush)
			return;
		deviceContext->CreateSolidColorBrush (convert (state.frameColor, state.globalAlpha),
											  state.frameBrush.adoptPtr ());
	}

	//-----------------------------------------------------------------------------
	void applyFillColor ()
	{
		if (state.fillBrush)
			return;
		deviceContext->CreateSolidColorBrush (convert (state.fillColor, state.globalAlpha),
											  state.fillBrush.adoptPtr ());
	}

	//-----------------------------------------------------------------------------
	void applyFontColor ()
	{
		if (state.fontBrush)
			return;
		deviceContext->CreateSolidColorBrush (convert (state.fontColor, state.globalAlpha),
											  state.fontBrush.adoptPtr ());
	}

	//-----------------------------------------------------------------------------
	CPoint lineWidthTransformMatrixOffset () const
	{
		int32_t lineWidthInt = static_cast<int32_t> (state.lineWidth);
		if (static_cast<CCoord> (lineWidthInt) == state.lineWidth && lineWidthInt % 2)
		{
			return {0.5, 0.5};
		}
		return {};
	}

	//-----------------------------------------------------------------------------
	void applyLineStyle ()
	{
		if (state.strokeStyle)
			return;

		ID2D1Factory* factory {};
		deviceContext->GetFactory (&factory);

		D2D1_STROKE_STYLE_PROPERTIES properties;
		switch (state.lineStyle.getLineCap ())
		{
			case CLineStyle::kLineCapButt:
				properties.startCap = properties.endCap = properties.dashCap = D2D1_CAP_STYLE_FLAT;
				break;
			case CLineStyle::kLineCapRound:
				properties.startCap = properties.endCap = properties.dashCap = D2D1_CAP_STYLE_ROUND;
				break;
			case CLineStyle::kLineCapSquare:
				properties.startCap = properties.endCap = properties.dashCap =
					D2D1_CAP_STYLE_SQUARE;
				break;
		}
		switch (state.lineStyle.getLineJoin ())
		{
			case CLineStyle::kLineJoinMiter:
				properties.lineJoin = D2D1_LINE_JOIN_MITER;
				break;
			case CLineStyle::kLineJoinRound:
				properties.lineJoin = D2D1_LINE_JOIN_ROUND;
				break;
			case CLineStyle::kLineJoinBevel:
				properties.lineJoin = D2D1_LINE_JOIN_BEVEL;
				break;
		}
		properties.dashOffset = static_cast<FLOAT> (state.lineStyle.getDashPhase ());
		properties.miterLimit = 10.f;
		if (state.lineStyle.getDashCount ())
		{
			properties.dashStyle = D2D1_DASH_STYLE_CUSTOM;
			FLOAT* lengths = new FLOAT[state.lineStyle.getDashCount ()];
			for (uint32_t i = 0; i < state.lineStyle.getDashCount (); i++)
				lengths[i] = static_cast<FLOAT> (state.lineStyle.getDashLengths ()[i]);
			factory->CreateStrokeStyle (properties, lengths, state.lineStyle.getDashCount (),
										state.strokeStyle.adoptPtr ());
			delete[] lengths;
		}
		else
		{
			properties.dashStyle = D2D1_DASH_STYLE_SOLID;
			factory->CreateStrokeStyle (properties, nullptr, 0, state.strokeStyle.adoptPtr ());
		}
	}

	struct State
	{
		CRect clip {};
		CLineStyle lineStyle {kLineSolid};
		CDrawMode drawMode {};
		COM::Ptr<ID2D1StrokeStyle> strokeStyle;
		COM::Ptr<ID2D1SolidColorBrush> fillBrush;
		COM::Ptr<ID2D1SolidColorBrush> frameBrush;
		COM::Ptr<ID2D1SolidColorBrush> fontBrush;
		CColor fillColor {kTransparentCColor};
		CColor frameColor {kTransparentCColor};
		CColor fontColor {kTransparentCColor};
		CCoord lineWidth {1.};
		double globalAlpha {1.};
		TransformMatrix tm {};
	};
	State state;
	std::stack<State> stateStack;
	double scaleFactor {1.};
	CRect applyClip {};

	const D2DGraphicsDevice& device;
	COM::Ptr<ID2D1DeviceContext> deviceContext;
};

//------------------------------------------------------------------------
D2DGraphicsDeviceContext::D2DGraphicsDeviceContext (const D2DGraphicsDevice& device,
													ID2D1DeviceContext* deviceContext)
{
	impl = std::make_unique<Impl> (device, deviceContext);
}

//------------------------------------------------------------------------
D2DGraphicsDeviceContext::~D2DGraphicsDeviceContext () noexcept { endDraw (); }

//------------------------------------------------------------------------
const IPlatformGraphicsDevice& D2DGraphicsDeviceContext::getDevice () const { return impl->device; }

//------------------------------------------------------------------------
PlatformGraphicsPathFactoryPtr D2DGraphicsDeviceContext::getGraphicsPathFactory () const
{
	return D2DGraphicsPathFactory::instance ();
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::beginDraw () const { return true; }

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::endDraw () const
{
	if (impl->applyClip.isEmpty () == false)
		impl->deviceContext->PopAxisAlignedClip ();
	impl->applyClip = {};
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawLine (LinePair line) const
{
	impl->doInContext (
		[&] (auto deviceContext) {
			impl->applyFrameColor ();
			impl->applyLineStyle ();

			CPoint start (line.first);
			CPoint end (line.second);
			if (impl->state.drawMode.integralMode ())
			{
				pixelAlign (impl->state.tm, start);
				pixelAlign (impl->state.tm, end);
			}
			deviceContext->DrawLine (convert (start), convert (end), impl->state.frameBrush.get (),
									 static_cast<FLOAT> (impl->state.lineWidth),
									 impl->state.strokeStyle.get ());
		},
		impl->lineWidthTransformMatrixOffset ());
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawLines (const LineList& lines) const
{
	impl->doInContext (
		[&] (auto deviceContext) {
			impl->applyFrameColor ();
			impl->applyLineStyle ();

			if (impl->state.drawMode.integralMode ())
			{
				for (const auto& line : lines)
				{
					CPoint start (line.first);
					CPoint end (line.second);
					pixelAlign (impl->state.tm, start);
					pixelAlign (impl->state.tm, end);
					deviceContext->DrawLine (
						convert (start), convert (end), impl->state.frameBrush.get (),
						static_cast<FLOAT> (impl->state.lineWidth), impl->state.strokeStyle.get ());
				}
			}
			else
			{
				for (const auto& line : lines)
				{
					CPoint start (line.first);
					CPoint end (line.second);
					deviceContext->DrawLine (
						convert (start), convert (end), impl->state.frameBrush.get (),
						static_cast<FLOAT> (impl->state.lineWidth), impl->state.strokeStyle.get ());
				}
			}
		},
		impl->lineWidthTransformMatrixOffset ());
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawPolygon (const PointList& polygonPointList,
											PlatformGraphicsDrawStyle drawStyle) const
{
	auto path = getGraphicsPathFactory ()->createPath ();
	if (!path)
		return false;

	path->beginSubpath (polygonPointList[0]);
	for (uint32_t i = 1; i < polygonPointList.size (); ++i)
	{
		path->addLine (polygonPointList[i]);
	}
	if (drawStyle == PlatformGraphicsDrawStyle::Filled ||
		drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
	{
		drawGraphicsPath (*path, PlatformGraphicsPathDrawMode::Filled, nullptr);
	}
	if (drawStyle == PlatformGraphicsDrawStyle::Stroked ||
		drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
	{
		drawGraphicsPath (*path, PlatformGraphicsPathDrawMode::Stroked, nullptr);
	}
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawRect (CRect rect, PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInContext ([&] (auto deviceContext) {
		if (drawStyle != PlatformGraphicsDrawStyle::Filled)
		{
			rect.right -= 1.;
			rect.bottom -= 1.;
		}
		if (drawStyle == PlatformGraphicsDrawStyle::Filled ||
			drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
		{
			impl->applyFillColor ();
			deviceContext->FillRectangle (convert (rect), impl->state.fillBrush.get ());
		}
		if (drawStyle == PlatformGraphicsDrawStyle::Stroked ||
			drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
		{
			rect.offset (impl->lineWidthTransformMatrixOffset ());
			impl->applyFrameColor ();
			impl->applyLineStyle ();
			deviceContext->DrawRectangle (convert (rect), impl->state.frameBrush.get (),
										  static_cast<FLOAT> (impl->state.lineWidth),
										  impl->state.strokeStyle.get ());
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawArc (CRect rect, double startAngle1, double endAngle2,
										PlatformGraphicsDrawStyle drawStyle) const
{
	auto path = getGraphicsPathFactory ()->createPath ();
	if (!path)
		return false;
	if (impl->state.drawMode.integralMode ())
		pixelAlign (impl->state.tm, rect);
	path->addArc (rect, startAngle1, endAngle2, true);
	if (drawStyle == PlatformGraphicsDrawStyle::Filled ||
		drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
		drawGraphicsPath (*path, PlatformGraphicsPathDrawMode::Filled, nullptr);
	if (drawStyle == PlatformGraphicsDrawStyle::Stroked ||
		drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
		drawGraphicsPath (*path, PlatformGraphicsPathDrawMode::Stroked, nullptr);
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawEllipse (CRect rect, PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInContext ([&] (auto deviceContext) {
		if (impl->state.drawMode.integralMode ())
			pixelAlign (impl->state.tm, rect);
		if (drawStyle == PlatformGraphicsDrawStyle::Stroked)
			rect.inset (0.5, 0.5);
		CPoint center (rect.getTopLeft ());
		center.offset (rect.getWidth () / 2., rect.getHeight () / 2.);
		D2D1_ELLIPSE ellipse;
		ellipse.point = convert (center);
		ellipse.radiusX = (FLOAT)(rect.getWidth () / 2.);
		ellipse.radiusY = (FLOAT)(rect.getHeight () / 2.);
		if (drawStyle == PlatformGraphicsDrawStyle::Filled ||
			drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
		{
			impl->applyFillColor ();
			deviceContext->FillEllipse (ellipse, impl->state.fillBrush.get ());
		}
		if (drawStyle == PlatformGraphicsDrawStyle::Stroked ||
			drawStyle == PlatformGraphicsDrawStyle::FilledAndStroked)
		{
			impl->applyFrameColor ();
			impl->applyLineStyle ();
			deviceContext->DrawEllipse (ellipse, impl->state.frameBrush.get (),
										static_cast<FLOAT> (impl->state.lineWidth),
										impl->state.strokeStyle.get ());
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawPoint (CPoint point, CColor color) const { return false; }

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawBitmap (IPlatformBitmap& bitmap, CRect dest, CPoint offset,
										   double alpha, BitmapInterpolationQuality quality) const
{
	D2DBitmap* d2dBitmap = dynamic_cast<D2DBitmap*> (&bitmap);
	if (!d2dBitmap || !d2dBitmap->getSource ())
		return false;
	auto d2d1Bitmap =
		D2DBitmapCache::getBitmap (d2dBitmap, impl->deviceContext.get (), impl->device.get ());
	if (!d2d1Bitmap)
		return false;

	double bitmapScaleFactor = d2dBitmap->getScaleFactor ();
	CGraphicsTransform bitmapTransform;
	bitmapTransform.scale (1. / bitmapScaleFactor, 1. / bitmapScaleFactor);
	auto originalTransformMatrix = impl->state.tm;
	TransformMatrix tm = originalTransformMatrix * bitmapTransform;
	setTransformMatrix (tm);
	bitmapTransform.inverse ().transform (dest);

	impl->doInContext ([&] (auto deviceContext) {
		auto bitmapSize = bitmap.getSize ();
		CRect d (dest);
		d.setWidth (bitmapSize.x);
		d.setHeight (bitmapSize.y);
		d.offset (-offset.x, -offset.y);
		d.makeIntegral ();
		CRect source;
		source.setWidth (d2d1Bitmap->GetSize ().width);
		source.setHeight (d2d1Bitmap->GetSize ().height);

		D2D1_BITMAP_INTERPOLATION_MODE mode;
		switch (quality)
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

		D2D1_RECT_F sourceRect = convert (source);
		deviceContext->DrawBitmap (d2d1Bitmap, convert (d),
								   static_cast<FLOAT> (alpha * impl->state.globalAlpha), mode,
								   &sourceRect);
	});
	setTransformMatrix (originalTransformMatrix);
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::clearRect (CRect rect) const
{
#if 1
	TransformGuard tmGuard (impl->deviceContext.get ());

	impl->deviceContext->SetTransform (convert (impl->state.tm));
	impl->deviceContext->PushAxisAlignedClip (convert (rect), D2D1_ANTIALIAS_MODE_ALIASED);
	impl->deviceContext->Clear (D2D1::ColorF (1.f, 1.f, 1.f, 0.f));
	impl->deviceContext->PopAxisAlignedClip ();
#else
	CRect oldClip = impl->state.clip;
	rect.bound (oldClip);
	setClipRect (rect);
	impl->doInContext (
		[] (auto deviceContext) { deviceContext->Clear (D2D1::ColorF (1.f, 1.f, 1.f, 0.f)); });
	setClipRect (oldClip);
#endif
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawGraphicsPath (IPlatformGraphicsPath& path,
												 PlatformGraphicsPathDrawMode mode,
												 TransformMatrix* transformation) const
{
	auto d2dPath = dynamic_cast<D2DGraphicsPath*> (&path);
	if (d2dPath == nullptr)
		return false;

	impl->doInContext ([&] (auto deviceContext) {
		COM::Ptr<ID2D1Geometry> path;
		if (transformation)
		{
			ID2D1Factory* factory {};
			deviceContext->GetFactory (&factory);
			path = COM::adopt<ID2D1Geometry> (
				d2dPath->createTransformedGeometry (factory, *transformation));
		}
		else
		{
			path = COM::share<ID2D1Geometry> (d2dPath->getPathGeometry ());
		}
		if (path)
		{
			if (mode == PlatformGraphicsPathDrawMode::Filled ||
				mode == PlatformGraphicsPathDrawMode::FilledEvenOdd)
			{
				impl->applyFillColor ();
				deviceContext->FillGeometry (path.get (), impl->state.fillBrush.get ());
			}
			else if (mode == PlatformGraphicsPathDrawMode::Stroked)
			{
				impl->applyFrameColor ();
				impl->applyLineStyle ();
				deviceContext->DrawGeometry (path.get (), impl->state.frameBrush.get (),
											 (FLOAT)impl->state.lineWidth,
											 impl->state.strokeStyle.get ());
			}
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::fillLinearGradient (IPlatformGraphicsPath& path,
												   const IPlatformGradient& gradient,
												   CPoint startPoint, CPoint endPoint, bool evenOdd,
												   TransformMatrix* transformation) const
{
	auto d2dPath = dynamic_cast<D2DGraphicsPath*> (&path);
	const auto d2dGradient = dynamic_cast<const D2DGradient*> (&gradient);
	if (d2dPath == nullptr || d2dGradient == nullptr)
		return false;
	impl->doInContext ([&] (auto deviceContext) {
		auto stopCollection = COM::adopt<ID2D1GradientStopCollection> (
			d2dGradient->create (deviceContext, static_cast<FLOAT> (impl->state.globalAlpha)));
		if (!stopCollection)
			return;
		COM::Ptr<ID2D1Geometry> path;
		if (transformation)
		{
			ID2D1Factory* factory {};
			deviceContext->GetFactory (&factory);
			path = COM::adopt<ID2D1Geometry> (
				d2dPath->createTransformedGeometry (factory, *transformation));
		}
		else
		{
			path = COM::share<ID2D1Geometry> (d2dPath->getPathGeometry ());
		}
		if (path)
		{
			COM::Ptr<ID2D1LinearGradientBrush> brush;
			D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties;
			properties.startPoint = convert (startPoint);
			properties.endPoint = convert (endPoint);
			if (SUCCEEDED (deviceContext->CreateLinearGradientBrush (
					properties, stopCollection.get (), brush.adoptPtr ())))
			{
				deviceContext->FillGeometry (path.get (), brush.get ());
			}
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::fillRadialGradient (IPlatformGraphicsPath& path,
												   const IPlatformGradient& gradient, CPoint center,
												   CCoord radius, CPoint originOffset, bool evenOdd,
												   TransformMatrix* transformation) const
{
	auto d2dPath = dynamic_cast<D2DGraphicsPath*> (&path);
	const auto d2dGradient = dynamic_cast<const D2DGradient*> (&gradient);
	if (d2dPath == nullptr || d2dGradient == nullptr)
		return false;
	impl->doInContext ([&] (auto deviceContext) {
		auto stopCollection = COM::adopt<ID2D1GradientStopCollection> (
			d2dGradient->create (deviceContext, static_cast<FLOAT> (impl->state.globalAlpha)));
		if (!stopCollection)
			return;
		COM::Ptr<ID2D1Geometry> path;
		if (transformation)
		{
			ID2D1Factory* factory {};
			deviceContext->GetFactory (&factory);
			path = COM::adopt<ID2D1Geometry> (
				d2dPath->createTransformedGeometry (factory, *transformation));
		}
		else
		{
			path = COM::share<ID2D1Geometry> (d2dPath->getPathGeometry ());
		}
		if (path)
		{
			COM::Ptr<ID2D1RadialGradientBrush> brush;
			D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES properties;
			properties.center = convert (center);
			properties.gradientOriginOffset = convert (originOffset);
			properties.radiusX = (FLOAT)radius;
			properties.radiusY = (FLOAT)radius;
			if (SUCCEEDED (deviceContext->CreateRadialGradientBrush (
					properties, stopCollection.get (), brush.adoptPtr ())))
			{
				deviceContext->FillGeometry (path.get (), brush.get ());
			}
		}
	});
	return true;
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::saveGlobalState () const { impl->stateStack.push (impl->state); }

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::restoreGlobalState () const
{
	vstgui_assert (impl->stateStack.empty () == false,
				   "Unbalanced calls to saveGlobalState and restoreGlobalState");
#if NDEBUG
	if (impl->stateStack.empty ())
		return;
#endif
	impl->state = impl->stateStack.top ();
	impl->stateStack.pop ();
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setLineStyle (const CLineStyle& style) const
{
	if (impl->state.lineStyle != style)
	{
		impl->state.lineStyle = style;
		impl->state.strokeStyle.reset ();
	}
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setLineWidth (CCoord width) const { impl->state.lineWidth = width; }

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setDrawMode (CDrawMode mode) const { impl->state.drawMode = mode; }

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setClipRect (CRect clip) const { impl->state.clip = clip; }

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setFillColor (CColor color) const
{
	if (impl->state.fillColor != color)
	{
		impl->state.fillColor = color;
		impl->state.fillBrush.reset ();
	}
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setFrameColor (CColor color) const
{
	if (impl->state.frameColor != color)
	{
		impl->state.frameColor = color;
		impl->state.frameBrush.reset ();
	}
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setFontColor (CColor color) const
{
	if (impl->state.fontColor != color)
	{
		impl->state.fontColor = color;
		impl->state.fontBrush.reset ();
	}
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setGlobalAlpha (double newAlpha) const
{
	if (impl->state.globalAlpha != newAlpha)
	{
		impl->state.globalAlpha = newAlpha;
		impl->state.fillBrush.reset ();
		impl->state.frameBrush.reset ();
		impl->state.fontBrush.reset ();
	}
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::setTransformMatrix (const TransformMatrix& tm) const
{
	impl->state.tm = tm;
}

//------------------------------------------------------------------------
const IPlatformGraphicsDeviceContextBitmapExt* D2DGraphicsDeviceContext::asBitmapExt () const
{
	return nullptr;
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::drawTextLayout (IDWriteTextLayout* textLayout, CPoint pos,
											   bool antialias)
{
	impl->doInContext ([&] (auto deviceContext) {
		deviceContext->SetTextAntialiasMode (antialias ? D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE
													   : D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
		if (impl->state.drawMode.integralMode ())
			pos.makeIntegral ();
		pos.y += 0.5;
		impl->applyFontColor ();
		deviceContext->DrawTextLayout (convert (pos), textLayout, impl->state.fontBrush.get ());
	});
}

//------------------------------------------------------------------------
} // VSTGUI