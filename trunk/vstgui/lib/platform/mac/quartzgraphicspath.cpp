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

#include "quartzgraphicspath.h"

#if MAC

#include "cgdrawcontext.h"
#include "cfontmac.h"

namespace VSTGUI {

#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
//-----------------------------------------------------------------------------
class QuartzGradient : public CGradient
{
public:
	QuartzGradient (double _color1Start, double _color2Start, const CColor& _color1, const CColor& _color2)
	: CGradient (_color1Start, _color2Start, _color1, _color2)
	, gradient (0)
	{
		CGColorRef cgColor1 = CGColorCreateGenericRGB (color1.red/255.f, color1.green/255.f, color1.blue/255.f, color1.alpha/255.f);
		CGColorRef cgColor2 = CGColorCreateGenericRGB (color2.red/255.f, color2.green/255.f, color2.blue/255.f, color2.alpha/255.f);
		const void* colors[] = { cgColor1, cgColor2 };
		CFArrayRef colorArray = CFArrayCreate (0, colors, 2, &kCFTypeArrayCallBacks);

		if (color1Start < 0) color1Start = 0;
		else if (color1Start > 1) color1Start = 1;
		if (color2Start < 0) color2Start = 0;
		else if (color2Start > 1) color2Start = 1;
		CGFloat locations[] = { color1Start, color2Start };
		
		gradient = CGGradientCreateWithColors (0, colorArray, locations);

		CFRelease (cgColor1);
		CFRelease (cgColor2);
		CFRelease (colorArray);
	}
	
	~QuartzGradient ()
	{
		if (gradient)
			CFRelease (gradient);
	}

