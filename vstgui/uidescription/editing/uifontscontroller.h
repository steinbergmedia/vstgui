// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../delegationcontroller.h"
#include "../../lib/cdatabrowser.h"
#include "../../lib/genericstringlistdatabrowsersource.h"
#include "../../lib/controls/ctextedit.h"

namespace VSTGUI {
class UIFontsDataSource;
//----------------------------------------------------------------------------------------------------
class UIFontsController : public NonAtomicReferenceCounted,
                          public DelegationController,
                          public GenericStringListDataBrowserSourceSelectionChanged
{
public:
	UIFontsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIFontsController () override;

protected:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl* pControl) override;

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;

	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UIFontsDataSource* dataSource;

	COptionMenu* fontMenu;
	CTextEdit* altTextEdit;
	CTextEdit* sizeTextEdit;
	CControl* boldControl;
	CControl* italicControl;
	CControl* strikethroughControl;
	CControl* underlineControl;
	
	std::string selectedFont;

	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
		kFontMainTag,
		kFontAltTag,
		kFontSizeTag,
		kFontStyleBoldTag,
		kFontStyleItalicTag,
		kFontStyleStrikethroughTag,
		kFontStyleUnderlineTag
	};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
