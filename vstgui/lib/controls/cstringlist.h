// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "clistcontrol.h"
#include "../cdrawdefs.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** A specialized list control drawer to draw strings
 *
 *	You set an instance of this class as the drawer in a CListControl instance and it draws the
 *	strings you setup via the provider function.
 *
 *	@ingroup new_in_4_9
 */
//------------------------------------------------------------------------
class StringListControlDrawer : public IListControlDrawer, public NonAtomicReferenceCounted
{
public:
	using Func = std::function<PlatformStringPtr (int32_t row)>;

	StringListControlDrawer ();
	~StringListControlDrawer () noexcept override;

	void setStringProvider (Func&& getStringFunc);
	void setStringProvider (const Func& getStringFunc);

	void setFont (CFontRef f);
	void setFontColor (CColor color);
	void setSelectedFontColor (CColor color);
	void setBackColor (CColor color);
	void setSelectedBackColor (CColor color);
	void setHoverColor (CColor color);
	void setLineColor (CColor color);
	void setLineWidth (CCoord width);
	void setTextInset (CCoord inset);
	void setTextAlign (CHoriTxtAlign align);

	CFontRef getFont () const;
	CColor getFontColor () const;
	CColor getSelectedFontColor () const;
	CColor getBackColor () const;
	CColor getSelectedBackColor () const;
	CColor getHoverColor () const;
	CColor getLineColor () const;
	CCoord getLineWidth () const;
	CCoord getTextInset () const;
	CHoriTxtAlign getTextAlign () const;

	void drawBackground (CDrawContext* context, CRect size) override;
	void drawRow (CDrawContext* context, CRect size, Row row) override;

private:
	PlatformStringPtr getString (int32_t row) const;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
