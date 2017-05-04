// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cgdrawcontext.h"

#if MAC

#include "macglobals.h"
#include "cgbitmap.h"
#include "quartzgraphicspath.h"
#include "cfontmac.h"
#include "../../cbitmap.h"

#ifndef CGFLOAT_DEFINED
	#define CGFLOAT_DEFINED
	using CGFloat = float;
#endif // CGFLOAT_DEFINED

namespace VSTGUI {

namespace CGDrawContextInternal {

//-----------------------------------------------------------------------------
static void addOvalToPath (CGContextRef c, CPoint center, CGFloat a, CGFloat b, CGFloat start_angle, CGFloat end_angle)
{
	CGContextSaveGState (c);
	CGContextTranslateCTM (c, static_cast<CGFloat> (center.x), static_cast<CGFloat> (center.y));
	CGContextScaleCTM (c, a, b);
	
	double startAngle = radians (start_angle);
	double endAngle = radians (end_angle);
	if (a != b)
	{
		startAngle = std::atan2 (std::sin (startAngle) * a, std::cos (startAngle) * b);
		endAngle = std::atan2 (std::sin (endAngle) * a, std::cos (endAngle) * b);
	}
	CGContextMoveToPoint (c, static_cast<CGFloat> (std::cos (startAngle)), static_cast<CGFloat> (std::sin (startAngle)));
	
	CGContextAddArc (c, 0, 0, 1, static_cast<CGFloat> (startAngle), static_cast<CGFloat> (endAngle), 0);
	
	CGContextRestoreGState(c);
}

}

//-----------------------------------------------------------------------------
CGDrawContext::CGDrawContext (CGContextRef cgContext, const CRect& rect)
: COffscreenContext (rect)
, cgContext (cgContext)
, scaleFactor (1.)
{
	CFRetain (cgContext);

	// Get the scale for the context to check if it is for a Retina display
	CGRect userRect = CGRectMake (0, 0, 100, 100);
	CGRect deviceRect = CGContextConvertRectToDeviceSpace (cgContext, userRect);
	scaleFactor = deviceRect.size.height / userRect.size.height;

	init ();
}

//-----------------------------------------------------------------------------
CGDrawContext::CGDrawContext (CGBitmap* _bitmap)
: COffscreenContext (new CBitmap (_bitmap))
, cgContext (_bitmap->createCGContext ())
, scaleFactor (_bitmap->getScaleFactor ())
{
	if (scaleFactor != 1.)
	{
		CGContextConcatCTM (cgContext, CGAffineTransformMakeScale (static_cast<CGFloat> (scaleFactor), static_cast<CGFloat> (scaleFactor)));
	}

	init ();
	bitmap->forget ();
}

//-----------------------------------------------------------------------------
void CGDrawContext::init ()
{
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

	CDrawContext::init ();
}

//-----------------------------------------------------------------------------
CGDrawContext::~CGDrawContext () noexcept
{
	CGContextRestoreGState (cgContext); // restore the original state
	CGContextRestoreGState (cgContext); // we need to do it twice !!!
	CFRelease (cgContext);
}

//-----------------------------------------------------------------------------
void CGDrawContext::endDraw ()
{
	CGContextSynchronize (cgContext);
	if (bitmap && bitmap->getPlatformBitmap ())
	{
		CGBitmap* cgBitmap = dynamic_cast<CGBitmap*> (bitmap->getPlatformBitmap ().get ());
		if (cgBitmap)
			cgBitmap->setDirty ();
	}
	bitmapDrawCount.clear ();
}

//-----------------------------------------------------------------------------
CGraphicsPath* CGDrawContext::createGraphicsPath ()
{
	return new QuartzGraphicsPath;
}

//-----------------------------------------------------------------------------
CGraphicsPath* CGDrawContext::createTextPath (const CFontRef font, UTF8StringPtr text)
{
	const CoreTextFont* ctFont = font->getPlatformFont ().cast<const CoreTextFont>();
	return ctFont ? new QuartzGraphicsPath (ctFont, text) : nullptr;
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == nullptr)
		return;

	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGPathDrawingMode cgMode;
		switch (mode)
		{
			case kPathFilledEvenOdd:
			{
				cgMode = kCGPathEOFill;
				break;
			}
			case kPathStroked:
			{
				cgMode = kCGPathStroke;
				applyLineStyle (context);
				break;
			}
			default:
			{
				cgMode = kCGPathFill;
				break;
			}
		}
		
