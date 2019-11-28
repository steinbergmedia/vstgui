// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "knobcreator.h"

#include "../../lib/controls/cknob.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
bool KnobBaseCreator::apply (CView* view, const UIAttributes& attributes,
                             const IUIDescription* description) const
{
	auto* knob = dynamic_cast<CKnobBase*> (view);
	if (!knob)
		return false;

	double d;
	if (attributes.getDoubleAttribute (kAttrAngleStart, d))
	{
		// convert from degree
		d = d / 180.f * static_cast<float> (Constants::pi);
		knob->setStartAngle (static_cast<float> (d));
	}
	if (attributes.getDoubleAttribute (kAttrAngleRange, d))
	{
		// convert from degree
		d = d / 180.f * static_cast<float> (Constants::pi);
		knob->setRangeAngle (static_cast<float> (d));
	}
	if (attributes.getDoubleAttribute (kAttrValueInset, d))
		knob->setInsetValue (d);
	if (attributes.getDoubleAttribute (kAttrZoomFactor, d))
		knob->setZoomFactor (static_cast<float> (d));

	return true;
}

//------------------------------------------------------------------------
bool KnobBaseCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrAngleStart);
	attributeNames.emplace_back (kAttrAngleRange);
	attributeNames.emplace_back (kAttrValueInset);
	attributeNames.emplace_back (kAttrZoomFactor);
	return true;
}

//------------------------------------------------------------------------
auto KnobBaseCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrAngleStart)
		return kFloatType;
	if (attributeName == kAttrAngleRange)
		return kFloatType;
	if (attributeName == kAttrValueInset)
		return kFloatType;
	if (attributeName == kAttrZoomFactor)
		return kFloatType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool KnobBaseCreator::getAttributeValue (CView* view, const string& attributeName,
                                         string& stringValue, const IUIDescription* desc) const
{
	auto* knob = dynamic_cast<CKnobBase*> (view);
	if (!knob)
		return false;

	if (attributeName == kAttrAngleStart)
	{
		stringValue =
		    UIAttributes::doubleToString ((knob->getStartAngle () / Constants::pi * 180.), 5);
		return true;
	}
	if (attributeName == kAttrAngleRange)
	{
		stringValue =
		    UIAttributes::doubleToString ((knob->getRangeAngle () / Constants::pi * 180.), 5);
		return true;
	}
	if (attributeName == kAttrValueInset)
	{
		stringValue = UIAttributes::doubleToString (knob->getInsetValue ());
		return true;
	}
	if (attributeName == kAttrZoomFactor)
	{
		stringValue = UIAttributes::doubleToString (knob->getZoomFactor ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
KnobCreator::KnobCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr KnobCreator::getViewName () const
{
	return kCKnob;
}

//------------------------------------------------------------------------
IdStringPtr KnobCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr KnobCreator::getDisplayName () const
{
	return "Knob";
}

//------------------------------------------------------------------------
CView* KnobCreator::create (const UIAttributes& attributes, const IUIDescription* description) const
{
	auto knob = new CKnob (CRect (0, 0, 70, 70), nullptr, -1, nullptr, nullptr);
	knob->setDrawStyle (CKnob::kCoronaDrawing | CKnob::kCoronaOutline | CKnob::kCoronaLineDashDot |
	                    CKnob::kCoronaLineCapButt | CKnob::kSkipHandleDrawing);
	knob->setCoronaColor (kRedCColor);
	knob->setColorShadowHandle (kBlackCColor);
	knob->setHandleLineWidth (8.);
	knob->setCoronaInset (12);
	knob->setCoronaOutlineWidthAdd (2.);
	knob->setCoronaDashDotLengths ({1.26, 0.1});
	knob->setValue (1.f);
	return knob;
}

//------------------------------------------------------------------------
bool KnobCreator::apply (CView* view, const UIAttributes& attributes,
                         const IUIDescription* description) const
{
	auto* knob = dynamic_cast<CKnob*> (view);
	if (!knob)
		return false;

	double d;
	if (attributes.getDoubleAttribute (kAttrCoronaInset, d))
		knob->setCoronaInset (d);
	if (attributes.getDoubleAttribute (kAttrHandleLineWidth, d))
		knob->setHandleLineWidth (d);
	if (attributes.getDoubleAttribute (kAttrCoronaOutlineWidthAdd, d))
		knob->setCoronaOutlineWidthAdd (d);

	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrCoronaColor), color, description))
		knob->setCoronaColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrHandleShadowColor), color, description))
		knob->setColorShadowHandle (color);
	if (stringToColor (attributes.getAttributeValue (kAttrHandleColor), color, description))
		knob->setColorHandle (color);

	UIAttributes::StringArray dashLengthsStrings;
	if (attributes.getStringArrayAttribute (kAttrCoronaDashDotLengths, dashLengthsStrings))
	{
		CLineStyle::CoordVector lengths;
		for (auto& str : dashLengthsStrings)
		{
			double value;
			if (UIAttributes::stringToDouble (str, value))
			{
				lengths.emplace_back (value);
			}
		}
		knob->setCoronaDashDotLengths (lengths);
	}

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrHandleBitmap), bitmap, description))
		knob->setHandleBitmap (bitmap);

	int32_t drawStyle = knob->getDrawStyle ();
	applyStyleMask (attributes.getAttributeValue (kAttrCircleDrawing), CKnob::kHandleCircleDrawing,
	                drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrCoronaDrawing), CKnob::kCoronaDrawing,
	                drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrCoronaFromCenter), CKnob::kCoronaFromCenter,
	                drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrCoronaInverted), CKnob::kCoronaInverted,
	                drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrCoronaDashDot), CKnob::kCoronaLineDashDot,
	                drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrCoronaOutline), CKnob::kCoronaOutline,
	                drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrCoronaLineCapButt),
	                CKnob::kCoronaLineCapButt, drawStyle);
	applyStyleMask (attributes.getAttributeValue (kAttrSkipHandleDrawing),
	                CKnob::kSkipHandleDrawing, drawStyle);
	knob->setDrawStyle (drawStyle);
	return KnobBaseCreator::apply (view, attributes, description);
}

