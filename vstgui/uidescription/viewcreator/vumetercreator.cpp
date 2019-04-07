// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vumetercreator.h"

#include "../../lib/controls/cvumeter.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
VuMeterCreator::VuMeterCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr VuMeterCreator::getViewName () const
{
	return kCVuMeter;
}

//------------------------------------------------------------------------
IdStringPtr VuMeterCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr VuMeterCreator::getDisplayName () const
{
	return "VU Meter";
}

//------------------------------------------------------------------------
CView* VuMeterCreator::create (const UIAttributes& attributes,
                               const IUIDescription* description) const
{
	return new CVuMeter (CRect (0, 0, 0, 0), nullptr, nullptr, 100);
}

//------------------------------------------------------------------------
bool VuMeterCreator::apply (CView* view, const UIAttributes& attributes,
                            const IUIDescription* description) const
{
	auto* vuMeter = dynamic_cast<CVuMeter*> (view);
	if (!vuMeter)
		return false;

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrOffBitmap), bitmap, description))
		vuMeter->setOffBitmap (bitmap);

	const auto* attr = attributes.getAttributeValue (kAttrOrientation);
	if (attr)
		vuMeter->setStyle (*attr == strVertical ? CVuMeter::kVertical : CVuMeter::kHorizontal);

	int32_t numLed;
	if (attributes.getIntegerAttribute (kAttrNumLed, numLed))
		vuMeter->setNbLed (numLed);

	double value;
	if (attributes.getDoubleAttribute (kAttrDecreaseStepValue, value))
		vuMeter->setDecreaseStepValue (static_cast<float> (value));
	return true;
}

//------------------------------------------------------------------------
bool VuMeterCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrOffBitmap);
	attributeNames.emplace_back (kAttrNumLed);
	attributeNames.emplace_back (kAttrOrientation);
	attributeNames.emplace_back (kAttrDecreaseStepValue);
	return true;
}

//------------------------------------------------------------------------
auto VuMeterCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrOffBitmap)
		return kBitmapType;
	if (attributeName == kAttrNumLed)
		return kIntegerType;
	if (attributeName == kAttrOrientation)
		return kListType;
	if (attributeName == kAttrDecreaseStepValue)
		return kFloatType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool VuMeterCreator::getAttributeValue (CView* view, const string& attributeName,
                                        string& stringValue, const IUIDescription* desc) const
{
	auto* vuMeter = dynamic_cast<CVuMeter*> (view);
	if (!vuMeter)
		return false;
	if (attributeName == kAttrOffBitmap)
	{
		CBitmap* bitmap = vuMeter->getOffBitmap ();
		if (bitmap)
		{
			bitmapToString (bitmap, stringValue, desc);
		}
		return true;
	}
	else if (attributeName == kAttrOrientation)
	{
		if (vuMeter->getStyle () & CVuMeter::kVertical)
			stringValue = strVertical;
		else
			stringValue = strHorizontal;
		return true;
	}
	else if (attributeName == kAttrNumLed)
	{
		stringValue = UIAttributes::integerToString (vuMeter->getNbLed ());
		return true;
	}
	else if (attributeName == kAttrDecreaseStepValue)
	{
		stringValue = UIAttributes::doubleToString (vuMeter->getDecreaseStepValue ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool VuMeterCreator::getPossibleListValues (const string& attributeName,
                                            ConstStringPtrList& values) const
{
	if (attributeName == kAttrOrientation)
	{
		return getStandardAttributeListValues (kAttrOrientation, values);
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