		CGContextSaveGState (context);
		if (t)
		{
			CGAffineTransform transform = QuartzGraphicsPath::createCGAffineTransform (*t);
			CGContextConcatCTM (context, transform);
		}
		if (getDrawMode ().integralMode () && getDrawMode ().aliasing ())
		{
			CGContextSaveGState (context);
			applyLineWidthCTM (context);
			path->pixelAlign (this);
			CGContextRestoreGState (context);
			CGContextAddPath (context, path->getCGPathRef ());
		}
		else
			CGContextAddPath (context, path->getCGPathRef ());
		
		CGContextRestoreGState (context);
		CGContextDrawPath (context, cgMode);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::fillLinearGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == nullptr)
		return;

	const QuartzGradient* cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (cgGradient == nullptr)
		return;

	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGContextSaveGState (context);
		CGPoint start = CGPointFromCPoint (startPoint);
		CGPoint end = CGPointFromCPoint (endPoint);
		if (getDrawMode ().integralMode ())
		{
			start = pixelAlligned (start);
			end = pixelAlligned (end);
		}
		if (t)
		{
			CGAffineTransform transform = QuartzGraphicsPath::createCGAffineTransform (*t);
			CGContextConcatCTM (context, transform);
		}
		if (getDrawMode ().integralMode () && getDrawMode ().aliasing ())
			path->pixelAlign (this);

		CGContextAddPath (context, path->getCGPathRef ());
		CGContextRestoreGState (context);

		if (evenOdd)
			CGContextEOClip (context);
		else
			CGContextClip (context);

		CGContextDrawLinearGradient (context, *cgGradient, start, end, kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::fillRadialGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset, bool evenOdd, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == nullptr)
		return;

	const QuartzGradient* cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (cgGradient == nullptr)
		return;

	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGContextSaveGState (context);
		if (t)
		{
			CGAffineTransform transform = QuartzGraphicsPath::createCGAffineTransform (*t);
			CGContextConcatCTM (context, transform);
		}
		if (getDrawMode ().integralMode () && getDrawMode ().aliasing ())
			path->pixelAlign (this);
		
		CGContextAddPath (context, path->getCGPathRef ());
		CGContextRestoreGState (context);
		
		if (evenOdd)
			CGContextEOClip (context);
		else
			CGContextClip (context);
		
		CPoint startCenter = center + originOffset;
		CGContextDrawRadialGradient (context, *cgGradient, CGPointFromCPoint (startCenter), 0, CGPointFromCPoint (center), static_cast<CGFloat> (radius), kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::saveGlobalState ()
{
	CGContextSaveGState (cgContext);
	CDrawContext::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void CGDrawContext::restoreGlobalState ()
{
	CDrawContext::restoreGlobalState ();
	CGContextRestoreGState (cgContext);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setGlobalAlpha (float newAlpha)
{
	if (newAlpha == currentState.globalAlpha)
		return;

	CGContextSetAlpha (cgContext, newAlpha);

	CDrawContext::setGlobalAlpha (newAlpha);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setLineStyle (const CLineStyle& style)
{
	if (currentState.lineStyle == style)
		return;

	CDrawContext::setLineStyle (style);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setLineWidth (CCoord width)
{
	if (currentState.frameWidth == width)
		return;

	CGContextSetLineWidth (cgContext, static_cast<CGFloat> (width));

	CDrawContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setDrawMode (CDrawMode mode)
{
	if (cgContext)
		CGContextSetShouldAntialias (cgContext, mode.antiAliasing ());

	CDrawContext::setDrawMode (mode);
}

//------------------------------------------------------------------------------
void CGDrawContext::setClipRect (const CRect &clip)
{
	CDrawContext::setClipRect (clip);
}

//------------------------------------------------------------------------------
void CGDrawContext::resetClipRect ()
{
	CDrawContext::resetClipRect ();
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawLine (const LinePair& line)
{
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		applyLineStyle (context);
		
		CGContextBeginPath (context);
		CGPoint first = CGPointFromCPoint (line.first);
		CGPoint second = CGPointFromCPoint (line.second);

		if (getDrawMode ().integralMode ())
		{
			first = pixelAlligned (first);
			second = pixelAlligned (second);
			applyLineWidthCTM (context);
		}

		CGContextMoveToPoint (context, first.x, first.y);
		CGContextAddLineToPoint (context, second.x, second.y);

		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawLines (const LineList& lines)
{
	if (lines.size () == 0)
		return;
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		applyLineStyle (context);
		CGPoint* cgPoints = new CGPoint[lines.size () * 2];
		uint32_t index = 0;
		for (const auto& line : lines)
		{
			cgPoints[index] = CGPointFromCPoint (line.first);
			cgPoints[index+1] = CGPointFromCPoint (line.second);
			if (getDrawMode ().integralMode ())
			{
				cgPoints[index] = pixelAlligned (cgPoints[index]);
				cgPoints[index+1] = pixelAlligned (cgPoints[index+1]);
			}
			index += 2;
		}

		if (getDrawMode ().integralMode ())
			applyLineWidthCTM (context);
		
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
		delete [] cgPoints;
		
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
	if (polygonPointList.size () == 0)
		return;
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineStyle (context);

		CGContextBeginPath (context);
		CGPoint p = CGPointFromCPoint(polygonPointList[0]);
		if (getDrawMode ().integralMode ())
			p = pixelAlligned (p);
		CGContextMoveToPoint (context, p.x, p.y);
		for (uint32_t i = 1; i < polygonPointList.size (); i++)
		{
			p = CGPointFromCPoint (polygonPointList[i]);
			if (getDrawMode ().integralMode ())
				p = pixelAlligned (p);
			CGContextAddLineToPoint (context, p.x, p.y);
		}
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::applyLineWidthCTM (CGContextRef context) const
{
	int32_t frameWidth = static_cast<int32_t> (currentState.frameWidth);
	if (frameWidth % 2)
		CGContextTranslateCTM (context, 0.5, 0.5);
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawRect (const CRect &rect, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGRect r = CGRectFromCRect (rect);
		if (drawStyle != kDrawFilled)
		{
			r.size.width -= 1.;
			r.size.height -= 1.;
		}

		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineStyle (context);

		if (getDrawMode ().integralMode ())
		{
			r = pixelAlligned (r);
			if (drawStyle != kDrawFilled)
				applyLineWidthCTM (context);
		}

		CGContextBeginPath (context);
		CGContextAddRect (context, r);
		CGContextDrawPath (context, m);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawEllipse (const CRect &rect, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGRect r = CGRectFromCRect (rect);
		if (drawStyle != kDrawFilled)
		{
			r.size.width -= 1.;
			r.size.height -= 1.;
		}

		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineStyle (context);
		if (getDrawMode ().integralMode ())
		{
			if (drawStyle != kDrawFilled)
				applyLineWidthCTM (context);
			r = pixelAlligned (r);
		}

		CGContextAddEllipseInRect (context, r);
		CGContextDrawPath (context, m);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawPoint (const CPoint &point, const CColor& color)
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
void CGDrawContext::drawArc (const CRect &rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle) // in degree
{
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineStyle (context);

		CGContextBeginPath (context);
		CGDrawContextInternal::addOvalToPath (context, CPoint (rect.left + rect.getWidth () / 2., rect.top + rect.getHeight () / 2.), static_cast<CGFloat> (rect.getWidth () / 2.), static_cast<CGFloat> (rect.getHeight () / 2.), _startAngle, _endAngle);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& inRect, const CNinePartTiledDescription& desc, float alpha)
{
	// TODO: When drawing on a scaled transform the bitmaps are not alligned correctly
	CDrawContext::drawBitmapNinePartTiled (bitmap, inRect, desc, alpha);
	return;
}

//-----------------------------------------------------------------------------
void CGDrawContext::fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect, float alpha)
{
	if (bitmap == nullptr || alpha == 0.f || srcRect.isEmpty () || dstRect.isEmpty ())
		return;

	if (!(srcRect.left == 0 && srcRect.right == 0 && srcRect.right == bitmap->getWidth () && srcRect.bottom == bitmap->getHeight ()))
	{
		// CGContextDrawTiledImage does not work with parts of a bitmap
		CDrawContext::fillRectWithBitmap(bitmap, srcRect, dstRect, alpha);
		return;
	}

	IPlatformBitmap* platformBitmap = bitmap->getBestPlatformBitmapForScaleFactor (scaleFactor);
	CPoint bitmapSize = platformBitmap->getSize ();
	if (srcRect.right > bitmapSize.x || srcRect.bottom > bitmapSize.y)
		return;

	CGBitmap* cgBitmap = platformBitmap ? dynamic_cast<CGBitmap*> (platformBitmap) : nullptr;
	CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : nullptr;
	if (image)
	{
		CGContextRef context = beginCGContext (false, true);
		if (context)
		{
			// TODO: Check if this works with retina images
			CGRect clipRect = CGRectFromCRect (dstRect);
			clipRect.origin.y = -(clipRect.origin.y) - clipRect.size.height;
			clipRect = pixelAlligned (clipRect);
			CGContextClipToRect (context, clipRect);
			
			CGRect r = {};
			r.size.width = CGImageGetWidth (image);
			r.size.height = CGImageGetHeight (image);
			
			CGContextDrawTiledImage (context, r, image);
			
			releaseCGContext (context);
		}
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawBitmap (CBitmap* bitmap, const CRect& inRect, const CPoint& inOffset, float alpha)
{
	if (bitmap == nullptr || alpha == 0.f)
		return;
	double transformedScaleFactor = scaleFactor;
	CGraphicsTransform t = getCurrentTransform ();
	if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
		transformedScaleFactor *= t.m11;
	IPlatformBitmap* platformBitmap = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor);
	CGBitmap* cgBitmap = platformBitmap ? dynamic_cast<CGBitmap*> (platformBitmap) : nullptr;
	CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : nullptr;
	if (image)
	{
		CGContextRef context = beginCGContext (false, true);
		if (context)
		{
			CGLayerRef layer = cgBitmap->getCGLayer ();
			if (layer == nullptr)
			{
				auto it = bitmapDrawCount.find (cgBitmap);
				if (it == bitmapDrawCount.end ())
				{
					bitmapDrawCount.emplace (cgBitmap, 1);
				}
				else
				{
					it->second++;
					layer = cgBitmap->createCGLayer (context);
				}
			}

			drawCGImageRef (context, image, layer, cgBitmap->getScaleFactor (), inRect, inOffset, alpha, bitmap);

			releaseCGContext (context);
		}
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawCGImageRef (CGContextRef context, CGImageRef image, CGLayerRef layer, double bitmapScaleFactor, const CRect& inRect, const CPoint& inOffset, float alpha, CBitmap* bitmap)
{
	CRect rect (inRect);
	CPoint offset (inOffset);
	
	CGContextSetAlpha (context, (CGFloat)alpha*currentState.globalAlpha);
	
	CGRect dest;
	dest.origin.x = static_cast<CGFloat> (rect.left - offset.x);
	dest.origin.y = static_cast<CGFloat> (-(rect.top) - (bitmap->getHeight () - offset.y));
	dest.size.width = static_cast<CGFloat> (bitmap->getWidth ());
	dest.size.height = static_cast<CGFloat> (bitmap->getHeight ());
	
	CGRect clipRect;
	clipRect.origin.x = static_cast<CGFloat> (rect.left);
	clipRect.origin.y = static_cast<CGFloat> (-(rect.top) - rect.getHeight ());
	clipRect.size.width = static_cast<CGFloat> (rect.getWidth ());
	clipRect.size.height = static_cast<CGFloat> (rect.getHeight ());
	

	if (bitmapScaleFactor != 1.)
	{
		CGContextConcatCTM (context, CGAffineTransformMakeScale (static_cast<CGFloat> (1./bitmapScaleFactor), static_cast<CGFloat> (1./bitmapScaleFactor)));
		CGAffineTransform transform = CGAffineTransformMakeScale (static_cast<CGFloat> (bitmapScaleFactor), static_cast<CGFloat> (bitmapScaleFactor));
		clipRect.origin = CGPointApplyAffineTransform (clipRect.origin, transform);
		clipRect.size = CGSizeApplyAffineTransform (clipRect.size, transform);
		dest.origin = CGPointApplyAffineTransform (dest.origin, transform);
		dest.size = CGSizeApplyAffineTransform (dest.size, transform);
	}
//	dest.origin = pixelAlligned (dest.origin);
	clipRect.origin = pixelAlligned (clipRect.origin);
	
	CGContextClipToRect (context, clipRect);
	
	if (layer)
	{
		CGContextDrawLayerInRect (context, dest, layer);
	}
	else
	{
		CGContextDrawImage (context, dest, image);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::clearRect (const CRect& rect)
{
	CGContextRef context = beginCGContext (true, getDrawMode ().integralMode ());
	if (context)
	{
		CGRect cgRect = CGRectFromCRect (rect);
		if (getDrawMode ().integralMode ())
		{
			cgRect = pixelAlligned (cgRect);
		}
		CGContextClearRect (context, cgRect);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFontColor (const CColor& color)
{
	if (currentState.fontColor == color)
		return;

	CDrawContext::setFontColor (color);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFrameColor (const CColor& color)
{
	if (currentState.frameColor == color)
		return;
	
	if (cgContext)
		CGContextSetStrokeColorWithColor (cgContext, getCGColor (color));

	CDrawContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFillColor (const CColor& color)
{
	if (currentState.fillColor == color)
		return;

	if (cgContext)
		CGContextSetFillColorWithColor (cgContext, getCGColor (color));

	CDrawContext::setFillColor (color);
}

#if DEBUG
bool showClip = false;
#endif

//-----------------------------------------------------------------------------
CGContextRef CGDrawContext::beginCGContext (bool swapYAxis, bool integralOffset)
{
	if (cgContext)
	{
		if (currentState.clipRect.isEmpty ())
			return nullptr;

		CGContextSaveGState (cgContext);

		CGRect cgClipRect = CGRectFromCRect (currentState.clipRect);
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

		if (getCurrentTransform ().isInvariant () == false)
		{
			CGraphicsTransform t = getCurrentTransform ();
			if (integralOffset)
			{
				CGPoint p = pixelAlligned (CGPointFromCPoint (CPoint (t.dx, t.dy)));
				t.dx = p.x;
				t.dy = p.y;
			}
			CGContextConcatCTM (cgContext, QuartzGraphicsPath::createCGAffineTransform (t));
		}
		
		if (!swapYAxis)
			CGContextScaleCTM (cgContext, 1, -1);
		
		return cgContext;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void CGDrawContext::releaseCGContext (CGContextRef context)
{
	if (context)
	{
		CGContextRestoreGState (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::applyLineStyle (CGContextRef context)
{
	switch (currentState.lineStyle.getLineCap ())
	{
		case CLineStyle::kLineCapButt: CGContextSetLineCap (context, kCGLineCapButt); break;
		case CLineStyle::kLineCapRound: CGContextSetLineCap (context, kCGLineCapRound); break;
		case CLineStyle::kLineCapSquare: CGContextSetLineCap (context, kCGLineCapSquare); break;
	}
	switch (currentState.lineStyle.getLineJoin ())
	{
		case CLineStyle::kLineJoinMiter: CGContextSetLineJoin (context, kCGLineJoinMiter); break;
		case CLineStyle::kLineJoinRound: CGContextSetLineJoin (context, kCGLineJoinRound); break;
		case CLineStyle::kLineJoinBevel: CGContextSetLineJoin (context, kCGLineJoinBevel); break;
	}
	if (currentState.lineStyle.getDashCount () > 0)
	{
		CGFloat* dashLengths = new CGFloat [currentState.lineStyle.getDashCount ()];
		for (uint32_t i = 0; i < currentState.lineStyle.getDashCount (); i++)
		{
			dashLengths[i] = static_cast<CGFloat> (currentState.frameWidth * currentState.lineStyle.getDashLengths ()[i]);
		}
		CGContextSetLineDash (context, static_cast<CGFloat> (currentState.lineStyle.getDashPhase ()), dashLengths, currentState.lineStyle.getDashCount ());
		delete [] dashLengths;
	}
}

//-----------------------------------------------------------------------------
CGRect CGDrawContext::pixelAlligned (const CGRect& r) const
{
	CGRect result;
	result.origin = CGContextConvertPointToDeviceSpace (cgContext, r.origin);
	result.size = CGContextConvertSizeToDeviceSpace (cgContext, r.size);
	result.origin.x = std::round (result.origin.x);
	result.origin.y = std::round (result.origin.y);
	result.size.width = std::round (result.size.width);
	result.size.height = std::round (result.size.height);
	result.origin = CGContextConvertPointToUserSpace (cgContext, result.origin);
	result.size = CGContextConvertSizeToUserSpace (cgContext, result.size);
	return result;
}

//-----------------------------------------------------------------------------
CGPoint CGDrawContext::pixelAlligned (const CGPoint& p) const
{
	CGPoint result = CGContextConvertPointToDeviceSpace (cgContext, p);
	result.x = std::round (result.x);
	result.y = std::round (result.y);
	result = CGContextConvertPointToUserSpace (cgContext, result);
	return result;
}

} // namespace

#endif // MAC

