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
{
}

//-----------------------------------------------------------------------------
UIViewSwitchContainer::~UIViewSwitchContainer () noexcept
{
	setController (nullptr);
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setController (const SharedPointer<IViewSwitchController>& _controller)
{
	controller = _controller;
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setCurrentViewIndex (int32_t viewIndex)
{
	using namespace Animation;

	if (controller && viewIndex != currentViewIndex)
	{
		auto view = controller->createViewForIndex (viewIndex);
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
				auto oldView = getView (0);
				if (oldView)
				{
					SharedPointer<IAnimationTarget> animation;
					switch (animationStyle)
					{
						case kFadeInOut:
						{
							animation = makeShared<ExchangeViewAnimation> (
								oldView, view, ExchangeViewAnimation::kAlphaValueFade);
							break;
						}
						case kMoveInOut:
						{
							ExchangeViewAnimation::AnimationStyle style = ExchangeViewAnimation::kPushInFromLeft;
							if (viewIndex > currentViewIndex)
							{
								style = ExchangeViewAnimation::kPushInFromRight;
							}
							animation = makeShared<ExchangeViewAnimation> (oldView, view, style);
							break;
						}
						case kPushInOut:
						{
							ExchangeViewAnimation::AnimationStyle style = ExchangeViewAnimation::kPushInOutFromLeft;
							if (viewIndex > currentViewIndex)
							{
								style = ExchangeViewAnimation::kPushInOutFromRight;
							}
							animation = makeShared<ExchangeViewAnimation> (oldView, view, style);
							break;
						}
					}
					if (animation)
					{
						SharedPointer<ITimingFunction> tf;
						switch (timingFunction)
						{
							case kEasyIn:
							{
								tf = makeShared<CubicBezierTimingFunction> (
									CubicBezierTimingFunction::easyIn (animationTime));
								break;
							}
							case kEasyOut:
							{
								tf = makeShared<CubicBezierTimingFunction> (
									CubicBezierTimingFunction::easyOut (animationTime));
								break;
							}
							case kEasyInOut:
							{
								tf = makeShared<CubicBezierTimingFunction> (
									CubicBezierTimingFunction::easyInOut (animationTime));
								break;
							}
							case kEasy:
							{
								tf = makeShared<CubicBezierTimingFunction> (
									CubicBezierTimingFunction::easy (animationTime));
								break;
							}
							default:
							{
								tf = makeShared<LinearTimingFunction> (animationTime);
								break;
							}
						}
						addAnimation ("UIViewSwitchContainer::setCurrentViewIndex", animation, tf);
					}
					else
					{
						removeAll ();
						addSubview (view);
					}
				}
				else
				{
					removeAll ();
					addSubview (view);
				}
			}
			else
			{
				CViewContainer::removeAll ();
				CViewContainer::addSubview (view);
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
void UIViewSwitchContainer::setTimingFunction (TimingFunction t)
{
	timingFunction = t;
}

//-----------------------------------------------------------------------------
bool UIViewSwitchContainer::attached (CViewContainer& parent)
{
	bool result = CViewContainer::attached (parent);
	CViewContainer::removeAll ();
	if (result && controller)
		controller->switchContainerAttached ();
	return result;
}

//-----------------------------------------------------------------------------
bool UIViewSwitchContainer::removed (CViewContainer& parent)
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

//------------------------------------------------------------------------
SharedPointer<UIDescriptionViewSwitchController> UIDescriptionViewSwitchController::make (
	const SharedPointer<UIViewSwitchContainer>& viewSwitch, const IUIDescription& uiDescription,
	const SharedPointer<IController>& uiController)
{
	auto instance =
		makeShared<UIDescriptionViewSwitchController> (viewSwitch, uiDescription, uiController);
	instance->init ();
	return instance;
}

//-----------------------------------------------------------------------------
UIDescriptionViewSwitchController::UIDescriptionViewSwitchController (
	const SharedPointer<UIViewSwitchContainer>& viewSwitch, const IUIDescription& uiDescription,
	const SharedPointer<IController>& uiController)
: IViewSwitchController (viewSwitch)
, uiDescription (uiDescription)
, uiController (uiController)
, switchControlTag (-1)
, currentIndex (-1)
, switchControl (nullptr)
{
}

//-----------------------------------------------------------------------------
SharedPointer<CView> UIDescriptionViewSwitchController::createViewForIndex (int32_t index)
{
	if (index >= 0 && index < (int32_t)templateNames.size ())
	{
		return uiDescription.createView (templateNames[static_cast<uint32_t> (index)].c_str (),
										 uiController);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
static SharedPointer<CControl> findControlForTag (CViewContainer& parent, int32_t tag,
												  bool reverse = true)
{
	SharedPointer<CControl> result;
	ViewIterator it (parent);
	while (*it)
	{
		auto view = *it;
		auto control = view.cast<CControl> ();
		if (control)
		{
			if (control->getTag () == tag)
				result = control;
		}
		else if (reverse)
		{
			if (auto container = view->asViewContainer ())
				result = findControlForTag (*container, tag);
		}
		if (result)
			break;
		++it;
	}
	if (result == nullptr && !reverse && parent.getParentView ())
		return findControlForTag (*parent.getParentView (), tag, reverse);
	return result;
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerAttached ()
{
	if (auto vs = viewSwitch.lock (); vs && switchControlTag != -1)
	{
		// find the switch Control
		if (auto pv = vs->getParentView ())
			switchControl = findControlForTag (*pv, switchControlTag, false);
		if (switchControl == nullptr && vs->getFrame ())
		{
			switchControl = findControlForTag (*vs->getFrame (), switchControlTag, true);
		}
		if (switchControl)
		{
			switchControl->registerControlListener (this);
			valueChanged (*switchControl.get ());
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerRemoved ()
{
	if (switchControl)
	{
		switchControl->unregisterControlListener (this);
		switchControl = nullptr;
		currentIndex = -1;
	}
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::valueChanged (CControl& pControl)
{
	auto norm = pControl.getValueNormalized ();
	auto index = std::min (static_cast<int32_t> (norm * static_cast<float> (templateNames.size ())), static_cast<int32_t> (templateNames.size () - 1));
	if (auto vs = viewSwitch.lock (); vs && index != currentIndex)
	{
		vs->setCurrentViewIndex (index);
		currentIndex = index;
	}
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

} // VSTGUI
