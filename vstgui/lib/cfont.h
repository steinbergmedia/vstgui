// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cfont__
#define __cfont__

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
	using PlatformFontPtr = SharedPointer<IPlatformFont>;
	
	CFontDesc (const UTF8String& name = "", const CCoord& size = 0, const int32_t style = 0);
	CFontDesc (const CFontDesc& font);
	~CFontDesc () noexcept override = default;

	//-----------------------------------------------------------------------------
	/// @name Size, Name and Style Methods
	//-----------------------------------------------------------------------------
	//@{
	const UTF8String& getName () const { return name; }		///< get the name of the font
	const CCoord& getSize () const { return size; }			///< get the height of the font
	const int32_t& getStyle () const { return style; }		///< get the style of the font

	virtual void setName (const UTF8String& newName);		///< set the name of the font
	virtual void setSize (CCoord newSize);					///< set the height of the font
	virtual void setStyle (int32_t newStyle);				///< set the style of the font @sa CTxtFace
	//@}

	virtual const PlatformFontPtr getPlatformFont () const;
	virtual const IFontPainter* getFontPainter () const;

	virtual CFontDesc& operator= (const CFontDesc&);
	virtual bool operator== (const CFontDesc&) const;
	virtual bool operator!= (const CFontDesc& other) const { return !(*this == other);}
	
	static void cleanup ();

protected:
	void beforeDelete () override;
	virtual void freePlatformFont ();

	UTF8String name;
	CCoord size;
	int32_t style;
	mutable PlatformFontPtr platformFont;
};

using CFontRef = CFontDesc*;

//-----------------------------------------------------------------------------
// Global fonts
//-----------------------------------------------------------------------------
extern const CFontRef kSystemFont;
extern const CFontRef kNormalFontVeryBig;
extern const CFontRef kNormalFontBig;
extern const CFontRef kNormalFont;
extern const CFontRef kNormalFontSmall;
extern const CFontRef kNormalFontSmaller;
extern const CFontRef kNormalFontVerySmall;
extern const CFontRef kSymbolFont;

} // namespace

#endif
