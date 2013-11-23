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

#ifndef CGFLOAT_DEFINED
	#define CGFLOAT_DEFINED
	typedef float CGFloat;
#endif // CGFLOAT_DEFINED

namespace VSTGUI {

static void addOvalToPath (CGContextRef c, CPoint center, CGFloat a, CGFloat b, CGFloat start_angle, CGFloat end_angle);

//-----------------------------------------------------------------------------
CGDrawContext::CGDrawContext (CGContextRef cgContext, const CRect& rect)
: COffscreenContext (rect)
, cgContext (cgContext)
{
	CFRetain (cgContext);

	init ();
}

//-----------------------------------------------------------------------------
CGDrawContext::CGDrawContext (CGBitmap* _bitmap)
: COffscreenContext (new CBitmap (_bitmap))
, cgContext (_bitmap->createCGContext ())
{
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
void CGDrawContext::drawGraphicsPath (CGraphicsPath* _path, PathDrawMode mode, CGraphicsTransform* t)
{
	QuartzGraphicsPath* path = dynamic_cast<QuartzGraphicsPath*> (_path);
	if (path == 0)
		return;

	CGContextRef cgContext = beginCGContext (true, currentState.drawMode.integralMode ());
	if (cgContext)
	{
		if (t)
		{
			CGContextSaveGState (cgContext);
			CGAffineTransform transform = QuartzGraphicsPath::createCGAfflineTransform (*t);
			CGContextConcatCTM (cgContext, transform);
			CGContextAddPath (cgContext, path->getCGPathRef ());
			CGContextRestoreGState (cgContext);
		}
		else
			CGContextAddPath (cgContext, path->getCGPathRef ());

		CGPathDrawingMode cgMode;
		switch (mode)
		{
			case kPathFilledEvenOdd: cgMode = kCGPathEOFill; break;
			case kPathStroked: 
			{
				cgMode = kCGPathStroke; 
				applyLineStyle (cgContext);
				break;
			}
			default: cgMode = kCGPathFill; break;
		}

		CGContextDrawPath (cgContext, cgMode);
		
		releaseCGContext (cgContext);
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

	CGContextRef cgContext = beginCGContext (true, currentState.drawMode.integralMode ());
	if (cgContext)
	{
		if (t)
		{
			CGContextSaveGState (cgContext);
			CGAffineTransform transform = QuartzGraphicsPath::createCGAfflineTransform (*t);
			CGContextConcatCTM (cgContext, transform);
			CGContextAddPath (cgContext, path->getCGPathRef ());
			CGContextRestoreGState (cgContext);
		}
		else
			CGContextAddPath (cgContext, path->getCGPathRef ());

		if (evenOdd)
			CGContextEOClip (cgContext);
		else
			CGContextClip (cgContext);

		CGContextDrawLinearGradient (cgContext, *cgGradient, CGPointMake (startPoint.x, startPoint.y), CGPointMake (endPoint.x, endPoint.y), kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
		
		releaseCGContext (cgContext);
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
void CGDrawContext::lineTo (const CPoint& point)
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
			CGContextMoveToPoint (context, round (currentState.penLoc.h), round (currentState.penLoc.v));
			CGContextAddLineToPoint (context, round (point.h), round (point.v));
		}
		else
		{
			CGContextMoveToPoint (context, currentState.penLoc.h, currentState.penLoc.v);
			CGContextAddLineToPoint (context, point.h, point.v);
		}
		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
	currentState.penLoc = point;
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawLines (const CPoint* points, const int32_t& numLines)
{
	CGContextRef context = beginCGContext (true, currentState.drawMode.integralMode ());
	if (context) 
	{
		applyLineStyle (context);

		if ((((int32_t)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);

		CGPoint* cgPoints = new CGPoint[numLines*2];
		for (int32_t i = 0; i < numLines * 2; i += 2)
		{
			if (currentState.drawMode.integralMode ())
			{
				cgPoints[i].x = round (points[i].x);
				cgPoints[i+1].x = round (points[i+1].x);
				cgPoints[i].y = round (points[i].y);
				cgPoints[i+1].y = round (points[i+1].y);
			}
			else
			{
				cgPoints[i].x = points[i].x;
				cgPoints[i+1].x = points[i+1].x;
				cgPoints[i].y = points[i].y;
				cgPoints[i+1].y = points[i+1].y;
			}
		}
		CGContextStrokeLineSegments (context, cgPoints, numLines*2);
		delete [] cgPoints;

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawPolygon (const CPoint* pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle)
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
		CGContextMoveToPoint (context, pPoints[0].h, pPoints[0].v);
		for (int32_t i = 1; i < numberOfPoints; i++)
			CGContextAddLineToPoint (context, pPoints[i].h, pPoints[i].v);
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
			r = CGRectMake (round (rect.left), round (rect.top + 1), round (rect.width () - 1), round (rect.height () - 1));
		}
		else
		{
			r = CGRectMake (rect.left, rect.top + 1, rect.width () - 1, rect.height () - 1);
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

		if (rect.width () != rect.height ())
		{
			CGContextSaveGState (context);

			CGContextBeginPath (context);

			CGRect cgRect = CGRectMake (rect.left, rect.top, rect.width (), rect.height ());
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
			CGFloat radius = rect.width () * 0.5;
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
	point2.h++;
	moveTo (point);
	lineTo (point2);

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
		addOvalToPath (context, CPoint (rect.left + rect.width () / 2, rect.top + rect.height () / 2), rect.width () / 2, rect.height () / 2, -_startAngle, -_endAngle);
		if (drawStyle == kDrawFilled || kDrawFilledAndStroked)
			CGContextAddLineToPoint (context, rect.left + rect.width () / 2, rect.top + rect.height () / 2);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawBitmap (CBitmap* bitmap, const CRect& inRect, const CPoint& inOffset, float alpha)
{
	if (bitmap == 0 || alpha == 0.f)
		return;
	CGBitmap* cgBitmap = bitmap->getPlatformBitmap () ? dynamic_cast<CGBitmap*> (bitmap->getPlatformBitmap ()) : 0;
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
			dest.origin.x = rect.left - offset.h;
			dest.origin.y = -(rect.top) - (bitmap->getHeight () - offset.v);
			dest.size.width = cgBitmap->getSize ().x;
			dest.size.height = cgBitmap->getSize ().y;
			
			CGRect clipRect2;
			clipRect2.origin.x = rect.left;
			clipRect2.origin.y = -(rect.top) - rect.height ();
			clipRect2.size.width = rect.width (); 
			clipRect2.size.height = rect.height ();
		
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
			cgRect = CGRectMake (round (rect.left), round (rect.top), round (rect.width ()), round (rect.height ()));
		else
			cgRect = CGRectMake (rect.left, rect.top, rect.width (), rect.height ());
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
		CGContextSetRGBStrokeColor (cgContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);

	CDrawContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFillColor (const CColor& color)
{
	if (currentState.fillColor == color)
		return;

	if (cgContext)
		CGContextSetRGBFillColor (cgContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);

	CDrawContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
CGContextRef CGDrawContext::beginCGContext (bool swapYAxis, bool integralOffset)
{
	if (cgContext)
	{
		CGContextSaveGState (cgContext);

		CGRect cgClipRect = CGRectMake (currentState.clipRect.left, currentState.clipRect.top, currentState.clipRect.width (), currentState.clipRect.height ());
		CGContextClipToRect (cgContext, cgClipRect);

		if (integralOffset)
			CGContextTranslateCTM (cgContext, round (currentState.offset.x), round (currentState.offset.y));
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
		for (int32_t i = 0; i < currentState.lineStyle.getDashCount (); i++)
		{
			dashLengths[i] = currentState.frameWidth * currentState.lineStyle.getDashLengths ()[i];
		}
		CGContextSetLineDash (context, currentState.lineStyle.getDashPhase (), dashLengths, currentState.lineStyle.getDashCount ());
		delete [] dashLengths;
	}
}

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

} // namespace

#endif // MAC

