// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/cviewcontainer.h"
#include "../lib/controls/icontrollistener.h"
#include "../lib/vstguifwd.h"
#include "uidescriptionfwd.h"
#include <vector>
#include <string>

namespace VSTGUI {
class IViewSwitchController;

//-----------------------------------------------------------------------------
class UIViewSwitchContainer : public CViewContainer
{
public:
	explicit UIViewSwitchContainer (const CRect& size);
	~UIViewSwitchContainer () noexcept override;

	IViewSwitchController* getController () const { return controller; }
	void setController (IViewSwitchController* controller);	// owns controller if it is a CBaseObject

	void setCurrentViewIndex (int32_t viewIndex);
	int32_t getCurrentViewIndex () const { return currentViewIndex; }

	void setAnimationTime (uint32_t ms);
	uint32_t getAnimationTime () const { return animationTime; }

	enum AnimationStyle {
		kFadeInOut,
		kMoveInOut,
		kPushInOut
	};

	void setAnimationStyle (AnimationStyle style);
	AnimationStyle getAnimationStyle () const { return animationStyle; }

	enum TimingFunction {
		kLinear,
		kEasyIn,
		kEasyOut,
		kEasyInOut,
		kEasy
	};

	void setTimingFunction (TimingFunction t);
	TimingFunction getTimingFunction () const { return timingFunction; }

	bool attached (CView* parent) override;
	bool removed (CView* parent) override;
//-----------------------------------------------------------------------------
	CLASS_METHODS (UIViewSwitchContainer, CViewContainer)
protected:
	IViewSwitchController* controller {nullptr};
	int32_t currentViewIndex {-1};
	uint32_t animationTime {120};
	AnimationStyle animationStyle {kFadeInOut};
	TimingFunction timingFunction {kLinear};
};

//-----------------------------------------------------------------------------
class IViewSwitchController
{
public:
	explicit IViewSwitchController (UIViewSwitchContainer* viewSwitch) : viewSwitch (viewSwitch) {}
	virtual ~IViewSwitchController () noexcept = default;

	void init () { viewSwitch->setController (this); }

	UIViewSwitchContainer* getViewSwitchContainer () const { return viewSwitch; }

	virtual CView* createViewForIndex (int32_t index) = 0;
	virtual void switchContainerAttached () = 0;
	virtual void switchContainerRemoved () = 0;
protected:
	UIViewSwitchContainer* viewSwitch;
};

//-----------------------------------------------------------------------------
class UIDescriptionViewSwitchController : public CBaseObject, public IViewSwitchController, public IControlListener
{
public:
	UIDescriptionViewSwitchController (UIViewSwitchContainer* viewSwitch, const IUIDescription* uiDescription, IController* uiController);

	CView* createViewForIndex (int32_t index) override;
	void switchContainerAttached () override;
	void switchContainerRemoved () override;

	void setTemplateNames (UTF8StringPtr templateNames); // comma separated
	void getTemplateNames (std::string& str); // comma separated

	void setSwitchControlTag (int32_t tag) { switchControlTag = tag; }
	int32_t getSwitchControlTag () const { return switchControlTag; }
protected:
	void valueChanged (CControl* pControl) override;

	const IUIDescription* uiDescription;
	IController* uiController;
	int32_t switchControlTag;
	int32_t currentIndex;
	SharedPointer<CControl> switchControl;
	std::vector<std::string> templateNames;
};

} // VSTGUI
