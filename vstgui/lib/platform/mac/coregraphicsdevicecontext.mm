// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../ccolor.h"
#include "../../cdrawdefs.h"
#include "../../cgraphicstransform.h"
#include "../../clinestyle.h"
#include "../../crect.h"
#include "cgbitmap.h"
#include "macglobals.h"
#include "coregraphicsdevicecontext.h"
#include "quartzgraphicspath.h"

#include <stack>

#if TARGET_OS_IPHONE
#include <ImageIO/ImageIO.h>
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
PlatformGraphicsDevicePtr
	CoreGraphicsDeviceFactory::getDeviceForScreen (ScreenInfo::Identifier screen) const
{
	static PlatformGraphicsDevicePtr gCoreGraphicsDevice = std::make_shared<CoreGraphicsDevice> ();
	return gCoreGraphicsDevice;
}

//------------------------------------------------------------------------
static CGPathDrawingMode convert (PlatformGraphicsPathDrawMode mode)
{
	switch (mode)
	{
		case PlatformGraphicsPathDrawMode::FilledEvenOdd:
			return kCGPathEOFill;
		case PlatformGraphicsPathDrawMode::Filled:
			return kCGPathFill;
		case PlatformGraphicsPathDrawMode::Stroked:
			return kCGPathStroke;
	}
	vstgui_assert (false);
}

//------------------------------------------------------------------------
static CGPathDrawingMode convert (PlatformGraphicsDrawStyle drawStyle)
{
	switch (drawStyle)
	{
		case PlatformGraphicsDrawStyle::Filled:
			return kCGPathFill;
		case PlatformGraphicsDrawStyle::FilledAndStroked:
			return kCGPathFillStroke;
		case PlatformGraphicsDrawStyle::Stroked:
			return kCGPathStroke;
	}
	vstgui_assert (false);
}

