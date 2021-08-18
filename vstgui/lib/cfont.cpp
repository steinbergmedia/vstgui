// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cfont.h"
#include "cstring.h"
#include "platform/platformfactory.h"
#include "platform/iplatformfont.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// Global Fonts
//-----------------------------------------------------------------------------
struct GlobalFonts
{
	SharedPointer<CFontDesc> systemFont;
	SharedPointer<CFontDesc> normalFontVeryBig;
	SharedPointer<CFontDesc> normalFontBig;
	SharedPointer<CFontDesc> normalFont;
	SharedPointer<CFontDesc> normalFontSmall;
	SharedPointer<CFontDesc> normalFontSmaller;
	SharedPointer<CFontDesc> normalFontVerySmall;
	SharedPointer<CFontDesc> symbolFont;
};
static GlobalFonts globalFonts;

//-----------------------------------------------------------------------------
CFontRef kSystemFont = nullptr;
CFontRef kNormalFontVeryBig = nullptr;
CFontRef kNormalFontBig = nullptr;
CFontRef kNormalFont = nullptr;
CFontRef kNormalFontSmall = nullptr;
CFontRef kNormalFontSmaller = nullptr;
CFontRef kNormalFontVerySmall = nullptr;
CFontRef kSymbolFont = nullptr;

//-----------------------------------------------------------------------------
void CFontDesc::init ()
{
#if MAC
#if TARGET_OS_IPHONE
	globalFonts.systemFont = makeOwned<CFontDesc> ("Helvetica", 12);
	globalFonts.normalFontVeryBig = makeOwned<CFontDesc> ("ArialMT", 18);
	globalFonts.normalFontBig = makeOwned<CFontDesc> ("ArialMT", 14);
	globalFonts.normalFont = makeOwned<CFontDesc> ("ArialMT", 12);
	globalFonts.normalFontSmall = makeOwned<CFontDesc> ("ArialMT", 11);
	globalFonts.normalFontSmaller = makeOwned<CFontDesc> ("ArialMT", 10);
	globalFonts.normalFontVerySmall = makeOwned<CFontDesc> ("ArialMT", 9);
	globalFonts.symbolFont = makeOwned<CFontDesc> ("Symbol", 12);
#else
	globalFonts.systemFont = makeOwned<CFontDesc> ("Lucida Grande", 12);
	globalFonts.normalFontVeryBig = makeOwned<CFontDesc> ("Arial", 18);
	globalFonts.normalFontBig = makeOwned<CFontDesc> ("Arial", 14);
	globalFonts.normalFont = makeOwned<CFontDesc> ("Arial", 12);
	globalFonts.normalFontSmall = makeOwned<CFontDesc> ("Arial", 11);
	globalFonts.normalFontSmaller = makeOwned<CFontDesc> ("Arial", 10);
	globalFonts.normalFontVerySmall = makeOwned<CFontDesc> ("Arial", 9);
	globalFonts.symbolFont = makeOwned<CFontDesc> ("Symbol", 12);
#endif

#elif WINDOWS
	globalFonts.systemFont = makeOwned<CFontDesc> ("Arial", 12);
	globalFonts.normalFontVeryBig = makeOwned<CFontDesc> ("Arial", 18);
	globalFonts.normalFontBig = makeOwned<CFontDesc> ("Arial", 14);
	globalFonts.normalFont = makeOwned<CFontDesc> ("Arial", 12);
	globalFonts.normalFontSmall = makeOwned<CFontDesc> ("Arial", 11);
	globalFonts.normalFontSmaller = makeOwned<CFontDesc> ("Arial", 10);
	globalFonts.normalFontVerySmall = makeOwned<CFontDesc> ("Arial", 9);
	globalFonts.symbolFont = makeOwned<CFontDesc> ("Symbol", 13);

#else
	globalFonts.systemFont = makeOwned<CFontDesc> ("Arial", 12);
	globalFonts.normalFontVeryBig = makeOwned<CFontDesc> ("Arial", 18);
	globalFonts.normalFontBig = makeOwned<CFontDesc> ("Arial", 14);
	globalFonts.normalFont = makeOwned<CFontDesc> ("Arial", 12);
	globalFonts.normalFontSmall = makeOwned<CFontDesc> ("Arial", 11);
	globalFonts.normalFontSmaller = makeOwned<CFontDesc> ("Arial", 10);
	globalFonts.normalFontVerySmall = makeOwned<CFontDesc> ("Arial", 9);
	globalFonts.symbolFont = makeOwned<CFontDesc> ("Symbol", 13);

#endif
	kSystemFont = globalFonts.systemFont;
	kNormalFontVeryBig = globalFonts.normalFontVeryBig;
	kNormalFontBig = globalFonts.normalFontBig;
	kNormalFont = globalFonts.normalFont;
	kNormalFontSmall = globalFonts.normalFontSmall;
	kNormalFontSmaller = globalFonts.normalFontSmaller;
	kNormalFontVerySmall = globalFonts.normalFontVerySmall;
	kSymbolFont = globalFonts.symbolFont;
}

//-----------------------------------------------------------------------------
void CFontDesc::cleanup ()
{
	globalFonts.systemFont = nullptr;
	globalFonts.normalFontVeryBig = nullptr;
	globalFonts.normalFontBig = nullptr;
	globalFonts.normalFont = nullptr;
	globalFonts.normalFontSmall = nullptr;
	globalFonts.normalFontSmaller = nullptr;
	globalFonts.normalFontVerySmall = nullptr;
	globalFonts.symbolFont = nullptr;

	kSystemFont = nullptr;
	kNormalFontVeryBig = nullptr;
	kNormalFontBig = nullptr;
	kNormalFont = nullptr;
	kNormalFontSmall = nullptr;
	kNormalFontSmaller = nullptr;
	kNormalFontVerySmall = nullptr;
	kSymbolFont = nullptr;
}

//-----------------------------------------------------------------------------
// CFontDesc Implementation
/*! @class CFontDesc
The CFontDesc class replaces the old font handling. You have now the possibilty to use whatever font you like
as long as it is available on the system. You should cache your own CFontDesc as this speeds up drawing on some systems.

\note New in 4.9: It's now possible to use custom fonts. Fonts must reside inside the Bundle/Package at PackageRoot/Resources/Fonts/.
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

//------------------------------------------------------------------------
CFontDesc::~CFontDesc () noexcept
{
	vstgui_assert (getNbReference () == 0, "Always use shared pointers with CFontDesc!");
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
		platformFont = getPlatformFactory ().createFont (name, size, style);
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

} // VSTGUI
