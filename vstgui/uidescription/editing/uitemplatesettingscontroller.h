// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "iaction.h"
#include "uidialogcontroller.h"
#include "../icontroller.h"
#include "../../lib/controls/ctextedit.h"
#include <string>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UITemplateSettingsController : public NonAtomicReferenceCounted,
                                     public IDialogController,
                                     public IController
{
public:
	UITemplateSettingsController (const std::string& templateName, UIDescription* description, IActionPerformer* actionPerformer);
	~UITemplateSettingsController () override = default;

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	void valueChanged (CControl* control) override;
	void onDialogButton1Clicked (UIDialogController*) override;
	void onDialogButton2Clicked (UIDialogController*) override;
	void onDialogShow (UIDialogController*) override;
protected:
	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);

	SharedPointer<UIDescription> description;
	std::string templateName;
	std::string newTemplateName;
	CPoint minSize;
	CPoint maxSize;
	CPoint originalMinSize;
	CPoint originalMaxSize;

	enum {
		kNameTag = 0,
		kMinWidthTag,
		kMinHeightTag,
		kMaxWidthTag,
		kMaxHeightTag,
		kMinUseCurrentTag,
		kMaxUseCurrentTag,
		kNumTags
	};
	CControl* controls[kNumTags];
	IActionPerformer* actionPerformer;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
