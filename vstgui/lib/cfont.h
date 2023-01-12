// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cstring.h"
#include <string>
#include <list>

namespace VSTGUI {

//-----------
// @brief Text Face
//-----------
enum CTxtFace
{
	kNormalFace    = 0,
	kBoldFace      = 1 << 1,
	kItalicFace    = 1 << 2,
	kUnderlineFace = 1 << 3,
	kStrikethroughFace = 1 << 4
};

//-----------------------------------------------------------------------------
// CFontDesc Declaration
//! @brief font class
//-----------------------------------------------------------------------------
class CFontDesc : public AtomicReferenceCounted
{
public:
	CFontDesc (const UTF8String& name = "", const CCoord& size = 0, const int32_t style = 0);
	CFontDesc (const CFontDesc& font);
	~CFontDesc () noexcept override;

	//-----------------------------------------------------------------------------
	/// @name Size, Name and Style Methods
	//-----------------------------------------------------------------------------
	//@{
	/** get the name of the font */
	const UTF8String& getName () const { return name; }
	/** get the height of the font */
	const CCoord& getSize () const { return size; }
	/** get the style of the font */
	const int32_t& getStyle () const { return style; }

	/** set the name of the font */
	virtual void setName (const UTF8String& newName);
	/** set the height of the font */
	virtual void setSize (CCoord newSize);
	/** set the style of the font @sa CTxtFace */
	virtual void setStyle (int32_t newStyle);
	//@}

	virtual const PlatformFontPtr getPlatformFont () const;
	virtual const IFontPainter* getFontPainter () const;

	virtual CFontDesc& operator= (const CFontDesc&);
	virtual bool operator== (const CFontDesc&) const;
	virtual bool operator!= (const CFontDesc& other) const { return !(*this == other);}

	static void init ();
	static void cleanup ();

protected:
	void beforeDelete () override;
	virtual void freePlatformFont ();

	UTF8String name;
	CCoord size;
	int32_t style;
	mutable PlatformFontPtr platformFont;
};

//-----------------------------------------------------------------------------
// Global fonts
//-----------------------------------------------------------------------------
extern CFontRef kSystemFont;
extern CFontRef kNormalFontVeryBig;
extern CFontRef kNormalFontBig;
extern CFontRef kNormalFont;
extern CFontRef kNormalFontSmall;
extern CFontRef kNormalFontSmaller;
extern CFontRef kNormalFontVerySmall;
extern CFontRef kSymbolFont;

} // VSTGUI
