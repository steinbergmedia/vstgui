//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
, controller (0)
, currentViewIndex (-1)
, animationTime (120)
, animationStyle (kFadeInOut)
{
}

//-----------------------------------------------------------------------------
UIViewSwitchContainer::~UIViewSwitchContainer ()
{
	setController (0);
}

//-----------------------------------------------------------------------------
void UIViewSwitchContainer::setController (IViewSwitchController* _controller)
{
	if (controller)
	{
		CBaseObject* obj = dynamic_cast<CBaseObject*> (controller);
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
			if (animationTime)
			{
				if (getFrame ())
					getFrame ()->getAnimator ()->removeAnimation (this, "UIViewSwitchContainer::setCurrentViewIndex");
				CView* oldView = getView (0);
				if (isAttached () && oldView && getFrame ())
				{
					Animation::IAnimationTarget* animation = 0;
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
						getFrame ()->getAnimator ()->addAnimation (this, "UIViewSwitchContainer::setCurrentViewIndex", animation, new Animation::LinearTimingFunction (animationTime));
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
, switchControl (0)
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
	return 0;
}

//-----------------------------------------------------------------------------
static CControl* findControlTag (CViewContainer* parent, int32_t tag, bool reverse = true)
{
	CControl* result = 0;
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
			CViewContainer* container = dynamic_cast<CViewContainer*> (view);
			if (container)
				result = findControlTag (container, tag);
		}
		if (result)
			break;
		++it;
	}
	if (result == 0 && !reverse)
		return findControlTag (dynamic_cast<CViewContainer*> (parent->getParentView ()), reverse);
	return result;
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerAttached ()
{
	if (switchControlTag != -1)
	{
		// find the switch Control
		switchControl = findControlTag (dynamic_cast<CViewContainer*> (viewSwitch->getParentView ()), switchControlTag, false);
		if (switchControl == 0)
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
		switchControl = 0;
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
				templateNames.push_back (name);
				start = pos+1;
				pos = temp.find (",", start, 1);
			}
			std::string name (temp, start, std::string::npos);
			templateNames.push_back (name);
		}
		else
		{
			// only one template name
			templateNames.push_back (temp);
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
