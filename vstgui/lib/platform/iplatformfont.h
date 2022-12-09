// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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

	virtual void drawString (const PlatformGraphicsDeviceContextPtr& context,
							 IPlatformString* string, const CPoint& p, const CColor& color,
							 bool antialias = true) const = 0;
	virtual CCoord getStringWidth (const PlatformGraphicsDeviceContextPtr& context,
								   IPlatformString* string, bool antialias = true) const = 0;
};

//-----------------------------------------------------------------------------
// IPlatformFont declaration
//! @brief platform font class
///
/// Encapsulation of a platform font.
//-----------------------------------------------------------------------------
class IPlatformFont : public AtomicReferenceCounted
{
public:
	/** returns the ascent line offset of the baseline of this font. If not supported returns -1 */
	virtual double getAscent () const = 0;
	/** returns the descent line offset of the baseline of this font. If not supported returns -1 */
	virtual double getDescent () const = 0;
	/** returns the space between lines for this font. If not supported returns -1 */
	virtual double getLeading () const = 0;
	/** returns the height of the highest capital letter for this font. If not supported returns -1
	 */
	virtual double getCapHeight () const = 0;

	virtual const IFontPainter* getPainter () const = 0;
};

} // VSTGUI
