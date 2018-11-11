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
	virtual void fontChanged (CFontChooser* chooser, CFontRef newFont) = 0;
};

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
struct CFontChooserUIDefinition
{
	CFontRef font;
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
	
	CFontChooserUIDefinition (CFontRef font = kSystemFont,
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
				 int32_t rowHeight = -1,
				 CCoord scrollbarWidth = 16)
	: font (font), rowHeight (rowHeight), fontColor (fontColor), selectionColor (selectionColor), rowlineColor (rowlineColor)
	, rowBackColor (rowBackColor), rowAlternateBackColor (rowAlternateBackColor), previewTextColor (previewTextColor), previewBackgroundColor (previewBackgroundColor)
	, scrollbarScrollerColor (scrollbarScrollerColor), scrollbarFrameColor (scrollbarFrameColor)
	, scrollbarBackgroundColor (scrollbarBackgroundColor), scrollbarWidth (scrollbarWidth)
	{}
};

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CFontChooser : public CViewContainer, public IControlListener, public GenericStringListDataBrowserSourceSelectionChanged
{
public:
	CFontChooser (IFontChooserDelegate* delegate, CFontRef initialFont = nullptr, const CFontChooserUIDefinition& uiDef = CFontChooserUIDefinition ());
	~CFontChooser () noexcept override;

	void setFont (CFontRef font);
	
protected:
	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;
	void valueChanged (CControl* pControl) override;
	bool attached (CView* parent) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	IFontChooserDelegate* delegate;
	CDataBrowser* fontBrowser;
	CTextEdit* sizeEdit;
	CCheckBox* boldBox;
	CCheckBox* italicBox;
	CCheckBox* underlineBox;
	CCheckBox* strikeoutBox;
	CView* fontPreviewView;
	GenericStringListDataBrowserSource::StringVector fontNames;
	
	CFontRef selFont;
};

} // VSTGUI

