//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

