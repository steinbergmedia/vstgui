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
	using namespace Animation;

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
					IAnimationTarget* animation = nullptr;
					switch (animationStyle)
					{
						case kFadeInOut:
						{
							animation = new ExchangeViewAnimation (oldView, view, ExchangeViewAnimation::kAlphaValueFade);
							break;
						}
						case kMoveInOut:
						{
							ExchangeViewAnimation::AnimationStyle style = ExchangeViewAnimation::kPushInFromLeft;
							if (viewIndex > currentViewIndex)
							{
								style = ExchangeViewAnimation::kPushInFromRight;
							}
							animation = new ExchangeViewAnimation (oldView, view, style);
							break;
						}
						case kPushInOut:
						{
							ExchangeViewAnimation::AnimationStyle style = ExchangeViewAnimation::kPushInOutFromLeft;
							if (viewIndex > currentViewIndex)
							{
								style = ExchangeViewAnimation::kPushInOutFromRight;
							}
							animation = new ExchangeViewAnimation (oldView, view, style);
							break;
						}
					}
					if (animation)
					{
						ITimingFunction* tf = nullptr;
						switch (timingFunction)
						{
							case kEasyIn:
							{
								tf = new CubicBezierTimingFunction (CubicBezierTimingFunction::easyIn (animationTime));
								break;
							}
							case kEasyOut:
							{
								tf = new CubicBezierTimingFunction (CubicBezierTimingFunction::easyOut (animationTime));
								break;
							}
							case kEasyInOut:
							{
								tf = new CubicBezierTimingFunction (CubicBezierTimingFunction::easyInOut (animationTime));
								break;
							}
							case kEasy:
							{
								tf = new CubicBezierTimingFunction (CubicBezierTimingFunction::easy (animationTime));
								break;
							}
							default:
							{
								tf = new LinearTimingFunction (animationTime);
								break;
							}
						}
						addAnimation ("UIViewSwitchContainer::setCurrentViewIndex", animation, tf);
					}
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
void UIViewSwitchContainer::setTimingFunction (TimingFunction t)
{
	timingFunction = t;
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
UIDescriptionViewSwitchController::UIDescriptionViewSwitchController (
    UIViewSwitchContainer* viewSwitch, const IUIDescription* uiDescription,
    IController* uiController)
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
static CControl* findControlForTag (CViewContainer* parent, int32_t tag, bool reverse = true)
{
	CControl* result = nullptr;
	ViewIterator it (parent);
	while (*it)
	{
		CView* view = *it;
		auto* control = dynamic_cast<CControl*> (view);
		if (control)
		{
			if (control->getTag () == tag)
				result = control;
		}
		else if (reverse)
		{
			if (auto container = view->asViewContainer ())
				result = findControlForTag (container, tag);
		}
		if (result)
			break;
		++it;
	}
	if (result == nullptr && !reverse && parent->getParentView ())
		return findControlForTag (parent->getParentView ()->asViewContainer (), tag, reverse);
	return result;
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerAttached ()
{
	if (switchControlTag != -1)
	{
		// find the switch Control
		switchControl = findControlForTag (viewSwitch->getParentView ()->asViewContainer (), switchControlTag, false);
		if (switchControl == nullptr)
		{
			switchControl = findControlForTag (viewSwitch->getFrame (), switchControlTag, true);
		}
		if (switchControl)
		{
			switchControl->registerControlListener (this);
			valueChanged (switchControl);
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
void UIDescriptionViewSwitchController::valueChanged (CControl* pControl)
{
	auto norm = pControl->getValueNormalized ();
	auto index = std::min (static_cast<int32_t> (norm * static_cast<float> (templateNames.size ())), static_cast<int32_t> (templateNames.size () - 1));
	if (index != currentIndex)
	{
		viewSwitch->setCurrentViewIndex (index);
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
