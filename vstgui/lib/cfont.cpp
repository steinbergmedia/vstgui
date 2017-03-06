//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
CFontDesc::CFontDesc (UTF8StringPtr inName, const CCoord& inSize, const int32_t inStyle)
: name (0)
, size (inSize)
, style (inStyle)
, platformFont (0)
{
	setName (inName);
}

//-----------------------------------------------------------------------------
CFontDesc::CFontDesc (const CFontDesc& font)
: name (0)
, size (0)
, style (0)
, platformFont (0)
{
	*this = font;
}

//-----------------------------------------------------------------------------
CFontDesc::~CFontDesc ()
{
	freePlatformFont ();
	setName (0);
}

//-----------------------------------------------------------------------------
IPlatformFont* CFontDesc::getPlatformFont ()
{
	if (platformFont == 0)
		platformFont = IPlatformFont::create (name, size, style);
	return platformFont;
}

//-----------------------------------------------------------------------------
IFontPainter* CFontDesc::getFontPainter ()
{
	IPlatformFont* pf = getPlatformFont ();
	if (pf)
		return pf->getPainter ();
	return 0;
}

//-----------------------------------------------------------------------------
void CFontDesc::freePlatformFont ()
{
	if (platformFont)
	{
		platformFont->forget ();
		platformFont = 0;
	}
}

//-----------------------------------------------------------------------------
void CFontDesc::setName (UTF8StringPtr newName)
{
	if (name && newName && UTF8StringView (name) == newName)
		return;

	String::free (name);
	name = String::newWithString (newName);
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
	if (UTF8StringView (name) != f.getName ())
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
