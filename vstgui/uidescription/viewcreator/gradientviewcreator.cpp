// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gradientviewcreator.h"

#include "../../lib/cgradientview.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//-----------------------------------------------------------------------------
CGradientViewCreator::CGradientViewCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CGradientViewCreator::getViewName () const
{
	return kCGradientView;
}

//------------------------------------------------------------------------
IdStringPtr CGradientViewCreator::getBaseViewName () const
{
	return kCView;
}

//------------------------------------------------------------------------
UTF8StringPtr CGradientViewCreator::getDisplayName () const
{
	return "Gradient View";
}

//------------------------------------------------------------------------
CView* CGradientViewCreator::create (const UIAttributes& attributes,
                                     const IUIDescription* description) const
{
	CGradientView* gradientView = new CGradientView (CRect (0, 0, 100, 100));
	if (description)
	{
		std::list<const std::string*> gradients;
		description->collectGradientNames (gradients);
		if (!gradients.empty ())
		{
			gradientView->setGradient (description->getGradient (gradients.front ()->c_str ()));
		}
	}
	return gradientView;
}

//------------------------------------------------------------------------
bool CGradientViewCreator::apply (CView* view, const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	auto* gv = dynamic_cast<CGradientView*> (view);
	if (gv == nullptr)
		return false;
	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
		gv->setFrameColor (color);

	double d;
	if (attributes.getDoubleAttribute (kAttrGradientAngle, d))
		gv->setGradientAngle (d);
	if (attributes.getDoubleAttribute (kAttrRoundRectRadius, d))
		gv->setRoundRectRadius (d);
	if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
		gv->setFrameWidth (d);

	bool b;
	if (attributes.getBooleanAttribute (kAttrDrawAntialiased, b))
		gv->setDrawAntialiased (b);

	const std::string* attr = attributes.getAttributeValue (kAttrGradientStyle);
	if (attr)
	{
		if (*attr == kRadial)
			gv->setGradientStyle (CGradientView::kRadialGradient);
		else
			gv->setGradientStyle (CGradientView::kLinearGradient);
	}
	CPoint p;
	if (attributes.getPointAttribute (kAttrRadialCenter, p))
		gv->setRadialCenter (p);
	if (attributes.getDoubleAttribute (kAttrRadialRadius, d))
		gv->setRadialRadius (d);

	attr = attributes.getAttributeValue (kAttrGradient);
	if (attr)
	{
		CGradient* gradient = description->getGradient (attr->c_str ());
		gv->setGradient (gradient);
	}
	else
	{ // support old version
		bool hasOldGradient = true;
		CColor startColor, endColor;
		if (!stringToColor (attributes.getAttributeValue (kAttrGradientStartColor), startColor,
		                    description))
			hasOldGradient = false;
		if (hasOldGradient && !stringToColor (attributes.getAttributeValue (kAttrGradientEndColor),
		                                      endColor, description))
			hasOldGradient = false;
		double startOffset = 0.0, endOffset = 1.0;
		if (hasOldGradient &&
		    !attributes.getDoubleAttribute (kAttrGradientStartColorOffset, startOffset))
			hasOldGradient = false;
		if (hasOldGradient &&
		    !attributes.getDoubleAttribute (kAttrGradientEndColorOffset, endOffset))
			hasOldGradient = false;
		if (hasOldGradient)
		{
			SharedPointer<CGradient> gradient =
			    owned (CGradient::create (startOffset, 1. - endOffset, startColor, endColor));
			gv->setGradient (gradient);
			addGradientToUIDescription (description, gradient, "GradientView");
		}
	}
	return true;
}

//------------------------------------------------------------------------
bool CGradientViewCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrGradientStyle);
	attributeNames.emplace_back (kAttrGradient);
	attributeNames.emplace_back (kAttrGradientAngle);
	attributeNames.emplace_back (kAttrRadialCenter);
	attributeNames.emplace_back (kAttrRadialRadius);
	attributeNames.emplace_back (kAttrFrameColor);
	attributeNames.emplace_back (kAttrRoundRectRadius);
	attributeNames.emplace_back (kAttrFrameWidth);
	attributeNames.emplace_back (kAttrDrawAntialiased);
	return true;
}

//------------------------------------------------------------------------
auto CGradientViewCreator::getAttributeType (const std::string& attributeName) const -> AttrType
{
	if (attributeName == kAttrGradientStyle)
		return kListType;
	if (attributeName == kAttrGradient)
		return kGradientType;
	if (attributeName == kAttrFrameColor)
		return kColorType;
	if (attributeName == kAttrGradientAngle)
		return kFloatType;
	if (attributeName == kAttrRoundRectRadius)
		return kFloatType;
	if (attributeName == kAttrFrameWidth)
		return kFloatType;
	if (attributeName == kAttrDrawAntialiased)
		return kBooleanType;
	if (attributeName == kAttrRadialCenter)
		return kPointType;
	if (attributeName == kAttrRadialRadius)
		return kFloatType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CGradientViewCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                              std::string& stringValue,
                                              const IUIDescription* desc) const
{
	auto* gv = dynamic_cast<CGradientView*> (view);
	if (gv == nullptr)
		return false;
	if (attributeName == kAttrFrameColor)
	{
		colorToString (gv->getFrameColor (), stringValue, desc);
		return true;
	}
	if (attributeName == kAttrGradientAngle)
	{
		stringValue = UIAttributes::doubleToString (gv->getGradientAngle ());
		return true;
	}
	if (attributeName == kAttrRoundRectRadius)
	{
		stringValue = UIAttributes::doubleToString (gv->getRoundRectRadius ());
		return true;
	}
	if (attributeName == kAttrFrameWidth)
	{
		stringValue = UIAttributes::doubleToString (gv->getFrameWidth ());
		return true;
	}
	if (attributeName == kAttrDrawAntialiased)
	{
		stringValue = gv->getDrawAntialised () ? strTrue : strFalse;
		return true;
	}
	if (attributeName == kAttrGradientStyle)
	{
		stringValue = gv->getGradientStyle () == CGradientView::kLinearGradient ? kLinear : kRadial;
		return true;
	}
	if (attributeName == kAttrRadialRadius)
	{
		stringValue = UIAttributes::doubleToString (gv->getRadialRadius ());
		return true;
	}
	if (attributeName == kAttrRadialCenter)
	{
		stringValue = UIAttributes::pointToString (gv->getRadialCenter ());
		return true;
	}
	if (attributeName == kAttrGradient)
	{
		CGradient* gradient = gv->getGradient ();
		UTF8StringPtr gradientName = gradient ? desc->lookupGradientName (gradient) : nullptr;
		stringValue = gradientName ? gradientName : "";
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CGradientViewCreator::getPossibleListValues (const std::string& attributeName,
                                                  std::list<const std::string*>& values) const
{
	if (attributeName == kAttrGradientStyle)
	{
		values.emplace_back (&kLinear);
		values.emplace_back (&kRadial);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CGradientViewCreator::getAttributeValueRange (const std::string& attributeName,
                                                   double& minValue, double& maxValue) const
{
	if (attributeName == kAttrGradientAngle)
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
