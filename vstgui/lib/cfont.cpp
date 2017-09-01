// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cfont.h"
#include "cstring.h"
#include "platform/iplatformfont.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// Global Fonts
//-----------------------------------------------------------------------------
#if MAC
	#if TARGET_OS_IPHONE
	static CFontDesc gSystemFont ("Helvetica", 12);
	static CFontDesc gNormalFontVeryBig ("ArialMT", 18);
	static CFontDesc gNormalFontBig ("ArialMT", 14);
	static CFontDesc gNormalFont ("ArialMT", 12);
	static CFontDesc gNormalFontSmall ("ArialMT", 11);
	static CFontDesc gNormalFontSmaller ("ArialMT", 10);
	static CFontDesc gNormalFontVerySmall ("ArialMT", 9);
	static CFontDesc gSymbolFont ("Symbol", 12);
	#else
	static CFontDesc gSystemFont ("Lucida Grande", 12);
	static CFontDesc gNormalFontVeryBig ("Arial", 18);
	static CFontDesc gNormalFontBig ("Arial", 14);
	static CFontDesc gNormalFont ("Arial", 12);
	static CFontDesc gNormalFontSmall ("Arial", 11);
	static CFontDesc gNormalFontSmaller ("Arial", 10);
	static CFontDesc gNormalFontVerySmall ("Arial", 9);
	static CFontDesc gSymbolFont ("Symbol", 12);
	#endif

#elif WINDOWS
	static CFontDesc gSystemFont ("Arial", 12);
	static CFontDesc gNormalFontVeryBig ("Arial", 18);
	static CFontDesc gNormalFontBig ("Arial", 14);
	static CFontDesc gNormalFont ("Arial", 12);
	static CFontDesc gNormalFontSmall ("Arial", 11);
	static CFontDesc gNormalFontSmaller ("Arial", 10);
	static CFontDesc gNormalFontVerySmall ("Arial", 9);
	static CFontDesc gSymbolFont ("Symbol", 13);

#else
	static CFontDesc gSystemFont ("Arial", 12);
	static CFontDesc gNormalFontVeryBig ("Arial", 18);
	static CFontDesc gNormalFontBig ("Arial", 14);
	static CFontDesc gNormalFont ("Arial", 12);
	static CFontDesc gNormalFontSmall ("Arial", 11);
	static CFontDesc gNormalFontSmaller ("Arial", 10);
	static CFontDesc gNormalFontVerySmall ("Arial", 9);
	static CFontDesc gSymbolFont ("Symbol", 13);

#endif

const CFontRef kSystemFont				= &gSystemFont;
const CFontRef kNormalFontVeryBig		= &gNormalFontVeryBig;
const CFontRef kNormalFontBig			= &gNormalFontBig;
const CFontRef kNormalFont				= &gNormalFont;
const CFontRef kNormalFontSmall			= &gNormalFontSmall;
const CFontRef kNormalFontSmaller		= &gNormalFontSmaller;
const CFontRef kNormalFontVerySmall		= &gNormalFontVerySmall;
const CFontRef kSymbolFont				= &gSymbolFont;

//-----------------------------------------------------------------------------
// CFontDesc Implementation
/*! @class CFontDesc
The CFontDesc class replaces the old font handling. You have now the possibilty to use whatever font you like
as long as it is available on the system. You should cache your own CFontDesc as this speeds up drawing on some systems.
*/
//-----------------------------------------------------------------------------
CFontDesc::CFontDesc (const UTF8String& inName, const CCoord& inSize, const int32_t inStyle)
: size (inSize)
, style (inStyle)
, platformFont (nullptr)
{
	setName (inName);
}

//-----------------------------------------------------------------------------
CFontDesc::CFontDesc (const CFontDesc& font)
: size (0)
, style (0)
, platformFont (nullptr)
{
	*this = font;
}

//-----------------------------------------------------------------------------
void CFontDesc::beforeDelete ()
{
	freePlatformFont ();
}

//-----------------------------------------------------------------------------
auto CFontDesc::getPlatformFont () const -> const PlatformFontPtr
{
	if (platformFont == nullptr)
		platformFont = IPlatformFont::create (name, size, style);
	return platformFont;
}

//-----------------------------------------------------------------------------
const IFontPainter* CFontDesc::getFontPainter () const
{
	IPlatformFont* pf = getPlatformFont ();
	if (pf)
		return pf->getPainter ();
	return nullptr;
}

//-----------------------------------------------------------------------------
void CFontDesc::freePlatformFont ()
{
	platformFont = nullptr;
}

//-----------------------------------------------------------------------------
void CFontDesc::setName (const UTF8String& newName)
{
	if (name == newName)
		return;

	name = newName;
	freePlatformFont ();
}

//-----------------------------------------------------------------------------
void CFontDesc::setSize (CCoord newSize)
{
	size = newSize;
	freePlatformFont ();
}

//-----------------------------------------------------------------------------
void CFontDesc::setStyle (int32_t newStyle)
{
	style = newStyle;
	freePlatformFont ();
}

//-----------------------------------------------------------------------------
CFontDesc& CFontDesc::operator = (const CFontDesc& f)
{
	setName (f.getName ());
	setSize (f.getSize ());
	setStyle (f.getStyle ());
	return *this;
}

//-----------------------------------------------------------------------------
bool CFontDesc::operator == (const CFontDesc& f) const
{
	if (size != f.getSize ())
		return false;
	if (style != f.getStyle ())
		return false;
	if (name != f.getName ())
		return false;
	return true;
}

//-----------------------------------------------------------------------------
void CFontDesc::cleanup ()
{
	gSystemFont.freePlatformFont ();
	gNormalFontVeryBig.freePlatformFont ();
	gNormalFontBig.freePlatformFont ();
	gNormalFont.freePlatformFont ();
	gNormalFontSmall.freePlatformFont ();
	gNormalFontSmaller.freePlatformFont ();
	gNormalFontVerySmall.freePlatformFont ();
	gSymbolFont.freePlatformFont ();
}

} // namespace
