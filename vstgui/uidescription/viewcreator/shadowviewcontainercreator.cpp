// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "shadowviewcontainercreator.h"

#include "../../lib/cshadowviewcontainer.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
ShadowViewContainerCreator::ShadowViewContainerCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr ShadowViewContainerCreator::getViewName () const
{
	return kCShadowViewContainer;
}

//------------------------------------------------------------------------
IdStringPtr ShadowViewContainerCreator::getBaseViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
UTF8StringPtr ShadowViewContainerCreator::getDisplayName () const
{
	return "Shadow View Container";
}

//------------------------------------------------------------------------
CView* ShadowViewContainerCreator::create (const UIAttributes& attributes,
                                           const IUIDescription* description) const
{
	return new CShadowViewContainer (CRect (0, 0, 200, 200));
}

//------------------------------------------------------------------------
bool ShadowViewContainerCreator::apply (CView* view, const UIAttributes& attributes,
                                        const IUIDescription* description) const
{
	auto* shadowView = dynamic_cast<CShadowViewContainer*> (view);
	if (!shadowView)
		return false;
	double d;
	if (attributes.getDoubleAttribute (kAttrShadowIntensity, d))
		shadowView->setShadowIntensity (static_cast<float> (d));
	if (attributes.getDoubleAttribute (kAttrShadowBlurSize, d))
		shadowView->setShadowBlurSize (d);
	CPoint p;
	if (attributes.getPointAttribute (kAttrShadowOffset, p))
		shadowView->setShadowOffset (p);
	return true;
}

//------------------------------------------------------------------------
bool ShadowViewContainerCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrShadowIntensity);
	attributeNames.emplace_back (kAttrShadowOffset);
	attributeNames.emplace_back (kAttrShadowBlurSize);
	return true;
}

//------------------------------------------------------------------------
auto ShadowViewContainerCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrShadowIntensity)
		return kFloatType;
	if (attributeName == kAttrShadowOffset)
		return kPointType;
	if (attributeName == kAttrShadowBlurSize)
		return kFloatType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool ShadowViewContainerCreator::getAttributeValue (CView* view, const string& attributeName,
                                                    string& stringValue,
                                                    const IUIDescription* desc) const
{
	auto* shadowView = dynamic_cast<CShadowViewContainer*> (view);
	if (!shadowView)
		return false;
	if (attributeName == kAttrShadowIntensity)
	{
		stringValue = UIAttributes::doubleToString (shadowView->getShadowIntensity ());
		return true;
	}
	else if (attributeName == kAttrShadowBlurSize)
	{
		stringValue = UIAttributes::doubleToString (shadowView->getShadowBlurSize ());
		return true;
	}
	else if (attributeName == kAttrShadowOffset)
	{
		stringValue = UIAttributes::pointToString (shadowView->getShadowOffset ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool ShadowViewContainerCreator::getAttributeValueRange (const string& attributeName,
                                                         double& minValue, double& maxValue) const
{
	if (attributeName == kAttrShadowBlurSize)
	{
		minValue = 0.8;
		maxValue = 20;
		return true;
	}
	else if (attributeName == kAttrShadowIntensity)
	{
		minValue = 0;
		maxValue = 1;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
