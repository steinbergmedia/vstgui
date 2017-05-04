// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __d2dfont__
#define __d2dfont__

#include "../../iplatformfont.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

struct IDWriteTextFormat;
struct IDWriteTextLayout;

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DFont : public IPlatformFont, public IFontPainter
{
public:
	D2DFont (const UTF8String& name, const CCoord& size, const int32_t& style);

	IDWriteTextLayout* createTextLayout (IPlatformString* string) const;

	static bool getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames);

protected:
	~D2DFont ();
	
	double getAscent () const override { return ascent; }
	double getDescent () const override { return descent; }
	double getLeading () const override { return leading; }
	double getCapHeight () const override { return capHeight; }

	const IFontPainter* getPainter () const override { return this; }

	void drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias = true) const override;
	CCoord getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias = true) const override;

	IDWriteTextFormat* textFormat;
	double ascent;
	double descent;
	double leading;
	double capHeight;
	int32_t style;
};

} // namespace

#endif // WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#endif // __d2dfont__