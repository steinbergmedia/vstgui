// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../iplatformfont.h"

#if WINDOWS

#include "../win32support.h"
#include "../../platformfactory.h"

struct IDWriteTextFormat;
struct IDWriteTextLayout;

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DFont final : public IPlatformFont, public IFontPainter
{
public:
	D2DFont (const UTF8String& name, const CCoord& size, const int32_t& style);
	~D2DFont () noexcept;

	IDWriteTextLayout* createTextLayout (const PlatformStringPtr& string) const;

	bool asLogFont (LOGFONTW& logfont) const;

	static bool getAllFontFamilies (const FontFamilyCallback& callback);

	static void initialize (const UTF8String& resourceBasePath);
	static void terminate ();

protected:
	double getAscent () const override { return ascent; }
	double getDescent () const override { return descent; }
	double getLeading () const override { return leading; }
	double getCapHeight () const override { return capHeight; }

	const IFontPainter* getPainter () const override { return this; }

	void drawString (const PlatformGraphicsDeviceContextPtr& context, const PlatformStringPtr& string,
					 const CPoint& p, const CColor& color, bool antialias = true) const override;
	CCoord getStringWidth (const PlatformGraphicsDeviceContextPtr& context, const PlatformStringPtr& string,
						   bool antialias = true) const override;

	IDWriteTextFormat* textFormat;
	double ascent;
	double descent;
	double leading;
	double capHeight;
	int32_t style;
};

} // VSTGUI

#endif // WINDOWS
