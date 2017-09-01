// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "cfontmac.h"
#import "../../cdrawcontext.h"

#if MAC
#import "macstring.h"
#import "cgdrawcontext.h"
#import "macglobals.h"
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct CTVersionCheck
{
	CTVersionCheck ()
	{
		version = CTGetCoreTextVersion ();
	}
	uint32_t version;
};

//-----------------------------------------------------------------------------
static uint32_t getCTVersion ()
{
	static CTVersionCheck gInstance;
	return gInstance.version;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformFont> IPlatformFont::create (const UTF8String& name, const CCoord& size, const int32_t& style)
{
	auto font = makeOwned<CoreTextFont> (name, size, style);
	if (font->getFontRef ())
		return std::move (font);
	return nullptr;
}

//-----------------------------------------------------------------------------
bool IPlatformFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
#if TARGET_OS_IPHONE
	NSArray* fonts = [UIFont familyNames];
#else
	NSArray* fonts = [(NSArray*)CTFontManagerCopyAvailableFontFamilyNames () autorelease];
#endif
	for (uint32_t i = 0; i < [fonts count]; i++)
	{
		NSString* font = [fonts objectAtIndex:i];
		fontFamilyNames.emplace_back ([font UTF8String]);
	}
	return true;
}

//-----------------------------------------------------------------------------
static CTFontRef CoreTextCreateTraitsVariant (CTFontRef fontRef, CTFontSymbolicTraits trait)
{
	CTFontRef traitsFontRef = CTFontCreateCopyWithSymbolicTraits (fontRef, CTFontGetSize (fontRef), nullptr, trait, trait);
	if (traitsFontRef)
	{
		CFRelease (fontRef);
		return traitsFontRef;
	}
	else if (trait == kCTFontItalicTrait)
	{
		CGAffineTransform transform = { 1, 0, -0.5, 1, 0, 0 };
		traitsFontRef = CTFontCreateCopyWithAttributes (fontRef, CTFontGetSize (fontRef), &transform, nullptr);
		if (traitsFontRef)
		{
			CFRelease (fontRef);
			return traitsFontRef;
		}
	}
	return fontRef;
}

