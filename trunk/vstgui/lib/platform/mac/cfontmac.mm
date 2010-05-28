//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
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

#import "cfontmac.h"
#import "../../cdrawcontext.h"

#if MAC

#import "cgdrawcontext.h"
#import <Cocoa/Cocoa.h>

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
	NSArray* fonts = [[NSFontManager sharedFontManager] availableFontFamilies];
	for (uint32_t i = 0; i < [fonts count]; i++)
	{
		NSString* font = [fonts objectAtIndex:i];
		fontFamilyNames.push_back (std::string ([font UTF8String]));
	}
	return true;
}

#if VSTGUI_USES_CORE_TEXT
//-----------------------------------------------------------------------------
CoreTextFont::CoreTextFont (UTF8StringPtr name, const CCoord& size, const int32_t& style)
: fontRef (0)
, underlineStyle (false)
{
	CFStringRef fontNameRef = CFStringCreateWithCString (0, name, kCFStringEncodingUTF8);
	if (fontNameRef)
	{
		fontRef = CTFontCreateWithName (fontNameRef, size, 0);
		if (fontRef && (style & kBoldFace || style & kItalicFace))
		{
			CTFontSymbolicTraits desiredTrait = 0;
			CTFontSymbolicTraits traitMask = 0;
			if (style & kBoldFace)
			{
				desiredTrait |= kCTFontBoldTrait;
				traitMask |= kCTFontBoldTrait;
			}
			if (style & kItalicFace)
			{
				desiredTrait |= kCTFontItalicTrait;
				traitMask |= kCTFontItalicTrait;
			}
			CTFontRef traitsFontRef = CTFontCreateCopyWithSymbolicTraits (fontRef, size, NULL, desiredTrait, traitMask);
			if (!traitsFontRef)
			{
				if (style & kBoldFace && style & ~kItalicFace)
				{
					CFStringRef boldFontNameRef = CFStringCreateWithFormat (0, 0, CFSTR("%s Bold"), name);
					if (boldFontNameRef)
					{
						traitsFontRef = CTFontCreateWithName (boldFontNameRef, size, 0);
						CFRelease (boldFontNameRef);
					}
				}
			}
			if (traitsFontRef)
			{
				CFRelease (fontRef);
				fontRef = traitsFontRef;
			}
		}
		CFRelease (fontNameRef);
	}
	underlineStyle = style & kUnderlineFace;
}

//-----------------------------------------------------------------------------
CoreTextFont::~CoreTextFont ()
{
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
void CoreTextFont::drawString (CDrawContext* context, UTF8StringPtr utf8String, const CPoint& point, bool antialias)
{
	CColor fontColor = context->getFontColor ();
	
	CFStringRef utf8Str = CFStringCreateWithCString (NULL, utf8String, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CGColorRef cgColor = CGColorCreateGenericRGB (fontColor.red/255.f, fontColor.green/255.f, fontColor.blue/255.f, fontColor.alpha/255.f);
		CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
		CFTypeRef values[] = { fontRef, cgColor };
		CFDictionaryRef attributes = CFDictionaryCreate (kCFAllocatorDefault, (const void**)&keys,(const void**)&values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFAttributedStringRef attrStr = CFAttributedStringCreate (0, utf8Str, attributes);
		CFRelease (attributes);
		if (attrStr)
		{
			CTLineRef line = CTLineCreateWithAttributedString (attrStr);
			if (line)
			{
				CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
				CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (true) : 0;
				if (cgContext)
				{
					CGContextSetShouldAntialias (cgContext, antialias);
					CGContextSetTextPosition (cgContext, point.x, point.y);
					CTLineDraw (line, cgContext);
					if (underlineStyle)
					{
						CGFloat underlineOffset = CTFontGetUnderlinePosition (fontRef);
						CGFloat underlineThickness = CTFontGetUnderlineThickness (fontRef);
						CGContextSetStrokeColorWithColor (cgContext, cgColor);
						CGContextSetLineWidth (cgContext, underlineThickness);
						CGPoint cgPoint = CGContextGetTextPosition (cgContext);
						CGContextBeginPath (cgContext);
						CGContextMoveToPoint (cgContext, point.x, point.y - underlineOffset);
						CGContextAddLineToPoint (cgContext, cgPoint.x, point.y - underlineOffset);
						CGContextDrawPath (cgContext, kCGPathStroke);
					}
					cgDrawContext->releaseCGContext (cgContext);
				}
				CFRelease (line);
			}
			CFRelease (attrStr);
		}
		CFRelease (cgColor);
		CFRelease (utf8Str);
	}
}

//-----------------------------------------------------------------------------
CCoord CoreTextFont::getStringWidth (CDrawContext* context, UTF8StringPtr utf8String, bool antialias)
{
	CCoord result = 0;
	CFStringRef utf8Str = CFStringCreateWithCString (NULL, utf8String, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CFStringRef keys[] = { kCTFontAttributeName };
		CFTypeRef values[] = { fontRef };
		CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,(const void**)&values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFAttributedStringRef attrStr = CFAttributedStringCreate (0, utf8Str, attributes);
		CFRelease (attributes);
		if (attrStr)
		{
			CTLineRef line = CTLineCreateWithAttributedString (attrStr);
			if (line)
			{
				result = floor (CTLineGetTypographicBounds (line, NULL, NULL, NULL) + (antialias ? 1.5 : 0.5));
				CFRelease (line);
			}
			CFRelease (attrStr);
		}
		CFRelease (utf8Str);
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
void ATSUFont::drawString (CDrawContext* context, UTF8StringPtr utf8String, const CPoint& point, bool antialias)
{
	if (atsuStyle == 0)
		return;

	CColor fontColor = context->getFontColor ();

	CFStringRef utf8Str = CFStringCreateWithCString (NULL, utf8String, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CGDrawContext* cgDrawContext = dynamic_cast<CGDrawContext*> (context);
		CGContextRef cgContext = cgDrawContext ? cgDrawContext->beginCGContext (true) : 0;
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
		CFRelease (utf8Str);
	}
}

//-----------------------------------------------------------------------------
CCoord ATSUFont::getStringWidth (CDrawContext* context, UTF8StringPtr utf8String, bool antialias)
{
	CCoord result = 0;
	if (atsuStyle)
	{
		CFStringRef utf8Str = CFStringCreateWithCString (NULL, utf8String, kCFStringEncodingUTF8);
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
			CFRelease (utf8Str);
		}
	}
	return result;
}

#endif

} // namespace

#endif // MAC
