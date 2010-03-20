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

#include "cgdrawcontext.h"

#if MAC

#include "macglobals.h"
#include "cgbitmap.h"

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
CGDrawContext::CGDrawContext (CGOffscreenBitmap* _bitmap)
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
	CGContextSetFillColorSpace (cgContext, GetGenericRGBColorSpace ());
	CGContextSetStrokeColorSpace (cgContext, GetGenericRGBColorSpace ()); 
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
	CBitmap* bitmap = getBitmap ();
	if (bitmap && bitmap->getPlatformBitmap ())
	{
		CGOffscreenBitmap* cgOffscreenBitmap = dynamic_cast<CGOffscreenBitmap*> (bitmap->getPlatformBitmap ());
		if (cgOffscreenBitmap)
			cgOffscreenBitmap->setDirty ();
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
void CGDrawContext::setLineStyle (CLineStyle style)
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
	if (currentState.drawMode == mode)
		return;

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
	CGContextRef context = beginCGContext (true);
	if (context)
	{
		applyLineDash ();

		if ((((int)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, currentState.penLoc.h, currentState.penLoc.v);
		CGContextAddLineToPoint (context, point.h, point.v);
		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
	currentState.penLoc = point;
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawLines (const CPoint* points, const long& numLines)
{
	CGContextRef context = beginCGContext (true);
	if (context) 
	{
		applyLineDash ();

		if ((((int)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);

		CGPoint* cgPoints = new CGPoint[numLines*2];
		for (long i = 0; i < numLines * 2; i += 2)
		{
			cgPoints[i].x = points[i].x;
			cgPoints[i+1].x = points[i+1].x;
			cgPoints[i].y = points[i].y;
			cgPoints[i+1].y = points[i+1].y;
		}
		CGContextStrokeLineSegments (context, cgPoints, numLines*2);
		delete [] cgPoints;

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawPolygon (const CPoint* pPoints, long numberOfPoints, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineDash ();

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, pPoints[0].h, pPoints[0].v);
		for (long i = 1; i < numberOfPoints; i++)
			CGContextAddLineToPoint (context, pPoints[i].h, pPoints[i].v);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawRect (const CRect &rect, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineDash ();

		CGRect r = CGRectMake (rect.left, rect.top+1, rect.width () - 1, rect.height () - 1);

		if ((((int)currentState.frameWidth) % 2))
			CGContextTranslateCTM (context, 0.5f, -0.5f);

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, r.origin.x, r.origin.y);
		CGContextAddLineToPoint (context, r.origin.x + r.size.width, r.origin.y);
		CGContextAddLineToPoint (context, r.origin.x + r.size.width, r.origin.y + r.size.height);
		CGContextAddLineToPoint (context, r.origin.x, r.origin.y + r.size.height);
		CGContextClosePath (context);

		CGContextDrawPath (context, m);

		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawEllipse (const CRect &rect, const CDrawStyle drawStyle)
{
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineDash ();

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
void CGDrawContext::drawPoint (const CPoint &point, CColor color)
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
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		applyLineDash ();

		CGContextBeginPath (context);
		addOvalToPath (context, CPoint (rect.left + rect.width () / 2, rect.top + rect.height () / 2), rect.width () / 2, rect.height () / 2, -_startAngle, -_endAngle);
		if (drawStyle == kDrawFilled || kDrawFilledAndStroked)
			CGContextAddLineToPoint (context, rect.left + rect.width () / 2, rect.top + rect.height () / 2);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::drawBitmap (CBitmap* bitmap, const CRect& rect, const CPoint& offset, float alpha)
{
	if (bitmap == 0 || alpha == 0.f)
		return;
	CGBitmap* cgBitmap = bitmap->getPlatformBitmap () ? dynamic_cast<CGBitmap*> (bitmap->getPlatformBitmap ()) : 0;
	CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : 0;
	if (image)
	{
		CGContextRef context = beginCGContext ();
		if (context)
		{
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

			CGContextDrawImage (context, dest, image);

			releaseCGContext (context);
		}
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::clearRect (const CRect& rect)
{
	CGContextRef context = beginCGContext (true);
	if (context)
	{
		CGRect cgRect = CGRectMake (rect.left, rect.top, rect.width (), rect.height ());
		CGContextClearRect (context, cgRect);
		releaseCGContext (context);
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFontColor (const CColor color)
{
	if (currentState.fontColor == color)
		return;

	CDrawContext::setFontColor (color);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFrameColor (const CColor color)
{
	if (currentState.frameColor == color)
		return;
		
	CGContextSetRGBStrokeColor (cgContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);

	CDrawContext::setFrameColor (color);
}

//-----------------------------------------------------------------------------
void CGDrawContext::setFillColor (const CColor color)
{
	if (currentState.fillColor == color)
		return;

	CGContextSetRGBFillColor (cgContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);

	CDrawContext::setFillColor (color);
}

//-----------------------------------------------------------------------------
CGContextRef CGDrawContext::beginCGContext (bool swapYAxis)
{
	if (cgContext)
	{
		CGContextSaveGState (cgContext);

		CGRect cgClipRect = CGRectMake (currentState.clipRect.left, currentState.clipRect.top, currentState.clipRect.width (), currentState.clipRect.height ());
		CGContextClipToRect (cgContext, cgClipRect);

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
		if (bitmap)
		{
			CGOffscreenBitmap* ob = dynamic_cast<CGOffscreenBitmap*> (bitmap->getPlatformBitmap ());
			if (ob)
				ob->setDirty ();
		}
	}
}

//-----------------------------------------------------------------------------
void CGDrawContext::applyLineDash ()
{
	if (currentState.lineStyle == kLineOnOffDash)
	{
		CGFloat offset = 0;
		CGFloat dotf[2] = { currentState.frameWidth, currentState.frameWidth };
		CGContextSetLineDash (cgContext, offset, dotf, 2);
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

