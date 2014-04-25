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

#include "cgdrawcontext.h"

#if MAC

#include "macglobals.h"
#include "cgbitmap.h"
#include "quartzgraphicspath.h"
#include "cfontmac.h"

#ifndef CGFLOAT_DEFINED
	#define CGFLOAT_DEFINED
	typedef float CGFloat;
#endif // CGFLOAT_DEFINED

namespace VSTGUI {

namespace CGDrawContextInternal {

//-----------------------------------------------------------------------------
static void addOvalToPath (CGContextRef c, CPoint center, CGFloat a, CGFloat b, CGFloat start_angle, CGFloat end_angle)
{
	CGContextSaveGState (c);
	CGContextTranslateCTM (c, center.x, center.y);
	CGContextScaleCTM (c, a, b);
	CGContextRotateCTM (c, radians (-90.f));
	
	CGContextMoveToPoint (c, cos (radians (start_angle)), sin (radians (start_angle)));
	
	CGContextAddArc(c, 0, 0, 1, radians (start_angle), radians (end_angle), 1);
	
	CGContextRestoreGState(c);
}

//-----------------------------------------------------------------------------
static CCoord roundCoord (const CCoord c, double scaleFactor)
{
	return round (c * scaleFactor * 10) / (10 * scaleFactor);
}

//-----------------------------------------------------------------------------
static void roundPoint (CPoint& p, double scaleFactor)
{
	p.x = roundCoord (p.x, scaleFactor);
	p.y = roundCoord (p.y, scaleFactor);
}

//-----------------------------------------------------------------------------
static CRect makeRectIntegral (const CRect r, double scaleFactor)
{
	CRect result;
	result.left = roundCoord (r.left, scaleFactor);
	result.right = roundCoord (r.right, scaleFactor);
	result.top = roundCoord (r.top, scaleFactor);
	result.bottom = roundCoord (r.bottom, scaleFactor);
	return result;
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
		CGContextConcatCTM (cgContext, CGAffineTransformMakeScale (scaleFactor, scaleFactor));
	}

