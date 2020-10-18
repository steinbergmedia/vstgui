// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "segmentbuttoncreator.h"

#include "../../lib/controls/csegmentbutton.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include <array>
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto SegmentButtonCreator::selectionModeStrings ()->SelectionModeStrings&
{
	static SelectionModeStrings strings = {"Single", "Single-Toggle", "Multiple"};
	return strings;
}

//------------------------------------------------------------------------
SegmentButtonCreator::SegmentButtonCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr SegmentButtonCreator::getViewName () const
{
	return kCSegmentButton;
}

//------------------------------------------------------------------------
IdStringPtr SegmentButtonCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr SegmentButtonCreator::getDisplayName () const
{
	return "Segment Button";
}

//------------------------------------------------------------------------
CView* SegmentButtonCreator::create (const UIAttributes& attributes,
                                     const IUIDescription* description) const
{
	CSegmentButton* button = new CSegmentButton (CRect (0, 0, 200, 20));
	updateSegmentCount (button, 4);
	return button;
}

//------------------------------------------------------------------------
void SegmentButtonCreator::updateSegmentCount (CSegmentButton* button, uint32_t numSegments) const
{
	if (button->getSegments ().size () != numSegments)
	{
		button->removeAllSegments ();
		for (uint32_t i = 0; i < numSegments; i++)
		{
			std::stringstream str;
			str << "Segment ";
			str << i + 1;
			CSegmentButton::Segment seg;
			seg.name = str.str ().c_str ();
			button->addSegment (std::move (seg));
		}
	}
}

//------------------------------------------------------------------------
void SegmentButtonCreator::updateSegments (CSegmentButton* button,
                                           const UIAttributes::StringArray& names) const
{
	button->removeAllSegments ();
	for (const auto& name : names)
	{
		CSegmentButton::Segment segment;
		segment.name = name.c_str ();
		button->addSegment (std::move (segment));
	}
}

//------------------------------------------------------------------------
bool SegmentButtonCreator::apply (CView* view, const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	auto* button = dynamic_cast<CSegmentButton*> (view);
	if (!button)
		return false;

	const auto* attr = attributes.getAttributeValue (kAttrFont);
	if (attr)
	{
		CFontRef font = description->getFont (attr->c_str ());
		if (font)
		{
			button->setFont (font);
		}
	}

	attr = attributes.getAttributeValue (kAttrStyle);
	if (attr)
	{
		if (*attr == strHorizontal)
			button->setStyle (CSegmentButton::Style::kHorizontal);
		else if (*attr == strVertical)
			button->setStyle (CSegmentButton::Style::kVertical);
		else if (*attr == strHorizontalInverse)
			button->setStyle (CSegmentButton::Style::kHorizontalInverse);
		else if (*attr == strVerticalInverse)
			button->setStyle (CSegmentButton::Style::kVerticalInverse);
	}

	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrTextColor), color, description))
		button->setTextColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrTextColorHighlighted), color,
	                   description))
		button->setTextColorHighlighted (color);
	if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
		button->setFrameColor (color);

	double d;
	if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
		button->setFrameWidth (d);
	if (attributes.getDoubleAttribute (kAttrRoundRadius, d))
		button->setRoundRadius (d);
	if (attributes.getDoubleAttribute (kAttrIconTextMargin, d))
		button->setTextMargin (d);

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

	UIAttributes::StringArray segmentNames;
	if (attributes.getStringArrayAttribute (kAttrSegmentNames, segmentNames))
		updateSegments (button, segmentNames);

	attr = attributes.getAttributeValue (kAttrTruncateMode);
	if (attr)
	{
		if (*attr == strHead)
			button->setTextTruncateMode (CDrawMethods::kTextTruncateHead);
		else if (*attr == strTail)
			button->setTextTruncateMode (CDrawMethods::kTextTruncateTail);
		else
			button->setTextTruncateMode (CDrawMethods::kTextTruncateNone);
	}
	attr = attributes.getAttributeValue (kAttrSelectionMode);
	if (attr)
	{
		using SelectionMode = CSegmentButton::SelectionMode;
		for (auto index = 0u; index <= static_cast<size_t> (SelectionMode::kMultiple); ++index)
		{
			if (*attr == selectionModeStrings ()[index])
			{
				button->setSelectionMode (static_cast<SelectionMode> (index));
				break;
			}
		}
	}
	return true;
}

