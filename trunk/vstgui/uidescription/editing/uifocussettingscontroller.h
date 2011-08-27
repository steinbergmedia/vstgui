#ifndef __uifocussettingscontroller__
#define __uifocussettingscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include <string>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIFocusSettingsController : public CBaseObject, public IController
{
public:
	UIFocusSettingsController (UIDescription* description);
	~UIFocusSettingsController ();

	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	void valueChanged (CControl* control);
protected:
	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

	SharedPointer<UIDescription> editDescription;
	UIAttributes* settings;

	enum {
		kEnabledTag = 0,
		kColorTag,
		kWidthTag,
		kNumTags
	};
	CControl* controls[kNumTags];
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uifocussettingscontroller__
