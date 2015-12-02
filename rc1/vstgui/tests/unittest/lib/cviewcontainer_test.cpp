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

#include "../../../lib/cframe.h"
#include "../../../lib/iviewlistener.h"
#include "../unittests.h"

namespace VSTGUI {

namespace {

class TestViewContainerListener : public IViewContainerListener
{
public:
	void viewContainerViewAdded (CViewContainer* container, CView* view) override
	{ viewAddedCalled = true; }
	void viewContainerViewRemoved (CViewContainer* container, CView* view) override
	{ viewRemovedCalled = true; }
	void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) override
	{ viewZOrderChangedCalled = true; }
	void viewContainerTransformChanged (CViewContainer* container) override
	{ transformChangedCalled = true; }

	bool viewAddedCalled {false};
	bool viewRemovedCalled {false};
	bool viewZOrderChangedCalled {false};
	bool transformChangedCalled {false};
};

class TestView1 : public CView
{
public:
	TestView1 () : CView (CRect (0, 0, 10, 10)) {}
};

class TestView2 : public CView
{
public:
	TestView2 () : CView (CRect (10, 10, 20, 20)) {}
};

class MouseEventCheckView : public CView
{
public:
	MouseEventCheckView () : CView (CRect ()) {}

	bool mouseDownCalled {false};
	bool mouseMovedCalled {false};
	bool mouseUpCalled {false};
	bool mouseCancelCalled {false};
	bool onDragEnterCalled {false};
	bool onDragLeaveCalled {false};
	bool onDragMoveCalled {false};
	bool onWheelCalled {false};

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override
	{
		mouseDownCalled = true;
		return kMouseEventHandled;
	}
	
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override
	{
		mouseMovedCalled = true;
		return kMouseEventHandled;
	}
	
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override
	{
		mouseUpCalled = true;
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseCancel () override
	{
		mouseCancelCalled = true;
		return kMouseEventHandled;
	}

	void onDragEnter (IDataPackage* drag, const CPoint& where) override
	{
		onDragEnterCalled = true;
		CView::onDragEnter (drag, where);
	}

	void onDragLeave (IDataPackage* drag, const CPoint& where) override
	{
		onDragLeaveCalled = true;
		CView::onDragLeave (drag, where);
	}

	void onDragMove (IDataPackage* drag, const CPoint& where) override
	{
		onDragMoveCalled = true;
		CView::onDragMove (drag, where);
	}

	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons) override
	{
		onWheelCalled = true;
		return true;
	}
	
 };

} // anonymous

