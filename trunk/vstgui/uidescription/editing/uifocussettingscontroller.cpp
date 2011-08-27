#include "uifocussettingscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uidialogcontroller.h"
#include "uieditcontroller.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/controls/coptionmenu.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIFocusSettingsController::UIFocusSettingsController (UIDescription* description)
: editDescription (description)
{
	settings = description->getCustomAttributes ("FocusDrawing", true);
	for (int32_t i = 0; i < kNumTags; i++)
		controls[i] = 0;
}

//----------------------------------------------------------------------------------------------------
UIFocusSettingsController::~UIFocusSettingsController ()
{
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIFocusSettingsController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIDialogController::kMsgDialogButton1Clicked)
	{
		if (controls[kEnabledTag])
		{
			settings->setBooleanAttribute ("enabled", controls[kEnabledTag]->getValue () == controls[kEnabledTag]->getMax () ? true : false);
		}
		if (controls[kColorTag])
		{
			COptionMenu* menu = dynamic_cast<COptionMenu*>(controls[kColorTag]);
			CMenuItem* item = menu->getCurrent ();
			if (item)
				settings->setAttribute ("color", item->getTitle ());
		}
		if (controls[kWidthTag])
		{
			settings->setDoubleAttribute ("width", controls[kWidthTag]->getValue ());
		}
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
CView* UIFocusSettingsController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kEnabledTag:
			{
				bool value = false;
				settings->getBooleanAttribute ("enabled", value);
				control->setValue (value ? control->getMax () : control->getMin ());
				controls[kEnabledTag] = control;
				break;
			}
			case kColorTag:
			{
				COptionMenu* menu = dynamic_cast<COptionMenu*>(control);
				if (menu)
				{
					controls[kColorTag] = control;
					const std::string* current = settings->getAttributeValue ("color");
					std::list<const std::string*> names;
					editDescription->collectColorNames (names);
					names.sort (UIEditController::std__stringCompare);
					int32_t index = 0;
					for (std::list<const std::string*>::const_iterator it = names.begin (); it != names.end (); it++, index++)
					{
						menu->addEntry (new CMenuItem ((*it)->c_str ()));
						if (current && *current == *(*it))
						{
							menu->setValue ((float)index);
						}
					}
				}
				break;
			}
			case kWidthTag:
			{
				controls[kWidthTag] = control;
				CTextEdit* edit = dynamic_cast<CTextEdit*>(control);
				if (edit)
				{
					edit->setStringToValueProc (stringToValue);
					edit->setValueToStringProc (valueToString);
				}
				double current = 1.;
				settings->getDoubleAttribute ("width", current);
				control->setValue ((float)current);
				break;
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
void UIFocusSettingsController::valueChanged (CControl* control)
{
}

//----------------------------------------------------------------------------------------------------
bool UIFocusSettingsController::valueToString (float value, char utf8String[256], void* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << (value == intValue ? intValue : value);
	strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFocusSettingsController::stringToValue (UTF8StringPtr txt, float& result, void* userData)
{
	if (txt)
	{
		float value = (float)strtod (txt, 0);
		result = value;
		return true;
	}
	return false;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
