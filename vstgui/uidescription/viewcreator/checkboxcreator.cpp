// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "checkboxcreator.h"

#include "../../lib/controls/cbuttons.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CheckBoxCreator::CheckBoxCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CheckBoxCreator::getViewName () const
{
	return kCCheckBox;
}

//------------------------------------------------------------------------
IdStringPtr CheckBoxCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr CheckBoxCreator::getDisplayName () const
{
	return "Checkbox";
}

//------------------------------------------------------------------------
CView* CheckBoxCreator::create (const UIAttributes& attributes,
                                const IUIDescription* description) const
{
	return new CCheckBox (CRect (0, 0, 100, 20), nullptr, -1, "Title");
}

//------------------------------------------------------------------------
bool CheckBoxCreator::apply (CView* view, const UIAttributes& attributes,
                             const IUIDescription* description) const
{
	auto* checkbox = dynamic_cast<CCheckBox*> (view);
	if (!checkbox)
		return false;

	const auto* attr = attributes.getAttributeValue (kAttrTitle);
	if (attr)
		checkbox->setTitle (attr->c_str ());

	attr = attributes.getAttributeValue (kAttrFont);
	if (attr)
	{
		CFontRef font = description->getFont (attr->c_str ());
		if (font)
		{
			checkbox->setFont (font);
		}
	}

	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrFontColor), color, description))
		checkbox->setFontColor (color);

	if (stringToColor (attributes.getAttributeValue (kAttrBoxframeColor), color, description))
		checkbox->setBoxFrameColor (color);

	if (stringToColor (attributes.getAttributeValue (kAttrBoxfillColor), color, description))
		checkbox->setBoxFillColor (color);

	if (stringToColor (attributes.getAttributeValue (kAttrCheckmarkColor), color, description))
		checkbox->setCheckMarkColor (color);

	int32_t style = checkbox->getStyle ();
	applyStyleMask (attributes.getAttributeValue (kAttrDrawCrossbox), CCheckBox::kDrawCrossBox,
	                style);
	applyStyleMask (attributes.getAttributeValue (kAttrAutosizeToFit), CCheckBox::kAutoSizeToFit,
	                style);
	checkbox->setStyle (style);

	double dv;
	if (attributes.getDoubleAttribute (kAttrFrameWidth, dv))
		checkbox->setFrameWidth (dv);
	if (attributes.getDoubleAttribute (kAttrRoundRectRadius, dv))
		checkbox->setRoundRectRadius (dv);
	return true;
}

//------------------------------------------------------------------------
bool CheckBoxCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrTitle);
	attributeNames.emplace_back (kAttrFont);
	attributeNames.emplace_back (kAttrFontColor);
	attributeNames.emplace_back (kAttrBoxframeColor);
	attributeNames.emplace_back (kAttrBoxfillColor);
	attributeNames.emplace_back (kAttrCheckmarkColor);
	attributeNames.emplace_back (kAttrFrameWidth);
	attributeNames.emplace_back (kAttrRoundRectRadius);
	attributeNames.emplace_back (kAttrAutosizeToFit);
	attributeNames.emplace_back (kAttrDrawCrossbox);
	return true;
}

//------------------------------------------------------------------------
auto CheckBoxCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrTitle)
		return kStringType;
	else if (attributeName == kAttrFont)
		return kFontType;
	else if (attributeName == kAttrFontColor)
		return kColorType;
	else if (attributeName == kAttrBoxframeColor)
		return kColorType;
	else if (attributeName == kAttrBoxfillColor)
		return kColorType;
	else if (attributeName == kAttrCheckmarkColor)
		return kColorType;
	else if (attributeName == kAttrFrameWidth)
		return kFloatType;
	else if (attributeName == kAttrRoundRectRadius)
		return kFloatType;
	else if (attributeName == kAttrAutosizeToFit)
		return kBooleanType;
	else if (attributeName == kAttrDrawCrossbox)
		return kBooleanType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CheckBoxCreator::getAttributeValue (CView* view, const string& attributeName,
                                         string& stringValue, const IUIDescription* desc) const
{
	auto* checkbox = dynamic_cast<CCheckBox*> (view);
	if (!checkbox)
		return false;

	if (attributeName == kAttrTitle)
	{
		stringValue = checkbox->getTitle ().getString ();
		return true;
	}
	else if (attributeName == kAttrFont)
	{
		UTF8StringPtr fontName = desc->lookupFontName (checkbox->getFont ());
		if (fontName)
		{
			stringValue = fontName;
			return true;
		}
		return false;
	}
	else if (attributeName == kAttrFontColor)
	{
		colorToString (checkbox->getFontColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrBoxframeColor)
	{
		colorToString (checkbox->getBoxFrameColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrBoxfillColor)
	{
		colorToString (checkbox->getBoxFillColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrCheckmarkColor)
	{
		colorToString (checkbox->getCheckMarkColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrAutosizeToFit)
	{
		if (checkbox->getStyle () & CCheckBox::kAutoSizeToFit)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	else if (attributeName == kAttrDrawCrossbox)
	{
		if (checkbox->getStyle () & CCheckBox::kDrawCrossBox)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	else if (attributeName == kAttrFrameWidth)
	{
		stringValue = UIAttributes::doubleToString (checkbox->getFrameWidth ());
		return true;
	}
	else if (attributeName == kAttrRoundRectRadius)
	{
		stringValue = UIAttributes::doubleToString (checkbox->getRoundRectRadius ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