	operator CGGradientRef () const { return gradient; }

protected:
	CGGradientRef gradient;
};
#endif

//-----------------------------------------------------------------------------
static CGAffineTransform createCGAfflineTransform (const CGraphicsTransform& t)
{
	CGAffineTransform transform;
	transform.a = t.m11;
	transform.b = t.m12;
	transform.c = t.m21;
	transform.d = t.m22;
	transform.tx = t.dx;
	transform.ty = t.dy;
	return transform;
}

//-----------------------------------------------------------------------------
inline CGContextRef beginCGContext (CDrawContext* drawContext)
{
	CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (drawContext);
	if (cgDrawContext)
		return cgDrawContext->beginCGContext (true);
	return 0;
}

//-----------------------------------------------------------------------------
inline void releaseCGContext (CDrawContext* drawContext, CGContextRef cgContext)
{
	CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (drawContext);
	if (cgDrawContext)
		cgDrawContext->releaseCGContext (cgContext);
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::QuartzGraphicsPath ()
: path (CGPathCreateMutable ())
{
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::~QuartzGraphicsPath ()
{
	CFRelease (path);
}

//-----------------------------------------------------------------------------
CGradient* QuartzGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
	return new QuartzGradient (color1Start, color2Start, color1, color2);
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::draw (CDrawContext* context, PathDrawMode mode, CGraphicsTransform* t)
{
	CGContextRef cgContext = beginCGContext (context);
	if (cgContext)
	{
		if (t)
		{
			CGContextSaveGState (cgContext);
			CGAffineTransform transform = createCGAfflineTransform (*t);
			CGContextConcatCTM (cgContext, transform);
			CGContextAddPath (cgContext, path);
			CGContextRestoreGState (cgContext);
		}
		else
			CGContextAddPath (cgContext, path);

		CGPathDrawingMode cgMode = kCGPathFill;
		switch (mode)
		{
			case kFilledEvenOdd: cgMode = kCGPathEOFill; break;
			case kStroked: cgMode = kCGPathStroke; break;
		}

		CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
		if (cgDrawContext)
			cgDrawContext->applyLineDash ();

		CGContextDrawPath (cgContext, cgMode);
		
		releaseCGContext (context, cgContext);
	}
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
	const QuartzGradient* cgGradient = dynamic_cast<const QuartzGradient*> (&gradient);
	if (cgGradient == 0)
		return;

	CGContextRef cgContext = beginCGContext (context);
	if (cgContext)
	{
		if (t)
		{
			CGContextSaveGState (cgContext);
			CGAffineTransform transform = createCGAfflineTransform (*t);
			CGContextConcatCTM (cgContext, transform);
			CGContextAddPath (cgContext, path);
			CGContextRestoreGState (cgContext);
		}
		else
			CGContextAddPath (cgContext, path);

		if (evenOdd)
			CGContextEOClip (cgContext);
		else
			CGContextClip (cgContext);

		CGContextDrawLinearGradient (cgContext, *cgGradient, CGPointMake (startPoint.x, startPoint.y), CGPointMake (endPoint.x, endPoint.y), 0);
		
		releaseCGContext (context, cgContext);
	}
#else
#warning drawing gradient not support when minimum required Mac OS X version is 10.4
#endif
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::closeSubpath ()
{
	CGPathCloseSubpath (path);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle)
{
	CGFloat centerX = rect.left + rect.getWidth () / 2.;
	CGFloat centerY = rect.top + rect.getHeight () / 2.;

	CGAffineTransform transform = CGAffineTransformMakeTranslation (centerX, centerY);
	transform = CGAffineTransformScale (transform, rect.getWidth () / 2, rect.getHeight () / 2);
	
	CGPathMoveToPoint (path, &transform, cos (radians (startAngle)), sin (radians (startAngle)));

	CGPathAddArc (path, &transform, 0, 0, 1, radians (startAngle), radians (endAngle), false);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end)
{
	if (CGPathIsEmpty (path) || getCurrentPosition () != start)
		CGPathMoveToPoint (path, 0, start.x, start.y);
	CGPathAddCurveToPoint (path, 0, control1.x, control1.y, control2.x, control2.y, end.x, end.y);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addEllipse (const CRect& rect)
{
	CGPathAddEllipseInRect (path, 0, CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ()));
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addLine (const CPoint& start, const CPoint& end)
{
	if (CGPathIsEmpty (path) || getCurrentPosition () != start)
		CGPathMoveToPoint (path, 0, start.x, start.y);
	CGPathAddLineToPoint (path, 0, end.x, end.y);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addRect (const CRect& rect)
{
	CGRect r = CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	CGPathAddRect (path, 0, r);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addPath (const CGraphicsPath& inPath, CGraphicsTransform* t)
{
	if (t)
	{
		CGAffineTransform transform = createCGAfflineTransform (*t);
		if (&inPath == this)
		{
			CGPathRef pathCopy = CGPathCreateCopy (path);
			CGPathAddPath (path, &transform, pathCopy);
			CFRelease (pathCopy);
		}
		else
		{
			const QuartzGraphicsPath* qp = dynamic_cast<const QuartzGraphicsPath*> (&inPath);
			if (qp)
				CGPathAddPath (path, &transform, qp->getCGPathRef ());
		}
	}
	else
	{
		if (&inPath == this)
		{
			CGPathRef pathCopy = CGPathCreateCopy (path);
			CGPathAddPath (path, 0, pathCopy);
			CFRelease (pathCopy);
		}
		else
		{
			const QuartzGraphicsPath* qp = dynamic_cast<const QuartzGraphicsPath*> (&inPath);
			if (qp)
				CGPathAddPath (path, 0, qp->getCGPathRef ());
		}
	}
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addString (const char* utf8String, CFontRef font, const CPoint& position)
{
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
	CoreTextFont* ctf = dynamic_cast<CoreTextFont*> (font->getPlatformFont ());
	if (ctf == 0)
		return;
	CTFontRef fontRef = ctf->getFontRef ();
	if (fontRef == 0)
		return;
	CFStringRef utf8Str = CFStringCreateWithCString (NULL, utf8String, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CFStringRef keys[] = { kCTFontAttributeName };
		CFTypeRef values[] = { fontRef };
		CFDictionaryRef attributes = CFDictionaryCreate (kCFAllocatorDefault, (const void**)&keys,(const void**)&values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFAttributedStringRef attrStr = CFAttributedStringCreate (0, utf8Str, attributes);
		CFRelease (attributes);
		if (attrStr)
		{
			CTLineRef lineRef = CTLineCreateWithAttributedString (attrStr);
			if (lineRef)
			{
				CFArrayRef array = CTLineGetGlyphRuns (lineRef);
				if (array)
				{
					CTRunRef runRef = (CTRunRef)CFArrayGetValueAtIndex (array, 0);
					if (runRef)
					{
						const CGGlyph* glyphs = CTRunGetGlyphsPtr (runRef);
						const CGPoint* points = CTRunGetPositionsPtr (runRef);
						if (glyphs && points)
						{
							CGAffineTransform textTransform = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
							textTransform = CGAffineTransformTranslate (textTransform, position.x, -position.y);
							CFIndex numGlyphs = CTRunGetGlyphCount (runRef);
							for (CFIndex i = 0; i < numGlyphs; i++)
							{
								CGPathRef glyphPath = CTFontCreatePathForGlyph (fontRef, glyphs[i], 0);
								if (glyphPath)
								{
									CGAffineTransform transform = CGAffineTransformTranslate (textTransform, points[i].x, points[i].y);
									CGPathAddPath (path, &transform, glyphPath);
									CFRelease (glyphPath);
								}
							}
						}
					}
				}
				CFRelease (lineRef);
			}
			CFRelease (attrStr);
		}
		CFRelease (utf8Str);
	}
#else
#warning adding a string to QuartzGraphicsPath is not supported when minimum required Mac OS X version is 10.4
#endif
}

//-----------------------------------------------------------------------------
CPoint QuartzGraphicsPath::getCurrentPosition () const
{
	CPoint p (0, 0);

	if (!CGPathIsEmpty (path))
	{
		CGPoint cgPoint = CGPathGetCurrentPoint (path);
		p.x = cgPoint.x;
		p.y = cgPoint.y;
	}

	return p;
}

//-----------------------------------------------------------------------------
CRect QuartzGraphicsPath::getBoundingBox () const
{
	CRect r;

	CGRect cgRect = CGPathGetBoundingBox (path);
	r.left = cgRect.origin.x;
	r.top = cgRect.origin.y;
	r.setWidth (cgRect.size.width);
	r.setHeight (cgRect.size.height);
	
	return r;
}

} // namespace


#endif
