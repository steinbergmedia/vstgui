// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformfont.h"
#include "../platformfactory.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
class Font
: public IPlatformFont
, public IFontPainter
{
public:
	Font (UTF8StringPtr name, const CCoord& size, const int32_t& style);
	~Font ();

	bool valid () const;

	double getAscent () const override;
	double getDescent () const override;
	double getLeading () const override;
	double getCapHeight () const override;
	const IFontPainter* getPainter () const override;

	void drawString (const PlatformGraphicsDeviceContextPtr& context, IPlatformString* string,
					 const CPoint& p, const CColor& color, bool antialias = true) const override;
	CCoord getStringWidth (const PlatformGraphicsDeviceContextPtr& context, IPlatformString* string,
						   bool antialias = true) const override;

	static bool getAllFamilies (const FontFamilyCallback& callback);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
