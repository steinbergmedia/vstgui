// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformfont.h"
#include "../platformfactory.h"

#if MAC
#include "../../ccolor.h"

#if TARGET_OS_IPHONE
	#include <CoreText/CoreText.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {
class MacString;

//-----------------------------------------------------------------------------
class CoreTextFont : public IPlatformFont, public IFontPainter
{
public:
	CoreTextFont (const UTF8String& name, const CCoord& size, const int32_t& style);

	double getAscent () const override;
	double getDescent () const override;
	double getLeading () const override;
	double getCapHeight () const override;
	
	const IFontPainter* getPainter () const override { return this; }
	
	CTFontRef getFontRef () const { return fontRef; }
	CGFloat getSize () const { return CTFontGetSize (fontRef); }

	static bool getAllFontFamilies (const FontFamilyCallback& callback) noexcept;
//------------------------------------------------------------------------------------
protected:
	~CoreTextFont () noexcept override;

	void drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias = true) const override;
	CCoord getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias = true) const override;
	CFDictionaryRef getStringAttributes (const CGColorRef color = nullptr) const;

	CTLineRef createCTLine (CDrawContext* context, MacString* macString) const;

	CTFontRef fontRef;
	int32_t style;
	bool underlineStyle;
	mutable CColor lastColor;
	mutable CFMutableDictionaryRef stringAttributes;
	double ascent;
	double descent;
	double leading;
	double capHeight;
};

} // VSTGUI

#endif // MAC
