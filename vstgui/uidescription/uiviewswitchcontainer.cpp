// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiviewswitchcontainer.h"
#include "iviewcreator.h"
#include "uidescription.h"
#include "../lib/cframe.h"
#include "../lib/controls/ccontrol.h"
#include "../lib/animation/timingfunctions.h"
#include "../lib/animation/animations.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
UIViewSwitchContainer::UIViewSwitchContainer (const CRect& size)
: CViewContainer (size)
, controller (nullptr)
, currentViewIndex (-1)
, animationTime (120)
, animationStyle (kFadeInOut)
{
}

//-----------------------------------------------------------------------------
UIViewSwitchContainer::~UIViewSwitchContainer () noexcept
{
	setController (nullptr);
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setController (IViewSwitchController* _controller)
{
	if (controller)
	{
		auto obj = dynamic_cast<IReference*> (controller);
		if (obj)
			obj->forget ();
	}
	controller = _controller;
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setCurrentViewIndex (int32_t viewIndex)
{
	if (controller && viewIndex != currentViewIndex)
	{
		CView* view = controller->createViewForIndex (viewIndex);
		if (view)
		{
			if (view->getAutosizeFlags () & kAutosizeAll)
			{
				CRect vs (getViewSize ());
				vs.offset (-vs.left, -vs.top);
				view->setViewSize (vs);
				view->setMouseableArea (vs);
			}
			if (isAttached () && animationTime)
			{
				removeAnimation ("UIViewSwitchContainer::setCurrentViewIndex");
				CView* oldView = getView (0);
				if (oldView)
				{
					Animation::IAnimationTarget* animation = nullptr;
					switch (animationStyle)
					{
						case kFadeInOut:
						{
							animation = new Animation::ExchangeViewAnimation (oldView, view, Animation::ExchangeViewAnimation::kAlphaValueFade);
							break;
						}
						case kMoveInOut:
						{
							Animation::ExchangeViewAnimation::AnimationStyle style = Animation::ExchangeViewAnimation::kPushInFromLeft;
							if (viewIndex > currentViewIndex)
							{
								style = Animation::ExchangeViewAnimation::kPushInFromRight;
							}
							animation = new Animation::ExchangeViewAnimation (oldView, view, style);
							break;
						}
						case kPushInOut:
						{
							Animation::ExchangeViewAnimation::AnimationStyle style = Animation::ExchangeViewAnimation::kPushInOutFromLeft;
							if (viewIndex > currentViewIndex)
							{
								style = Animation::ExchangeViewAnimation::kPushInOutFromRight;
							}
							animation = new Animation::ExchangeViewAnimation (oldView, view, style);
							break;
						}
					}
					if (animation)
						addAnimation ("UIViewSwitchContainer::setCurrentViewIndex", animation, new Animation::LinearTimingFunction (animationTime));
					else
					{
						removeAll ();
						addView (view);
					}
				}
				else
				{
					removeAll ();
					addView (view);
				}
			}
			else
			{
				CViewContainer::removeAll ();
				CViewContainer::addView (view);
			}
			currentViewIndex = viewIndex;
			invalid ();
		}
	}
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setAnimationTime (uint32_t ms)
{
	animationTime = ms;
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setAnimationStyle (AnimationStyle style)
{
	animationStyle = style;
}

//-----------------------------------------------------------------------------
bool UIViewSwitchContainer::attached (CView* parent)
{
	bool result = CViewContainer::attached (parent);
	CViewContainer::removeAll ();
	if (result && controller)
		controller->switchContainerAttached ();
	return result;
}

//-----------------------------------------------------------------------------
bool UIViewSwitchContainer::removed (CView* parent)
{
	if (isAttached ())
	{
		removeAnimation ("UIViewSwitchContainer::setCurrentViewIndex");
		bool result = CViewContainer::removed (parent);
		if (result && controller)
			controller->switchContainerRemoved ();
		CViewContainer::removeAll ();
		return result;
	}
	return false;
}

//-----------------------------------------------------------------------------
UIDescriptionViewSwitchController::UIDescriptionViewSwitchController (UIViewSwitchContainer* viewSwitch, const IUIDescription* uiDescription, IController* uiController)
: IViewSwitchController (viewSwitch)
, uiDescription (uiDescription)
, uiController (uiController)
, switchControlTag (-1)
, currentIndex (-1)
, switchControl (nullptr)
{
	init ();
}

//-----------------------------------------------------------------------------
CView* UIDescriptionViewSwitchController::createViewForIndex (int32_t index)
{
	if (index >= 0 && index < (int32_t)templateNames.size ())
	{
		return uiDescription->createView (templateNames[static_cast<uint32_t> (index)].c_str (), uiController);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
static CControl* findControlTag (CViewContainer* parent, int32_t tag, bool reverse = true)
{
	CControl* result = nullptr;
	ViewIterator it (parent);
	while (*it)
	{
		CView* view = *it;
		CControl* control = dynamic_cast<CControl*> (view);
		if (control)
		{
			if (control->getTag () == tag)
				result = control;
		}
		else if (reverse)
		{
			if (auto container = view->asViewContainer ())
				result = findControlTag (container, tag);
		}
		if (result)
			break;
		++it;
	}
	if (result == nullptr && !reverse && parent->getParentView ())
		return findControlTag (parent->getParentView ()->asViewContainer (), tag, reverse);
	return result;
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerAttached ()
{
	if (switchControlTag != -1)
	{
		// find the switch Control
		switchControl = findControlTag (viewSwitch->getParentView ()->asViewContainer (), switchControlTag, false);
		if (switchControl == nullptr)
		{
			switchControl = findControlTag (viewSwitch->getFrame (), switchControlTag, true);
		}
		if (switchControl)
		{
			switchControl->addDependency (this);
			switchControl->remember ();
			notify (switchControl, CControl::kMessageValueChanged);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerRemoved ()
{
	if (switchControl)
	{
		switchControl->removeDependency (this);
		switchControl->forget ();
		switchControl = nullptr;
		currentIndex = -1;
	}
}

//-----------------------------------------------------------------------------
CMessageResult UIDescriptionViewSwitchController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (sender == switchControl && message == CControl::kMessageValueChanged)
	{
		float norm = switchControl->getValueNormalized ();
		int32_t index = std::min<int32_t> ((int32_t)(norm * (float)templateNames.size ()), (int32_t)templateNames.size ()-1);
		if (index != currentIndex)
		{
			viewSwitch->setCurrentViewIndex (index);
			currentIndex = index;
		}
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::setTemplateNames (UTF8StringPtr _templateNames)
{
	templateNames.clear ();
	if (_templateNames)
	{
		std::string temp (_templateNames);
		size_t start = 0;
		size_t pos = temp.find (",", start, 1);
		if (pos != std::string::npos)
		{
			while (pos != std::string::npos)
			{
				std::string name (temp, start, pos - start);
				templateNames.emplace_back (name);
				start = pos+1;
				pos = temp.find (",", start, 1);
			}
			std::string name (temp, start, std::string::npos);
			templateNames.emplace_back (name);
		}
		else
		{
			// only one template name
			templateNames.emplace_back (temp);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::getTemplateNames (std::string& str)
{
	str.clear ();
	for (size_t i = 0; i < templateNames.size (); i++)
	{
		str += templateNames[i];
		if (i != templateNames.size () - 1)
		{
			str += ",";
		}
	}
}

} // namespace
