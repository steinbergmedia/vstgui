//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "uiviewswitchcontainer.h"
#include "uiviewcreator.h"
#include "../lib/cframe.h"
#include "../lib/animation/animations.h"
#include "../lib/animation/timingfunctions.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
UIViewSwitchContainer::UIViewSwitchContainer (const CRect& size)
: CViewContainer (size, 0)
, controller (0)
, currentViewIndex (0)
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
	if (controller)
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
			#if 1
			if (getFrame ())
				getFrame ()->getAnimator ()->removeAnimation (this, "UIViewSwitchContainer::setCurrentViewIndex");
			CView* oldView = getView (0);
			if (oldView && getFrame ())
			{
				getFrame ()->getAnimator ()->addAnimation (this, "UIViewSwitchContainer::setCurrentViewIndex", new Animation::ExchangeViewAnimation (oldView, view, Animation::ExchangeViewAnimation::kAlphaValueFade), new Animation::LinearTimingFunction (120));
			}
			else
			{
				removeAll ();
				addView (view);
			}
			#else
			CViewContainer::removeAll ();
			CViewContainer::addView (view);
			#endif
			currentViewIndex = viewIndex;
			invalid ();
		}
	}
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
		CViewContainer::removeAll ();
		return result;
	}
	return false;
}

//-----------------------------------------------------------------------------
UIDescriptionViewSwitchController::UIDescriptionViewSwitchController (UIViewSwitchContainer* viewSwitch, UIDescription* uiDescription, IController* uiController)
: IViewSwitchController (viewSwitch)
, uiDescription (uiDescription)
, uiController (uiController)
, switchControlTag (-1)
, currentIndex (-1)
{
	init ();
}

//-----------------------------------------------------------------------------
CView* UIDescriptionViewSwitchController::createViewForIndex (int32_t index)
{
	if (index < (int32_t)templateNames.size ())
	{
		return uiDescription->createView (templateNames[index].c_str (), uiController);
	}
	return 0;
}

//-----------------------------------------------------------------------------
static CControl* findControlTag (CViewContainer* parent, int32_t tag)
{
	CControl* result = 0;
	for (int32_t i = 0; i < parent->getNbViews (); i++)
	{
		CView* view = parent->getView (i);
		CControl* control = dynamic_cast<CControl*> (view);
		if (control)
		{
			if (control->getTag () == tag)
				result = control;
		}
		else
		{
			CViewContainer* container = dynamic_cast<CViewContainer*> (view);
			if (container)
				result = findControlTag (container, tag);
		}
		if (result)
			break;
	}
	return result;
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::switchContainerAttached ()
{
	if (switchControlTag != -1)
	{
		// find the switch Control
		CControl* control = findControlTag (viewSwitch->getFrame (), switchControlTag);
		if (control)
		{
			control->addListener (this);
			valueChanged (control);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescriptionViewSwitchController::valueChanged (CControl* pControl)
{
	float value = pControl->getValue ();
	float min = pControl->getMin ();
	float max = pControl->getMax ();
	float norm = (value - min) / (max - min);
	int32_t index = std::min<int32_t> ((int32_t)(norm * (float)templateNames.size ()), templateNames.size ()-1);
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

/// @cond ignore
//-----------------------------------------------------------------------------
class UIViewSwitchContainerCreator : public IViewCreator
{
public:
	UIViewSwitchContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "UIViewSwitchContainer"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const 
	{
		UIViewSwitchContainer* vsc = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		new UIDescriptionViewSwitchController (vsc, dynamic_cast<UIDescription*> (description), description->getController ());
		return vsc;
	}

	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		UIViewSwitchContainer* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
		if (!viewSwitch)
			return false;
		const std::string* attr = attributes.getAttributeValue ("template-names");
		if (attr)
		{
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				controller->setTemplateNames (attr->c_str ());
			}
		}
		attr = attributes.getAttributeValue ("template-switch-control");
		if (attr)
		{
			rememberAttributeValueString (view, "template-switch-control", *attr);
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				int32_t tag = description->getTagForName (attr->c_str ());
				controller->setSwitchControlTag (tag);
			}
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("template-names");
		attributeNames.push_back ("template-switch-control");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "template-names") return kStringType;
		if (attributeName == "template-switch-control") return kTagType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		UIViewSwitchContainer* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
		if (!viewSwitch)
			return false;
		if (attributeName == "template-names")
		{
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				controller->getTemplateNames (stringValue);
				return true;
			}
		}
		if (attributeName == "template-switch-control")
		{
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				if (getRememberedAttributeValueString (view, "template-switch-control", stringValue))
					return true;
				UTF8StringPtr controlTag = desc->lookupControlTagName (controller->getSwitchControlTag ());
				if (controlTag)
				{
					stringValue = controlTag;
					return true;
				}
				return true;
			}
		}
		return false;
	}
};
UIViewSwitchContainerCreator __gUIViewSwitchContainerCreator;

/// @endcond

} // namespace
