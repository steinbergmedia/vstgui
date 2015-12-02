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

#include "uifocussettingscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uidialogcontroller.h"
#include "uieditcontroller.h"
#include "../uiattributes.h"
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
CView* UIFocusSettingsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
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
				#if VSTGUI_HAS_FUNCTIONAL
					edit->setStringToValueFunction (stringToValue);
					edit->setValueToStringFunction (valueToString);
				#else
					edit->setStringToValueProc (stringToValue);
					edit->setValueToStringProc (valueToString);
				#endif
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

} // namespace

#endif // VSTGUI_LIVE_EDITING
