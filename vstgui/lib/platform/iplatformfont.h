// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformfont__
#define __iplatformfont__

#include "../vstguifwd.h"
#include <list>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// IFontPainter Declaration
//! @brief font paint interface
//-----------------------------------------------------------------------------
class IFontPainter
{
public:
	virtual ~IFontPainter () noexcept = default;

	virtual void drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias = true) const = 0;
	virtual CCoord getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias = true) const = 0;
};

//-----------------------------------------------------------------------------
// IPlatformFont declaration
//! @brief platform font class
///
/// Encapsulation of a platform font. You should never need to call IPlatformFont::create(..), instead use CFontDesc::getPlatformFont().
//-----------------------------------------------------------------------------
class IPlatformFont : public AtomicReferenceCounted
{
public:
	static SharedPointer<IPlatformFont> create (const UTF8String& name, const CCoord& size, const int32_t& style);
	static bool getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames);
	
	virtual double getAscent () const = 0;		///< returns the ascent line offset of the baseline of this font. If not supported returns -1
	virtual double getDescent () const = 0;		///< returns the descent line offset of the baseline of this font. If not supported returns -1
	virtual double getLeading () const = 0;		///< returns the space between lines for this font. If not supported returns -1
	virtual double getCapHeight () const = 0;	///< returns the height of the highest capital letter for this font. If not supported returns -1

	virtual const IFontPainter* getPainter () const = 0;
};

}

#endif // __iplatformfont__