//------------------------------------------------------------------------
auto CoreGraphicsDevice::createBitmapContext (const PlatformBitmapPtr& bitmap) const
	-> PlatformGraphicsDeviceContextPtr
{
	auto cgBitmap = bitmap.cast<CGBitmap> ();
	if (cgBitmap)
	{
		auto scaleFactor = bitmap->getScaleFactor ();
		auto cgContext = cgBitmap->createCGContext ();
		CGContextConcatCTM (cgContext,
							CGAffineTransformMakeScale (static_cast<CGFloat> (scaleFactor),
														static_cast<CGFloat> (scaleFactor)));
		auto bitmapContext = std::make_shared<CoreGraphicsBitmapContext> (
			*this, cgContext, [cgBitmap = shared (cgBitmap)] () { cgBitmap->setDirty (); });
		CFRelease (cgContext);
		return bitmapContext;
	}
	return nullptr;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct CoreGraphicsDeviceContext::Impl
{
	//-----------------------------------------------------------------------------
	Impl (const CoreGraphicsDevice& d, CGContextRef cgContext) : device (d), cgContext (cgContext)
	{
		CFRetain (cgContext);

		auto userRect = CGRectMake (0, 0, 100, 100);
		auto deviceRect = CGContextConvertRectToDeviceSpace (cgContext, userRect);
		backendScaleFactor = deviceRect.size.height / userRect.size.height;

		CGContextSaveGState (cgContext);
		CGContextSetAllowsAntialiasing (cgContext, true);
		CGContextSetAllowsFontSmoothing (cgContext, true);
		CGContextSetAllowsFontSubpixelPositioning (cgContext, true);
		CGContextSetAllowsFontSubpixelQuantization (cgContext, true);
		CGContextSetShouldAntialias (cgContext, false);
		CGContextSetFillColorSpace (cgContext, GetCGColorSpace ());
		CGContextSetStrokeColorSpace (cgContext, GetCGColorSpace ());
		CGContextSaveGState (cgContext);
		CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
		CGContextSetTextMatrix (cgContext, cgCTM);
	}

	//-----------------------------------------------------------------------------
	~Impl ()
	{
		CGContextRestoreGState (cgContext); // restore the original state
		CGContextRestoreGState (cgContext); // we need to do it twice !!!
		CFRelease (cgContext);
	}

	//-----------------------------------------------------------------------------
	template<typename Proc>
	static void DoGraphicStateSave (CGContextRef context, Proc proc)
	{
		CGContextSaveGState (context);
		proc ();
		CGContextRestoreGState (context);
	}

	//------------------------------------------------------------------------
	template<typename Proc>
	void doInCGContext (bool swapYAxis, bool integralOffset, Proc proc) const
	{
		if (state.clipRect.isEmpty ())
			return;

		CGContextSaveGState (cgContext);

		CGRect cgClipRect = CGRectFromCRect (state.clipRect);
		if (integralOffset)
			cgClipRect = pixelAlligned (cgClipRect);
		CGContextClipToRect (cgContext, cgClipRect);
#if DEBUG
		if (showClip)
		{
			CGContextSetRGBFillColor (cgContext, 1, 0, 0, 0.5);
			CGContextFillRect (cgContext, cgClipRect);
		}
#endif

		if (state.tm.isInvariant () == false)
		{
			auto t = state.tm;
			if (integralOffset)
			{
				CGPoint p = pixelAlligned (CGPointFromCPoint (CPoint (t.dx, t.dy)));
				t.dx = p.x;
				t.dy = p.y;
			}
			CGContextConcatCTM (cgContext, createCGAffineTransform (t));
		}

		if (!swapYAxis)
			CGContextScaleCTM (cgContext, 1, -1);

		CGContextSetAlpha (cgContext, state.globalAlpha);

		proc (cgContext);

		CGContextRestoreGState (cgContext);
	}

	//-----------------------------------------------------------------------------
	CGRect pixelAlligned (const CGRect& r) const
	{
		CGRect result;
		result.origin = CGContextConvertPointToDeviceSpace (cgContext, r.origin);
		result.size = CGContextConvertSizeToDeviceSpace (cgContext, r.size);
		result.origin.x = static_cast<CGFloat> (std::round (result.origin.x));
		result.origin.y = static_cast<CGFloat> (std::round (result.origin.y));
		result.size.width = static_cast<CGFloat> (std::round (result.size.width));
		result.size.height = static_cast<CGFloat> (std::round (result.size.height));
		result.origin = CGContextConvertPointToUserSpace (cgContext, result.origin);
		result.size = CGContextConvertSizeToUserSpace (cgContext, result.size);
		return result;
	}

	//-----------------------------------------------------------------------------
	CGPoint pixelAlligned (const CGPoint& p) const
	{
		CGPoint result = CGContextConvertPointToDeviceSpace (cgContext, p);
		result.x = static_cast<CGFloat> (std::round (result.x));
		result.y = static_cast<CGFloat> (std::round (result.y));
		result = CGContextConvertPointToUserSpace (cgContext, result);
		return result;
	}

	//------------------------------------------------------------------------
	void applyLineStyle () const
	{
		switch (state.lineStyle.getLineCap ())
		{
			case CLineStyle::kLineCapButt:
				CGContextSetLineCap (cgContext, kCGLineCapButt);
				break;
			case CLineStyle::kLineCapRound:
				CGContextSetLineCap (cgContext, kCGLineCapRound);
				break;
			case CLineStyle::kLineCapSquare:
				CGContextSetLineCap (cgContext, kCGLineCapSquare);
				break;
		}
		switch (state.lineStyle.getLineJoin ())
		{
			case CLineStyle::kLineJoinMiter:
				CGContextSetLineJoin (cgContext, kCGLineJoinMiter);
				break;
			case CLineStyle::kLineJoinRound:
				CGContextSetLineJoin (cgContext, kCGLineJoinRound);
				break;
			case CLineStyle::kLineJoinBevel:
				CGContextSetLineJoin (cgContext, kCGLineJoinBevel);
				break;
		}
		if (state.lineStyle.getDashCount () > 0)
		{
			CGFloat* dashLengths = new CGFloat[state.lineStyle.getDashCount ()];
			for (uint32_t i = 0; i < state.lineStyle.getDashCount (); i++)
			{
				dashLengths[i] =
					static_cast<CGFloat> (state.lineWidth * state.lineStyle.getDashLengths ()[i]);
			}
			CGContextSetLineDash (cgContext, static_cast<CGFloat> (state.lineStyle.getDashPhase ()),
								  dashLengths, state.lineStyle.getDashCount ());
			delete[] dashLengths;
		}
	}

	//-----------------------------------------------------------------------------
	void applyLineWidthCTM () const
	{
		int32_t lineWidthInt = static_cast<int32_t> (state.lineWidth);
		if (static_cast<CCoord> (lineWidthInt) == state.lineWidth && lineWidthInt % 2)
			CGContextTranslateCTM (cgContext, 0.5, 0.5);
	}

	//------------------------------------------------------------------------
	void setBitmapInterpolationQuality (BitmapInterpolationQuality q) const
	{
		switch (q)
		{
			case BitmapInterpolationQuality::kLow:
			{
				CGContextSetShouldAntialias (cgContext, false);
				CGContextSetInterpolationQuality (cgContext, kCGInterpolationNone);
				break;
			}
			case BitmapInterpolationQuality::kMedium:
			{
				CGContextSetShouldAntialias (cgContext, true);
				CGContextSetInterpolationQuality (cgContext, kCGInterpolationMedium);
				break;
			}
			case BitmapInterpolationQuality::kHigh:
			{
				CGContextSetShouldAntialias (cgContext, true);
				CGContextSetInterpolationQuality (cgContext, kCGInterpolationHigh);
				break;
			}
			case BitmapInterpolationQuality::kDefault:
			{
				CGContextSetShouldAntialias (cgContext, true);
				CGContextSetInterpolationQuality (cgContext, kCGInterpolationDefault);
				break;
			}
			default:
				vstgui_assert (false);
				break;
		}
	}

	//-----------------------------------------------------------------------------
	void drawCGImageRef (CGContextRef context, CGImageRef image, CGLayerRef layer,
						 double bitmapScaleFactor, const CRect& inRect, const CPoint& inOffset,
						 float alpha) const
	{
		CRect rect (inRect);
		CPoint offset (inOffset);
		CGSize bitmapNormSize;
		if (layer)
		{
			bitmapNormSize = CGLayerGetSize (layer);
		}
		else
		{
			bitmapNormSize.width = CGImageGetWidth (image);
			bitmapNormSize.height = CGImageGetHeight (image);
		}
		bitmapNormSize.width /= bitmapScaleFactor;
		bitmapNormSize.height /= bitmapScaleFactor;

		CGContextSetAlpha (context, (CGFloat)alpha * state.globalAlpha);

		CGRect dest;
		dest.origin.x = static_cast<CGFloat> (rect.left - offset.x);
		dest.origin.y = static_cast<CGFloat> (-(rect.top) - (bitmapNormSize.height - offset.y));
		dest.size = bitmapNormSize;

		CGRect cgClipRect;
		cgClipRect.origin.x = static_cast<CGFloat> (rect.left);
		cgClipRect.origin.y = static_cast<CGFloat> (-(rect.top) - rect.getHeight ());
		cgClipRect.size.width = static_cast<CGFloat> (rect.getWidth ());
		cgClipRect.size.height = static_cast<CGFloat> (rect.getHeight ());

		if (bitmapScaleFactor != 1.)
		{
			CGContextConcatCTM (context, CGAffineTransformMakeScale (
											 static_cast<CGFloat> (1. / bitmapScaleFactor),
											 static_cast<CGFloat> (1. / bitmapScaleFactor)));
			CGAffineTransform transform = CGAffineTransformMakeScale (
				static_cast<CGFloat> (bitmapScaleFactor), static_cast<CGFloat> (bitmapScaleFactor));
			cgClipRect.origin = CGPointApplyAffineTransform (cgClipRect.origin, transform);
			cgClipRect.size = CGSizeApplyAffineTransform (cgClipRect.size, transform);
			dest.origin = CGPointApplyAffineTransform (dest.origin, transform);
			dest.size = CGSizeApplyAffineTransform (dest.size, transform);
		}
		cgClipRect.origin = pixelAlligned (cgClipRect.origin);

		CGContextClipToRect (context, cgClipRect);

		if (layer)
		{
			CGContextDrawLayerInRect (context, dest, layer);
		}
		else
		{
			CGContextDrawImage (context, dest, image);
		}
	}

	//------------------------------------------------------------------------
	const CoreGraphicsDevice& device;
	CGContextRef cgContext {nullptr};

	struct State
	{
		CLineStyle lineStyle {kLineSolid};
		CCoord lineWidth {1};
		CDrawMode drawMode {};
		CRect clipRect {};
		double globalAlpha {1.};
		TransformMatrix tm {};
	};

	State state;
	std::stack<State> stateStack;

	double backendScaleFactor {1.};

	using BitmapDrawCountMap = std::map<CGBitmap*, int32_t>;
	BitmapDrawCountMap bitmapDrawCount;

#if defined(VSTGUI_TEXTRENDERING_LEGACY_INCONSISTENCY) &&                                          \
	VSTGUI_TEXTRENDERING_LEGACY_INCONSISTENCY == 1
	static constexpr bool shouldSmoothFonts = true;
#else
	static constexpr bool shouldSmoothFonts = false;
#endif

#if DEBUG
	bool showClip {false};
#endif
};

//------------------------------------------------------------------------
CoreGraphicsDeviceContext::CoreGraphicsDeviceContext (const CoreGraphicsDevice& device,
													  void* cgContext)
{
	impl = std::make_unique<Impl> (device, static_cast<CGContextRef> (cgContext));
}

//------------------------------------------------------------------------
CoreGraphicsDeviceContext::~CoreGraphicsDeviceContext () noexcept {}

//------------------------------------------------------------------------
const IPlatformGraphicsDevice& CoreGraphicsDeviceContext::getDevice () const
{
	return impl->device;
}

//------------------------------------------------------------------------
PlatformGraphicsPathFactoryPtr CoreGraphicsDeviceContext::getGraphicsPathFactory () const
{
	return CGGraphicsPathFactory::instance ();
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::beginDraw () const { return true; }

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::endDraw () const
{
	impl->bitmapDrawCount.clear ();
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawLine (LinePair line) const
{
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		impl->applyLineStyle ();

		CGContextBeginPath (context);
		CGPoint first = CGPointFromCPoint (line.first);
		CGPoint second = CGPointFromCPoint (line.second);

		if (impl->state.drawMode.integralMode ())
		{
			first = impl->pixelAlligned (first);
			second = impl->pixelAlligned (second);
			impl->applyLineWidthCTM ();
		}

		CGContextMoveToPoint (context, first.x, first.y);
		CGContextAddLineToPoint (context, second.x, second.y);

		CGContextDrawPath (context, kCGPathStroke);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawLines (const LineList& lines) const
{
	vstgui_assert (lines.empty () == false);
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		impl->applyLineStyle ();
		static constexpr auto numStackPoints = 32;
		CGPoint stackPoints[numStackPoints];

		auto cgPointsPtr = (lines.size () * 2) < numStackPoints
							   ? nullptr
							   : std::unique_ptr<CGPoint[]> (new CGPoint[lines.size () * 2]);
		auto cgPoints = cgPointsPtr ? cgPointsPtr.get () : stackPoints;
		uint32_t index = 0;
		if (impl->state.drawMode.integralMode ())
		{
			for (const auto& line : lines)
			{
				cgPoints[index] = impl->pixelAlligned (CGPointFromCPoint (line.first));
				cgPoints[index + 1] = impl->pixelAlligned (CGPointFromCPoint (line.second));
				index += 2;
			}
		}
		else
		{
			for (const auto& line : lines)
			{
				cgPoints[index] = CGPointFromCPoint (line.first);
				cgPoints[index + 1] = CGPointFromCPoint (line.second);
				index += 2;
			}
		}

		if (impl->state.drawMode.integralMode ())
			impl->applyLineWidthCTM ();

		const size_t maxPointsPerIteration = 16;
		const CGPoint* pointPtr = cgPoints;
		size_t numPoints = lines.size () * 2;
		while (numPoints)
		{
			size_t np = std::min (numPoints, std::min (maxPointsPerIteration, numPoints));
			CGContextStrokeLineSegments (context, pointPtr, np);
			numPoints -= np;
			pointPtr += np;
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawPolygon (const PointList& polygonPointList,
											 PlatformGraphicsDrawStyle drawStyle) const
{
	vstgui_assert (polygonPointList.empty () == false);
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		CGPathDrawingMode m = convert (drawStyle);
		impl->applyLineStyle ();

		CGContextBeginPath (context);
		CGPoint p = CGPointFromCPoint (polygonPointList[0]);
		if (impl->state.drawMode.integralMode ())
			p = impl->pixelAlligned (p);
		CGContextMoveToPoint (context, p.x, p.y);
		for (uint32_t i = 1; i < polygonPointList.size (); i++)
		{
			p = CGPointFromCPoint (polygonPointList[i]);
			if (impl->state.drawMode.integralMode ())
				p = impl->pixelAlligned (p);
			CGContextAddLineToPoint (context, p.x, p.y);
		}
		CGContextDrawPath (context, m);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawRect (CRect rect, PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		CGRect r = CGRectFromCRect (rect);
		if (drawStyle != PlatformGraphicsDrawStyle::Filled)
		{
			r.size.width -= 1.;
			r.size.height -= 1.;
		}

		CGPathDrawingMode m = convert (drawStyle);
		impl->applyLineStyle ();

		if (impl->state.drawMode.integralMode ())
		{
			r = impl->pixelAlligned (r);
			if (drawStyle != PlatformGraphicsDrawStyle::Filled)
				impl->applyLineWidthCTM ();
		}

		CGContextBeginPath (context);
		CGContextAddRect (context, r);
		CGContextDrawPath (context, m);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawArc (CRect rect, double startAngle1, double endAngle2,
										 PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		CGPathDrawingMode m = convert (drawStyle);
		impl->applyLineStyle ();

		CGContextBeginPath (context);

		CPoint center (rect.left + rect.getWidth () / 2., rect.top + rect.getHeight () / 2.);
		auto rX = rect.getWidth () / 2.;
		auto rY = rect.getHeight () / 2.;

		Impl::DoGraphicStateSave (context, [&] () {
			CGContextTranslateCTM (context, static_cast<CGFloat> (center.x),
								   static_cast<CGFloat> (center.y));
			CGContextScaleCTM (context, rX, rY);

			auto startAngle = radians (startAngle1);
			auto endAngle = radians (endAngle2);
			if (rX != rY)
			{
				startAngle = std::atan2 (std::sin (startAngle) * rX, std::cos (startAngle) * rY);
				endAngle = std::atan2 (std::sin (endAngle) * rX, std::cos (endAngle) * rY);
			}
			CGContextMoveToPoint (context, static_cast<CGFloat> (std::cos (startAngle)),
								  static_cast<CGFloat> (std::sin (startAngle)));
			CGContextAddArc (context, 0, 0, 1, static_cast<CGFloat> (startAngle),
							 static_cast<CGFloat> (endAngle), 0);
		});
		CGContextDrawPath (context, m);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawEllipse (CRect rect, PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		CGRect r = CGRectFromCRect (rect);
		if (drawStyle != PlatformGraphicsDrawStyle::Filled)
		{
			r.size.width -= 1.;
			r.size.height -= 1.;
		}

		CGPathDrawingMode m = convert (drawStyle);
		impl->applyLineStyle ();
		if (impl->state.drawMode.integralMode ())
		{
			if (drawStyle != PlatformGraphicsDrawStyle::Filled)
				impl->applyLineWidthCTM ();
			r = impl->pixelAlligned (r);
		}

		CGContextAddEllipseInRect (context, r);
		CGContextDrawPath (context, m);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawPoint (CPoint point, CColor color) const { return false; }

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawBitmap (IPlatformBitmap& bitmap, CRect dest, CPoint offset,
											double alpha, BitmapInterpolationQuality quality) const
{
	auto cgBitmap = dynamic_cast<CGBitmap*> (&bitmap);
	if (!cgBitmap)
		return false;
	auto cgImage = cgBitmap->getCGImage ();
	if (!cgImage)
		return false;
	impl->doInCGContext (false, true, [&] (auto context) {
		impl->setBitmapInterpolationQuality (quality);

		CGLayerRef layer = nullptr;
		if (impl->backendScaleFactor == 1.)
		{
			layer = cgBitmap->getCGLayer ();
			if (layer == nullptr)
			{
				auto it = impl->bitmapDrawCount.find (cgBitmap);
				if (it == impl->bitmapDrawCount.end ())
				{
					impl->bitmapDrawCount.emplace (cgBitmap, 1);
				}
				else
				{
					it->second++;
					layer = cgBitmap->createCGLayer (context);
				}
			}
		}

		impl->drawCGImageRef (context, cgImage, layer, cgBitmap->getScaleFactor (), dest, offset,
							  alpha);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::clearRect (CRect rect) const
{
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		CGRect cgRect = CGRectFromCRect (rect);
		if (impl->state.drawMode.integralMode ())
			cgRect = impl->pixelAlligned (cgRect);
		CGContextClearRect (context, cgRect);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawGraphicsPath (IPlatformGraphicsPath& path,
												  PlatformGraphicsPathDrawMode mode,
												  TransformMatrix* transformation) const
{
	auto cgPath = dynamic_cast<CGGraphicsPath*> (&path);
	if (!cgPath)
		return false;

	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		auto cgMode = convert (mode);
		if (cgMode == kCGPathStroke)
			impl->applyLineStyle ();
		Impl::DoGraphicStateSave (context, [&] () {
			if (transformation)
			{
				CGAffineTransform transform = createCGAffineTransform (*transformation);
				CGContextConcatCTM (context, transform);
			}
			if (impl->state.drawMode.integralMode () && impl->state.drawMode.aliasing ())
			{
				Impl::DoGraphicStateSave (context, [&] () {
					impl->applyLineWidthCTM ();
					cgPath->pixelAlign (
						[] (const CGPoint& p, void* context) {
							auto devContext = reinterpret_cast<Impl*> (context);
							return devContext->pixelAlligned (p);
						},
						impl.get ());
				});
				CGContextAddPath (context, cgPath->getCGPathRef ());
			}
			else
				CGContextAddPath (context, cgPath->getCGPathRef ());
		});
		CGContextDrawPath (context, cgMode);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::fillLinearGradient (IPlatformGraphicsPath& path,
													const IPlatformGradient& gradient,
													CPoint startPoint, CPoint endPoint,
													bool evenOdd,
													TransformMatrix* transformation) const
{
	auto cgPath = dynamic_cast<CGGraphicsPath*> (&path);
	if (!cgPath)
		return false;
	auto cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (!cgGradient)
		return false;

	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		CGPoint start = CGPointFromCPoint (startPoint);
		CGPoint end = CGPointFromCPoint (endPoint);
		Impl::DoGraphicStateSave (context, [&] () {
			if (impl->state.drawMode.integralMode ())
			{
				start = impl->pixelAlligned (start);
				end = impl->pixelAlligned (end);
			}
			if (transformation)
			{
				CGAffineTransform transform = createCGAffineTransform (*transformation);
				CGContextConcatCTM (context, transform);
			}
			if (impl->state.drawMode.integralMode () && impl->state.drawMode.aliasing ())
			{
				cgPath->pixelAlign (
					[] (const CGPoint& p, void* context) {
						auto devContext = reinterpret_cast<Impl*> (context);
						return devContext->pixelAlligned (p);
					},
					impl.get ());
			}

			CGContextAddPath (context, cgPath->getCGPathRef ());
		});

		if (evenOdd)
			CGContextEOClip (context);
		else
			CGContextClip (context);

		CGContextDrawLinearGradient (context, *cgGradient, start, end,
									 kCGGradientDrawsBeforeStartLocation |
										 kCGGradientDrawsAfterEndLocation);
	});
	return true;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::fillRadialGradient (IPlatformGraphicsPath& path,
													const IPlatformGradient& gradient,
													CPoint center, CCoord radius,
													CPoint originOffset, bool evenOdd,
													TransformMatrix* transformation) const
{
	auto cgPath = dynamic_cast<CGGraphicsPath*> (&path);
	if (!cgPath)
		return false;
	auto cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (!cgGradient)
		return false;

	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		Impl::DoGraphicStateSave (context, [&] () {
			if (transformation)
			{
				CGAffineTransform transform = createCGAffineTransform (*transformation);
				CGContextConcatCTM (context, transform);
			}
			if (impl->state.drawMode.integralMode () && impl->state.drawMode.aliasing ())
			{
				cgPath->pixelAlign (
					[] (const CGPoint& p, void* context) {
						auto devContext = reinterpret_cast<Impl*> (context);
						return devContext->pixelAlligned (p);
					},
					impl.get ());
			}

			CGContextAddPath (context, cgPath->getCGPathRef ());
		});

		if (evenOdd)
			CGContextEOClip (context);
		else
			CGContextClip (context);

		CPoint startCenter = center + originOffset;
		CGContextDrawRadialGradient (context, *cgGradient, CGPointFromCPoint (startCenter), 0,
									 CGPointFromCPoint (center), static_cast<CGFloat> (radius),
									 kCGGradientDrawsBeforeStartLocation |
										 kCGGradientDrawsAfterEndLocation);
	});
	return true;
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::saveGlobalState () const
{
	CGContextSaveGState (impl->cgContext);
	impl->stateStack.push (impl->state);
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::restoreGlobalState () const
{
	vstgui_assert (impl->stateStack.empty () == false,
				   "Unbalanced calls to saveGlobalState and restoreGlobalState");
#if NDEBUG
	if (impl->stateStack.empty ())
		return;
#endif
	CGContextRestoreGState (impl->cgContext);
	impl->state = impl->stateStack.top ();
	impl->stateStack.pop ();
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setLineStyle (const CLineStyle& style) const
{
	impl->state.lineStyle = style;
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setLineWidth (CCoord width) const
{
	impl->state.lineWidth = width;
	CGContextSetLineWidth (impl->cgContext, static_cast<CGFloat> (width));
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setDrawMode (CDrawMode mode) const
{
	impl->state.drawMode = mode;
	CGContextSetShouldAntialias (impl->cgContext, mode.antiAliasing ());
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setClipRect (CRect clip) const { impl->state.clipRect = clip; }

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setFillColor (CColor color) const
{
	CGContextSetFillColorWithColor (impl->cgContext, getCGColor (color));
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setFrameColor (CColor color) const
{
	CGContextSetStrokeColorWithColor (impl->cgContext, getCGColor (color));
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setGlobalAlpha (double newAlpha) const
{
	impl->state.globalAlpha = newAlpha;
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::setTransformMatrix (const TransformMatrix& tm) const
{
	impl->state.tm = tm;
}

//------------------------------------------------------------------------
const IPlatformGraphicsDeviceContextBitmapExt* CoreGraphicsDeviceContext::asBitmapExt () const
{
	return nullptr;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::drawBitmapNinePartTiled (IPlatformBitmap& bitmap, CRect dest,
														 const CNinePartTiledDescription& desc,
														 double alpha,
														 BitmapInterpolationQuality quality) const
{
	// Not Supported
	return false;
}

//------------------------------------------------------------------------
bool CoreGraphicsDeviceContext::fillRectWithBitmap (IPlatformBitmap& bitmap, CRect srcRect,
													CRect dstRect, double alpha,
													BitmapInterpolationQuality quality) const
{
#if 0
	auto cgBitmap = dynamic_cast<CGBitmap*> (&bitmap);
	if (!cgBitmap)
		return false;
	auto cgImage = cgBitmap->getCGImage ();
	if (cgImage)
		return false;
	impl->doInCGContext (false, true, [&] (auto context) {
		impl->setBitmapInterpolationQuality (quality);

		// TODO: Check if this works with retina images

		CGRect clipRect = CGRectFromCRect (dstRect);
		clipRect.origin.y = -(clipRect.origin.y) - clipRect.size.height;
		clipRect = impl->pixelAlligned (clipRect);
		CGContextClipToRect (context, clipRect);

		CGRect r = {};
		r.size.width = CGImageGetWidth (cgImage);
		r.size.height = CGImageGetHeight (cgImage);

		CGContextDrawTiledImage (context, r, cgImage);
	});
#endif
	return false;
}

//------------------------------------------------------------------------
void CoreGraphicsDeviceContext::drawCTLine (CTLineRef line, CGPoint cgPoint, CTFontRef fontRef,
											CColor color, bool underline, bool strikeThrough,
											bool antialias) const
{
	impl->doInCGContext (true, impl->state.drawMode.integralMode (), [&] (auto context) {
		if (impl->state.drawMode.integralMode ())
			cgPoint = impl->pixelAlligned (cgPoint);
		CGContextSetShouldAntialias (context, antialias);
		CGContextSetShouldSmoothFonts (context, impl->shouldSmoothFonts);
		CGContextSetShouldSubpixelPositionFonts (context, true);
		CGContextSetShouldSubpixelQuantizeFonts (context, true);
		CGContextSetTextPosition (context, cgPoint.x, cgPoint.y);
		CTLineDraw (line, context);
		CGColorRef cgColorRef = nullptr;
		if (underline)
		{
			cgColorRef = getCGColor (color);
			CGFloat underlineOffset = CTFontGetUnderlinePosition (fontRef) - 1.f;
			CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
			CGContextSetStrokeColorWithColor (context, cgColorRef);
			CGContextSetLineWidth (context, underlineThickness);
			auto cgPoint2 = CGContextGetTextPosition (context);
			CGContextBeginPath (context);
			CGContextMoveToPoint (context, cgPoint.x, cgPoint.y - underlineOffset);
			CGContextAddLineToPoint (context, cgPoint2.x, cgPoint.y - underlineOffset);
			CGContextDrawPath (context, kCGPathStroke);
		}
		if (strikeThrough)
		{
			if (!cgColorRef)
				cgColorRef = getCGColor (color);
			CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
			CGFloat offset = CTFontGetXHeight (fontRef) * 0.5f;
			CGContextSetStrokeColorWithColor (context, cgColorRef);
			CGContextSetLineWidth (context, underlineThickness);
			auto cgPoint2 = CGContextGetTextPosition (context);
			CGContextBeginPath (context);
			CGContextMoveToPoint (context, cgPoint.x, cgPoint.y - offset);
			CGContextAddLineToPoint (context, cgPoint2.x, cgPoint.y - offset);
			CGContextDrawPath (context, kCGPathStroke);
		}
	});
}

//------------------------------------------------------------------------
CoreGraphicsBitmapContext::CoreGraphicsBitmapContext (const CoreGraphicsDevice& device,
													  void* cgContext, EndDrawFunc&& f)
: CoreGraphicsDeviceContext (device, cgContext), endDrawFunc (std::move (f))
{
}

//------------------------------------------------------------------------
bool CoreGraphicsBitmapContext::endDraw () const
{
	endDrawFunc ();
	return CoreGraphicsDeviceContext::endDraw ();
}

//------------------------------------------------------------------------
} // VSTGUI
