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

#include "quartzgraphicspath.h"

#if MAC

#include "cgdrawcontext.h"
#include "cfontmac.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGAffineTransform QuartzGraphicsPath::createCGAffineTransform (const CGraphicsTransform& t)
{
	CGAffineTransform transform;
	transform.a = static_cast<CGFloat> (t.m11);
	transform.b = static_cast<CGFloat> (t.m21);
	transform.c = static_cast<CGFloat> (t.m12);
	transform.d = static_cast<CGFloat> (t.m22);
	transform.tx = static_cast<CGFloat> (t.dx);
	transform.ty = static_cast<CGFloat> (t.dy);
	return transform;
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::QuartzGraphicsPath ()
: path (0)
, originalTextPath (0)
, isPixelAlligned (false)
{
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::QuartzGraphicsPath (const CoreTextFont* font, UTF8StringPtr text)
: isPixelAlligned (false)
{
	path = CGPathCreateMutable ();
	
    CFStringRef str = CFStringCreateWithCString (kCFAllocatorDefault, text, kCFStringEncodingUTF8);
	const void* keys [] = {kCTFontAttributeName};
	const void* values [] = {font->getFontRef ()};
	CFDictionaryRef dict = CFDictionaryCreate (kCFAllocatorDefault, keys, values, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef attrString = CFAttributedStringCreate (kCFAllocatorDefault, str, dict);
	CFRelease (dict);
	CFRelease (str);

    CTLineRef line = CTLineCreateWithAttributedString (attrString);
	if (line != 0)
	{
		CCoord capHeight = font->getCapHeight ();
		CFArrayRef runArray = CTLineGetGlyphRuns (line);
		for (CFIndex runIndex = 0; runIndex < CFArrayGetCount (runArray); runIndex++)
		{
			CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex (runArray, runIndex);
			CTFontRef runFont = (CTFontRef)CFDictionaryGetValue (CTRunGetAttributes (run), kCTFontAttributeName);
			CFIndex glyphCount = CTRunGetGlyphCount (run);
			for (CFRange glyphRange = CFRangeMake (0, 1); glyphRange.location < glyphCount; ++glyphRange.location)
			{
				CGGlyph glyph;
				CGPoint position;
				CTRunGetGlyphs (run, glyphRange, &glyph);
				CTRunGetPositions (run, glyphRange, &position);
				
				CGPathRef letter = CTFontCreatePathForGlyph (runFont, glyph, NULL);
				CGAffineTransform t = CGAffineTransformMakeTranslation (position.x, position.y);
				t = CGAffineTransformScale (t, 1, -1);
				t = CGAffineTransformTranslate (t, 0, static_cast<CGFloat> (-capHeight));
				CGPathAddPath (path, &t, letter);
				CGPathRelease (letter);
			}
		}
		CFRelease (line);
	}
	CFRelease (attrString);
	originalTextPath = path;
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::~QuartzGraphicsPath ()
{
	dirty ();
	if (originalTextPath)
		CFRelease (originalTextPath);
}

//-----------------------------------------------------------------------------
CGradient* QuartzGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	return new QuartzGradient (color1Start, color2Start, color1, color2);
}

//-----------------------------------------------------------------------------
CGPathRef QuartzGraphicsPath::getCGPathRef ()
{
	if (path == 0)
	{
		if (originalTextPath)
		{
			path = originalTextPath;
			return path;
		}
		path = CGPathCreateMutable ();
		for (ElementList::const_iterator it = elements.begin (); it != elements.end (); it++)
		{
			const Element& e = (*it);
			switch (e.type)
			{
				case Element::kArc:
				{
					CCoord radiusX = (e.instruction.arc.rect.right - e.instruction.arc.rect.left) / 2.;
					CCoord radiusY = (e.instruction.arc.rect.bottom - e.instruction.arc.rect.top) / 2.;
					
					CGFloat centerX = static_cast<CGFloat> (e.instruction.arc.rect.left + radiusX);
					CGFloat centerY = static_cast<CGFloat> (e.instruction.arc.rect.top + radiusY);

					CGAffineTransform transform = CGAffineTransformMakeTranslation (centerX, centerY);
					transform = CGAffineTransformScale (transform, static_cast<CGFloat> (radiusX), static_cast<CGFloat> (radiusY));
					
					double startAngle = radians (e.instruction.arc.startAngle);
					double endAngle = radians (e.instruction.arc.endAngle);
					if (radiusX != radiusY)
					{
						startAngle = atan2 (sin (startAngle) * radiusX, cos (startAngle) * radiusY);
						endAngle = atan2 (sin (endAngle) * radiusX, cos (endAngle) * radiusY);
					}
					if (CGPathIsEmpty (path))
						CGPathMoveToPoint (path, &transform, static_cast<CGFloat> (std::cos (startAngle)), static_cast<CGFloat> (std::sin (startAngle)));

					CGPathAddArc (path, &transform, 0, 0, 1, static_cast<CGFloat> (startAngle), static_cast<CGFloat> (endAngle), !e.instruction.arc.clockwise);
					break;
				}
				case Element::kEllipse:
				{
					CCoord width = e.instruction.rect.right - e.instruction.rect.left;
					CCoord height = e.instruction.rect.bottom - e.instruction.rect.top;
					CGPathAddEllipseInRect (path, 0, CGRectMake (static_cast<CGFloat> (e.instruction.rect.left), static_cast<CGFloat> (e.instruction.rect.top), static_cast<CGFloat> (width), static_cast<CGFloat> (height)));
					break;
				}
				case Element::kRect:
				{
					CCoord width = e.instruction.rect.right - e.instruction.rect.left;
					CCoord height = e.instruction.rect.bottom - e.instruction.rect.top;
					CGPathAddRect (path, 0, CGRectMake (static_cast<CGFloat> (e.instruction.rect.left), static_cast<CGFloat> (e.instruction.rect.top), static_cast<CGFloat> (width), static_cast<CGFloat> (height)));
					break;
				}
				case Element::kLine:
				{
					CGPathAddLineToPoint (path, 0, static_cast<CGFloat> (e.instruction.point.x), static_cast<CGFloat> (e.instruction.point.y));
					break;
				}
				case Element::kBezierCurve:
				{
					CGPathAddCurveToPoint (path, 0, static_cast<CGFloat> (e.instruction.curve.control1.x), static_cast<CGFloat> (e.instruction.curve.control1.y), static_cast<CGFloat> (e.instruction.curve.control2.x), static_cast<CGFloat> (e.instruction.curve.control2.y), static_cast<CGFloat> (e.instruction.curve.end.x), static_cast<CGFloat> (e.instruction.curve.end.y));
					break;
				}
				case Element::kBeginSubpath:
				{
					CGPathMoveToPoint (path, 0, static_cast<CGFloat> (e.instruction.point.x), static_cast<CGFloat> (e.instruction.point.y));
					break;
				}
				case Element::kCloseSubpath:
				{
					CGPathCloseSubpath (path);
					break;
				}
			}
		}
	}
	return path;
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::dirty ()
{
	if (path)
	{
		if (originalTextPath != path)
			CFRelease (path);
		path = 0;
	}
}

//-----------------------------------------------------------------------------
bool QuartzGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled, CGraphicsTransform* transform)
{
	CGPathRef cgPath = getCGPathRef ();
	if (cgPath)
	{
		CGPoint cgPoint = CGPointFromCPoint (p);
		CGAffineTransform cgTransform;
		if (transform)
			cgTransform = createCGAffineTransform (*transform);
		return CGPathContainsPoint (cgPath, transform ? &cgTransform : 0, cgPoint, evenOddFilled);
	}
	return false;
}

//-----------------------------------------------------------------------------
CPoint QuartzGraphicsPath::getCurrentPosition ()
{
	CPoint p (0, 0);

	CGPathRef cgPath = getCGPathRef ();
	if (cgPath && !CGPathIsEmpty (cgPath))
	{
		CGPoint cgPoint = CGPathGetCurrentPoint (cgPath);
		p.x = cgPoint.x;
		p.y = cgPoint.y;
	}

	return p;
}

//-----------------------------------------------------------------------------
CRect QuartzGraphicsPath::getBoundingBox ()
{
	CRect r;

	CGPathRef cgPath = getCGPathRef ();
	if (cgPath)
		r = CRectFromCGRect (CGPathGetBoundingBox (cgPath));
	return r;
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::pixelAlign (CDrawContext* context)
{
	CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*>(context);
	if (cgDrawContext == 0)
		return;

	if (isPixelAlligned)
		dirty ();

	struct PathIterator
	{
		CGMutablePathRef path;
		const CGDrawContext& context;
		
		PathIterator (const CGDrawContext& context)
		: context (context)
		{
			path = CGPathCreateMutable ();
		}
		void apply (const CGPathElement* element)
		{
			switch (element->type)
			{
				case kCGPathElementMoveToPoint:
				{
					element->points[0] = context.pixelAlligned (element->points[0]);
					CGPathMoveToPoint (path, 0, element->points[0].x, element->points[0].y);
					break;
				}
				case kCGPathElementAddLineToPoint:
				{
					element->points[0] = context.pixelAlligned (element->points[0]);
					CGPathAddLineToPoint (path, 0, element->points[0].x, element->points[0].y);
					break;
				}
				case kCGPathElementAddQuadCurveToPoint:
				{
					element->points[0] = context.pixelAlligned (element->points[0]);
					element->points[1] = context.pixelAlligned (element->points[1]);
					CGPathAddQuadCurveToPoint (path, 0, element->points[0].x, element->points[0].y, element->points[1].x, element->points[1].y);
					break;
				}
				case kCGPathElementAddCurveToPoint:
				{
					element->points[0] = context.pixelAlligned (element->points[0]);
					element->points[1] = context.pixelAlligned (element->points[1]);
					element->points[2] = context.pixelAlligned (element->points[2]);
					CGPathAddCurveToPoint (path, 0, element->points[0].x, element->points[0].y, element->points[1].x, element->points[1].y, element->points[2].x, element->points[2].y);
					break;
				}
				case kCGPathElementCloseSubpath:
				{
					CGPathCloseSubpath (path);
					break;
				}
			}
		}
		
		static void apply (void* info, const CGPathElement* element)
		{
			PathIterator* This = static_cast<PathIterator*>(info);
			This->apply (element);
		}
	};
	PathIterator iterator (*cgDrawContext);
	CGPathApply (getCGPathRef (), &iterator, PathIterator::apply);
	dirty ();
	path = iterator.path;
	isPixelAlligned = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
QuartzGradient::QuartzGradient (const ColorStopMap& map)
: CGradient (map)
, gradient (0)
{
}

//-----------------------------------------------------------------------------
QuartzGradient::QuartzGradient (double _color1Start, double _color2Start, const CColor& _color1, const CColor& _color2)
: CGradient (_color1Start, _color2Start, _color1, _color2)
, gradient (0)
{
}

//-----------------------------------------------------------------------------
QuartzGradient::~QuartzGradient ()
{
	releaseCGGradient ();
}

//-----------------------------------------------------------------------------
void QuartzGradient::addColorStop (const std::pair<double, CColor>& colorStop)
{
	CGradient::addColorStop (colorStop);
	releaseCGGradient ();
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
void QuartzGradient::addColorStop (std::pair<double, CColor>&& colorStop)
{
	CGradient::addColorStop (colorStop);
	releaseCGGradient ();
}
#endif

//-----------------------------------------------------------------------------
void QuartzGradient::createCGGradient () const
{
	CGFloat* locations = new CGFloat [colorStops.size ()];
	CFMutableArrayRef colors = CFArrayCreateMutable (kCFAllocatorDefault, static_cast<CFIndex> (colorStops.size ()), &kCFTypeArrayCallBacks);

	uint32_t index = 0;
	for (ColorStopMap::const_iterator it = colorStops.begin (); it != colorStops.end (); ++it, ++index)
	{
		locations[index] = static_cast<CGFloat> (it->first);
		CColor color = it->second;
		CFArrayAppendValue (colors, getCGColor (color));
	}

	gradient = CGGradientCreateWithColors (GetCGColorSpace (), colors, locations);
	
	CFRelease (colors);
	delete [] locations;
}

//-----------------------------------------------------------------------------
void QuartzGradient::releaseCGGradient ()
{
	if (gradient)
	{
		CFRelease (gradient);
		gradient = 0;
	}
}

//-----------------------------------------------------------------------------
QuartzGradient::operator CGGradientRef () const
{
	if (gradient == 0)
	{
		createCGGradient ();
	}
	return gradient;
}

//-----------------------------------------------------------------------------
CGradient* CGradient::create (const ColorStopMap& colorStopMap)
{
	return new QuartzGradient (colorStopMap);
}

} // namespace


#endif
