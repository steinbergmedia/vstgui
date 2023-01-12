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

#ifndef MAC_OS_X_VERSION_10_14
#define MAC_OS_X_VERSION_10_14      101400
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct RegisterBundleFonts
{
	static void init ()
	{
		static RegisterBundleFonts instance;
	}
private:
	static void ErrorApplierFunction (const void *value, void *context)
	{
		auto error = CFErrorRef (value);
		CFShow (error);
	}

#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

	RegisterBundleFonts ()
	{
		fontUrls = CFArrayCreateMutable (kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		auto fontTypes = {CFSTR ("ttf"), CFSTR ("ttc"), CFSTR ("otf")};
		for (auto& t : fontTypes)
			getUrlsForType (t, fontUrls);
		if (CFArrayGetCount (fontUrls) == 0)
			return;
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_14
		CTFontManagerRegisterFontURLs (
			fontUrls, kCTFontManagerScopeProcess, true, [] (CFArrayRef errors, bool done) {
				CFArrayApplyFunction (errors, CFRangeMake (0, CFArrayGetCount (errors)),
									  ErrorApplierFunction, nullptr);
				return true;
			});
#else
		CFArrayRef errors;
		if (!CTFontManagerRegisterFontsForURLs (fontUrls, kCTFontManagerScopeProcess, &errors))
		{
			CFArrayApplyFunction (errors, CFRangeMake (0, CFArrayGetCount (errors)),
			                      ErrorApplierFunction, nullptr);
			CFRelease (errors);
		}
#endif
	}
	~RegisterBundleFonts ()
	{
		if (CFArrayGetCount (fontUrls) == 0)
			return;
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_14
		CTFontManagerUnregisterFontURLs (
			fontUrls, kCTFontManagerScopeProcess, [] (CFArrayRef errors, bool done) {
				CFArrayApplyFunction (errors, CFRangeMake (0, CFArrayGetCount (errors)),
									  ErrorApplierFunction, nullptr);
				return true;
			});
#else
		CFArrayRef errors;
		if (!CTFontManagerUnregisterFontsForURLs (fontUrls, kCTFontManagerScopeProcess, &errors))
		{
			CFArrayApplyFunction (errors, CFRangeMake (0, CFArrayGetCount (errors)),
			                      ErrorApplierFunction, nullptr);
			CFRelease (errors);
		}
#endif
	}

#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14
#pragma clang diagnostic pop
#endif

	void getUrlsForType (CFStringRef fontType, CFMutableArrayRef& array)
	{
		if (auto a = CFBundleCopyResourceURLsOfType (getBundleRef (), fontType, CFSTR ("Fonts")))
		{
			CFArrayAppendArray (array, a, CFRangeMake (0, CFArrayGetCount (a)));
			CFRelease (a);
		}
	}

	CFMutableArrayRef fontUrls;
};

//-----------------------------------------------------------------------------
bool CoreTextFont::getAllFontFamilies (const FontFamilyCallback& callback) noexcept
{
	RegisterBundleFonts::init ();
#if TARGET_OS_IPHONE
	NSArray* fonts = [UIFont familyNames];
#else
	NSArray* fonts = [(NSArray*)CTFontManagerCopyAvailableFontFamilyNames () autorelease];
#endif
	fonts = [fonts sortedArrayUsingSelector:@selector (localizedCaseInsensitiveCompare:)];
	for (uint32_t i = 0; i < [fonts count]; i++)
	{
		NSString* font = [fonts objectAtIndex:i];
		if (!callback ([font UTF8String]))
			break;
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
	RegisterBundleFonts::init ();
	CFStringRef fontNameRef = fromUTF8String<CFStringRef> (name);
	if (fontNameRef)
	{
		if (@available (macOS 10.10, *))
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

} // VSTGUI

#endif // MAC
