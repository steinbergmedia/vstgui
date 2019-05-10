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
UITemplateSettingsController::UITemplateSettingsController (const std::string& templateName, UIDescription* description, IActionPerformer* actionPerformer)
: description (description)
, templateName (templateName)
, newTemplateName (templateName)
, actionPerformer (actionPerformer)
{
	const UIAttributes* attr = description->getViewAttributes (templateName.data ());
	if (attr)
	{
		if (attr->getPointAttribute (kTemplateAttributeMinSize, minSize) == false)
		{
			minSize.x = -1.;
			minSize.y = -1.;
		}
		if (attr->getPointAttribute (kTemplateAttributeMaxSize, maxSize) == false)
		{
			maxSize.x = -1.;
			maxSize.y = -1.;
		}
	}
	originalMinSize = minSize;
	originalMaxSize = maxSize;
	for (auto& control : controls)
		control = nullptr;
}

//------------------------------------------------------------------------
void UITemplateSettingsController::onDialogButton1Clicked (UIDialogController*)
{
	actionPerformer->beginGroupAction ("Change Template Settings");
	if (templateName != newTemplateName)
	{
		actionPerformer->performTemplateNameChange (templateName.data (), newTemplateName.data ());
	}
	if (minSize != originalMinSize || maxSize != originalMaxSize)
	{
		actionPerformer->performTemplateMinMaxSizeChange (newTemplateName.data (), minSize,
		                                                  maxSize);
	}
	actionPerformer->finishGroupAction ();
}

//------------------------------------------------------------------------
void UITemplateSettingsController::onDialogButton2Clicked (UIDialogController*)
{
}

//------------------------------------------------------------------------
void UITemplateSettingsController::onDialogShow (UIDialogController*)
{
}

//----------------------------------------------------------------------------------------------------
CView* UITemplateSettingsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription*)
{
	CTextEdit* control = dynamic_cast<CTextEdit*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kNameTag:
			{
				controls[kNameTag] = control;
				control->setText (templateName.data ());
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
					edit->setText (newTemplateName.data ());
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
				const UIAttributes* attr = description->getViewAttributes (templateName.data ());
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
	std::strcpy (utf8String, str.str ().data ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UITemplateSettingsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, nullptr, 10) : 0;
	result = (float)value;
	return true;
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