//------------------------------------------------------------------------
bool KnobCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrCircleDrawing);
	attributeNames.emplace_back (kAttrCoronaDrawing);
	attributeNames.emplace_back (kAttrCoronaOutline);
	attributeNames.emplace_back (kAttrCoronaFromCenter);
	attributeNames.emplace_back (kAttrCoronaInverted);
	attributeNames.emplace_back (kAttrCoronaDashDot);
	attributeNames.emplace_back (kAttrCoronaLineCapButt);
	attributeNames.emplace_back (kAttrSkipHandleDrawing);
	attributeNames.emplace_back (kAttrCoronaInset);
	attributeNames.emplace_back (kAttrCoronaColor);
	attributeNames.emplace_back (kAttrHandleShadowColor);
	attributeNames.emplace_back (kAttrHandleColor);
	attributeNames.emplace_back (kAttrHandleLineWidth);
	attributeNames.emplace_back (kAttrCoronaOutlineWidthAdd);
	attributeNames.emplace_back (kAttrCoronaDashDotLengths);
	attributeNames.emplace_back (kAttrHandleBitmap);
	return KnobBaseCreator::getAttributeNames (attributeNames);
}

//------------------------------------------------------------------------
auto KnobCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrCircleDrawing)
		return kBooleanType;
	if (attributeName == kAttrCoronaDrawing)
		return kBooleanType;
	if (attributeName == kAttrCoronaOutline)
		return kBooleanType;
	if (attributeName == kAttrCoronaFromCenter)
		return kBooleanType;
	if (attributeName == kAttrCoronaInverted)
		return kBooleanType;
	if (attributeName == kAttrCoronaDashDot)
		return kBooleanType;
	if (attributeName == kAttrCoronaLineCapButt)
		return kBooleanType;
	if (attributeName == kAttrSkipHandleDrawing)
		return kBooleanType;
	if (attributeName == kAttrCoronaInset)
		return kFloatType;
	if (attributeName == kAttrCoronaColor)
		return kColorType;
	if (attributeName == kAttrHandleShadowColor)
		return kColorType;
	if (attributeName == kAttrHandleColor)
		return kColorType;
	if (attributeName == kAttrHandleLineWidth)
		return kFloatType;
	if (attributeName == kAttrCoronaOutlineWidthAdd)
		return kFloatType;
	if (attributeName == kAttrCoronaDashDotLengths)
		return kStringType;
	if (attributeName == kAttrHandleBitmap)
		return kBitmapType;
	return KnobBaseCreator::getAttributeType (attributeName);
}

//------------------------------------------------------------------------
bool KnobCreator::getAttributeValue (CView* view, const string& attributeName, string& stringValue,
                                     const IUIDescription* desc) const
{
	auto* knob = dynamic_cast<CKnob*> (view);
	if (!knob)
		return false;

	if (attributeName == kAttrCoronaInset)
	{
		stringValue = UIAttributes::doubleToString (knob->getCoronaInset ());
		return true;
	}
	if (attributeName == kAttrHandleLineWidth)
	{
		stringValue = UIAttributes::doubleToString (knob->getHandleLineWidth ());
		return true;
	}
	if (attributeName == kAttrCoronaOutlineWidthAdd)
	{
		stringValue = UIAttributes::doubleToString (knob->getCoronaOutlineWidthAdd ());
		return true;
	}
	if (attributeName == kAttrCoronaColor)
	{
		colorToString (knob->getCoronaColor (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrHandleShadowColor)
	{
		colorToString (knob->getColorShadowHandle (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrHandleColor)
	{
		colorToString (knob->getColorHandle (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrHandleBitmap)
	{
		CBitmap* bitmap = knob->getHandleBitmap ();
		if (bitmap)
		{
			return bitmapToString (bitmap, stringValue, desc);
		}
	}
	if (attributeName == kAttrCircleDrawing)
	{
		if (knob->getDrawStyle () & CKnob::kHandleCircleDrawing)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaDrawing)
	{
		if (knob->getDrawStyle () & CKnob::kCoronaDrawing)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaFromCenter)
	{
		if (knob->getDrawStyle () & CKnob::kCoronaFromCenter)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaInverted)
	{
		if (knob->getDrawStyle () & CKnob::kCoronaInverted)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaDashDot)
	{
		if (knob->getDrawStyle () & CKnob::kCoronaLineDashDot)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaOutline)
	{
		if (knob->getDrawStyle () & CKnob::kCoronaOutline)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaLineCapButt)
	{
		if (knob->getDrawStyle () & CKnob::kCoronaLineCapButt)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrSkipHandleDrawing)
	{
		if (knob->getDrawStyle () & CKnob::kSkipHandleDrawing)
			stringValue = strTrue;
		else
			stringValue = strFalse;
		return true;
	}
	if (attributeName == kAttrCoronaDashDotLengths)
	{
		const auto& lengths = knob->getCoronaDashDotLengths ();
		UIAttributes::StringArray lengthStrings;
		for (auto value : lengths)
		{
			lengthStrings.emplace_back (UIAttributes::doubleToString (value));
		}
		stringValue = UIAttributes::stringArrayToString (lengthStrings);
		return true;
	}
	return KnobBaseCreator::getAttributeValue (view, attributeName, stringValue, desc);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
