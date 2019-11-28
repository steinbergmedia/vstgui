// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "sliderviewcreator.h"

#include "../../lib/controls/cslider.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto SliderBaseCreator::modeStrings () -> ModeStrings&
{
	static ModeStrings strings = {"touch", "relative touch", "free click", "ramp", "use global"};
	return strings;
}

//------------------------------------------------------------------------
bool SliderBaseCreator::apply (CView* view, const UIAttributes& attributes,
                               const IUIDescription* description) const
{
	auto* slider = dynamic_cast<CSliderBase*> (view);
	if (!slider)
		return false;

	// support old attribute name and convert it
	const auto* freeClickAttr = attributes.getAttributeValue ("free-click");
	if (freeClickAttr)
	{
		slider->setSliderMode (*freeClickAttr == strTrue ? CSliderMode::FreeClick :
		                                                   CSliderMode::Touch);
	}

	const auto* modeAttr = attributes.getAttributeValue (kAttrMode);
	if (modeAttr)
	{
		for (auto index = 0u; index <= static_cast<size_t> (CSliderMode::UseGlobal); ++index)
		{
			if (*modeAttr == modeStrings ()[index])
			{
				slider->setSliderMode (static_cast<CSliderMode> (index));
				break;
			}
		}
	}

	CPoint p;
	if (attributes.getPointAttribute (kAttrHandleOffset, p))
		slider->setOffsetHandle (p);

	double d;
	if (attributes.getDoubleAttribute (kAttrZoomFactor, d))
		slider->setZoomFactor (static_cast<float> (d));

	const auto* orientationAttr = attributes.getAttributeValue (kAttrOrientation);
	if (orientationAttr)
	{
		int32_t style = slider->getStyle ();
		if (*orientationAttr == strVertical)
		{
			setBit (style, CSlider::kHorizontal, false);
			setBit (style, CSlider::kVertical, true);
		}
		else
		{
			setBit (style, CSlider::kVertical, false);
			setBit (style, CSlider::kHorizontal, true);
		}
		slider->setStyle (style);
	}
	const auto* reverseOrientationAttr = attributes.getAttributeValue (kAttrReverseOrientation);
	if (reverseOrientationAttr)
	{
		int32_t style = slider->getStyle ();
		if (*reverseOrientationAttr == strTrue)
		{
			if (style & CSlider::kVertical)
			{
				setBit (style, CSlider::kBottom, false);
				setBit (style, CSlider::kTop, true);
			}
			else if (style & CSlider::kHorizontal)
			{
				setBit (style, CSlider::kLeft, false);
				setBit (style, CSlider::kRight, true);
			}
		}
		else
		{
			if (style & CSlider::kVertical)
			{
				setBit (style, CSlider::kTop, false);
				setBit (style, CSlider::kBottom, true);
			}
			else if (style & CSlider::kHorizontal)
			{
				setBit (style, CSlider::kRight, false);
				setBit (style, CSlider::kLeft, true);
			}
		}
		slider->setStyle (style);
	}

	return true;
}

//------------------------------------------------------------------------
bool SliderBaseCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrMode);
	attributeNames.emplace_back (kAttrOrientation);
	attributeNames.emplace_back (kAttrReverseOrientation);
	attributeNames.emplace_back (kAttrHandleOffset);
	attributeNames.emplace_back (kAttrZoomFactor);
	return true;
}

//------------------------------------------------------------------------
auto SliderBaseCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrMode)
		return kListType;
	if (attributeName == kAttrHandleOffset)
		return kPointType;
	if (attributeName == kAttrZoomFactor)
		return kFloatType;
	if (attributeName == kAttrOrientation)
		return kListType;
	if (attributeName == kAttrReverseOrientation)
		return kBooleanType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool SliderBaseCreator::getAttributeValue (CView* view, const string& attributeName,
                                           string& stringValue, const IUIDescription* desc) const
{
	auto* slider = dynamic_cast<CSliderBase*> (view);
	if (!slider)
		return false;

	if (attributeName == kAttrMode)
	{
		stringValue = modeStrings ()[static_cast<size_t> (slider->getSliderMode ())];
		return true;
	}
	if (attributeName == kAttrHandleOffset)
	{
		stringValue = UIAttributes::pointToString (slider->getOffsetHandle ());
		return true;
	}
	if (attributeName == kAttrZoomFactor)
	{
		stringValue = UIAttributes::doubleToString (slider->getZoomFactor ());
		return true;
	}
	if (attributeName == kAttrOrientation)
	{
		if (slider->getStyle () & CSlider::kVertical)
			stringValue = strVertical;
		else
			stringValue = strHorizontal;
		return true;
	}
	if (attributeName == kAttrReverseOrientation)
	{
		int32_t style = slider->getStyle ();
		stringValue = strFalse;
		if (((style & CSlider::kVertical) && (style & CSlider::kTop)) ||
		    ((style & CSlider::kHorizontal) && (style & CSlider::kRight)))
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool SliderBaseCreator::getPossibleListValues (const string& attributeName,
                                               ConstStringPtrList& values) const
{
	if (attributeName == kAttrOrientation)
	{
		return getStandardAttributeListValues (kAttrOrientation, values);
	}
	if (attributeName == kAttrMode)
	{
		for (auto& str : modeStrings ())
			values.emplace_back (&str);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
SliderCreator::SliderCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr SliderCreator::getViewName () const
{
	return kCSlider;
}

//------------------------------------------------------------------------
IdStringPtr SliderCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr SliderCreator::getDisplayName () const
{
	return "Slider";
}

//------------------------------------------------------------------------
CView* SliderCreator::create (const UIAttributes& attributes,
                              const IUIDescription* description) const
{
	return new CSlider (CRect (0, 0, 0, 0), nullptr, -1, 0, 0, nullptr, nullptr);
}

//------------------------------------------------------------------------
bool SliderCreator::apply (CView* view, const UIAttributes& attributes,
                           const IUIDescription* description) const
{
	auto* slider = dynamic_cast<CSlider*> (view);
	if (!slider)
		return false;

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrHandleBitmap), bitmap, description))
		slider->setHandle (bitmap);

	CPoint p;
	if (attributes.getPointAttribute (kAttrBitmapOffset, p))
		slider->setBackgroundOffset (p);

	int32_t drawStyle = slider->getDrawStyle ();
	applyStyleMask (attributes.getAttributeValue (kAttrDrawFrame), CSlider::kDrawFrame, drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrDrawBack), CSlider::kDrawBack, drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrDrawValue), CSlider::kDrawValue, drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrDrawValueFromCenter),
	                CSlider::kDrawValueFromCenter, drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrDrawValueInverted), CSlider::kDrawInverted,
	                drawStyle);
	slider->setDrawStyle (drawStyle);

	CCoord lineWidth;
	if (attributes.getDoubleAttribute (kAttrFrameWidth, lineWidth))
		slider->setFrameWidth (lineWidth);

	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrDrawFrameColor), color, description))
		slider->setFrameColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrDrawBackColor), color, description))
		slider->setBackColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrDrawValueColor), color, description))
		slider->setValueColor (color);
	return SliderBaseCreator::apply (view, attributes, description);
}

