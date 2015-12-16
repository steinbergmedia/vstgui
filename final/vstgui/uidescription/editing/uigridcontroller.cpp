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
			#if VSTGUI_HAS_FUNCTIONAL
				gridControls[0]->setStringToValueFunction (stringToValue);
				gridControls[0]->setValueToStringFunction (valueToString);
			#else
				gridControls[0]->setStringToValueProc (stringToValue);
				gridControls[0]->setValueToStringProc (valueToString);
			#endif
				gridControls[0]->setValue ((float)size.x);
				break;
			}
			case kGridYTag:
			{
				gridControls[1] = control;
			#if VSTGUI_HAS_FUNCTIONAL
				gridControls[1]->setStringToValueFunction (stringToValue);
				gridControls[1]->setValueToStringFunction (valueToString);
			#else
				gridControls[1]->setStringToValueProc (stringToValue);
				gridControls[1]->setValueToStringProc (valueToString);
			#endif
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
	int32_t value = txt ? (int32_t)strtol (txt, 0, 10) : 0;
	result = (float)value;
	return true;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
