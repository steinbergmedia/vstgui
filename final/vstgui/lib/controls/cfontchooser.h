//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#ifndef __cfontchooser__
#define __cfontchooser__

#include "../vstguifwd.h"
#include "../cviewcontainer.h"
#include "../cfont.h"
#include "../cdatabrowser.h"
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
class CFontChooser : public CViewContainer, public IControlListener, public IGenericStringListDataBrowserSourceSelectionChanged
{
public:
	CFontChooser (IFontChooserDelegate* delegate, CFontRef initialFont = 0, const CFontChooserUIDefinition& uiDef = CFontChooserUIDefinition ());
	~CFontChooser ();

	void setFont (CFontRef font);
	
protected:
	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	IFontChooserDelegate* delegate;
	CDataBrowser* fontBrowser;
	CTextEdit* sizeEdit;
	CCheckBox* boldBox;
	CCheckBox* italicBox;
	CCheckBox* underlineBox;
	CCheckBox* strikeoutBox;
	CView* fontPreviewView;
	std::vector<std::string> fontNames;
	
	CFontRef selFont;
};

} // namespace

#endif
