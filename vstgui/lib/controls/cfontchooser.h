// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include "../cviewcontainer.h"
#include "../cfont.h"
#include "../cdatabrowser.h"
#include "../genericstringlistdatabrowsersource.h"
#include "icontrollistener.h"

namespace VSTGUI {

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IFontChooserDelegate
{
public:
	virtual void fontChanged (CFontChooser& chooser, SharedPointer<CFontDesc> newFont) = 0;
};

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
struct CFontChooserUIDefinition
{
	SharedPointer<CFontDesc> font;
	int32_t rowHeight;
	CColor fontColor;
	CColor selectionColor;
	CColor rowlineColor;
	CColor rowBackColor;
	CColor rowAlternateBackColor;
	CColor previewTextColor;
	CColor previewBackgroundColor;
	CColor scrollbarScrollerColor;
	CColor scrollbarFrameColor;
	CColor scrollbarBackgroundColor;
	CCoord scrollbarWidth;

	CFontChooserUIDefinition (SharedPointer<CFontDesc> font = kSystemFont,
							  const CColor& fontColor = kWhiteCColor,
							  const CColor& selectionColor = kBlueCColor,
							  const CColor& rowlineColor = kGreyCColor,
							  const CColor& rowBackColor = kTransparentCColor,
							  const CColor& rowAlternateBackColor = kTransparentCColor,
							  const CColor& previewTextColor = kBlackCColor,
							  const CColor& previewBackgroundColor = kWhiteCColor,
							  const CColor& scrollbarScrollerColor = kBlueCColor,
							  const CColor& scrollbarFrameColor = kBlackCColor,
							  const CColor& scrollbarBackgroundColor = kGreyCColor,
							  int32_t rowHeight = -1, CCoord scrollbarWidth = 16)
	: font (font)
	, rowHeight (rowHeight)
	, fontColor (fontColor)
	, selectionColor (selectionColor)
	, rowlineColor (rowlineColor)
	, rowBackColor (rowBackColor)
	, rowAlternateBackColor (rowAlternateBackColor)
	, previewTextColor (previewTextColor)
	, previewBackgroundColor (previewBackgroundColor)
	, scrollbarScrollerColor (scrollbarScrollerColor)
	, scrollbarFrameColor (scrollbarFrameColor)
	, scrollbarBackgroundColor (scrollbarBackgroundColor)
	, scrollbarWidth (scrollbarWidth)
	{}
};

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CFontChooser : public CViewContainer, public IControlListener, public GenericStringListDataBrowserSourceSelectionChanged
{
public:
	CFontChooser (IFontChooserDelegate* delegate, const SharedPointer<CFontDesc>& initialFont = {},
				  const CFontChooserUIDefinition& uiDef = CFontChooserUIDefinition ());
	~CFontChooser () noexcept override;

	void setFont (const SharedPointer<CFontDesc>& font);

protected:
	void dbSelectionChanged (int32_t selectedRow,
							 GenericStringListDataBrowserSource& source) override;
	void valueChanged (CControl& pControl) override;
	bool attached (CViewContainer& parent) override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	IFontChooserDelegate* delegate;
	SharedPointer<CDataBrowser> fontBrowser;
	SharedPointer<CTextEdit> sizeEdit;
	SharedPointer<CCheckBox> boldBox;
	SharedPointer<CCheckBox> italicBox;
	SharedPointer<CCheckBox> underlineBox;
	SharedPointer<CCheckBox> strikeoutBox;
	SharedPointer<CView> fontPreviewView;
	SharedPointer<CFontDesc> selFont;
	GenericStringListDataBrowserSource::StringVector fontNames;
};

} // VSTGUI

