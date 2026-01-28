// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../uidescription/delegationcontroller.h"
#include "../../../uidescription/uiattributes.h"
#include "../../../lib/controls/ccontrol.h"
#include "../unittests.h"
#include "uiviewcreator/helpers.h"

namespace VSTGUI {

namespace {

class Controller : public IController,
				   public NonAtomicReferenceCounted
{
public:
	mutable bool funcCalled {false};

	void valueChanged (CControl& pControl) override { funcCalled = true; }

	int32_t controlModifierClicked (CControl& pControl, CButtonState button) override
	{
		funcCalled = true;
		return 0;
	}

	void controlBeginEdit (CControl& pControl) override { funcCalled = true; }

	void controlEndEdit (CControl& pControl) override { funcCalled = true; }

	void controlTagWillChange (CControl& pControl) override { funcCalled = true; }

	void controlTagDidChange (CControl& pControl) override { funcCalled = true; }

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

	SharedPointer<CView> createView (const UIAttributes& attributes,
									 const IUIDescription& description) override
	{
		funcCalled = true;
		return nullptr;
	}

	SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
									 const UIAttributes& attributes,
									 const IUIDescription& description) override
	{
		funcCalled = true;
		return view;
	}

	SharedPointer<IController> createSubController (UTF8StringPtr name,
													const IUIDescription& description) override
	{
		funcCalled = true;
		return nullptr;
	}
};

struct DelegationControllerAdapter : DelegationController,
									 NonAtomicReferenceCounted
{
	using DelegationController::DelegationController;
};

struct DummyControl : CControl
{
	DummyControl () : CControl (CRect {}) {}
	CLASS_METHODS_NOCOPY (DummyControl, CControl);
	void draw (CDrawContext* pContext) override {};
};

DummyControl gDummyControlInstance;
} // anonymous

TEST_CASE (DelegationControllerTest, ValueChanged)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.valueChanged (gDummyControlInstance);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlModifierClicked)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.controlModifierClicked (gDummyControlInstance, kLButton);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlBeginEdit)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.controlBeginEdit (gDummyControlInstance);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlEndEdit)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.controlEndEdit (gDummyControlInstance);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlTagWillChange)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.controlTagWillChange (gDummyControlInstance);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, ControlTagDidChange)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.controlTagDidChange (gDummyControlInstance);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, GetTagForName)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.getTagForName ("", 0);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, GetControlListener)
{
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.getControlListener ("");
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, CreateView)
{
	DummyUIDescription uiDesc;
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	UIAttributes a;
	dc.createView (a, uiDesc);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, VerifyView)
{
	DummyUIDescription uiDesc;
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	UIAttributes a;
	dc.verifyView (nullptr, a, uiDesc);
	EXPECT (myController->funcCalled);
}

TEST_CASE (DelegationControllerTest, CreateSubController)
{
	DummyUIDescription uiDesc;
	auto myController = makeOwned<Controller> ();
	DelegationControllerAdapter dc (myController);
	dc.createSubController ("", uiDesc);
	EXPECT (myController->funcCalled);
}

} // VSTGUI
