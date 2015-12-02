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
