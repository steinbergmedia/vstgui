// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uifocussettingscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uidialogcontroller.h"
#include "uieditcontroller.h"
#include "uiactions.h"
#include "../uiattributes.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/controls/coptionmenu.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIFocusSettingsController::UIFocusSettingsController (UIDescription* description, IActionPerformer* actionPerformer)
: editDescription (description)
, actionPerformer (actionPerformer)
{
	originalSettings = editDescription->getFocusDrawingSettings ();
	for (auto& control : controls)
		control = nullptr;
}

//------------------------------------------------------------------------
void UIFocusSettingsController::onDialogButton1Clicked (UIDialogController*)
{
	FocusDrawingSettings fd;

	if (controls[kEnabledTag])
		fd.enabled =
		    (controls[kEnabledTag]->getValue () == controls[kEnabledTag]->getMax ()) ? true : false;
	if (controls[kColorTag])
	{
		COptionMenu* menu = dynamic_cast<COptionMenu*> (controls[kColorTag]);
		CMenuItem* item = menu->getCurrent ();
		if (item)
			fd.colorName = item->getTitle ();
	}
	if (controls[kWidthTag])
		fd.width = controls[kWidthTag]->getValue ();
	if (originalSettings != fd)
		actionPerformer->performChangeFocusDrawingSettings (fd);
}

//------------------------------------------------------------------------
void UIFocusSettingsController::onDialogButton2Clicked (UIDialogController*)
{
}

//------------------------------------------------------------------------
void UIFocusSettingsController::onDialogShow (UIDialogController*)
{
}

//----------------------------------------------------------------------------------------------------
CView* UIFocusSettingsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	auto* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kEnabledTag:
			{
				control->setValue (originalSettings.enabled ? control->getMax () : control->getMin ());
				controls[kEnabledTag] = control;
				break;
			}
			case kColorTag:
			{
				auto* menu = dynamic_cast<COptionMenu*>(control);
				if (menu)
				{
					controls[kColorTag] = control;
					std::list<const std::string*> names;
					editDescription->collectColorNames (names);
					names.sort (UIEditController::std__stringCompare);
					int32_t index = 0;
					for (auto& name : names)
					{
						menu->addEntry (new CMenuItem (name->c_str ()));
						if (originalSettings.colorName == *name)
						{
							menu->setValue ((float)index);
						}
						index++;
					}
				}
				break;
			}
			case kWidthTag:
			{
				controls[kWidthTag] = control;
				auto* edit = dynamic_cast<CTextEdit*>(control);
				if (edit)
				{
					edit->setStringToValueFunction (stringToValue);
					edit->setValueToStringFunction (valueToString);
				}
				control->setValue (static_cast<float> (originalSettings.width));
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
bool UIFocusSettingsController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << (value == intValue ? intValue : value);
	std::strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFocusSettingsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	if (txt)
	{
		float value = UTF8StringView (txt).toFloat ();
		result = value;
		return true;
	}
	return false;
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
