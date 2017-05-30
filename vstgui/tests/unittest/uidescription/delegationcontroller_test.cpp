// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../uidescription/delegationcontroller.h"
#include "../../../uidescription/uiattributes.h"

namespace VSTGUI {

namespace {

class Controller : public IController
{
public:
	mutable bool funcCalled {false};
	
	void valueChanged (CControl* pControl) override
	{
		funcCalled = true;
	}

	int32_t controlModifierClicked (CControl* pControl, CButtonState button) override
	{
		funcCalled = true;
		return 0;
	}
	
	void controlBeginEdit (CControl* pControl) override
	{
		funcCalled = true;
	}
	
	void controlEndEdit (CControl* pControl) override
	{
		funcCalled = true;
	}
	
	void controlTagWillChange (CControl* pControl) override
	{
		funcCalled = true;
	}
	
	void controlTagDidChange (CControl* pControl) override
	{
		funcCalled = true;
	}
	
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
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		funcCalled = true;
		return view;
	}
	
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override
	{
		funcCalled = true;
		return nullptr;
	}

};

} // anonymous

TESTCASE(DelegationControllerTest,
	
	TEST(valueChanged,
		Controller myController;
		DelegationController dc (&myController);
		dc.valueChanged (nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(controlModifierClicked,
		Controller myController;
		DelegationController dc (&myController);
		dc.controlModifierClicked (nullptr, kLButton);
		EXPECT(myController.funcCalled);
	);

	TEST(controlBeginEdit,
		Controller myController;
		DelegationController dc (&myController);
		dc.controlBeginEdit (nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(controlEndEdit,
		Controller myController;
		DelegationController dc (&myController);
		dc.controlEndEdit (nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(controlTagWillChange,
		Controller myController;
		DelegationController dc (&myController);
		dc.controlTagWillChange (nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(controlTagDidChange,
		Controller myController;
		DelegationController dc (&myController);
		dc.controlTagDidChange (nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(getTagForName,
		Controller myController;
		DelegationController dc (&myController);
		dc.getTagForName ("", 0);
		EXPECT(myController.funcCalled);
	);

	TEST(getControlListener,
		Controller myController;
		DelegationController dc (&myController);
		dc.getControlListener ("");
		EXPECT(myController.funcCalled);
	);

	TEST(createView,
		Controller myController;
		DelegationController dc (&myController);
		UIAttributes a;
		dc.createView (a, nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(verifyView,
		Controller myController;
		DelegationController dc (&myController);
		UIAttributes a;
		dc.verifyView (nullptr, a, nullptr);
		EXPECT(myController.funcCalled);
	);

	TEST(createSubController,
		Controller myController;
		DelegationController dc (&myController);
		dc.createSubController ("", nullptr);
		EXPECT(myController.funcCalled);
	);

);


} // VSTGUI