//-----------------------------------------------------------------------------
CoreTextFont::CoreTextFont (const UTF8String& name, const CCoord& size, const int32_t& style)
: fontRef (nullptr)
, style (style)
, underlineStyle (false)
, lastColor (MakeCColor (0,0,0,0))
, stringAttributes (nullptr)
, ascent (0.)
, descent (0.)
, leading (0.)
, capHeight (0.)
{
	CFStringRef fontNameRef = fromUTF8String<CFStringRef> (name);
	if (fontNameRef)
	{
		if (getCTVersion () >= 0x00060000)
		{
			CFMutableDictionaryRef attributes = CFDictionaryCreateMutable (kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionaryAddValue (attributes, kCTFontFamilyNameAttribute, fontNameRef);
			CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes (attributes);
			fontRef = CTFontCreateWithFontDescriptor (descriptor, static_cast<CGFloat> (size), nullptr);
			CFRelease (attributes);
			CFRelease (descriptor);
		}
		else
		{
			fontRef = CTFontCreateWithName (fontNameRef, static_cast<CGFloat> (size), nullptr);
		}

		if (style & kBoldFace)
			fontRef = CoreTextCreateTraitsVariant (fontRef, kCTFontBoldTrait);
		if (style & kItalicFace)
			fontRef = CoreTextCreateTraitsVariant (fontRef, kCTFontItalicTrait);
		if (fontRef)
		{
			ascent = CTFontGetAscent (fontRef);
			descent = CTFontGetDescent (fontRef);
			leading = CTFontGetLeading (fontRef);
			capHeight = CTFontGetCapHeight (fontRef);
		}
	}
}

//-----------------------------------------------------------------------------
CoreTextFont::~CoreTextFont () noexcept
{
	if (stringAttributes)
		CFRelease (stringAttributes);
	if (fontRef)
		CFRelease (fontRef);
}

//-----------------------------------------------------------------------------
double CoreTextFont::getAscent () const
{
	return ascent;
}

//-----------------------------------------------------------------------------
double CoreTextFont::getDescent () const
{
	return descent;
}

//-----------------------------------------------------------------------------
double CoreTextFont::getLeading () const
{
	return leading;
}

//-----------------------------------------------------------------------------
double CoreTextFont::getCapHeight () const
{
	return capHeight;
}

//-----------------------------------------------------------------------------
CFDictionaryRef CoreTextFont::getStringAttributes (const CGColorRef color) const
{
	if (stringAttributes == nullptr)
	{
		stringAttributes = CFDictionaryCreateMutable (kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue (stringAttributes, kCTFontAttributeName, fontRef);
	}
	if (color)
	{
		CFDictionarySetValue (stringAttributes, kCTForegroundColorAttributeName, color);
	}
	return stringAttributes;
}

//-----------------------------------------------------------------------------
CTLineRef CoreTextFont::createCTLine (CDrawContext* context, MacString* macString) const
{
	CColor fontColor = context ? context->getFontColor () : kBlackCColor;
	if (context)
	{
		if (macString->getCTLineFontRef () == this && macString->getCTLineColor () == fontColor)
		{
			CTLineRef line = macString->getCTLine ();
			CFRetain (line);
			return line;
		}
	}
	CFStringRef cfStr = macString->getCFString ();
	if (cfStr == nullptr)
	{
	#if DEBUG
		DebugPrint ("Empty CFStringRef in MacString. This is unexpected !\n");
	#endif
		return nullptr;
	}

	CGColorRef cgColorRef = nullptr;
	if (fontColor != lastColor)
	{
		cgColorRef = getCGColor (fontColor);
		lastColor = fontColor;
	}
	CFAttributedStringRef attrStr = CFAttributedStringCreate (kCFAllocatorDefault, cfStr, getStringAttributes (cgColorRef));
	if (attrStr)
	{
		CTLineRef line = CTLineCreateWithAttributedString (attrStr);
		if (context && line)
		{
			macString->setCTLine (line, this, fontColor);
		}
		CFRelease (attrStr);
		return line;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
void CoreTextFont::drawString (CDrawContext* context, IPlatformString* string, const CPoint& point, bool antialias) const
{
	MacString* macString = dynamic_cast<MacString*> (string);
	if (macString == nullptr)
		return;

	CTLineRef line = createCTLine (context, macString);
	if (line)
	{
		bool integralMode = context->getDrawMode ().integralMode ();
		CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
		CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (true, integralMode) : nullptr;
		if (cgContext)
		{
			CGPoint cgPoint = CGPointFromCPoint (point);
			if (integralMode)
				cgPoint = cgDrawContext->pixelAlligned (cgPoint);
			CGContextSetShouldAntialias (cgContext, antialias);
			CGContextSetShouldSmoothFonts (cgContext, true);
			CGContextSetShouldSubpixelPositionFonts (cgContext, true);
			CGContextSetShouldSubpixelQuantizeFonts (cgContext, true);
			CGContextSetTextPosition (cgContext, static_cast<CGFloat> (point.x), cgPoint.y);
			CTLineDraw (line, cgContext);
			if (style & kUnderlineFace)
			{
				CGColorRef cgColorRef = getCGColor (context->getFontColor ());
				CGFloat underlineOffset = CTFontGetUnderlinePosition (fontRef) - 1.f;
				CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
				CGContextSetStrokeColorWithColor (cgContext, cgColorRef);
				CGContextSetLineWidth (cgContext, underlineThickness);
				cgPoint = CGContextGetTextPosition (cgContext);
				CGContextBeginPath (cgContext);
				CGContextMoveToPoint (cgContext, static_cast<CGFloat> (point.x), cgPoint.y - underlineOffset);
				CGContextAddLineToPoint (cgContext, cgPoint.x, cgPoint.y - underlineOffset);
				CGContextDrawPath (cgContext, kCGPathStroke);
			}
			if (style & kStrikethroughFace)
			{
				CGColorRef cgColorRef = getCGColor (context->getFontColor ());
				CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
				CGFloat offset = CTFontGetXHeight (fontRef) * 0.5f;
				CGContextSetStrokeColorWithColor (cgContext, cgColorRef);
				CGContextSetLineWidth (cgContext, underlineThickness);
				cgPoint = CGContextGetTextPosition (cgContext);
				CGContextBeginPath (cgContext);
				CGContextMoveToPoint (cgContext, static_cast<CGFloat> (point.x), cgPoint.y - offset);
				CGContextAddLineToPoint (cgContext, cgPoint.x, cgPoint.y - offset);
				CGContextDrawPath (cgContext, kCGPathStroke);
			}
			cgDrawContext->releaseCGContext (cgContext);
		}
		CFRelease (line);
	}
}

//-----------------------------------------------------------------------------
CCoord CoreTextFont::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias) const
{
	CCoord result = 0;
	MacString* macString = dynamic_cast<MacString*> (string);
	if (macString == nullptr)
		return result;
	
	CTLineRef line = createCTLine (context, macString);
	if (line)
	{
		result = CTLineGetTypographicBounds (line, nullptr, nullptr, nullptr);
		CFRelease (line);
	}
	return result;
}

} // namespace

#endif // MAC