	init ();
	bitmap->forget ();
}

//-----------------------------------------------------------------------------
void CGDrawContext::init ()
{
	CGContextSaveGState (cgContext);
	CGContextSetShouldAntialias (cgContext, false);
	CGContextSetFillColorSpace (cgContext, GetCGColorSpace ());
	CGContextSetStrokeColorSpace (cgContext, GetCGColorSpace ()); 
	CGContextSaveGState (cgContext);
	CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
	CGContextSetTextMatrix (cgContext, cgCTM);

	CDrawContext::init ();
}

//-----------------------------------------------------------------------------
CGDrawContext::~CGDrawContext ()
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
		CGBitmap* cgBitmap = dynamic_cast<CGBitmap*> (bitmap->getPlatformBitmap ());
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
	const CoreTextFont* ctFont = dynamic_cast<const CoreTextFont*>(font->getPlatformFont ());
	return ctFont ? new QuartzGraphicsPath (ctFont, text) : 0;
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == 0)
		return;

	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context)
	{
		if (t)
		{
			CGContextSaveGState (context);
			CGAffineTransform transform = QuartzGraphicsPath::createCGAfflineTransform (*t);
			CGContextConcatCTM (context, transform);
			CGContextAddPath (context, path->getCGPathRef ());
			CGContextRestoreGState (context);
		}
		else
			CGContextAddPath (context, path->getCGPathRef ());

		CGPathDrawingMode cgMode;
		switch (mode)
		{
			case kPathFilledEvenOdd: cgMode = kCGPathEOFill; break;
			case kPathStroked: 
			{
				cgMode = kCGPathStroke; 
				applyLineStyle (context);
				if ((((int32_t)currentState.frameWidth) % 2))
					CGContextTranslateCTM (context, 0.5f, -0.5f);
				break;
			}
			default: cgMode = kCGPathFill; break;
		}

		CGContextDrawPath (context, cgMode);
		
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::fillLinearGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == 0)
		return;

	const QuartzGradient* cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (cgGradient == 0)
		return;

	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context)
	{
		if (t)
		{
			CGContextSaveGState (context);
			CGAffineTransform transform = QuartzGraphicsPath::createCGAfflineTransform (*t);
			CGContextConcatCTM (context, transform);
			CGContextAddPath (context, path->getCGPathRef ());
			CGContextRestoreGState (context);
		}
		else
			CGContextAddPath (context, path->getCGPathRef ());

		if (evenOdd)
			CGContextEOClip (context);
		else
			CGContextClip (context);

		CGContextDrawLinearGradient (context, *cgGradient, CGPointMake (startPoint.x, startPoint.y), CGPointMake (endPoint.x, endPoint.y), kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
		
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::fillRadialGradient (CGraphicsPath* _path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset, bool evenOdd, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == 0)
		return;

	const QuartzGradient* cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (cgGradient == 0)
		return;

	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context)
	{
		if (t)
		{
			CGContextSaveGState (context);
			CGAffineTransform transform = QuartzGraphicsPath::createCGAfflineTransform (*t);
			CGContextConcatCTM (context, transform);
			CGContextAddPath (context, path->getCGPathRef ());
			CGContextRestoreGState (context);
		}
		else
			CGContextAddPath (context, path->getCGPathRef ());

		if (evenOdd)
			CGContextEOClip (context);
		else
			CGContextClip (context);

		CGContextDrawRadialGradient (context, *cgGradient, CGPointMake (center.x + originOffset.x, center.y + originOffset.y), 0, CGPointMake (center.x, center.y), radius, kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
		
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

	CGContextSetLineWidth (cgContext, width);

	CDrawContext::setLineWidth (width);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setDrawMode (CDrawMode mode)
{
	if (cgContext)
		CGContextSetShouldAntialias (cgContext, mode == kAntiAliasing ? true : false);

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
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context)
	{
		applyLineStyle (context);
		
		if ((((int32_t)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);
		
		CGContextBeginPath (context);
		if (currentState.drawMode.integralMode ())
		{
			CPoint p (line.first);
			CGDrawContextInternal::roundPoint (p, scaleFactor);
			CGContextMoveToPoint (context, p.x, p.y);
			p = line.second;
			CGDrawContextInternal::roundPoint (p, scaleFactor);
			CGContextAddLineToPoint (context, p.x, p.y);
		}
		else
		{
			CGContextMoveToPoint (context, line.first.x, line.first.y);
			CGContextAddLineToPoint (context, line.second.x, line.second.y);
		}
		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawLines (const LineList& lines)
{
	if (lines.size () == 0)
		return;
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context)
	{
		applyLineStyle (context);
		
		if ((((int32_t)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);
		
		CGPoint* cgPoints = new CGPoint[lines.size () * 2];
		uint32_t index = 0;
		VSTGUI_RANGE_BASED_FOR_LOOP(LineList, lines, LinePair, line)
			if (currentState.drawMode.integralMode ())
			{
				cgPoints[index].x = CGDrawContextInternal::roundCoord (line.first.x, scaleFactor);
				cgPoints[index+1].x = CGDrawContextInternal::roundCoord (line.second.x, scaleFactor);
				cgPoints[index].y = CGDrawContextInternal::roundCoord (line.first.y, scaleFactor);
				cgPoints[index+1].y = CGDrawContextInternal::roundCoord (line.second.y, scaleFactor);
			}
			else
			{
				cgPoints[index].x = line.first.x;
				cgPoints[index+1].x = line.second.x;
				cgPoints[index].y = line.first.y;
				cgPoints[index+1].y = line.second.y;
			}
			index += 2;
		VSTGUI_RANGE_BASED_FOR_LOOP_END
		CGContextStrokeLineSegments (context, cgPoints, lines.size () * 2);
		delete [] cgPoints;
		
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
	if (polygonPointList.size () == 0)
		return;
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
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
		CGContextMoveToPoint (context, polygonPointList[0].x, polygonPointList[0].y);
		for (uint32_t i = 1; i < polygonPointList.size (); i++)
			CGContextAddLineToPoint (context, polygonPointList[i].x, polygonPointList[i].y);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawRect (const CRect &rect, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
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

		CGRect r;
		if (currentState.drawMode.integralMode ())
		{
			CRect ir = CGDrawContextInternal::makeRectIntegral (rect, scaleFactor);
			r = CGRectMake (ir.left, ir.top + 1, ir.getWidth () - 1, ir.getHeight () - 1);
		}
		else
		{
			r = CGRectMake (rect.left, rect.top + 1, rect.getWidth () - 1, rect.getHeight () - 1);
		}

		if ((((int32_t)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);

		CGContextBeginPath (context);
		CGContextAddRect (context, r);
		CGContextDrawPath (context, m);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawEllipse (const CRect &rect, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
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

		if (rect.getWidth () != rect.getHeight ())
		{
			CGContextSaveGState (context);

			CGContextBeginPath (context);

			CGRect cgRect = CGRectFromCRect (rect);
			CGPoint center = CGPointMake (CGRectGetMidX (cgRect), CGRectGetMidY (cgRect));
			CGFloat a = CGRectGetWidth (cgRect) / 2.;
			CGFloat b = CGRectGetHeight (cgRect) / 2.;

		    CGContextTranslateCTM (context, center.x, center.y);
		    CGContextScaleCTM (context, a, b);
		    CGContextMoveToPoint (context, 1, 0);
		    CGContextAddArc (context, 0, 0, 1, radians (0), radians (360), 0);

			CGContextClosePath (context);
			CGContextRestoreGState (context);
			CGContextDrawPath (context, m);
		}
		else
		{
			CGFloat radius = rect.getWidth () * 0.5;
			CGContextBeginPath (context);
			CGContextAddArc (context, rect.left + radius, rect.top + radius, radius, radians (0), radians (360), 0);
			CGContextClosePath (context);
			CGContextDrawPath (context, m);
		}
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
	drawLine (std::make_pair (point, point2));

	restoreGlobalState ();
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawArc (const CRect &rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle) // in degree
{
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
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
		CGDrawContextInternal::addOvalToPath (context, CPoint (rect.left + rect.getWidth () / 2., rect.top + rect.getHeight () / 2.), rect.getWidth () / 2., rect.getHeight () / 2., -_startAngle, -_endAngle);
		if (drawStyle == kDrawFilled || kDrawFilledAndStroked)
			CGContextAddLineToPoint (context, rect.left + rect.getWidth () / 2., rect.top + rect.getHeight () / 2.);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawBitmap (CBitmap* bitmap, const CRect& inRect, const CPoint& inOffset, float alpha)
{
	if (bitmap == 0 || alpha == 0.f)
		return;
	IPlatformBitmap* platformBitmap = bitmap->getBestPlatformBitmapForScaleFactor (scaleFactor);
	CGBitmap* cgBitmap = platformBitmap ? dynamic_cast<CGBitmap*> (platformBitmap) : 0;
	CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : 0;
	if (image)
	{
		CGContextRef context = beginCGContext (false, true);
		if (context)
		{
			CRect rect (inRect);
			rect.makeIntegral ();
			CPoint offset (inOffset);
			offset.makeIntegral ();

			CGContextSetAlpha (context, (CGFloat)alpha*currentState.globalAlpha);

			CGRect dest;
			dest.origin.x = rect.left - offset.x;
			dest.origin.y = -(rect.top) - (bitmap->getHeight () - offset.y);
			dest.size.width = bitmap->getWidth ();
			dest.size.height = bitmap->getHeight ();
			
			CGRect clipRect2;
			clipRect2.origin.x = rect.left;
			clipRect2.origin.y = -(rect.top) - rect.getHeight ();
			clipRect2.size.width = rect.getWidth ();
			clipRect2.size.height = rect.getHeight ();
		
			const double bitmapScaleFactor = cgBitmap->getScaleFactor ();
			if (bitmapScaleFactor != 1.)
			{
				CGContextConcatCTM (context, CGAffineTransformMakeScale (1./bitmapScaleFactor, 1./bitmapScaleFactor));
				CGAffineTransform transform = CGAffineTransformMakeScale (bitmapScaleFactor, bitmapScaleFactor);
				clipRect2.origin = CGPointApplyAffineTransform (clipRect2.origin, transform);
				clipRect2.size = CGSizeApplyAffineTransform (clipRect2.size, transform);
				dest.origin = CGPointApplyAffineTransform (dest.origin, transform);
				dest.size = CGSizeApplyAffineTransform (dest.size, transform);
			}
			
			CGContextClipToRect (context, clipRect2);

			CGLayerRef layer = cgBitmap->getCGLayer ();
			if (layer == 0)
			{
				BitmapDrawCountMap::iterator it = bitmapDrawCount.find (cgBitmap);
				if (it == bitmapDrawCount.end ())
				{
					bitmapDrawCount.insert (std::pair<CGBitmap*, int32_t> (cgBitmap, 1));
					CGContextDrawImage (context, dest, image);
				}
				else
				{
					it->second++;
					layer = cgBitmap->createCGLayer (context);
				}
			}
			if (layer)
			{
				CGContextDrawLayerInRect (context, dest, layer);
			}

			releaseCGContext (context);
		}
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::clearRect (const CRect& rect)
{
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context)
	{
		CGRect cgRect;
		if (currentState.drawMode.integralMode ())
		{
			CRect ir = CGDrawContextInternal::makeRectIntegral (rect, scaleFactor);
			cgRect = CGRectFromCRect (ir);
		}
		else
			cgRect = CGRectFromCRect (rect);
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

//-----------------------------------------------------------------------------
CGContextRef CGDrawContext::beginCGContext (bool swapYAxis, bool integralOffset)
{
	if (cgContext)
	{
		CGContextSaveGState (cgContext);

		CRect clipRect = CGDrawContextInternal::makeRectIntegral (currentState.clipRect, scaleFactor);

		CGRect cgClipRect = CGRectFromCRect (clipRect);
		CGContextClipToRect (cgContext, cgClipRect);

		if (getCurrentTransform ().isInvariant () == false)
			CGContextConcatCTM (cgContext, QuartzGraphicsPath::createCGAfflineTransform (getCurrentTransform ()));
		
		if (integralOffset)
		{
			CPoint offset (currentState.offset);
			CGDrawContextInternal::roundPoint (offset, scaleFactor);
			CGContextTranslateCTM (cgContext, offset.x, offset.y);
		}
		else
			CGContextTranslateCTM (cgContext, currentState.offset.x, currentState.offset.y);

		if (!swapYAxis)
			CGContextScaleCTM (cgContext, 1, -1);
		return cgContext;
	}
	return 0;
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
			dashLengths[i] = currentState.frameWidth * currentState.lineStyle.getDashLengths ()[i];
		}
		CGContextSetLineDash (context, currentState.lineStyle.getDashPhase (), dashLengths, currentState.lineStyle.getDashCount ());
		delete [] dashLengths;
	}
}

} // namespace

#endif // MAC

