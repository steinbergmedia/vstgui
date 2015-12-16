//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
	for (int32_t i = 0; i < kNumTags; i++)
		controls[i] = 0;
}

//----------------------------------------------------------------------------------------------------
UITemplateSettingsController::~UITemplateSettingsController ()
{
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
			#if VSTGUI_HAS_FUNCTIONAL
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
			#else
				control->setStringToValueProc (stringToValue);
				control->setValueToStringProc (valueToString);
			#endif
				control->setValue ((float)minSize.x);
				break;
			}
			case kMinHeightTag:
			{
				controls[kMinHeightTag] = control;
			#if VSTGUI_HAS_FUNCTIONAL
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
			#else
				control->setStringToValueProc (stringToValue);
				control->setValueToStringProc (valueToString);
			#endif
				control->setValue ((float)minSize.y);
				break;
			}
			case kMaxWidthTag:
			{
				controls[kMaxWidthTag] = control;
			#if VSTGUI_HAS_FUNCTIONAL
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
			#else
				control->setStringToValueProc (stringToValue);
				control->setValueToStringProc (valueToString);
			#endif
				control->setValue ((float)maxSize.x);
				break;
			}
			case kMaxHeightTag:
			{
				controls[kMaxHeightTag] = control;
			#if VSTGUI_HAS_FUNCTIONAL
				control->setStringToValueFunction (stringToValue);
				control->setValueToStringFunction (valueToString);
			#else
				control->setStringToValueProc (stringToValue);
				control->setValueToStringProc (valueToString);
			#endif
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
	int32_t value = txt ? (int32_t)strtol (txt, 0, 10) : 0;
	result = (float)value;
	return true;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