TESTCASE(CViewContainerTest,

	static CViewContainer* container = nullptr;

	SETUP(
		container = new CViewContainer (CRect (0, 0, 200, 200));
	);

	TEARDOWN(
		container->forget();
		container = 0;
	);
	
	TEST(changeViewZOrder,
		CView* view1 = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		CView* view3 = new CView (CRect (0, 0, 10, 10));
		container->addView (view1);
		container->addView (view2);
		container->addView (view3);
		EXPECT(container->changeViewZOrder (view3, 1));
		EXPECT(container->getView (0) == view1);
		EXPECT(container->getView (1) == view3);
		EXPECT(container->getView (2) == view2);
		EXPECT(container->getView (3) == nullptr);
		EXPECT(container->changeViewZOrder (view3, 4) == false);
	);

	TEST(addView,
		CView* view = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		
		EXPECT(container->addView (view));
		EXPECT(container->addView (view2));
		
		EXPECT(container->isChild (view));
		EXPECT(container->isChild (view2));
	);

	TEST(addView2,
		auto v = new TestView1 ();
		CRect r (30, 40, 50, 60);
		EXPECT(container->addView (v, r, false));
		EXPECT(v->getMouseEnabled () == false);
		EXPECT(v->getMouseableArea () == r);
	);

	TEST(addViewTwice,
		CView* view = new CView (CRect (0, 0, 10, 10));
		EXPECT(container->addView (view));
		EXPECT_EXCEPTION(container->addView (view), "view is already added to a container view");
	);

	TEST(addViewToTwoContainer,
		CView* view = new CView (CRect (0, 0, 10, 10));
		EXPECT(container->addView (view));
		auto c2 = owned (new CViewContainer (CRect ()));
		EXPECT_EXCEPTION(c2->addView (view), "view is already added to a container view");
	);

	TEST(addViewBeforeOtherView,
		CView* view = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		
		EXPECT(container->addView (view));
		EXPECT(container->addView (view2, view));
		
		EXPECT(container->getView (0) == view2)
		EXPECT(container->getView (1) == view)
	);

	TEST(removeView,
		OwningPointer<CView> view = new CView (CRect (0, 0, 10, 10));
		CView* view2 = new CView (CRect (0, 0, 10, 10));
		
		container->addView (view);
		container->addView (view2);
		container->removeView (view, false);
		
		EXPECT(container->isChild (view) == false)
		EXPECT(container->isChild (view2))
	);

	TEST(removeAllViews,
		OwningPointer<CView> view = new CView (CRect (0, 0, 10, 10));
		OwningPointer<CView> view2 = new CView (CRect (0, 0, 10, 10));
		
		container->addView (view);
		container->addView (view2);
		container->removeAll (false);
		
		EXPECT(container->isChild (view) == false)
		EXPECT(container->isChild (view2) == false)
		EXPECT(container->hasChildren () == false)
	);
	
	TEST(advanceNextFocusView,
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

		EXPECT(container->advanceNextFocusView (0, true) == true)
		EXPECT(frame->getFocusView () == view3)
		EXPECT(container->advanceNextFocusView (view3) == false)
		frame->setFocusView (0);
		EXPECT(container->advanceNextFocusView (0) == true)
		EXPECT(frame->getFocusView () == view1)
		frame->close ();
	);

	TEST(autoSizeAll,
		CView* view = new CView (container->getViewSize ());
		view->setAutosizeFlags (kAutosizeAll);
		container->addView (view);
		container->setAutosizingEnabled (true);
		EXPECT(container->getAutosizingEnabled ());
		container->setViewSize (CRect (0, 0, 500, 500));
		EXPECT(view->getViewSize ().left == 0)
		EXPECT(view->getViewSize ().top == 0)
		EXPECT(view->getViewSize ().right == 500)
		EXPECT(view->getViewSize ().bottom == 500)
		container->setAutosizingEnabled (false);
		EXPECT(container->getAutosizingEnabled () == false);
	);

	TEST(sizeToFit,
		CRect r (10, 10, 20, 20);
		CView* view = new CView (r);
		container->addView (view);
		container->sizeToFit ();
		EXPECT(container->getViewSize ().right == 30)
		EXPECT(container->getViewSize ().bottom == 30)
	);

	TEST(getViewAt,
		CRect r (10, 10, 20, 20);
		CView* view = new CView (r);
		container->addView (view);
		EXPECT(view == container->getViewAt (r.getTopLeft ()));
		EXPECT(nullptr == container->getViewAt (CPoint (0, 0)));
	);

	TEST(getViewAtDeep,
		CRect r (10, 10, 20, 20);
		CViewContainer* container2 = new CViewContainer (r);
		container->addView (container2);
		CRect r2 (2, 2, 4, 4);
		CView* view = new CView (r2);
		container2->addView (view);
		EXPECT(container->getViewAt (CPoint (12, 12)) == nullptr);
		EXPECT(container->getViewAt (CPoint (12, 12), GetViewOptions (GetViewOptions::kDeep)) == view);
		EXPECT(container->getViewAt (CPoint (11, 11), GetViewOptions (GetViewOptions::kDeep)) == nullptr);
		EXPECT(container->getViewAt (CPoint (11, 11), GetViewOptions (GetViewOptions::kDeep|GetViewOptions::kIncludeViewContainer)) == container2);
	);

	TEST(listener,
		 TestViewContainerListener listener;
		 container->registerViewContainerListener (&listener);
		 auto view = new CView (CRect (0, 0, 0, 0));
		 container->addView (view);
		 EXPECT(listener.viewAddedCalled == true);
		 container->removeView (view, false);
		 EXPECT(listener.viewRemovedCalled == true);
		 auto view2 = new CView (CRect (0, 0, 0, 0));
		 container->addView (view);
		 container->addView (view2);
		 container->changeViewZOrder (view2, 0);
		 EXPECT(listener.viewZOrderChangedCalled == true);
		 container->setTransform(CGraphicsTransform ().translate(1., 1.));
		 EXPECT(listener.transformChangedCalled == true);
		 container->unregisterViewContainerListener (&listener);
	);
	
	TEST(backgroundColor,
		container->setBackgroundColor (kGreenCColor);
		EXPECT(container->getBackgroundColor () == kGreenCColor);
		container->setBackgroundColorDrawStyle (kDrawFilledAndStroked);
		EXPECT(container->getBackgroundColorDrawStyle () == kDrawFilledAndStroked);
		container->setBackgroundColorDrawStyle (kDrawFilled);
		EXPECT(container->getBackgroundColorDrawStyle () == kDrawFilled);
		container->setBackgroundColorDrawStyle (kDrawStroked);
		EXPECT(container->getBackgroundColorDrawStyle () == kDrawStroked);
	);

	TEST(backgroundOffset,
		container->setBackgroundOffset (CPoint (10, 10));
		EXPECT(container->getBackgroundOffset () == CPoint (10, 10));
	);

	TEST(getChildViewsOfType,
		container->addView (new TestView1 ());
		container->addView (new TestView2 ());
		std::vector<TestView1*> r;
		container->getChildViewsOfType<TestView1> (r);
		EXPECT(r.size () == 1);
		EXPECT(r[0] == container->getView (0));
		std::vector<TestView2*> r2;
		container->getChildViewsOfType<TestView2> (r2);
		EXPECT(r2.size () == 1);
		EXPECT(r2[0] == container->getView (1));
		auto c2 = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		c2->addView (container);
		r.clear ();
		c2->getChildViewsOfType<TestView1> (r);
		EXPECT(r.size () == 0);
		c2->getChildViewsOfType<TestView1> (r, true);
		EXPECT(r.size () == 1);
		c2->removeView (container, false);
	);
	
	TEST(iterator,
		auto v1 = new TestView1 ();
		auto v2 = new TestView2 ();
		container->addView (v1);
		container->addView (v2);
		ViewIterator it (container);
		EXPECT(*it == v1);
		++it;
		EXPECT(*it == v2);
		--it;
		EXPECT(*it == v1);
		auto it2 = it++;
		EXPECT(*it2 == v1);
		EXPECT(*it == v2);
	);

	TEST(reverseIterator,
		auto v1 = new TestView1 ();
		auto v2 = new TestView2 ();
		container->addView (v1);
		container->addView (v2);
		ReverseViewIterator it (container);
		EXPECT(*it == v2);
		++it;
		EXPECT(*it == v1);
		--it;
		EXPECT(*it == v2);
		auto it2 = it++;
		EXPECT(*it2 == v2);
		EXPECT(*it == v1);
		++it;
		EXPECT(*it == nullptr);
	);
	
	TEST(mouseEventsInEmptyContainer,
		CPoint p;
		EXPECT(container->onMouseDown (p, kLButton) == kMouseEventNotHandled);
		EXPECT(container->onMouseMoved (p, kLButton) == kMouseEventNotHandled);
		EXPECT(container->onMouseUp (p, kLButton) == kMouseEventNotHandled);
		EXPECT(container->onMouseCancel () == kMouseEventHandled);
		EXPECT(container->onWheel (p, kMouseWheelAxisX, 1.f, 0) == false);
		EXPECT(container->onWheel (p, 1.f, 0) == false);
		EXPECT(container->onDrop (nullptr, p) == false);
	);

	TEST(mouseEvents,
		auto v1 = new MouseEventCheckView ();
		auto v2 = new MouseEventCheckView ();
		CRect r1 (0, 0, 50, 50);
		CRect r2 (50, 0, 100, 50);
		v1->setViewSize (r1);
		v1->setMouseableArea (r1);
		v2->setViewSize (r2);
		v2->setMouseableArea (r2);
		container->addView (v1);
		container->addView (v2);
		
		CPoint p (10, 10);
		EXPECT(container->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT(v1->mouseDownCalled);
		EXPECT(v2->mouseDownCalled == false);
		EXPECT(container->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT(v1->mouseMovedCalled);
		EXPECT(v2->mouseMovedCalled == false);
		EXPECT(container->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT(v1->mouseUpCalled);
		EXPECT(v2->mouseUpCalled == false);
		
		p (60, 10);
		EXPECT(container->onWheel (p, 0.5f, 0));
		EXPECT(v1->onWheelCalled == false);
		EXPECT(v2->onWheelCalled);
	);

	TEST(mouseCancel,
		auto v1 = new MouseEventCheckView ();
		CRect r1 (0, 0, 50, 50);
		v1->setViewSize (r1);
		v1->setMouseableArea (r1);
		container->addView (v1);
		
		CPoint p (10, 10);
		EXPECT(container->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT(v1->mouseDownCalled);
		container->onMouseCancel ();
		EXPECT(v1->mouseCancelCalled);
	);

	TEST(dragEvents,
		auto v1 = new MouseEventCheckView ();
		auto v2 = new MouseEventCheckView ();
		CRect r1 (0, 0, 50, 50);
		CRect r2 (50, 0, 100, 50);
		v1->setViewSize (r1);
		v1->setMouseableArea (r1);
		v2->setViewSize (r2);
		v2->setMouseableArea (r2);
		container->addView (v1);
		container->addView (v2);

		CPoint p (10, 10);
		container->onDragEnter (nullptr, p);
		EXPECT(v1->onDragEnterCalled);
		EXPECT(v2->onDragEnterCalled == false);
		container->onDragMove (nullptr, p);
		EXPECT(v1->onDragMoveCalled);
		EXPECT(v2->onDragMoveCalled == false);
		container->onDragLeave (nullptr, p);
		EXPECT(v1->onDragLeaveCalled);
		EXPECT(v2->onDragLeaveCalled == false);
	);

	TEST(dragMoveBetweenTwoViews,
		auto v1 = new MouseEventCheckView ();
		auto v2 = new MouseEventCheckView ();
		CRect r1 (0, 0, 50, 50);
		CRect r2 (50, 0, 100, 50);
		v1->setViewSize (r1);
		v1->setMouseableArea (r1);
		v2->setViewSize (r2);
		v2->setMouseableArea (r2);
		container->addView (v1);
		container->addView (v2);
		CPoint p (10, 10);
		container->onDragEnter (nullptr, p);
		EXPECT(v1->onDragEnterCalled);
		EXPECT(v2->onDragEnterCalled == false);
		p (60, 10);
		container->onDragMove (nullptr, p);
		EXPECT(v1->onDragLeaveCalled);
		EXPECT(v2->onDragEnterCalled);
	);

	TEST(mouseDownOnTransparentViewWithoutMouseSupportHidingSubviewWithMouseSupport,
		CRect r1 (0, 0, 50, 50);
		auto v1 = new MouseEventCheckView ();
		auto v2 = new CView (r1);
		v2->setTransparency (false);
		v1->setViewSize (r1);
		v1->setMouseableArea (r1);
		container->addView (v1);
		container->addView (v2);
		CPoint p1 (10, 10);
		EXPECT(container->onMouseDown (p1, kLButton) == kMouseEventNotImplemented);
		EXPECT(v1->mouseDownCalled == false);
		EXPECT(container->onMouseMoved (p1, kLButton) == kMouseEventNotHandled);
		EXPECT(container->onMouseUp (p1, kLButton) == kMouseEventNotHandled);
	);
	
	TEST(getViewsAt,
		auto v1 = new TestView1 ();
		auto v2 = new TestView1 ();
		auto c1 = new CViewContainer (CRect (0, 0, 10, 10));
		auto v1c1 = new TestView1 ();
		v1c1->setVisible (false);
		auto v2c1 = new TestView1 ();
		c1->addView (v1c1);
		c1->addView (v2c1);
		v2->setMouseEnabled (false);
		container->addView (v1);
		container->addView (v2);
		container->addView (c1);
		CViewContainer::ViewList views;
		container->getViewsAt (CPoint (0, 0), views, GetViewOptions (GetViewOptions::kNone));
		EXPECT (views.size () == 2);
		views.clear ();
		container->getViewsAt (CPoint (0, 0), views, GetViewOptions (GetViewOptions::kMouseEnabled));
		EXPECT (views.size () == 1);
		views.clear ();
		container->getViewsAt (CPoint (0, 0), views, GetViewOptions (GetViewOptions::kIncludeViewContainer));
		EXPECT (views.size () == 3);
		views.clear ();
		container->getViewsAt (CPoint (0, 0), views, GetViewOptions (GetViewOptions::kDeep));
		EXPECT (views.size () == 3);
		views.clear ();
		container->getViewsAt (CPoint (0, 0), views, GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kIncludeInvisible));
		EXPECT (views.size () == 4);
	);
	
	TEST(getContainerAt,
		auto c1 = new CViewContainer (CRect (0, 0, 10, 10));
		auto c2 = new CViewContainer (CRect (0, 0, 10, 10));
		auto c3 = new CViewContainer (CRect (0, 0, 10, 10));
		c2->setMouseEnabled (false);
		c3->setVisible (false);
		c2->addView (c3);
		container->addView (c1);
		container->addView (c2);
		auto res = container->getContainerAt (CPoint(0, 0), GetViewOptions (GetViewOptions::kNone));
		EXPECT(res == container);
		res = container->getContainerAt (CPoint(0, 0), GetViewOptions (GetViewOptions::kDeep));
		EXPECT(res == c2);
		res = container->getContainerAt (CPoint(0, 0), GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kIncludeInvisible));
		EXPECT(res == c3);
		res = container->getContainerAt (CPoint(0, 0), GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kMouseEnabled));
		EXPECT(res == c1);
	);
	
); // TESTCASE

} // namespaces

