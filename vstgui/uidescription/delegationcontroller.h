// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "icontroller.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class DelegationController : public IController
{
public:
	explicit DelegationController (IController* controller) : controller (controller) {}

	// IControlListener
	void valueChanged (CControl* pControl) override { controller->valueChanged (pControl); }
	int32_t controlModifierClicked (CControl* pControl, CButtonState button) override { return controller->controlModifierClicked (pControl, button); }
	void controlBeginEdit (CControl* pControl) override { controller->controlBeginEdit (pControl); }
	void controlEndEdit (CControl* pControl) override { controller->controlEndEdit (pControl); }
	void controlTagWillChange (VSTGUI::CControl* pControl) override { controller->controlTagWillChange (pControl); }
	void controlTagDidChange (VSTGUI::CControl* pControl) override { controller->controlTagDidChange (pControl); }
	// IController
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override { return controller->getTagForName (name, registeredTag); }
	IControlListener* getControlListener (UTF8StringPtr name) override { return controller->getControlListener (name); }
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override { return controller->createView (attributes, description); }
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override { return controller->verifyView (view, attributes, description); }
	IController* createSubController (IdStringPtr name, const IUIDescription* description) override { return controller->createSubController (name, description); }
protected:
	IController* controller;
};

} // VSTGUI
