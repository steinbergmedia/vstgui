// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "textbuttoncreator.h"

#include "../../lib/controls/cbuttons.h"
#include "../../lib/algorithm.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto TextButtonCreator::getPositionStrings () -> PositionStringArray&
{
	static PositionStringArray positionsStrings = {
	    {strLeft, "center above text", "center below text", strRight}};
	static_assert (positionsStrings.size () == CDrawMethods::kIconRight + 1, "Update needed!");
	return positionsStrings;
}

//------------------------------------------------------------------------
TextButtonCreator::TextButtonCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr TextButtonCreator::getViewName () const
{
	return kCTextButton;
}

//------------------------------------------------------------------------
IdStringPtr TextButtonCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr TextButtonCreator::getDisplayName () const
{
	return "Text Button";
}

//------------------------------------------------------------------------
CView* TextButtonCreator::create (const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	CTextButton* button = new CTextButton (CRect (0, 0, 100, 20), nullptr, -1, "");
	if (!description->lookupGradientName (button->getGradient ()))
		addGradientToUIDescription (description, button->getGradient (),
		                            "Default TextButton Gradient");
	if (!description->lookupGradientName (button->getGradientHighlighted ()))
		addGradientToUIDescription (description, button->getGradientHighlighted (),
		                            "Default TextButton Gradient Highlighted");
	return button;
}

//------------------------------------------------------------------------
bool TextButtonCreator::apply (CView* view, const UIAttributes& attributes,
                               const IUIDescription* description) const
{
	auto* button = dynamic_cast<CTextButton*> (view);
	if (!button)
		return false;

	const auto* attr = attributes.getAttributeValue (kAttrTitle);
	if (attr)
		button->setTitle (attr->c_str ());

	attr = attributes.getAttributeValue (kAttrFont);
	if (attr)
	{
		CFontRef font = description->getFont (attr->c_str ());
		if (font)
		{
			button->setFont (font);
		}
	}

	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrTextColor), color, description))
		button->setTextColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrTextColorHighlighted), color,
	                   description))
		button->setTextColorHighlighted (color);
	if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
		button->setFrameColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrFrameColorHighlighted), color,
	                   description))
		button->setFrameColorHighlighted (color);

	double d;
	if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
		button->setFrameWidth (d);
	if (attributes.getDoubleAttribute (kAttrRoundRadius, d))
		button->setRoundRadius (d);
	if (attributes.getDoubleAttribute (kAttrIconTextMargin, d))
		button->setTextMargin (d);

	attr = attributes.getAttributeValue (kAttrKickStyle);
	if (attr)
	{
		button->setStyle (*attr == strTrue ? CTextButton::kKickStyle : CTextButton::kOnOffStyle);
	}

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrIcon), bitmap, description))
		button->setIcon (bitmap);
	if (stringToBitmap (attributes.getAttributeValue (kAttrIconHighlighted), bitmap, description))
		button->setIconHighlighted (bitmap);

	attr = attributes.getAttributeValue (kAttrIconPosition);
	if (attr)
	{
		if (auto index =
		        indexOf (getPositionStrings ().begin (), getPositionStrings ().end (), *attr))
			button->setIconPosition (static_cast<CDrawMethods::IconPosition> (*index));
	}
	attr = attributes.getAttributeValue (kAttrTextAlignment);
	if (attr)
	{
		CHoriTxtAlign align = kCenterText;
		if (*attr == strLeft)
			align = kLeftText;
		else if (*attr == strRight)
			align = kRightText;
		button->setTextAlignment (align);
	}
	const auto* gradientName = attributes.getAttributeValue (kAttrGradient);
	if (gradientName)
		button->setGradient (description->getGradient (gradientName->c_str ()));
	const auto* gradientHighlightedName = attributes.getAttributeValue (kAttrGradientHighlighted);
	if (gradientHighlightedName)
		button->setGradientHighlighted (
		    description->getGradient (gradientHighlightedName->c_str ()));

	if (gradientName == nullptr && gradientHighlightedName == nullptr)
	{
		bool hasOldGradient = true;
		CColor startColor, highlightedStartColor, endColor, highlightedEndColor;
		if (!stringToColor (attributes.getAttributeValue (kAttrGradientStartColor), startColor,
		                    description))
			hasOldGradient = false;
		if (hasOldGradient &&
		    !stringToColor (attributes.getAttributeValue (kAttrGradientStartColorHighlighted),
		                    highlightedStartColor, description))
			hasOldGradient = false;
		if (hasOldGradient && !stringToColor (attributes.getAttributeValue (kAttrGradientEndColor),
		                                      endColor, description))
			hasOldGradient = false;
		if (hasOldGradient &&
		    !stringToColor (attributes.getAttributeValue (kAttrGradientEndColorHighlighted),
		                    highlightedEndColor, description))
			hasOldGradient = false;
		if (hasOldGradient)
		{
			SharedPointer<CGradient> gradient =
			    owned (CGradient::create (0, 1, startColor, endColor));
			button->setGradient (gradient);
			addGradientToUIDescription (description, gradient, "TextButton");
			gradient = owned (CGradient::create (0, 1, highlightedStartColor, highlightedEndColor));
			button->setGradientHighlighted (gradient);
			addGradientToUIDescription (description, gradient, "TextButton Highlighted");
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool TextButtonCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrKickStyle);
	attributeNames.emplace_back (kAttrTitle);
	attributeNames.emplace_back (kAttrFont);
	attributeNames.emplace_back (kAttrTextColor);
	attributeNames.emplace_back (kAttrTextColorHighlighted);
	attributeNames.emplace_back (kAttrGradient);
	attributeNames.emplace_back (kAttrGradientHighlighted);
	attributeNames.emplace_back (kAttrFrameColor);
	attributeNames.emplace_back (kAttrFrameColorHighlighted);
	attributeNames.emplace_back (kAttrRoundRadius);
	attributeNames.emplace_back (kAttrFrameWidth);
	attributeNames.emplace_back (kAttrIconTextMargin);
	attributeNames.emplace_back (kAttrTextAlignment);
	attributeNames.emplace_back (kAttrIcon);
	attributeNames.emplace_back (kAttrIconHighlighted);
	attributeNames.emplace_back (kAttrIconPosition);
	return true;
}

