// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cfontwin32__
#define __cfontwin32__

#include "../iplatformfont.h"

#if WINDOWS

#include "win32support.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GdiPlusFont : public IPlatformFont, public IFontPainter
{
public:
	GdiPlusFont (const UTF8String& name, const CCoord& size, const int32_t& style);

	Gdiplus::Font* getFont () const { return font; }

	static bool getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames);
protected:
	~GdiPlusFont () noexcept;
	
	double getAscent () const override;
	double getDescent () const override;
	double getLeading () const override;
	double getCapHeight () const override;

	const IFontPainter* getPainter () const override { return this; }

	void drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias = true) const override;
	CCoord getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias = true) const override;

	Gdiplus::Font* font;
	INT gdiStyle;
};

} // namespace

#endif // WINDOWS

#endif
