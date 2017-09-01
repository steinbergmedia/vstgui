// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uigridcontroller.h"

#if VSTGUI_LIVE_EDITING

#include "../uiattributes.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIGridController::UIGridController (IController* baseController, UIDescription* description)
: DelegationController (baseController)
, editDescription (description)
{
	UIAttributes* attributes = editDescription->getCustomAttributes ("UIGridController", true);
	if (attributes)
	{
		attributes->getPointAttribute ("Size", size);
	}
}

//----------------------------------------------------------------------------------------------------
UIGridController::~UIGridController ()
{
	UIAttributes* attributes = editDescription->getCustomAttributes ("UIGridController", true);
	if (attributes)
	{
		attributes->setPointAttribute ("Size", size);
	}
}

//----------------------------------------------------------------------------------------------------
void UIGridController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kGridXTag:
		{
			size.x = control->getValue ();
			break;
		}
		case kGridYTag:
		{
			size.y = control->getValue ();
			break;
		}
	}
	UIAttributes* attributes = editDescription->getCustomAttributes ("UIGridController", true);
	if (attributes)
	{
		attributes->setPointAttribute ("Size", size);
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIGridController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CTextEdit* control = dynamic_cast<CTextEdit*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kGridXTag:
			{
				gridControls[0] = control;
				gridControls[0]->setStringToValueFunction (stringToValue);
				gridControls[0]->setValueToStringFunction (valueToString);
				gridControls[0]->setValue ((float)size.x);
				break;
			}
			case kGridYTag:
			{
				gridControls[1] = control;
				gridControls[1]->setStringToValueFunction (stringToValue);
				gridControls[1]->setValueToStringFunction (valueToString);
				gridControls[1]->setValue ((float)size.y);
				break;
			}
		}
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIGridController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
bool UIGridController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	std::strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIGridController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, nullptr, 10) : 0;
	result = (float)value;
	return true;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
