// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../uidescription/delegationcontroller.h"
#include "../../../uidescription/uiattributes.h"
#include "../unittests.h"

namespace VSTGUI {

namespace {

class Controller : public IController
{
public:
	mutable bool funcCalled {false};

	void valueChanged (CControl* pControl) override { funcCalled = true; }

	int32_t controlModifierClicked (CControl* pControl, CButtonState button) override
	{
		funcCalled = true;
		return 0;
	}

	void controlBeginEdit (CControl* pControl) override { funcCalled = true; }

	void controlEndEdit (CControl* pControl) override { funcCalled = true; }

	void controlTagWillChange (CControl* pControl) override { funcCalled = true; }

	void controlTagDidChange (CControl* pControl) override { funcCalled = true; }

	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override
	{
		funcCalled = true;
		return registeredTag;
	}

	IControlListener* getControlListener (UTF8StringPtr controlTagName) override
	{
		funcCalled = true;
		return this;
	}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		funcCalled = true;
		return nullptr;
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		funcCalled = true;
		return view;
	}

	IController* createSubController (UTF8StringPtr name,
	                                  const IUIDescription* description) override
	{
		funcCalled = true;
		return nullptr;
	}
};

} // anonymous

TEST_CASE (DelegationControllerTest, ValueChanged)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.valueChanged (nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlModifierClicked)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.controlModifierClicked (nullptr, kLButton);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlBeginEdit)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.controlBeginEdit (nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlEndEdit)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.controlEndEdit (nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlTagWillChange)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.controlTagWillChange (nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlTagDidChange)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.controlTagDidChange (nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, GetTagForName)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.getTagForName ("", 0);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, GetControlListener)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.getControlListener ("");
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, CreateView)
{
	Controller myController;
	DelegationController dc (&myController);
	UIAttributes a;
	dc.createView (a, nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, VerifyView)
{
	Controller myController;
	DelegationController dc (&myController);
	UIAttributes a;
	dc.verifyView (nullptr, a, nullptr);
	EXPECT (myController.funcCalled);
}

TEST_CASE (DelegationControllerTest, CreateSubController)
{
	Controller myController;
	DelegationController dc (&myController);
	dc.createSubController ("", nullptr);
	EXPECT (myController.funcCalled);
}

} // VSTGUI
