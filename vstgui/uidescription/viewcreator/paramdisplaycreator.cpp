// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "paramdisplaycreator.h"

#include "../../lib/controls/cparamdisplay.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
ParamDisplayCreator::ParamDisplayCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr ParamDisplayCreator::getViewName () const
{
	return kCParamDisplay;
}

//------------------------------------------------------------------------
IdStringPtr ParamDisplayCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr ParamDisplayCreator::getDisplayName () const
{
	return "Parameter Display";
}

//------------------------------------------------------------------------
CView* ParamDisplayCreator::create (const UIAttributes& attributes,
                                    const IUIDescription* description) const
{
	return new CParamDisplay (CRect (0, 0, 100, 20));
}

//------------------------------------------------------------------------
bool ParamDisplayCreator::apply (CView* view, const UIAttributes& attributes,
                                 const IUIDescription* description) const
{
	auto* display = dynamic_cast<CParamDisplay*> (view);
	if (!display)
		return false;

	const auto* fontAttr = attributes.getAttributeValue (kAttrFont);
	if (fontAttr)
	{
		CFontRef font = description->getFont (fontAttr->c_str ());
		if (font)
		{
			display->setFont (font);
		}
	}

	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrFontColor), color, description))
		display->setFontColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrBackColor), color, description))
		display->setBackColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
		display->setFrameColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrShadowColor), color, description))
		display->setShadowColor (color);

	CPoint p;
	if (attributes.getPointAttribute (kAttrTextInset, p))
		display->setTextInset (p);
	if (attributes.getPointAttribute (kAttrTextShadowOffset, p))
		display->setShadowTextOffset (p);
	if (attributes.getPointAttribute (kAttrBackgroundOffset, p))
		display->setBackOffset (p);
	bool b;
	if (attributes.getBooleanAttribute (kAttrFontAntialias, b))
		display->setAntialias (b);

	const auto* textAlignmentAttr = attributes.getAttributeValue (kAttrTextAlignment);
	if (textAlignmentAttr)
	{
		CHoriTxtAlign align = kCenterText;
		if (*textAlignmentAttr == strLeft)
			align = kLeftText;
		else if (*textAlignmentAttr == strRight)
			align = kRightText;
		display->setHoriAlign (align);
	}
	double d;
	if (attributes.getDoubleAttribute (kAttrRoundRectRadius, d))
		display->setRoundRectRadius (d);
	if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
		display->setFrameWidth (d);
	if (attributes.getDoubleAttribute (kAttrTextRotation, d))
		display->setTextRotation (d);

	int32_t style = display->getStyle ();
	applyStyleMask (attributes.getAttributeValue (kAttrStyle3DIn), CParamDisplay::k3DIn, style);
	applyStyleMask (attributes.getAttributeValue (kAttrStyle3DOut), CParamDisplay::k3DOut, style);
	applyStyleMask (attributes.getAttributeValue (kAttrStyleNoFrame), CParamDisplay::kNoFrame,
	                style);
	applyStyleMask (attributes.getAttributeValue (kAttrStyleNoDraw), CParamDisplay::kNoDrawStyle,
	                style);
	applyStyleMask (attributes.getAttributeValue (kAttrStyleNoText), CParamDisplay::kNoTextStyle,
	                style);
	applyStyleMask (attributes.getAttributeValue (kAttrStyleShadowText), CParamDisplay::kShadowText,
	                style);
	applyStyleMask (attributes.getAttributeValue (kAttrStyleRoundRect),
	                CParamDisplay::kRoundRectStyle, style);
	display->setStyle (style);

	const auto* precisionAttr = attributes.getAttributeValue (kAttrValuePrecision);
	if (precisionAttr)
	{
		uint8_t precision = (uint8_t)strtol (precisionAttr->c_str (), nullptr, 10);
		display->setPrecision (precision);
	}

	return true;
}

//------------------------------------------------------------------------
bool ParamDisplayCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrFont);
	attributeNames.emplace_back (kAttrFontColor);
	attributeNames.emplace_back (kAttrBackColor);
	attributeNames.emplace_back (kAttrFrameColor);
	attributeNames.emplace_back (kAttrShadowColor);
	attributeNames.emplace_back (kAttrRoundRectRadius);
	attributeNames.emplace_back (kAttrFrameWidth);
	attributeNames.emplace_back (kAttrTextAlignment);
	attributeNames.emplace_back (kAttrTextInset);
	attributeNames.emplace_back (kAttrTextShadowOffset);
	attributeNames.emplace_back (kAttrValuePrecision);
	attributeNames.emplace_back (kAttrBackgroundOffset);
	attributeNames.emplace_back (kAttrFontAntialias);
	attributeNames.emplace_back (kAttrStyle3DIn);
	attributeNames.emplace_back (kAttrStyle3DOut);
	attributeNames.emplace_back (kAttrStyleNoFrame);
	attributeNames.emplace_back (kAttrStyleNoText);
	attributeNames.emplace_back (kAttrStyleNoDraw);
	attributeNames.emplace_back (kAttrStyleShadowText);
	attributeNames.emplace_back (kAttrStyleRoundRect);
	attributeNames.emplace_back (kAttrTextRotation);
	return true;
}

