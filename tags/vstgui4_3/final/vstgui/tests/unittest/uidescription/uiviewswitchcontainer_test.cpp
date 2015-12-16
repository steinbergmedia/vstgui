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
#include "../../../uidescription/uiviewswitchcontainer.h"
#include "uidescriptionadapter.h"
#include "../../../lib/cstring.h"
#include "../../../lib/controls/cbuttons.h"

namespace VSTGUI {

struct View1 : public CView
{
	View1 () : CView (CRect ()) {}
};

struct View2 : public CView
{
	View2 () : CView (CRect ()) {}
};

struct View3 : public CView
{
	View3 () : CView (CRect ()) { setAutosizeFlags(kAutosizeAll); }
};

struct TestUIDescription : public UIDescriptionAdapter
{
	CView* createView (UTF8StringPtr name, IController* controller) const override
	{
		if (UTF8StringView (name) == "v1")
			return new View1 ();
		else if (UTF8StringView (name) == "v2")
			return new View2 ();
		else if (UTF8StringView (name) == "v3")
			return new View3 ();
		return nullptr;
	}
};

TESTCASE(UIViewSwitchControllerTest,

	TEST (switchViaIndex,
		TestUIDescription uiDesc;
		auto rootView = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto viewSwitch = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		auto controller = new UIDescriptionViewSwitchController (viewSwitch, &uiDesc, nullptr);
		controller->setTemplateNames ("v1,v2");
		EXPECT(container->addView (viewSwitch));
		container->attached (rootView);
		EXPECT(viewSwitch->getView (0) == nullptr);
		viewSwitch->setCurrentViewIndex (0);
		EXPECT(dynamic_cast<View1*> (viewSwitch->getView (0)));
		viewSwitch->setCurrentViewIndex (1);
		EXPECT(dynamic_cast<View2*> (viewSwitch->getView (0)));
		container->removed (rootView);
	);

	TEST (switchViaControl,
		TestUIDescription uiDesc;
		auto rootView = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto viewSwitch = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		auto control = new COnOffButton (CRect (0, 0, 0, 0));
		control->setTag (1);
		auto controller = new UIDescriptionViewSwitchController (viewSwitch, &uiDesc, nullptr);
		controller->setTemplateNames ("v1,v2");
		controller->setSwitchControlTag (1);
		EXPECT(container->addView (control));
		EXPECT(container->addView (viewSwitch));
		container->attached (rootView);
		EXPECT(dynamic_cast<View1*> (viewSwitch->getView (0)));
		control->setValue (1.f);
		EXPECT(dynamic_cast<View2*> (viewSwitch->getView (0)));
		container->removed (rootView);
	);

	TEST (autosizeAll,
		TestUIDescription uiDesc;
		auto rootView = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto viewSwitch = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		auto controller = new UIDescriptionViewSwitchController (viewSwitch, &uiDesc, nullptr);
		controller->setTemplateNames ("v3");
		EXPECT(container->addView (viewSwitch));
		container->attached (rootView);
		EXPECT(viewSwitch->getView (0) == nullptr);
		viewSwitch->setCurrentViewIndex (0);
		auto view = viewSwitch->getView (0);
		EXPECT(dynamic_cast<View3*> (view));
		EXPECT(view->getViewSize () == container->getViewSize ());
		container->removed (rootView);
	);

	TEST (noAnimation,
		TestUIDescription uiDesc;
		auto rootView = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto viewSwitch = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		viewSwitch->setAnimationTime (0);
		auto controller = new UIDescriptionViewSwitchController (viewSwitch, &uiDesc, nullptr);
		controller->setTemplateNames ("v1");
		EXPECT(container->addView (viewSwitch));
		container->attached (rootView);
		EXPECT(viewSwitch->getView (0) == nullptr);
		viewSwitch->setCurrentViewIndex (0);
		EXPECT(dynamic_cast<View1*> (viewSwitch->getView (0)));

		container->removed (rootView);
	);

);

} // VSTGUI
