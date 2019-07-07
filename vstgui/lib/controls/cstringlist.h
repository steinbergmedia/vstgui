// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../ccolor.h"
#include "../cdrawcontext.h"
#include "clistcontrol.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class StringListControlDrawer : public IListControlDrawer, public NonAtomicReferenceCounted
{
public:
	using Func = std::function<SharedPointer<IPlatformString> (int32_t row)>;

	StringListControlDrawer () = default;

	void setGetStringFunc (Func&& getStringFunc);
	void setGetStringFunc (const Func& getStringFunc);

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

	void drawRow (CDrawContext* context, CRect size, int32_t row, int32_t flags) override;

private:
	Func func;

	SharedPointer<CFontDesc> font {kNormalFont};
	CColor fontColor {kBlackCColor};
	CColor fontColorSelected {kWhiteCColor};
	CColor backColor {kWhiteCColor};
	CColor backColorSelected {kBlueCColor};
	CColor hoverColor {MakeCColor (0, 0, 0, 100)};
	CColor lineColor {kBlackCColor};
	CCoord lineWidth {1.};
	CCoord textInset {5.};
	CHoriTxtAlign textAlign {kLeftText};
};

//------------------------------------------------------------------------
} // VSTGUI
