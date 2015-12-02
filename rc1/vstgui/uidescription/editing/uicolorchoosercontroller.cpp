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

#include "uicolorchoosercontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uicolor.h"
#include "uicolorslider.h"
#include "../uiattributes.h"
#include "../iuidescription.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIColorChooserController::UIColorChooserController (IController* baseController, UIColor* color)
: DelegationController (baseController)
, color (color)
{
	color->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIColorChooserController::~UIColorChooserController ()
{
	color->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::updateColorSlider (CControl* control)
{
	float value = 0.f;
	switch (control->getTag ())
	{
		case kHueTag:
		{
			value = (float)color->getHue ();
			break;
		}
		case kSaturationTag:
		{
			value = (float)color->getSaturation ();
			break;
		}
		case kLightnessTag:
		{
			value = (float)color->getLightness ();
			break;
		}
		case kRedTag:
		{
			value = (float)color->getRed ();
			break;
		}
		case kGreenTag:
		{
			value = (float)color->getGreen ();
			break;
		}
		case kBlueTag:
		{
			value = (float)color->getBlue ();
			break;
		}
		case kAlphaTag:
		{
			value = (float)color->getAlpha ();
			break;
		}
		default:
			return;
	}
	control->setValue (value);
	control->invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::updateColorSliders ()
{
	for (ControlList::const_iterator it = controls.begin (); it != controls.end (); it++)
	{
		updateColorSlider (*it);
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIColorChooserController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIColor::kMsgChanged || message == UIColor::kMsgEditChange)
	{
		updateColorSliders ();
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
bool UIColorChooserController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	CParamDisplay* display = static_cast<CParamDisplay*>(userData);
	std::stringstream str;
	switch (display->getTag ())
	{
		case kSaturationTag:
		case kLightnessTag:
		{
			str << (uint32_t)(value * 100);
			str << " %";
			break;
		}
		case kHueTag:
		{
			str << (uint32_t)value;
			str << kDegreeSymbol;
			break;
		}
		default:
		{
			str << (uint32_t)value;
			break;
		}
	}
	strncpy (utf8String, str.str ().c_str (), 255);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIColorChooserController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	std::locale origLocale;
	std::locale::global (std::locale::classic ());
	
	char* endptr = 0;
	result = (float)strtod (txt, &endptr);

	std::locale::global (origLocale);
	if (endptr != txt)
	{
		CParamDisplay* display = static_cast<CParamDisplay*>(userData);
		switch (display->getTag ())
		{
			case kSaturationTag:
			case kLightnessTag:
			{
				result /= 100.f;
				break;
			}
			default:
				break;
		}
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
CView* UIColorChooserController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "UIColorSlider")
		{
			const std::string* controlTagStr = attributes.getAttributeValue ("control-tag");
			int32_t tag = controlTagStr ? description->getTagForName (controlTagStr->c_str ()) : -1;
			if (tag != -1)
			{
				return new UIColorSlider (color, tag);
			}
		}
	}

	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIColorChooserController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*>(view);
	if (control && control->getTag () >= 0)
	{
		controls.push_back (control);
		CTextEdit* textEdit = dynamic_cast<CTextEdit*> (control);
		if (textEdit)
		{
		#if VSTGUI_HAS_FUNCTIONAL
			textEdit->setValueToStringFunction (valueToString);
			textEdit->setStringToValueFunction (stringToValue);
		#else
			textEdit->setValueToStringProc (valueToString, textEdit);
			textEdit->setStringToValueProc (stringToValue, textEdit);
		#endif
		}
		updateColorSlider (control);
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIColorChooserController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::controlBeginEdit (CControl* pControl)
{
	if (pControl->getTag () >= kHueTag && pControl->getTag () <= kAlphaTag)
	{
		color->beginEdit ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::controlEndEdit (CControl* pControl)
{
	if (pControl->getTag () >= kHueTag && pControl->getTag () <= kAlphaTag)
	{
		color->endEdit ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kHueTag:
		{
			color->setHue (pControl->getValue ());
			break;
		}
		case kSaturationTag:
		{
			color->setSaturation (pControl->getValue ());
			break;
		}
		case kLightnessTag:
		{
			color->setLightness (pControl->getValue ());
			break;
		}
		case kRedTag:
		{
			color->setRed (pControl->getValue ());
			break;
		}
		case kGreenTag:
		{
			color->setGreen (pControl->getValue ());
			break;
		}
		case kBlueTag:
		{
			color->setBlue (pControl->getValue ());
			break;
		}
		case kAlphaTag:
		{
			color->setAlpha (pControl->getValue ());
			break;
		}
	}
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
