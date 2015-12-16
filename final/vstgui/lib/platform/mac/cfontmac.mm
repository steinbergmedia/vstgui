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
IPlatformFont* IPlatformFont::create (UTF8StringPtr name, const CCoord& size, const int32_t& style)
{
	CoreTextFont* font = new CoreTextFont (name, size, style);
	if (font->getFontRef ())
		return font;
	font->forget ();
	return 0;
}

//-----------------------------------------------------------------------------
bool IPlatformFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
#if TARGET_OS_IPHONE
	NSArray* fonts = [UIFont familyNames];
#elif MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
	NSArray* fonts = [(NSArray*)CTFontManagerCopyAvailableFontFamilyNames () autorelease];
#else
	NSArray* fonts = [[NSFontManager sharedFontManager] availableFontFamilies];
#endif
	for (uint32_t i = 0; i < [fonts count]; i++)
	{
		NSString* font = [fonts objectAtIndex:i];
		fontFamilyNames.push_back (std::string ([font UTF8String]));
	}
	return true;
}

//-----------------------------------------------------------------------------
static CTFontRef CoreTextCreateTraitsVariant (CTFontRef fontRef, CTFontSymbolicTraits trait)
{
	CTFontRef traitsFontRef = CTFontCreateCopyWithSymbolicTraits (fontRef, CTFontGetSize (fontRef), NULL, trait, trait);
	if (traitsFontRef)
	{
		CFRelease (fontRef);
		return traitsFontRef;
	}
	else if (trait == kCTFontItalicTrait)
	{
		CGAffineTransform transform = { 1, 0, -0.5, 1, 0, 0 };
		traitsFontRef = CTFontCreateCopyWithAttributes (fontRef, CTFontGetSize (fontRef), &transform, NULL);
		if (traitsFontRef)
		{
			CFRelease (fontRef);
			return traitsFontRef;
		}
	}
	return fontRef;
}

//-----------------------------------------------------------------------------
CoreTextFont::CoreTextFont (UTF8StringPtr name, const CCoord& size, const int32_t& style)
: fontRef (0)
, style (style)
, underlineStyle (false)
, lastColor (MakeCColor (0,0,0,0))
, stringAttributes (0)
, ascent (0.)
, descent (0.)
, leading (0.)
, capHeight (0.)
{
	CFStringRef fontNameRef = CFStringCreateWithCString (kCFAllocatorDefault, name, kCFStringEncodingUTF8);
	if (fontNameRef)
	{
		if (getCTVersion () >= 0x00060000)
		{
			CFMutableDictionaryRef attributes = CFDictionaryCreateMutable (kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionaryAddValue (attributes, kCTFontFamilyNameAttribute, fontNameRef);
			CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes (attributes);
			fontRef = CTFontCreateWithFontDescriptor (descriptor, static_cast<CGFloat> (size), 0);
			CFRelease (attributes);
			CFRelease (descriptor);
		}
		else
		{
			fontRef = CTFontCreateWithName (fontNameRef, static_cast<CGFloat> (size), 0);
		}

		if (style & kBoldFace)
			fontRef = CoreTextCreateTraitsVariant (fontRef, kCTFontBoldTrait);
		if (style & kItalicFace)
			fontRef = CoreTextCreateTraitsVariant (fontRef, kCTFontItalicTrait);
		CFRelease (fontNameRef);
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
CoreTextFont::~CoreTextFont ()
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
CFDictionaryRef CoreTextFont::getStringAttributes (const CGColorRef color)
{
	if (stringAttributes == 0)
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
CTLineRef CoreTextFont::createCTLine (CDrawContext* context, MacString* macString)
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
	if (cfStr == 0)
	{
	#if DEBUG
		DebugPrint ("Empty CFStringRef in MacString. This is unexpected !\n");
	#endif
		return NULL;
	}

	CGColorRef cgColorRef = 0;
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

	return NULL;
}

//-----------------------------------------------------------------------------
void CoreTextFont::drawString (CDrawContext* context, IPlatformString* string, const CPoint& point, bool antialias)
{
	MacString* macString = dynamic_cast<MacString*> (string);
	if (macString == 0)
		return;

	CTLineRef line = createCTLine (context, macString);
	if (line)
	{
		bool integralMode = context->getDrawMode ().integralMode ();
		CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
		CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (true, integralMode) : 0;
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
CCoord CoreTextFont::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias)
{
	CCoord result = 0;
	MacString* macString = dynamic_cast<MacString*> (string);
	if (macString == 0)
		return result;
	
	CTLineRef line = createCTLine (context, macString);
	if (line)
	{
		result = CTLineGetTypographicBounds (line, NULL, NULL, NULL);
		CFRelease (line);
	}
	return result;
}

} // namespace

#endif // MAC
