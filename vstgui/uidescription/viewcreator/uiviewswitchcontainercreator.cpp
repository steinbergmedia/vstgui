// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiviewswitchcontainercreator.h"

#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include "../uiviewswitchcontainer.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto UIViewSwitchContainerCreator::timingFunctionStrings () -> TimingFunctionStrings&
{
	static TimingFunctionStrings strings = {"linear", "easy-in", "easy-out", "easy-in-out", "easy"};
	return strings;
}

//------------------------------------------------------------------------
auto UIViewSwitchContainerCreator::animationStyleStrings () -> AnimationStyleStrings&
{
	static AnimationStyleStrings strings = {"fade", "move", "push"};
	return strings;
}

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
	const auto* attr = attributes.getAttributeValue (kAttrTemplateNames);
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
		for (auto index = 0u; index <= UIViewSwitchContainer::kPushInOut; ++index)
		{
			if (*attr == animationStyleStrings ()[index])
			{
				viewSwitch->setAnimationStyle (
				    static_cast<UIViewSwitchContainer::AnimationStyle> (index));
				break;
			}
		}
	}

	attr = attributes.getAttributeValue (kAttrAnimationTimingFunction);
	if (attr)
	{
		for (auto index = 0u; index <= UIViewSwitchContainer::kEasy; ++index)
		{
			if (*attr == timingFunctionStrings ()[index])
			{
				viewSwitch->setTimingFunction (
				    static_cast<UIViewSwitchContainer::TimingFunction> (index));
				break;
			}
		}
	}

	int32_t animationTime;
	if (attributes.getIntegerAttribute (kAttrAnimationTime, animationTime))
	{
		viewSwitch->setAnimationTime (static_cast<uint32_t> (animationTime));
	}
	return true;
}

//------------------------------------------------------------------------
bool UIViewSwitchContainerCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrTemplateNames);
	attributeNames.emplace_back (kAttrTemplateSwitchControl);
	attributeNames.emplace_back (kAttrAnimationStyle);
	attributeNames.emplace_back (kAttrAnimationTimingFunction);
	attributeNames.emplace_back (kAttrAnimationTime);
	return true;
}

//------------------------------------------------------------------------
auto UIViewSwitchContainerCreator::getAttributeType (const string& attributeName) const -> AttrType
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
bool UIViewSwitchContainerCreator::getAttributeValue (CView* view, const string& attributeName,
                                                      string& stringValue,
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
		stringValue = animationStyleStrings ()[viewSwitch->getAnimationStyle ()];
		return true;
	}
	else if (attributeName == kAttrAnimationTimingFunction)
	{
		stringValue = timingFunctionStrings ()[viewSwitch->getTimingFunction ()];
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool UIViewSwitchContainerCreator::getPossibleListValues (const string& attributeName,
                                                          ConstStringPtrList& values) const
{
	if (attributeName == kAttrAnimationStyle)
	{
		for (auto& str : animationStyleStrings ())
			values.emplace_back (&str);
		return true;
	}
	if (attributeName == kAttrAnimationTimingFunction)
	{
		for (auto& str : timingFunctionStrings ())
			values.emplace_back (&str);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
