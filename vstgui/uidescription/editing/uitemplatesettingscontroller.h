// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uitemplatesettingscontroller__
#define __uitemplatesettingscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../icontroller.h"
#include "../../lib/controls/ctextedit.h"
#include <string>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UITemplateSettingsController : public CBaseObject, public IController
{
public:
	UITemplateSettingsController (const std::string& templateName, UIDescription* description);
	~UITemplateSettingsController () override = default;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	void valueChanged (CControl* control) override;
protected:
	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);

	SharedPointer<UIDescription> description;
	std::string templateName;
	std::string newTemplateName;
	CPoint minSize;
	CPoint maxSize;

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
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uitemplatesettingscontroller__
