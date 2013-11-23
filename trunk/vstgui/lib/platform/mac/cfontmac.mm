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

#import "cfontmac.h"
#import "../../cdrawcontext.h"

#if MAC
#import "macstring.h"
#import "cgdrawcontext.h"
#import "macglobals.h"
#if !TARGET_OS_IPHONE
#import <Cocoa/Cocoa.h>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
IPlatformFont* IPlatformFont::create (UTF8StringPtr name, const CCoord& size, const int32_t& style)
{
	#if VSTGUI_USES_CORE_TEXT
	CoreTextFont* font = new CoreTextFont (name, size, style);
	if (font->getFontRef ())
		return font;
	font->forget ();
	#else
	ATSUFont* font = new ATSUFont (name, size, style);
	if (font->getATSUStyle ())
		return font;
	font->forget ();
	#endif
	return 0;
}

//-----------------------------------------------------------------------------
bool IPlatformFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
#if TARGET_OS_IPHONE
	return false;
#else
	NSArray* fonts = [[NSFontManager sharedFontManager] availableFontFamilies];
	for (uint32_t i = 0; i < [fonts count]; i++)
	{
		NSString* font = [fonts objectAtIndex:i];
		fontFamilyNames.push_back (std::string ([font UTF8String]));
	}
	return true;
#endif
}

#if VSTGUI_USES_CORE_TEXT
//-----------------------------------------------------------------------------
static CGColorRef createCGColorFromCColor (const CColor& c)
{
	CGFloat compontents[] = {
		(CGFloat)c.red / (CGFloat)255.,
		(CGFloat)c.green / (CGFloat)255.,
		(CGFloat)c.blue / (CGFloat)255.,
		(CGFloat)c.alpha / (CGFloat)255.
	};
	return CGColorCreate (GetCGColorSpace (), compontents);
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
	return fontRef;
}

//-----------------------------------------------------------------------------
CoreTextFont::CoreTextFont (UTF8StringPtr name, const CCoord& size, const int32_t& style)
: fontRef (0)
, style (style)
, underlineStyle (false)
, lastColor (MakeCColor (0,0,0,0))
, stringAttributes (0)
{
	CFStringRef fontNameRef = CFStringCreateWithCString (kCFAllocatorDefault, name, kCFStringEncodingUTF8);
	if (fontNameRef)
	{
		fontRef = CTFontCreateWithName (fontNameRef, size, 0);
		if (style & kBoldFace)
			fontRef = CoreTextCreateTraitsVariant (fontRef, kCTFontBoldTrait);
		if (style & kItalicFace)
			fontRef = CoreTextCreateTraitsVariant (fontRef, kCTFontItalicTrait);
		CFRelease (fontNameRef);
	}
}

//-----------------------------------------------------------------------------
CoreTextFont::~CoreTextFont ()
{
	if (stringAttributes)
		CFRelease (stringAttributes);
	CFRelease (fontRef);
}

//-----------------------------------------------------------------------------
double CoreTextFont::getAscent () const
{
	return CTFontGetAscent (fontRef);
}

//-----------------------------------------------------------------------------
double CoreTextFont::getDescent () const
{
	return CTFontGetDescent (fontRef);
}

//-----------------------------------------------------------------------------
double CoreTextFont::getLeading () const
{
	return CTFontGetLeading (fontRef);
}

