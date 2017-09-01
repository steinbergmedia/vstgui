// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uitemplatesettingscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uidialogcontroller.h"
#include "../uiattributes.h"
#include "../../lib/controls/ctextedit.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UITemplateSettingsController::UITemplateSettingsController (const std::string& templateName, UIDescription* description)
: description (description)
, templateName (templateName)
, newTemplateName (templateName)
{
	const UIAttributes* attr = description->getViewAttributes (templateName.c_str());
	if (attr)
	{
		if (attr->getPointAttribute ("minSize", minSize) == false)
		{
			minSize.x = -1.;
			minSize.y = -1.;
		}
		if (attr->getPointAttribute ("maxSize", maxSize) == false)
		{
			maxSize.x = -1.;
			maxSize.y = -1.;
		}
	}
	for (auto& control : controls)
		control = nullptr;
}

//----------------------------------------------------------------------------------------------------
CMessageResult UITemplateSettingsController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIDialogController::kMsgDialogButton1Clicked)
	{
		if (templateName != newTemplateName)
		{
			description->changeTemplateName (templateName.c_str (), newTemplateName.c_str ());
		}
		UIAttributes* attr = const_cast<UIAttributes*> (description->getViewAttributes (templateName.c_str()));
		if (attr)
		{
			if (minSize.x == -1. && minSize.y == -1.)
			{
				attr->removeAttribute ("minSize");
			}
			else
			{
				attr->setPointAttribute ("minSize", minSize);
			}
			if (maxSize.x == -1. && maxSize.y == -1.)
			{
				attr->removeAttribute ("maxSize");
			}
			else
			{
				attr->setPointAttribute ("maxSize", maxSize);
			}
		}
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
CView* UITemplateSettingsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CTextEdit* control = dynamic_cast<CTextEdit*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kNameTag:
			{
				controls[kNameTag] = control;
				control->setText (templateName.c_str ());
				break;
			}
			case kMinWidthTag:
			{
				controls[kMinWidthTag] = control;
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
				control->setValue ((float)minSize.x);
				break;
			}
			case kMinHeightTag:
			{
				controls[kMinHeightTag] = control;
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
				control->setValue ((float)minSize.y);
				break;
			}
			case kMaxWidthTag:
			{
				controls[kMaxWidthTag] = control;
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
				control->setValue ((float)maxSize.x);
				break;
			}
			case kMaxHeightTag:
			{
				controls[kMaxHeightTag] = control;
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
				control->setValue ((float)maxSize.y);
				break;
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
void UITemplateSettingsController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kNameTag:
		{
			CTextEdit* edit = dynamic_cast<CTextEdit*>(control);
			if (edit)
			{
				if (!edit->getText ().empty ())
					newTemplateName = edit->getText ();
				else
					edit->setText (newTemplateName.c_str ());
			}
			break;
		}
		case kMinWidthTag:
		{
			minSize.x = control->getValue ();
			break;
		}
		case kMinHeightTag:
		{
			minSize.y = control->getValue ();
			break;
		}
		case kMaxWidthTag:
		{
			maxSize.x = control->getValue ();
			break;
		}
		case kMaxHeightTag:
		{
			maxSize.y = control->getValue ();
			break;
		}
		case kMinUseCurrentTag:
		case kMaxUseCurrentTag:
		{
			if (control->getValue() == control->getMax())
			{
				const UIAttributes* attr = description->getViewAttributes (templateName.c_str());
				if (attr)
				{
					CPoint currentSize;
					if (attr->getPointAttribute ("size", currentSize))
					{
						if (control->getTag () == kMinUseCurrentTag)
						{
							minSize = currentSize;
							if (controls[kMinWidthTag])
							{
								controls[kMinWidthTag]->setValue ((float)minSize.x);
								controls[kMinWidthTag]->invalid ();
							}
							if (controls[kMinHeightTag])
							{
								controls[kMinHeightTag]->setValue ((float)minSize.y);
								controls[kMinHeightTag]->invalid ();
							}
						}
						else
						{
							maxSize = currentSize;
							if (controls[kMaxWidthTag])
							{
								controls[kMaxWidthTag]->setValue ((float)maxSize.x);
								controls[kMaxWidthTag]->invalid ();
							}
							if (controls[kMaxHeightTag])
							{
								controls[kMaxHeightTag]->setValue ((float)maxSize.y);
								controls[kMaxHeightTag]->invalid ();
							}
						}
					}
				}
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UITemplateSettingsController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	std::strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UITemplateSettingsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, nullptr, 10) : 0;
	result = (float)value;
	return true;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
