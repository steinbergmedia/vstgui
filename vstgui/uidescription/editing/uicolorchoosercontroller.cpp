// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicolorchoosercontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uicolorslider.h"
#include "../uiattributes.h"
#include "../iuidescription.h"
#include "../../lib/dragging.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIColorChooserDropTarget : public NonAtomicReferenceCounted, public DropTargetAdapter
{
public:
	UIColorChooserDropTarget (UIColor* color) : color (color) {}
	DragOperation onDragEnter (DragEventData eventData) override
	{
		IDataPackage::Type type;
		const void* item;
		if (eventData.drag->getData (0, item, type) > 0 && type == IDataPackage::kText)
		{
			if (CColor::isColorRepresentation (static_cast<const char*> (item)))
			{
				CColor dragColor;
				if (dragColor.fromString (static_cast<const char*> (item)) && *color != dragColor)
				{
					colorString = static_cast<const char*> (item);
					return DragOperation::Copy;
				}
			}
		}

		return DragOperation::None;
	}
	DragOperation onDragMove (DragEventData eventData) override
	{
		return colorString.empty () ? DragOperation::None : DragOperation::Copy;
	}
	void onDragLeave (DragEventData eventData) override { colorString.clear (); }
	bool onDrop (DragEventData eventData) override
	{
		if (!colorString.empty ())
		{
			CColor dragColor;
			if (dragColor.fromString (colorString.data ()))
			{
				color->beginEdit ();
				*color = dragColor;
				color->endEdit ();
				return true;
			}
		}
		return false;
	}

private:
	UIColor* color;
	std::string colorString;
};

//----------------------------------------------------------------------------------------------------
UIColorChooserController::UIColorChooserController (IController* baseController, UIColor* color)
: DelegationController (baseController)
, color (color)
{
	color->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UIColorChooserController::~UIColorChooserController ()
{
	color->unregisterListener (this);
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
	for (auto& control : controls)
		updateColorSlider (control);
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::uiColorChanged (UIColor* c)
{
	updateColorSliders ();
}

//----------------------------------------------------------------------------------------------------
bool UIColorChooserController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	auto* display = static_cast<CParamDisplay*>(userData);
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
	
	char* endptr = nullptr;
	result = (float)strtod (txt, &endptr);

	std::locale::global (origLocale);
	if (endptr != txt)
	{
		auto* display = static_cast<CParamDisplay*>(userData);
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
	auto* control = dynamic_cast<CControl*>(view);
	if (control && control->getTag () >= 0)
	{
		controls.emplace_back (control);
		auto* textEdit = dynamic_cast<CTextEdit*> (control);
		if (textEdit)
		{
			textEdit->setValueToStringFunction (valueToString);
			textEdit->setStringToValueFunction (stringToValue);
		}
		updateColorSlider (control);
	}
	else if (auto container = view->asViewContainer ())
	{
		container->setDropTarget (makeOwned<UIColorChooserDropTarget> (color));
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

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
