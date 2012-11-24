#ifndef __uitemplatesettingscontroller__
#define __uitemplatesettingscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include <string>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UITemplateSettingsController : public CBaseObject, public IController
{
public:
	UITemplateSettingsController (const std::string& templateName, UIDescription* description);
	~UITemplateSettingsController ();

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* control) VSTGUI_OVERRIDE_VMETHOD;
protected:
	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

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