//------------------------------------------------------------------------
bool SliderCreator::getAttributeNames (StringList& attributeNames) const
{
	SliderBaseCreator::getAttributeNames (attributeNames);
	attributeNames.emplace_back (kAttrHandleBitmap);
	attributeNames.emplace_back (kAttrBitmapOffset);
	attributeNames.emplace_back (kAttrDrawFrame);
	attributeNames.emplace_back (kAttrDrawBack);
	attributeNames.emplace_back (kAttrDrawValue);
	attributeNames.emplace_back (kAttrDrawValueFromCenter);
	attributeNames.emplace_back (kAttrDrawValueInverted);
	attributeNames.emplace_back (kAttrFrameWidth);
	attributeNames.emplace_back (kAttrDrawFrameColor);
	attributeNames.emplace_back (kAttrDrawBackColor);
	attributeNames.emplace_back (kAttrDrawValueColor);
	return true;
}

//------------------------------------------------------------------------
auto SliderCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrHandleBitmap)
		return kBitmapType;
	if (attributeName == kAttrBitmapOffset)
		return kPointType;
	if (attributeName == kAttrDrawFrame)
		return kBooleanType;
	if (attributeName == kAttrDrawBack)
		return kBooleanType;
	if (attributeName == kAttrDrawValue)
		return kBooleanType;
	if (attributeName == kAttrDrawValueFromCenter)
		return kBooleanType;
	if (attributeName == kAttrDrawValueInverted)
		return kBooleanType;
	if (attributeName == kAttrFrameWidth)
		return kFloatType;
	if (attributeName == kAttrDrawFrameColor)
		return kColorType;
	if (attributeName == kAttrDrawBackColor)
		return kColorType;
	if (attributeName == kAttrDrawValueColor)
		return kColorType;
	return SliderBaseCreator::getAttributeType (attributeName);
}

//------------------------------------------------------------------------
bool SliderCreator::getAttributeValue (CView* view, const string& attributeName,
                                       string& stringValue, const IUIDescription* desc) const
{
	auto* slider = dynamic_cast<CSlider*> (view);
	if (!slider)
		return false;
	if (attributeName == kAttrHandleBitmap)
	{
		CBitmap* bitmap = slider->getHandle ();
		if (bitmap)
		{
			bitmapToString (bitmap, stringValue, desc);
		}
		return true;
	}
	if (attributeName == kAttrBitmapOffset)
	{
		stringValue = UIAttributes::pointToString (slider->getBackgroundOffset ());
		return true;
	}
	if (attributeName == kAttrDrawFrame)
	{
		if (slider->getDrawStyle () & CSlider::kDrawFrame)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrDrawBack)
	{
		if (slider->getDrawStyle () & CSlider::kDrawBack)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrDrawValue)
	{
		if (slider->getDrawStyle () & CSlider::kDrawValue)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrDrawValueFromCenter)
	{
		if (slider->getDrawStyle () & CSlider::kDrawValueFromCenter)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrDrawValueInverted)
	{
		if (slider->getDrawStyle () & CSlider::kDrawInverted)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrDrawFrameColor)
	{
		colorToString (slider->getFrameColor (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrDrawBackColor)
	{
		colorToString (slider->getBackColor (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrDrawValueColor)
	{
		colorToString (slider->getValueColor (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrFrameWidth)
	{
		stringValue = UIAttributes::doubleToString (slider->getFrameWidth ());
		return true;
	}
	return SliderBaseCreator::getAttributeValue (view, attributeName, stringValue, desc);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