//-----------------------------------------------------------------------------
double CoreTextFont::getCapHeight () const
{
	return CTFontGetCapHeight (fontRef);
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
void CoreTextFont::drawString (CDrawContext* context, const CString& string, const CPoint& point, bool antialias)
{
	const MacString* macString = dynamic_cast<const MacString*> (string.getPlatformString ());
	CFStringRef utf8Str = macString ? macString->getCFString () : 0;
	if (utf8Str)
	{
		CColor fontColor = context->getFontColor ();
		CGColorRef cgColorRef = 0;
		if (fontColor != lastColor)
		{
			cgColorRef = createCGColorFromCColor (fontColor);
			lastColor = fontColor;
		}
		CFAttributedStringRef attrStr = CFAttributedStringCreate (kCFAllocatorDefault, utf8Str, getStringAttributes (cgColorRef));
		if (attrStr)
		{
			CTLineRef line = CTLineCreateWithAttributedString (attrStr);
			if (line)
			{
				CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
				CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (true, context->getDrawMode () == kAliasing) : 0;
				if (cgContext)
				{
					CGContextSetShouldAntialias (cgContext, antialias);
					CGContextSetShouldSmoothFonts (cgContext, true);
					CGContextSetTextPosition (cgContext, point.x, point.y);
					CTLineDraw (line, cgContext);
					if (style & kUnderlineFace)
					{
						if (cgColorRef == 0)
							cgColorRef = createCGColorFromCColor (fontColor);
						CGFloat underlineOffset = CTFontGetUnderlinePosition (fontRef) - 1.;
						CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
						CGContextSetStrokeColorWithColor (cgContext, cgColorRef);
						CGContextSetLineWidth (cgContext, underlineThickness);
						CGPoint cgPoint = CGContextGetTextPosition (cgContext);
						CGContextBeginPath (cgContext);
						CGContextMoveToPoint (cgContext, point.x, point.y - underlineOffset);
						CGContextAddLineToPoint (cgContext, cgPoint.x, point.y - underlineOffset);
						CGContextDrawPath (cgContext, kCGPathStroke);
					}
					if (style & kStrikethroughFace)
					{
						CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
						CGFloat offset = CTFontGetXHeight (fontRef) * 0.5;
						CGContextSetStrokeColorWithColor (cgContext, cgColorRef);
						CGContextSetLineWidth (cgContext, underlineThickness);
						CGPoint cgPoint = CGContextGetTextPosition (cgContext);
						CGContextBeginPath (cgContext);
						CGContextMoveToPoint (cgContext, point.x, point.y - offset);
						CGContextAddLineToPoint (cgContext, cgPoint.x, point.y - offset);
						CGContextDrawPath (cgContext, kCGPathStroke);
					}	
					cgDrawContext->releaseCGContext (cgContext);
				}
				CFRelease (line);
			}
			CFRelease (attrStr);
		}
		if (cgColorRef)
			CFRelease (cgColorRef);
	}
}

//-----------------------------------------------------------------------------
CCoord CoreTextFont::getStringWidth (CDrawContext* context, const CString& string, bool antialias)
{
	CCoord result = 0;
	const MacString* macString = dynamic_cast<const MacString*> (string.getPlatformString ());
	CFStringRef utf8Str = macString ? macString->getCFString () : 0;
	if (utf8Str)
	{
		CFAttributedStringRef attrStr = CFAttributedStringCreate (kCFAllocatorDefault, utf8Str, getStringAttributes ());
		if (attrStr)
		{
			CTLineRef line = CTLineCreateWithAttributedString (attrStr);
			if (line)
			{
				result = CTLineGetTypographicBounds (line, NULL, NULL, NULL);
				CFRelease (line);
			}
			CFRelease (attrStr);
		}
	}
	return result;
}

#else // VSTGUI_USES_CORE_TEXT
//-----------------------------------------------------------------------------
ATSUFont::ATSUFont (UTF8StringPtr name, const CCoord& size, const int32_t& style)
: atsuStyle (0)
{
	OSStatus status = ATSUCreateStyle (&atsuStyle);
	if (status == noErr)
	{
		ATSUFontID atsuFontID;
		status = ATSUFindFontFromName (name, strlen (name), kFontFullName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &atsuFontID);
		if (status != noErr)
			status = ATSUFindFontFromName (name, strlen (name), kFontFamilyName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &atsuFontID);
		if (status == noErr)
		{
			Fixed atsuSize = FloatToFixed ((float)size);
			Boolean italic = style & kItalicFace;
			Boolean underline = style & kUnderlineFace;
			Boolean bold = style & kBoldFace;
			ATSUAttributeTag  theTags[] =  { kATSUFontTag, kATSUSizeTag, kATSUQDItalicTag, kATSUQDUnderlineTag, kATSUQDBoldfaceTag};
			ByteCount        theSizes[] = { sizeof(ATSUFontID), sizeof(Fixed), sizeof (Boolean), sizeof (Boolean), sizeof (Boolean) };
			ATSUAttributeValuePtr theValues[] = {&atsuFontID, &atsuSize, &italic, &underline, &bold};
			status = ATSUSetAttributes (atsuStyle, 5, theTags, theSizes, theValues);
		}
		if (status != noErr)
		{
			ATSUDisposeStyle (atsuStyle);
			atsuStyle = 0;
		}
	}
}

//-----------------------------------------------------------------------------
ATSUFont::~ATSUFont ()
{
	if (atsuStyle)
		ATSUDisposeStyle (atsuStyle);
}

//-----------------------------------------------------------------------------
void ATSUFont::drawString (CDrawContext* context, const CString& string, const CPoint& point, bool antialias)
{
	if (atsuStyle == 0)
		return;

	CColor fontColor = context->getFontColor ();

	const MacString* macString = dynamic_cast<const MacString*> (string.getPlatformString ());
	CFStringRef utf8Str = macString ? macString->getCFString () : 0;
	if (utf8Str)
	{
		CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
		CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (false) : 0;
		if (cgContext)
		{
			OSStatus status;
			ATSURGBAlphaColor color = {fontColor.red/255.f, fontColor.green/255.f, fontColor.blue/255.f, fontColor.alpha/255.f};
			ATSUAttributeTag  colorTag[] =  { kATSURGBAlphaColorTag };
			ByteCount        colorSize[] = { sizeof(ATSURGBAlphaColor) };
			ATSUAttributeValuePtr colorValue [] = { &color };
			status = ATSUSetAttributes (atsuStyle, 1, colorTag, colorSize, colorValue);

			CGContextSetShouldAntialias (cgContext, antialias);

			CFIndex stringLength = CFStringGetLength (utf8Str);
			UniChar* textBuffer = (UniChar*)malloc (stringLength*sizeof (UniChar));
			CFStringGetCharacters (utf8Str, CFRangeMake (0, stringLength), textBuffer);

			ATSUTextLayout textLayout;
			status = ATSUCreateTextLayout (&textLayout);
			status = ATSUSetTextPointerLocation (textLayout, textBuffer, kATSUFromTextBeginning, kATSUToTextEnd, stringLength);
			status = ATSUSetRunStyle (textLayout, atsuStyle, kATSUFromTextBeginning, kATSUToTextEnd);
			status = ATSUSetTransientFontMatching (textLayout, true);
			
			ATSUAttributeTag		theTags[]	= { kATSUCGContextTag };
			ByteCount				theSizes[]	= { sizeof (CGContextRef) };
			ATSUAttributeValuePtr	theValues[]	= { &cgContext };
			status = ATSUSetLayoutControls (textLayout, 1, theTags, theSizes, theValues);

			status = ATSUDrawText (textLayout, kATSUFromTextBeginning, kATSUToTextEnd, X2Fix(point.h), X2Fix(point.v*-1.f));
			
			ATSUDisposeTextLayout (textLayout);
			free (textBuffer);
			
			cgDrawContext->releaseCGContext (cgContext);
		}
	}
}

//-----------------------------------------------------------------------------
CCoord ATSUFont::getStringWidth (CDrawContext* context, const CString& string, bool antialias)
{
	CCoord result = 0;
	if (atsuStyle)
	{
		const MacString* macString = dynamic_cast<const MacString*> (string.getPlatformString ());
		CFStringRef utf8Str = macString ? macString->getCFString () : 0;
		if (utf8Str)
		{
			OSStatus status;
			CFIndex stringLength = CFStringGetLength (utf8Str);
			UniChar* textBuffer = (UniChar*)malloc (stringLength*sizeof (UniChar));
			CFStringGetCharacters (utf8Str, CFRangeMake (0, stringLength), textBuffer);

			ATSUTextLayout textLayout;
			status = ATSUCreateTextLayout (&textLayout);
			status = ATSUSetTextPointerLocation (textLayout, textBuffer, kATSUFromTextBeginning, kATSUToTextEnd, stringLength);
			status = ATSUSetRunStyle (textLayout, atsuStyle, kATSUFromTextBeginning, kATSUToTextEnd);
			status = ATSUSetTransientFontMatching (textLayout, true);
			
			CGDrawContext* cgDrawContext = context ? dynamic_cast<CGDrawContext*> (context) : 0;
			CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (true) : 0;
			if (cgContext)
			{
				ATSUAttributeTag		theTags[]	= { kATSUCGContextTag };
				ByteCount				theSizes[]	= { sizeof (CGContextRef) };
				ATSUAttributeValuePtr	theValues[]	= { &cgContext };
				status = ATSUSetLayoutControls (textLayout, 1, theTags, theSizes, theValues);
			}

			ATSUTextMeasurement iBefore, iAfter, ascent, descent; 
			status = ATSUGetUnjustifiedBounds (textLayout, 0, kATSUToTextEnd, &iBefore, &iAfter, &ascent, &descent);
			result = Fix2X (iAfter);
			
			ATSUDisposeTextLayout (textLayout);
			free (textBuffer);

			if (context)
			{
				cgDrawContext->releaseCGContext (cgContext);
			}
		}
	}
	return result;
}

#endif

} // namespace

#endif // MAC