//------------------------------------------------------------------------
auto ParamDisplayCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrFont)
		return kFontType;
	else if (attributeName == kAttrFontColor)
		return kColorType;
	else if (attributeName == kAttrBackColor)
		return kColorType;
	else if (attributeName == kAttrFrameColor)
		return kColorType;
	else if (attributeName == kAttrShadowColor)
		return kColorType;
	else if (attributeName == kAttrFontAntialias)
		return kBooleanType;
	else if (attributeName == kAttrStyle3DIn)
		return kBooleanType;
	else if (attributeName == kAttrStyle3DOut)
		return kBooleanType;
	else if (attributeName == kAttrStyleNoFrame)
		return kBooleanType;
	else if (attributeName == kAttrStyleNoText)
		return kBooleanType;
	else if (attributeName == kAttrStyleNoDraw)
		return kBooleanType;
	else if (attributeName == kAttrStyleShadowText)
		return kBooleanType;
	else if (attributeName == kAttrStyleRoundRect)
		return kBooleanType;
	else if (attributeName == kAttrRoundRectRadius)
		return kFloatType;
	else if (attributeName == kAttrFrameWidth)
		return kFloatType;
	else if (attributeName == kAttrTextAlignment)
		return kStringType;
	else if (attributeName == kAttrTextInset)
		return kPointType;
	else if (attributeName == kAttrTextShadowOffset)
		return kPointType;
	else if (attributeName == kAttrValuePrecision)
		return kIntegerType;
	else if (attributeName == kAttrTextRotation)
		return kFloatType;
	else if (attributeName == kAttrBackgroundOffset)
		return kPointType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool ParamDisplayCreator::getAttributeValue (CView* view, const string& attributeName,
                                             string& stringValue, const IUIDescription* desc) const
{
	auto* pd = dynamic_cast<CParamDisplay*> (view);
	if (pd == nullptr)
		return false;
	if (attributeName == kAttrFont)
	{
		UTF8StringPtr fontName = desc->lookupFontName (pd->getFont ());
		if (fontName)
		{
			stringValue = fontName;
			return true;
		}
		return false;
	}
	else if (attributeName == kAttrFontColor)
	{
		colorToString (pd->getFontColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrBackColor)
	{
		colorToString (pd->getBackColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrFrameColor)
	{
		colorToString (pd->getFrameColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrShadowColor)
	{
		colorToString (pd->getShadowColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrTextInset)
	{
		stringValue = UIAttributes::pointToString (pd->getTextInset ());
		return true;
	}
	else if (attributeName == kAttrTextShadowOffset)
	{
		stringValue = UIAttributes::pointToString (pd->getShadowTextOffset ());
		return true;
	}
	else if (attributeName == kAttrFontAntialias)
	{
		stringValue = pd->getAntialias () ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyle3DIn)
	{
		stringValue = pd->getStyle () & CParamDisplay::k3DIn ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyle3DOut)
	{
		stringValue = pd->getStyle () & CParamDisplay::k3DOut ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyleNoFrame)
	{
		stringValue = pd->getStyle () & CParamDisplay::kNoFrame ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyleNoText)
	{
		stringValue = pd->getStyle () & CParamDisplay::kNoTextStyle ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyleNoDraw)
	{
		stringValue = pd->getStyle () & CParamDisplay::kNoDrawStyle ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyleShadowText)
	{
		stringValue = pd->getStyle () & CParamDisplay::kShadowText ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrStyleRoundRect)
	{
		stringValue = pd->getStyle () & CParamDisplay::kRoundRectStyle ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrRoundRectRadius)
	{
		stringValue = UIAttributes::doubleToString (pd->getRoundRectRadius ());
		return true;
	}
	else if (attributeName == kAttrFrameWidth)
	{
		stringValue = UIAttributes::doubleToString (pd->getFrameWidth ());
		return true;
	}
	else if (attributeName == kAttrTextAlignment)
	{
		CHoriTxtAlign align = pd->getHoriAlign ();
		switch (align)
		{
			case kLeftText: stringValue = strLeft; break;
			case kRightText: stringValue = strRight; break;
			case kCenterText: stringValue = strCenter; break;
		}
		return true;
	}
	else if (attributeName == kAttrValuePrecision)
	{
		stringValue = UIAttributes::integerToString (static_cast<int32_t> (pd->getPrecision ()));
		return true;
	}
	else if (attributeName == kAttrTextRotation)
	{
		stringValue = UIAttributes::doubleToString (pd->getTextRotation ());
		return true;
	}
	else if (attributeName == kAttrBackgroundOffset)
	{
		stringValue = UIAttributes::pointToString (pd->getBackOffset ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool ParamDisplayCreator::getAttributeValueRange (const string& attributeName, double& minValue,
                                                  double& maxValue) const
{
	if (attributeName == kAttrTextRotation)
	{
		minValue = 0.;
		maxValue = 360.;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