//------------------------------------------------------------------------
bool SegmentButtonCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrStyle);
	attributeNames.emplace_back (kAttrSelectionMode);
	attributeNames.emplace_back (kAttrSegmentNames);
	attributeNames.emplace_back (kAttrFont);
	attributeNames.emplace_back (kAttrTextColor);
	attributeNames.emplace_back (kAttrTextColorHighlighted);
	attributeNames.emplace_back (kAttrGradient);
	attributeNames.emplace_back (kAttrGradientHighlighted);
	attributeNames.emplace_back (kAttrFrameColor);
	attributeNames.emplace_back (kAttrRoundRadius);
	attributeNames.emplace_back (kAttrFrameWidth);
	attributeNames.emplace_back (kAttrIconTextMargin);
	attributeNames.emplace_back (kAttrTextAlignment);
	attributeNames.emplace_back (kAttrTruncateMode);
	return true;
}

//------------------------------------------------------------------------
auto SegmentButtonCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrStyle)
		return kListType;
	if (attributeName == kAttrSelectionMode)
		return kListType;
	if (attributeName == kAttrSegmentNames)
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
	if (attributeName == kAttrFrameWidth)
		return kFloatType;
	if (attributeName == kAttrRoundRadius)
		return kFloatType;
	if (attributeName == kAttrIconTextMargin)
		return kFloatType;
	if (attributeName == kAttrTextAlignment)
		return kStringType;
	if (attributeName == kAttrTruncateMode)
		return kListType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool SegmentButtonCreator::getPossibleListValues (const string& attributeName,
                                                  ConstStringPtrList& values) const
{
	if (attributeName == kAttrStyle)
	{
		if (getStandardAttributeListValues (kAttrOrientation, values))
		{
			static std::string kHorizontalInverse = strHorizontalInverse;
			static std::string kVerticalInverse = strVerticalInverse;

			values.emplace_back (&kHorizontalInverse);
			values.emplace_back (&kVerticalInverse);
			return true;
		}
	}
	else if (attributeName == kAttrSelectionMode)
	{
		for (auto& str : selectionModeStrings ())
			values.emplace_back (&str);
		return true;
	}
	else if (attributeName == kAttrTruncateMode)
	{
		return getStandardAttributeListValues (kAttrTruncateMode, values);
	}
	return false;
}

//------------------------------------------------------------------------
bool SegmentButtonCreator::getAttributeValue (CView* view, const string& attributeName,
                                              string& stringValue, const IUIDescription* desc) const
{
	auto* button = dynamic_cast<CSegmentButton*> (view);
	if (!button)
		return false;
	if (attributeName == kAttrFont)
	{
		UTF8StringPtr fontName = desc->lookupFontName (button->getFont ());
		if (fontName)
		{
			stringValue = fontName;
			return true;
		}
		return false;
	}
	else if (attributeName == kAttrSegmentNames)
	{
		const CSegmentButton::Segments& segments = button->getSegments ();
		UIAttributes::StringArray stringArray;
		for (const auto& segment : segments)
			stringArray.emplace_back (segment.name.getString ());
		stringValue = UIAttributes::stringArrayToString (stringArray);
		return true;
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
	else if (attributeName == kAttrStyle)
	{
		switch (button->getStyle ())
		{
			case CSegmentButton::Style::kHorizontal:
			{
				stringValue = strHorizontal;
				return true;
			}
			case CSegmentButton::Style::kHorizontalInverse:
			{
				stringValue = strHorizontalInverse;
				return true;
			}
			case CSegmentButton::Style::kVertical:
			{
				stringValue = strVertical;
				return true;
			}
			case CSegmentButton::Style::kVerticalInverse:
			{
				stringValue = strVerticalInverse;
				return true;
			}
		}
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
		if (gradient)
		{
			UTF8StringPtr gradientName = desc->lookupGradientName (gradient);
			stringValue = gradientName ? gradientName : "";
		}
		return true;
	}
	else if (attributeName == kAttrGradientHighlighted)
	{
		CGradient* gradient = button->getGradientHighlighted ();
		if (gradient)
		{
			UTF8StringPtr gradientName = desc->lookupGradientName (gradient);
			stringValue = gradientName ? gradientName : "";
		}
		return true;
	}
	else if (attributeName == kAttrTruncateMode)
	{
		switch (button->getTextTruncateMode ())
		{
			case CDrawMethods::kTextTruncateHead: stringValue = strHead; break;
			case CDrawMethods::kTextTruncateTail: stringValue = strTail; break;
			case CDrawMethods::kTextTruncateNone: stringValue = ""; break;
		}
		return true;
	}
	else if (attributeName == kAttrSelectionMode)
	{
		stringValue = selectionModeStrings ()[static_cast<size_t> (button->getSelectionMode ())];
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
