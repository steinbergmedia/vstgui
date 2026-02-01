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
SharedPointer<CFontDesc> kSystemFont = {};
SharedPointer<CFontDesc> kNormalFontVeryBig = {};
SharedPointer<CFontDesc> kNormalFontBig = {};
SharedPointer<CFontDesc> kNormalFont = {};
SharedPointer<CFontDesc> kNormalFontSmall = {};
SharedPointer<CFontDesc> kNormalFontSmaller = {};
SharedPointer<CFontDesc> kNormalFontVerySmall = {};
SharedPointer<CFontDesc> kSymbolFont = {};

//-----------------------------------------------------------------------------
void CFontDesc::init ()
{
#if MAC
#if TARGET_OS_IPHONE
	globalFonts.systemFont = makeShared<CFontDesc> ("Helvetica", 12);
	globalFonts.normalFontVeryBig = makeShared<CFontDesc> ("ArialMT", 18);
	globalFonts.normalFontBig = makeShared<CFontDesc> ("ArialMT", 14);
	globalFonts.normalFont = makeShared<CFontDesc> ("ArialMT", 12);
	globalFonts.normalFontSmall = makeShared<CFontDesc> ("ArialMT", 11);
	globalFonts.normalFontSmaller = makeShared<CFontDesc> ("ArialMT", 10);
	globalFonts.normalFontVerySmall = makeShared<CFontDesc> ("ArialMT", 9);
	globalFonts.symbolFont = makeShared<CFontDesc> ("Symbol", 12);
#else
	globalFonts.systemFont = makeShared<CFontDesc> ("Lucida Grande", 12);
	globalFonts.normalFontVeryBig = makeShared<CFontDesc> ("Arial", 18);
	globalFonts.normalFontBig = makeShared<CFontDesc> ("Arial", 14);
	globalFonts.normalFont = makeShared<CFontDesc> ("Arial", 12);
	globalFonts.normalFontSmall = makeShared<CFontDesc> ("Arial", 11);
	globalFonts.normalFontSmaller = makeShared<CFontDesc> ("Arial", 10);
	globalFonts.normalFontVerySmall = makeShared<CFontDesc> ("Arial", 9);
	globalFonts.symbolFont = makeShared<CFontDesc> ("Symbol", 12);
#endif

#elif WINDOWS
	globalFonts.systemFont = makeShared<CFontDesc> ("Arial", 12);
	globalFonts.normalFontVeryBig = makeShared<CFontDesc> ("Arial", 18);
	globalFonts.normalFontBig = makeShared<CFontDesc> ("Arial", 14);
	globalFonts.normalFont = makeShared<CFontDesc> ("Arial", 12);
	globalFonts.normalFontSmall = makeShared<CFontDesc> ("Arial", 11);
	globalFonts.normalFontSmaller = makeShared<CFontDesc> ("Arial", 10);
	globalFonts.normalFontVerySmall = makeShared<CFontDesc> ("Arial", 9);
	globalFonts.symbolFont = makeShared<CFontDesc> ("Symbol", 13);

#else
	globalFonts.systemFont = makeShared<CFontDesc> ("Arial", 12);
	globalFonts.normalFontVeryBig = makeShared<CFontDesc> ("Arial", 18);
	globalFonts.normalFontBig = makeShared<CFontDesc> ("Arial", 14);
	globalFonts.normalFont = makeShared<CFontDesc> ("Arial", 12);
	globalFonts.normalFontSmall = makeShared<CFontDesc> ("Arial", 11);
	globalFonts.normalFontSmaller = makeShared<CFontDesc> ("Arial", 10);
	globalFonts.normalFontVerySmall = makeShared<CFontDesc> ("Arial", 9);
	globalFonts.symbolFont = makeShared<CFontDesc> ("Symbol", 13);

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
The CFontDesc class replaces the old font handling. You have now the possibility to use whatever font you like
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
	vstgui_assert (getNbReference () <= 0, "Always use shared pointers with CFontDesc!");
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
	auto pf = getPlatformFont ();
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
