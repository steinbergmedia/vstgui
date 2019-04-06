// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiviewswitchcontainercreator.h"

#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include "../uiviewswitchcontainer.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
UIViewSwitchContainerCreator::UIViewSwitchContainerCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr UIViewSwitchContainerCreator::getViewName () const
{
	return kUIViewSwitchContainer;
}

//------------------------------------------------------------------------
IdStringPtr UIViewSwitchContainerCreator::getBaseViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
UTF8StringPtr UIViewSwitchContainerCreator::getDisplayName () const
{
	return "View Switch Container";
}

//------------------------------------------------------------------------
CView* UIViewSwitchContainerCreator::create (const UIAttributes& attributes,
                                             const IUIDescription* description) const
{
	UIViewSwitchContainer* vsc = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
	new UIDescriptionViewSwitchController (vsc, description, description->getController ());
	return vsc;
}

//------------------------------------------------------------------------
bool UIViewSwitchContainerCreator::apply (CView* view, const UIAttributes& attributes,
                                          const IUIDescription* description) const
{
	auto* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
	if (!viewSwitch)
		return false;
	const std::string* attr = attributes.getAttributeValue (kAttrTemplateNames);
	if (attr)
	{
		auto* controller =
		    dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
		if (controller)
		{
			controller->setTemplateNames (attr->c_str ());
		}
	}
	attr = attributes.getAttributeValue (kAttrTemplateSwitchControl);
	if (attr)
	{
		auto* controller =
		    dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
		if (controller)
		{
			int32_t tag = description->getTagForName (attr->c_str ());
			controller->setSwitchControlTag (tag);
		}
	}
	attr = attributes.getAttributeValue (kAttrAnimationStyle);
	if (attr)
	{
		UIViewSwitchContainer::AnimationStyle style = UIViewSwitchContainer::kFadeInOut;
		if (*attr == kMoveInOut)
			style = UIViewSwitchContainer::kMoveInOut;
		else if (*attr == kPushInOut)
			style = UIViewSwitchContainer::kPushInOut;
		viewSwitch->setAnimationStyle (style);
	}

	attr = attributes.getAttributeValue (kAttrAnimationTimingFunction);
	if (attr)
	{
		UIViewSwitchContainer::TimingFunction tf = UIViewSwitchContainer::kLinear;
		if (*attr == kEasyIn)
			tf = UIViewSwitchContainer::kEasyIn;
		else if (*attr == kEasyOut)
			tf = UIViewSwitchContainer::kEasyOut;
		else if (*attr == kEasyInOut)
			tf = UIViewSwitchContainer::kEasyInOut;
		else if (*attr == kEasy)
			tf = UIViewSwitchContainer::kEasy;
		viewSwitch->setTimingFunction (tf);
	}

	int32_t animationTime;
	if (attributes.getIntegerAttribute (kAttrAnimationTime, animationTime))
	{
		viewSwitch->setAnimationTime (static_cast<uint32_t> (animationTime));
	}
	return true;
}

//------------------------------------------------------------------------
bool UIViewSwitchContainerCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrTemplateNames);
	attributeNames.emplace_back (kAttrTemplateSwitchControl);
	attributeNames.emplace_back (kAttrAnimationStyle);
	attributeNames.emplace_back (kAttrAnimationTimingFunction);
	attributeNames.emplace_back (kAttrAnimationTime);
	return true;
}

//------------------------------------------------------------------------
auto UIViewSwitchContainerCreator::getAttributeType (const std::string& attributeName) const
    -> AttrType
{
	if (attributeName == kAttrTemplateNames)
		return kStringType;
	if (attributeName == kAttrTemplateSwitchControl)
		return kTagType;
	if (attributeName == kAttrAnimationStyle)
		return kListType;
	if (attributeName == kAttrAnimationTimingFunction)
		return kListType;
	if (attributeName == kAttrAnimationTime)
		return kIntegerType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool UIViewSwitchContainerCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                                      std::string& stringValue,
                                                      const IUIDescription* desc) const
{
	auto* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
	if (!viewSwitch)
		return false;
	if (attributeName == kAttrTemplateNames)
	{
		auto* controller =
		    dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
		if (controller)
		{
			controller->getTemplateNames (stringValue);
			return true;
		}
	}
	else if (attributeName == kAttrTemplateSwitchControl)
	{
		auto* controller =
		    dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
		if (controller)
		{
			UTF8StringPtr controlTag =
			    desc->lookupControlTagName (controller->getSwitchControlTag ());
			if (controlTag)
			{
				stringValue = controlTag;
				return true;
			}
			return true;
		}
	}
	else if (attributeName == kAttrAnimationTime)
	{
		stringValue =
		    UIAttributes::integerToString (static_cast<int32_t> (viewSwitch->getAnimationTime ()));
		return true;
	}
	else if (attributeName == kAttrAnimationStyle)
	{
		switch (viewSwitch->getAnimationStyle ())
		{
			case UIViewSwitchContainer::kFadeInOut:
			{
				stringValue = kFadeInOut;
				return true;
			}
			case UIViewSwitchContainer::kMoveInOut:
			{
				stringValue = kMoveInOut;
				return true;
			}
			case UIViewSwitchContainer::kPushInOut:
			{
				stringValue = kPushInOut;
				return true;
			}
		}
	}
	else if (attributeName == kAttrAnimationTimingFunction)
	{
		switch (viewSwitch->getTimingFunction ())
		{
			case UIViewSwitchContainer::kLinear:
			{
				stringValue = kLinear;
				return true;
			}
			case UIViewSwitchContainer::kEasyIn:
			{
				stringValue = kEasyIn;
				return true;
			}
			case UIViewSwitchContainer::kEasyOut:
			{
				stringValue = kEasyOut;
				return true;
			}
			case UIViewSwitchContainer::kEasyInOut:
			{
				stringValue = kEasyInOut;
				return true;
			}
			case UIViewSwitchContainer::kEasy:
			{
				stringValue = kEasy;
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
bool UIViewSwitchContainerCreator::getPossibleListValues (
    const std::string& attributeName, std::list<const std::string*>& values) const
{
	if (attributeName == kAttrAnimationStyle)
	{
		values.emplace_back (&kFadeInOut);
		values.emplace_back (&kMoveInOut);
		values.emplace_back (&kPushInOut);
		return true;
	}
	if (attributeName == kAttrAnimationTimingFunction)
	{
		values.emplace_back (&kLinear);
		values.emplace_back (&kEasyIn);
		values.emplace_back (&kEasyOut);
		values.emplace_back (&kEasyInOut);
		values.emplace_back (&kEasy);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
