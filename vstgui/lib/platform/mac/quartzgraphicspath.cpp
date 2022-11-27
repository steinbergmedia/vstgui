// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "quartzgraphicspath.h"

#if MAC

#include "cfontmac.h"

#include "../../cgradient.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGAffineTransform createCGAffineTransform (const CGraphicsTransform& t)
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
static CGMutablePathRef createTextPath (const CoreTextFont* font, UTF8StringPtr text)
{
	auto textPath = CGPathCreateMutable ();

	CFStringRef str = CFStringCreateWithCString (kCFAllocatorDefault, text, kCFStringEncodingUTF8);
	const void* keys[] = {kCTFontAttributeName};
	const void* values[] = {font->getFontRef ()};
	CFDictionaryRef dict =
	    CFDictionaryCreate (kCFAllocatorDefault, keys, values, 1,
	                        &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFAttributedStringRef attrString = CFAttributedStringCreate (kCFAllocatorDefault, str, dict);
	CFRelease (dict);
	CFRelease (str);

	CTLineRef line = CTLineCreateWithAttributedString (attrString);
	if (line != nullptr)
	{
		CCoord capHeight = font->getCapHeight ();
		CFArrayRef runArray = CTLineGetGlyphRuns (line);
		for (CFIndex runIndex = 0; runIndex < CFArrayGetCount (runArray); runIndex++)
		{
			CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex (runArray, runIndex);
			CTFontRef runFont =
			    (CTFontRef)CFDictionaryGetValue (CTRunGetAttributes (run), kCTFontAttributeName);
			CFIndex glyphCount = CTRunGetGlyphCount (run);
			for (CFRange glyphRange = CFRangeMake (0, 1); glyphRange.location < glyphCount;
			     ++glyphRange.location)
			{
				CGGlyph glyph;
				CGPoint position;
				CTRunGetGlyphs (run, glyphRange, &glyph);
				CTRunGetPositions (run, glyphRange, &position);
				CGPathRef letter = CTFontCreatePathForGlyph (runFont, glyph, nullptr);
				CGAffineTransform t = CGAffineTransformMakeTranslation (position.x, position.y);
				t = CGAffineTransformScale (t, 1, -1);
				t = CGAffineTransformTranslate (t, 0, static_cast<CGFloat> (-capHeight));
				CGPathAddPath (textPath, &t, letter);
				CGPathRelease (letter);
			}
		}
		CFRelease (line);
	}
	CFRelease (attrString);
	return textPath;
}

//-----------------------------------------------------------------------------
PlatformGraphicsPathFactoryPtr CGGraphicsPathFactory::instance ()
{
	static PlatformGraphicsPathFactoryPtr factory = std::make_shared<CGGraphicsPathFactory> ();
	return factory;
}

//-----------------------------------------------------------------------------
PlatformGraphicsPathPtr
	CGGraphicsPathFactory::createPath ([[maybe_unused]] PlatformGraphicsPathFillMode fillMode)
{
	return std::make_unique<CGGraphicsPath> (CGPathCreateMutable ());
}

//-----------------------------------------------------------------------------
PlatformGraphicsPathPtr CGGraphicsPathFactory::createTextPath (const PlatformFontPtr& font,
															   UTF8StringPtr text)
{
	if (auto ctFont = font.cast<CoreTextFont> ())
	{
		return std::make_unique<CGGraphicsPath> (VSTGUI::createTextPath (ctFont, text));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
CGGraphicsPath::CGGraphicsPath (CGMutablePathRef inPath)
{
	if (inPath)
		path = inPath;
	else
		path = CGPathCreateMutable ();
}

//-----------------------------------------------------------------------------
CGGraphicsPath::~CGGraphicsPath () noexcept
{
	CFRelease (path);
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise)
{
	CCoord radiusX = (rect.right - rect.left) / 2.;
	CCoord radiusY = (rect.bottom - rect.top) / 2.;
	CGFloat centerX = static_cast<CGFloat> (rect.left + radiusX);
	CGFloat centerY = static_cast<CGFloat> (rect.top + radiusY);
	CGAffineTransform transform = CGAffineTransformMakeTranslation (centerX, centerY);
	transform = CGAffineTransformScale (transform, static_cast<CGFloat> (radiusX),
	                                    static_cast<CGFloat> (radiusY));
	startAngle = radians (startAngle);
	endAngle = radians (endAngle);
	if (radiusX != radiusY)
	{
		startAngle = std::atan2 (std::sin (startAngle) * radiusX, std::cos (startAngle) * radiusY);
		endAngle = std::atan2 (std::sin (endAngle) * radiusX, std::cos (endAngle) * radiusY);
	}
	if (CGPathIsEmpty (path))
	{
		CGPathMoveToPoint (path, &transform, static_cast<CGFloat> (std::cos (startAngle)),
		                   static_cast<CGFloat> (std::sin (startAngle)));
	}
	CGPathAddArc (path, &transform, 0, 0, 1, static_cast<CGFloat> (startAngle),
	              static_cast<CGFloat> (endAngle), !clockwise);
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::addEllipse (const CRect& rect)
{
	CGPathAddEllipseInRect (path, nullptr,
	                        CGRectMake (static_cast<CGFloat> (rect.left),
	                                    static_cast<CGFloat> (rect.top),
	                                    static_cast<CGFloat> (rect.getWidth ()),
	                                    static_cast<CGFloat> (rect.getHeight ())));
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::addRect (const CRect& rect)
{
	CGPathAddRect (path, nullptr,
	               CGRectMake (static_cast<CGFloat> (rect.left), static_cast<CGFloat> (rect.top),
	                           static_cast<CGFloat> (rect.getWidth ()),
	                           static_cast<CGFloat> (rect.getHeight ())));
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::addLine (const CPoint& to)
{
	CGPathAddLineToPoint (path, nullptr, static_cast<CGFloat> (to.x), static_cast<CGFloat> (to.y));
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::addBezierCurve (const CPoint& control1, const CPoint& control2,
                                     const CPoint& end)
{
	CGPathAddCurveToPoint (path, nullptr, static_cast<CGFloat> (control1.x),
	                       static_cast<CGFloat> (control1.y), static_cast<CGFloat> (control2.x),
	                       static_cast<CGFloat> (control2.y), static_cast<CGFloat> (end.x),
	                       static_cast<CGFloat> (end.y));
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::beginSubpath (const CPoint& start)
{
	CGPathMoveToPoint (path, nullptr, static_cast<CGFloat> (start.x),
	                   static_cast<CGFloat> (start.y));
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::closeSubpath ()
{
	CGPathCloseSubpath (path);
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::finishBuilding ()
{
}

//-----------------------------------------------------------------------------
void CGGraphicsPath::pixelAlign (const PixelAlignPointFunc& func, void* context)
{
	struct PathIterator
	{
		const PixelAlignPointFunc& pixelAlignFunc;
		void* context;
		CGMutablePathRef path;

		explicit PathIterator (const PixelAlignPointFunc& inPixelAlignFunc, void* inContext)
		: pixelAlignFunc (inPixelAlignFunc), context (inContext), path (CGPathCreateMutable ())
		{
		}
		void apply (const CGPathElement* element)
		{
			switch (element->type)
			{
				case kCGPathElementMoveToPoint:
				{
					element->points[0] = pixelAlignFunc (element->points[0], context);
					CGPathMoveToPoint (path, nullptr, element->points[0].x, element->points[0].y);
					break;
				}
				case kCGPathElementAddLineToPoint:
				{
					element->points[0] = pixelAlignFunc (element->points[0], context);
					CGPathAddLineToPoint (path, nullptr, element->points[0].x,
					                      element->points[0].y);
					break;
				}
				case kCGPathElementAddQuadCurveToPoint:
				{
					element->points[0] = pixelAlignFunc (element->points[0], context);
					element->points[1] = pixelAlignFunc (element->points[1], context);
					CGPathAddQuadCurveToPoint (path, nullptr, element->points[0].x,
					                           element->points[0].y, element->points[1].x,
					                           element->points[1].y);
					break;
				}
				case kCGPathElementAddCurveToPoint:
				{
					element->points[0] = pixelAlignFunc (element->points[0], context);
					element->points[1] = pixelAlignFunc (element->points[1], context);
					element->points[2] = pixelAlignFunc (element->points[2], context);
					CGPathAddCurveToPoint (path, nullptr, element->points[0].x,
					                       element->points[0].y, element->points[1].x,
					                       element->points[1].y, element->points[2].x,
					                       element->points[2].y);
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
			PathIterator* This = static_cast<PathIterator*> (info);
			This->apply (element);
		}
	};
	PathIterator iterator (func, context);
	CGPathApply (path, &iterator, PathIterator::apply);
	CFRelease (path);
	path = iterator.path;
}

//-----------------------------------------------------------------------------
bool CGGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled,
                              CGraphicsTransform* transform) const
{
	auto cgPoint = CGPointFromCPoint (p);
	CGAffineTransform cgTransform;
	if (transform)
		cgTransform = createCGAffineTransform (*transform);
	return CGPathContainsPoint (path, transform ? &cgTransform : nullptr, cgPoint, evenOddFilled);
}

//-----------------------------------------------------------------------------
CRect CGGraphicsPath::getBoundingBox () const
{
	auto cgRect = CGPathGetBoundingBox (path);
	return CRectFromCGRect (cgRect);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
QuartzGradient::~QuartzGradient () noexcept
{
	releaseCGGradient ();
}

//-----------------------------------------------------------------------------
void QuartzGradient::changed ()
{
	releaseCGGradient ();
}

//-----------------------------------------------------------------------------
void QuartzGradient::createCGGradient () const
{
	assert (gradient == nullptr);
	auto locations = new CGFloat[getColorStops ().size ()];
	auto colors =
	    CFArrayCreateMutable (kCFAllocatorDefault, static_cast<CFIndex> (getColorStops ().size ()),
	                          &kCFTypeArrayCallBacks);

	uint32_t index = 0;
	for (const auto& it : getColorStops ())
	{
		locations[index] = static_cast<CGFloat> (it.first);
		CColor color = it.second;
		CFArrayAppendValue (colors, getCGColor (color));
		++index;
	}

	gradient = CGGradientCreateWithColors (GetCGColorSpace (), colors, locations);

	CFRelease (colors);
	delete[] locations;
}

//-----------------------------------------------------------------------------
void QuartzGradient::releaseCGGradient ()
{
	if (gradient)
	{
		CFRelease (gradient);
		gradient = nullptr;
	}
}

//-----------------------------------------------------------------------------
QuartzGradient::operator CGGradientRef () const
{
	if (gradient == nullptr)
	{
		createCGGradient ();
	}
	return gradient;
}

} // VSTGUI

#endif
