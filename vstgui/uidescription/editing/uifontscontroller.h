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
	UIFontsController (const SharedPointer<IController>& baseController,
					   const SharedPointer<UIDescription>& description,
					   WeakPointer<IActionPerformer> actionPerformer);
	~UIFontsController () override;

protected:
	SharedPointer<CView> createView (const UIAttributes& attributes,
									 const IUIDescription& description) override;
	SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
									 const UIAttributes& attributes,
									 const IUIDescription& description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl& pControl) override;

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;

	static bool valueToString (float value, char utf8String[256], CParamDisplay& userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit& userData);

	SharedPointer<UIDescription> editDescription;
	WeakPointer<IActionPerformer> actionPerformer;
	SharedPointer<UIFontsDataSource> dataSource;

	SharedPointer<COptionMenu> fontMenu;
	SharedPointer<CTextEdit> altTextEdit;
	SharedPointer<CTextEdit> sizeTextEdit;
	SharedPointer<CControl> boldControl;
	SharedPointer<CControl> italicControl;
	SharedPointer<CControl> strikethroughControl;
	SharedPointer<CControl> underlineControl;

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
