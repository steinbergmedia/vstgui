// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dgraphicscontext.h"
#include "d2dgradient.h"
#include "d2dgraphicspath.h"
#include "d2dbitmap.h"
#include "d2dbitmapcache.h"
#include "d2dgradient.h"
#include "d2d.h"
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
namespace {

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
struct D2DBitmapDeviceContext : D2DGraphicsDeviceContext
{
	static PlatformGraphicsDeviceContextPtr make (const D2DGraphicsDevice& device,
												  ID2D1DeviceContext* deviceContext,
												  const SharedPointer<D2DBitmap>& bitmap)
	{
		D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1 (
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

		COM::Ptr<ID2D1Bitmap1> bitmapTarget;
		deviceContext->CreateBitmapFromWicBitmap (bitmap->getSource (), &props,
												  bitmapTarget.adoptPtr ());
		if (!bitmapTarget)
			return nullptr;

		deviceContext->SetTarget (bitmapTarget.get ());

		TransformMatrix tm;
		tm.scale (bitmap->getScaleFactor (), bitmap->getScaleFactor ());
		deviceContext->SetTransform (convert (tm));
		return std::make_shared<D2DBitmapDeviceContext> (device, bitmap, deviceContext,
														 TransformMatrix {});
	}

	D2DBitmapDeviceContext (const D2DGraphicsDevice& device, const SharedPointer<D2DBitmap>& bitmap,
							ID2D1DeviceContext* deviceContext, const TransformMatrix& tm)
	: D2DGraphicsDeviceContext (device, deviceContext, tm), bitmap (bitmap)
	{
	}

	bool endDraw () const
	{
		if (D2DGraphicsDeviceContext::endDraw ())
		{
			COM::Ptr<ID2D1Image> image;
			getID2D1DeviceContext ()->GetTarget (image.adoptPtr ());
			if (!image)
				return false;
			COM::Ptr<ID2D1Bitmap1> bitmapTarget;
			if (FAILED (image->QueryInterface<ID2D1Bitmap1> (bitmapTarget.adoptPtr ())))
				return false;
			auto pa = bitmap->lockPixels (true);
			if (!pa)
				return false;
			auto size = bitmap->getSize ();
			D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1 (
				D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
			COM::Ptr<ID2D1Bitmap1> cpuBitmap;
			auto hr = getID2D1DeviceContext ()->CreateBitmap (
				D2D1_SIZE_U {static_cast<UINT32> (size.x), static_cast<UINT32> (size.y)}, nullptr,
				pa->getBytesPerRow (), props, cpuBitmap.adoptPtr ());
			if (FAILED (hr))
				return false;
			D2D1_POINT_2U dp {};
			D2D1_RECT_U dstRect {0, 0, static_cast<UINT32> (size.x), static_cast<UINT32> (size.y)};
			hr = cpuBitmap->CopyFromBitmap (&dp, bitmapTarget.get (), &dstRect);
			if (FAILED (hr))
				return false;
			D2D1_MAPPED_RECT mappedRect {};
			hr = cpuBitmap->Map (D2D1_MAP_OPTIONS_READ, &mappedRect);
			if (FAILED (hr))
				return false;
			auto height = static_cast<uint32_t> (size.y);
			if (pa->getBytesPerRow () == mappedRect.pitch)
			{
				memcpy (pa->getAddress (), mappedRect.bits, mappedRect.pitch * height);
			}
			else
			{
				auto src = mappedRect.bits;
				for (auto row = 0u; row < height; ++row, src += mappedRect.pitch)
				{
					auto dst = pa->getAddress () + pa->getBytesPerRow () * row;
					memcpy (dst, src, pa->getBytesPerRow ());
				}
			}
			cpuBitmap->Unmap ();
			return true;
		}
		return false;
	}

	SharedPointer<D2DBitmap> bitmap;
};

//------------------------------------------------------------------------
} // anonymous namespace

//------------------------------------------------------------------------
struct D2DGraphicsDeviceFactory::Impl
{
	std::vector<std::shared_ptr<D2DGraphicsDevice>> devices;
};

//------------------------------------------------------------------------
D2DGraphicsDeviceFactory::D2DGraphicsDeviceFactory () { impl = std::make_unique<Impl> (); }

//------------------------------------------------------------------------
D2DGraphicsDeviceFactory::~D2DGraphicsDeviceFactory () noexcept {}

//------------------------------------------------------------------------
PlatformGraphicsDevicePtr
	D2DGraphicsDeviceFactory::getDeviceForScreen (ScreenInfo::Identifier screen) const
{
	if (impl->devices.empty ())
		return nullptr;
	return impl->devices.front ();
}

//------------------------------------------------------------------------
PlatformGraphicsDevicePtr D2DGraphicsDeviceFactory::find (ID2D1Device* dev) const
{
	auto it = std::find_if (impl->devices.begin (), impl->devices.end (),
							[dev] (const auto& el) { return el->get () == dev; });
	if (it != impl->devices.end ())
		return *it;
	return nullptr;
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceFactory::addDevice (const std::shared_ptr<D2DGraphicsDevice>& device) const
{
	impl->devices.push_back (device);
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceFactory::removeDevice (const std::shared_ptr<D2DGraphicsDevice>& device) const
{
	auto it = std::find (impl->devices.begin (), impl->devices.end (), device);
	if (it != impl->devices.end ())
		impl->devices.erase (it);
}

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
	auto d2dBitmap = bitmap.cast<D2DBitmap> ();
	if (d2dBitmap)
	{

		COM::Ptr<ID2D1DeviceContext> deviceContext;
		auto hr =
	    impl->device->CreateDeviceContext (D2D1_DEVICE_CONTEXT_OPTIONS_NONE, deviceContext.adoptPtr ());
		if (FAILED (hr))
			return nullptr;

		D2DBitmapCache::removeBitmap (d2dBitmap);

		return D2DBitmapDeviceContext::make (*this, deviceContext.get (), d2dBitmap);
	}
	return nullptr;
}

//------------------------------------------------------------------------
ID2D1Device* D2DGraphicsDevice::get () const { return impl->device.get (); }

//------------------------------------------------------------------------
struct D2DGraphicsDeviceContext::Impl
{
	Impl (const D2DGraphicsDevice& device, ID2D1DeviceContext* deviceContext,
		  const TransformMatrix& tm)
	: device (device), deviceContext (COM::share (deviceContext)), globalTM (tm)
	{
#if DEBUG
		COM::Ptr<ID2D1Device> d2dDevice;
		deviceContext->GetDevice (d2dDevice.adoptPtr ());
		vstgui_assert (d2dDevice.get () == device.get ());
#endif
	}

	template<typename Proc>
	void doInContext (Proc p, CPoint transformOffset = {})
	{
		if (state.clip.isEmpty ())
			return;

		TransformGuard tmGuard (deviceContext.get ());

		auto transform = convert (tmGuard.matrix) * globalTM * state.tm;
		transform.scale (scaleFactor, scaleFactor);
		transform.translate (transformOffset);
		auto newClip = state.clip;
		globalTM.transform (newClip);
		if (applyClip != newClip)
		{
			if (applyClip.isEmpty () == false)
				deviceContext->PopAxisAlignedClip ();
			if (newClip.isEmpty () == false)
				deviceContext->PushAxisAlignedClip (convert (newClip), D2D1_ANTIALIAS_MODE_ALIASED);
			applyClip = newClip;
		}
		deviceContext->SetTransform (convert (transform));

		p (deviceContext.get ());
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
	void applyFontColor (CColor color)
	{
		if (state.fontColor != color)
			state.fontBrush.reset ();
		if (state.fontBrush)
			return;
		state.fontColor = color;
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

		COM::Ptr<ID2D1Factory> factory {};
		deviceContext->GetFactory (factory.adoptPtr ());

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
	bool beginDrawCalled {false};
	TransformMatrix globalTM;

	const D2DGraphicsDevice& device;
	COM::Ptr<ID2D1DeviceContext> deviceContext;

#if defined(VSTGUI_TEXTRENDERING_LEGACY_INCONSISTENCY) &&                                          \
	VSTGUI_TEXTRENDERING_LEGACY_INCONSISTENCY == 1
	static constexpr auto antialiasMode = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
#else
	static constexpr auto antialiasMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
#endif
};

//------------------------------------------------------------------------
D2DGraphicsDeviceContext::D2DGraphicsDeviceContext (const D2DGraphicsDevice& device,
													ID2D1DeviceContext* deviceContext,
													const TransformMatrix& tm)
{
	impl = std::make_unique<Impl> (device, deviceContext, tm);
}

//------------------------------------------------------------------------
D2DGraphicsDeviceContext::~D2DGraphicsDeviceContext () noexcept { endDraw (); }

//------------------------------------------------------------------------
ID2D1DeviceContext* D2DGraphicsDeviceContext::getID2D1DeviceContext () const
{
	return impl->deviceContext.get ();
}

//------------------------------------------------------------------------
const IPlatformGraphicsDevice& D2DGraphicsDeviceContext::getDevice () const { return impl->device; }

//------------------------------------------------------------------------
PlatformGraphicsPathFactoryPtr D2DGraphicsDeviceContext::getGraphicsPathFactory () const
{
	return D2DGraphicsPathFactory::instance ();
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::beginDraw () const
{
	impl->beginDrawCalled = true;
	impl->deviceContext->BeginDraw ();
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::endDraw () const
{
	if (impl->applyClip.isEmpty () == false)
		impl->deviceContext->PopAxisAlignedClip ();
	impl->applyClip = {};
	if (impl->beginDrawCalled)
	{
		auto hr = impl->deviceContext->EndDraw ();
		vstgui_assert (SUCCEEDED (hr));
		impl->beginDrawCalled = false;
	}
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
	path->finishBuilding ();
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
	path->finishBuilding ();

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

	auto originalClip = impl->state.clip;
	auto cr = dest;
	impl->state.tm.transform (cr);
	cr.bound (originalClip);
	impl->state.clip = cr;
	
	double bitmapScaleFactor = d2dBitmap->getScaleFactor ();
	CGraphicsTransform bitmapTransform;
	bitmapTransform.scale (1. / bitmapScaleFactor, 1. / bitmapScaleFactor);
	auto originalTransformMatrix = impl->state.tm;
	TransformMatrix tm = originalTransformMatrix * bitmapTransform;
	setTransformMatrix (tm);
	auto invBitmapTransform = bitmapTransform.inverse ();
	invBitmapTransform.transform (dest);
	invBitmapTransform.transform (offset);

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
	impl->state.clip = originalClip;
	return true;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::clearRect (CRect rect) const
{
#if 1
	TransformGuard tmGuard (impl->deviceContext.get ());

	impl->deviceContext->SetTransform (convert (impl->globalTM * impl->state.tm));
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
			COM::Ptr<ID2D1Factory> factory {};
			deviceContext->GetFactory (factory.adoptPtr ());
			path = COM::adopt<ID2D1Geometry> (
				d2dPath->createTransformedGeometry (factory.get (), *transformation));
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
			COM::Ptr<ID2D1Factory> factory {};
			deviceContext->GetFactory (factory.adoptPtr ());
			path = COM::adopt<ID2D1Geometry> (
				d2dPath->createTransformedGeometry (factory.get (), *transformation));
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
			COM::Ptr<ID2D1Factory> factory {};
			deviceContext->GetFactory (factory.adoptPtr ());
			path = COM::adopt<ID2D1Geometry> (
				d2dPath->createTransformedGeometry (factory.get (), *transformation));
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
	return this;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::drawBitmapNinePartTiled (IPlatformBitmap& bitmap, CRect dest,
														const CNinePartTiledDescription& desc,
														double alpha,
														BitmapInterpolationQuality quality) const
{
	return false;
}

//------------------------------------------------------------------------
bool D2DGraphicsDeviceContext::fillRectWithBitmap (IPlatformBitmap& bitmap, CRect srcRect,
												   CRect dstRect, double alpha,
												   BitmapInterpolationQuality quality) const
{
	D2DBitmap* d2dBitmap = dynamic_cast<D2DBitmap*> (&bitmap);
	if (!d2dBitmap || !d2dBitmap->getSource ())
		return false;
	auto d2d1Bitmap =
		D2DBitmapCache::getBitmap (d2dBitmap, impl->deviceContext.get (), impl->device.get ());
	if (!d2d1Bitmap)
		return false;

	auto originalClip = impl->state.clip;
	auto cr = dstRect;
	impl->state.tm.transform (cr);
	cr.bound (originalClip);

	double bitmapScaleFactor = d2dBitmap->getScaleFactor ();
	CGraphicsTransform bitmapTransform;
	bitmapTransform.scale (1. / bitmapScaleFactor, 1. / bitmapScaleFactor);
	auto invBitmapTransform = bitmapTransform.inverse ();
	invBitmapTransform.transform (dstRect);
	invBitmapTransform.transform (srcRect);

	D2D1_IMAGE_BRUSH_PROPERTIES imageBrushProp = {};
	imageBrushProp.sourceRectangle = convert (srcRect);
	imageBrushProp.extendModeX = imageBrushProp.extendModeY = D2D1_EXTEND_MODE_WRAP;
	switch (quality)
	{
		case BitmapInterpolationQuality::kLow:
			imageBrushProp.interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
			break;

		case BitmapInterpolationQuality::kMedium:
		case BitmapInterpolationQuality::kHigh:
		default:
			imageBrushProp.interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;
			break;
	}
	CGraphicsTransform brushTransform;
	brushTransform.translate (dstRect.getTopLeft ());

	D2D1_BRUSH_PROPERTIES brushProp = {};
	brushProp.opacity = 1.f;
	brushProp.transform = convert (brushTransform);

	COM::Ptr<ID2D1ImageBrush> brush;
	auto hr = impl->deviceContext->CreateImageBrush (d2d1Bitmap, imageBrushProp, brushProp,
													 brush.adoptPtr ());
	if (FAILED (hr))
		return false;

	impl->state.clip = cr;

	auto originalTransformMatrix = impl->state.tm;
	TransformMatrix tm = originalTransformMatrix * bitmapTransform;
	setTransformMatrix (tm);

	impl->doInContext ([&] (auto deviceContext) {
		deviceContext->FillRectangle (convert (dstRect), brush.get ());
	});

	setTransformMatrix (originalTransformMatrix);
	impl->state.clip = originalClip;
	return true;
}

//------------------------------------------------------------------------
void D2DGraphicsDeviceContext::drawTextLayout (IDWriteTextLayout* textLayout, CPoint pos,
											   CColor color, bool antialias)
{
	impl->doInContext ([&] (auto deviceContext) {
		deviceContext->SetTextAntialiasMode (antialias ? impl->antialiasMode
													   : D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
		if (impl->state.drawMode.integralMode ())
			pos.makeIntegral ();
		pos.y += 0.5;
		impl->applyFontColor (color);
		deviceContext->DrawTextLayout (convert (pos), textLayout, impl->state.fontBrush.get ());
	});
}

//------------------------------------------------------------------------
} // VSTGUI
