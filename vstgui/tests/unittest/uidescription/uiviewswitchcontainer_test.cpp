// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
		viewSwitch->setAnimationTime (0);
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
		viewSwitch->setAnimationTime (0);
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
	    control->valueChanged ();
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