//------------------------------------------------------------------------
auto TextButtonCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrTitle)
		return kStringType;
	if (attributeName == kAttrFont)
		return kFontType;
	if (attributeName == kAttrTextColor)
		return kColorType;
	if (attributeName == kAttrTextColorHighlighted)
		return kColorType;
	if (attributeName == kAttrGradient)
		return kGradientType;
	if (attributeName == kAttrGradientHighlighted)
		return kGradientType;
	if (attributeName == kAttrFrameColor)
		return kColorType;
	if (attributeName == kAttrFrameColorHighlighted)
		return kColorType;
	if (attributeName == kAttrFrameWidth)
		return kFloatType;
	if (attributeName == kAttrRoundRadius)
		return kFloatType;
	if (attributeName == kAttrKickStyle)
		return kBooleanType;
	if (attributeName == kAttrIcon)
		return kBitmapType;
	if (attributeName == kAttrIconHighlighted)
		return kBitmapType;
	if (attributeName == kAttrIconPosition)
		return kListType;
	if (attributeName == kAttrIconTextMargin)
		return kFloatType;
	if (attributeName == kAttrTextAlignment)
		return kStringType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool TextButtonCreator::getPossibleListValues (const string& attributeName,
                                               ConstStringPtrList& values) const
{
	if (attributeName == kAttrIconPosition)
	{
		for (const auto& s : getPositionStrings ())
			values.emplace_back (&s);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextButtonCreator::getAttributeValue (CView* view, const string& attributeName,
                                           string& stringValue, const IUIDescription* desc) const
{
	auto* button = dynamic_cast<CTextButton*> (view);
	if (!button)
		return false;
	if (attributeName == kAttrTitle)
	{
		stringValue = button->getTitle ().getString ();
		return true;
	}
	else if (attributeName == kAttrFont)
	{
		UTF8StringPtr fontName = desc->lookupFontName (button->getFont ());
		if (fontName)
		{
			stringValue = fontName;
			return true;
		}
		return false;
	}
	else if (attributeName == kAttrTextColor)
	{
		colorToString (button->getTextColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrTextColorHighlighted)
	{
		colorToString (button->getTextColorHighlighted (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrFrameColor)
	{
		colorToString (button->getFrameColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrFrameColorHighlighted)
	{
		colorToString (button->getFrameColorHighlighted (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrFrameWidth)
	{
		stringValue = UIAttributes::doubleToString (button->getFrameWidth ());
		return true;
	}
	else if (attributeName == kAttrRoundRadius)
	{
		stringValue = UIAttributes::doubleToString (button->getRoundRadius ());
		return true;
	}
	else if (attributeName == kAttrKickStyle)
	{
		stringValue = button->getStyle () == CTextButton::kKickStyle ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrIcon)
	{
		CBitmap* bitmap = button->getIcon ();
		if (bitmap)
		{
			return bitmapToString (bitmap, stringValue, desc);
		}
	}
	else if (attributeName == kAttrIconHighlighted)
	{
		CBitmap* bitmap = button->getIconHighlighted ();
		if (bitmap)
		{
			return bitmapToString (bitmap, stringValue, desc);
		}
	}
	else if (attributeName == kAttrIconPosition)
	{
		auto pos = button->getIconPosition ();
		vstgui_assert (pos < getPositionStrings ().size ());
		stringValue = getPositionStrings ()[pos];
		return true;
	}
	else if (attributeName == kAttrIconTextMargin)
	{
		stringValue = UIAttributes::doubleToString (button->getTextMargin ());
		return true;
	}
	else if (attributeName == kAttrTextAlignment)
	{
		CHoriTxtAlign align = button->getTextAlignment ();
		switch (align)
		{
			case kLeftText: stringValue = strLeft; break;
			case kRightText: stringValue = strRight; break;
			case kCenterText: stringValue = strCenter; break;
		}
		return true;
	}
	else if (attributeName == kAttrGradient)
	{
		CGradient* gradient = button->getGradient ();
		UTF8StringPtr gradientName = gradient ? desc->lookupGradientName (gradient) : nullptr;
		stringValue = gradientName ? gradientName : "";
		return true;
	}
	else if (attributeName == kAttrGradientHighlighted)
	{
		CGradient* gradient = button->getGradientHighlighted ();
		UTF8StringPtr gradientName = gradient ? desc->lookupGradientName (gradient) : nullptr;
		stringValue = gradientName ? gradientName : "";
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
