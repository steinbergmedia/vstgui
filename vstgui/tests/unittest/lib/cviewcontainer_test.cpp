//
//  cviewcontainer_test.cpp
//  vstgui
//
//  Created by Arne Scheffler on 4/1/13.
//
//

#include "../../../lib/cframe.h"
#include "../unittests.h"

namespace VSTGUI {

TESTCASE(CViewContainerTest,

	CViewContainer* container = nullptr;

	SETUP(
		container = new CViewContainer (CRect (0, 0, 200, 200));
	);

	TEARDOWN(
		container->forget();
		container = 0;
	);
	
	TEST(ChangeViewZOrder,
		CView* view1 = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		CView* view3 = new CView (CRect (0, 0, 10, 10));
		container->addView (view1);
		container->addView (view2);
		container->addView (view3);
		container->changeViewZOrder (view3, 1);
		EXPECT (container->getView (0) == view1)
		EXPECT (container->getView (1) == view3)
		EXPECT (container->getView (2) == view2)
	);

	TEST(AddView,
		CView* view = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		
		container->addView (view);
		container->addView (view2);
		
		EXPECT (container->isChild (view))
		EXPECT (container->isChild (view2))
	);

	TEST(AddViewBeforeOtherView,
		CView* view = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		
		container->addView (view);
		container->addView (view2, view);
		
		EXPECT (container->getView (0) == view2)
		EXPECT (container->getView (1) == view)
	);

	TEST(RemoveView,
		OwningPointer<CView> view = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		
		container->addView (view);
		container->addView (view2);
		container->removeView (view, false);
		
		EXPECT (container->isChild (view) == false)
		EXPECT (container->isChild (view2))
	);

	TEST(RemoveAllViews,
		OwningPointer<CView> view = new CView (CRect (0, 0, 10, 10));
		OwningPointer<CView> view2 = new CView (CRect (0, 0, 10, 10));
		
		container->addView (view);
		container->addView (view2);
		container->removeAll (false);
		
		EXPECT (container->isChild (view) == false)
		EXPECT (container->isChild (view2) == false)
		EXPECT (container->hasChildren () == false)
	);
	
	TEST(AdvanceNextFocusView,
		CFrame* frame = new CFrame (CRect (0, 0, 10, 10), 0);
		CView* view1 = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		CView* view3 = new CView (CRect (0, 0, 10, 10));
		view1->setWantsFocus (true);
		view2->setWantsFocus (true);
		view3->setWantsFocus (true);
		
		container->addView (view1);
		container->addView (view2);
		container->addView (view3);
		frame->addView (container);
		container->remember ();
		frame->attached (frame);

		EXPECT (container->advanceNextFocusView (0, true) == true)
		EXPECT (frame->getFocusView () == view3)
		EXPECT (container->advanceNextFocusView (view3) == false)
		frame->setFocusView (0);
		EXPECT (container->advanceNextFocusView (0) == true)
		EXPECT (frame->getFocusView () == view1)
		frame->close ();
	);

	TEST(AutoSizeAll,
		CView* view = new CView (container->getViewSize ());
		view->setAutosizeFlags (kAutosizeAll);
		container->addView (view);
		container->setAutosizingEnabled (true);
		container->setViewSize (CRect (0, 0, 500, 500));
		EXPECT(view->getViewSize ().left == 0)
		EXPECT(view->getViewSize ().top == 0)
		EXPECT(view->getViewSize ().right == 500)
		EXPECT(view->getViewSize ().bottom == 500)
	);

); // TESTCASE

} // namespaces

