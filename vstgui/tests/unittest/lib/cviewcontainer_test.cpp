// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cframe.h"
#include "../../../lib/iviewlistener.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/dragging.h"
#include "../../../lib/events.h"
#include "../unittests.h"
#include "eventhelpers.h"
#include <vector>

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

class MouseEventCheckView : public CView, public DropTargetAdapter
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

	SharedPointer<IDropTarget> getDropTarget () override { return this; }
	DragOperation onDragEnter (DragEventData data) override
	{
		onDragEnterCalled = true;
		return DragOperation::None;
	}

	void onDragLeave (DragEventData data) override
	{
		onDragLeaveCalled = true;
	}

	DragOperation onDragMove (DragEventData data) override
	{
		onDragMoveCalled = true;
		return DragOperation::None;
	}

	void onMouseWheelEvent (MouseWheelEvent& event) override
	{
		onWheelCalled = true;
		event.consumed = true;
	}
	
 };

} // anonymous

TEST_SUITE_SETUP (CViewContainerTest)
{
	SharedPointer<CViewContainer> container = makeOwned<CViewContainer> (CRect (0, 0, 200, 200));
	TEST_SUITE_SET_STORAGE (SharedPointer<CViewContainer>, container);
}

TEST_SUITE_TEARDOWN (CViewContainerTest)
{
	TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>) = nullptr;
}

TEST_CASE (CViewContainerTest, ChangeViewZOrder)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);
	CView* view1 = new CView (CRect (0, 0, 10, 10));
	CView* view2 = new CView (CRect (0, 0, 10, 10));
	CView* view3 = new CView (CRect (0, 0, 10, 10));
	container->addView (view1);
	container->addView (view2);
	container->addView (view3);
	EXPECT(container->changeViewZOrder (view3, 0));
	EXPECT(container->getView (0) == view3);
	EXPECT(container->getView (1) == view1);
	EXPECT(container->getView (2) == view2);
	EXPECT(container->getView (3) == nullptr);
	EXPECT(container->changeViewZOrder (view3, 4) == false);
	EXPECT(container->changeViewZOrder (view3, 0));
	EXPECT(container->changeViewZOrder (view3, 1));
	EXPECT(container->getView (0) == view1);
	EXPECT(container->getView (1) == view3);
	EXPECT(container->getView (2) == view2);
}

TEST_CASE (CViewContainerTest, AddView)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CView* view = new CView (CRect (0, 0, 10, 10));
	CView* view2 = new CView (CRect (0, 0, 10, 10));
	
	EXPECT(container->addView (view));
	EXPECT(container->addView (view2));
	
	EXPECT(container->isChild (view));
	EXPECT(container->isChild (view2));
}

TEST_CASE (CViewContainerTest, AddView2)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto v = new TestView1 ();
	CRect r (30, 40, 50, 60);
	EXPECT(container->addView (v, r, false));
	EXPECT(v->getMouseEnabled () == false);
	EXPECT(v->getMouseableArea () == r);
}

TEST_CASE (CViewContainerTest, AddViewTwice)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CView* view = new CView (CRect (0, 0, 10, 10));
	EXPECT (container->addView (view));
	EXPECT_EXCEPTION (container->addView (view), "view is already added to a container view");
}

TEST_CASE (CViewContainerTest, AddViewToTwoContainer)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CView* view = new CView (CRect (0, 0, 10, 10));
	EXPECT (container->addView (view));
	auto c2 = owned (new CViewContainer (CRect ()));
	EXPECT_EXCEPTION (c2->addView (view), "view is already added to a container view");
}

TEST_CASE (CViewContainerTest, AddViewBeforeOtherView)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CView* view = new CView (CRect (0, 0, 10, 10));
	CView* view2 = new CView (CRect (0, 0, 10, 10));

	EXPECT (container->addView (view));
	EXPECT (container->addView (view2, view));

	EXPECT (container->getView (0) == view2)
	EXPECT (container->getView (1) == view)
}

TEST_CASE (CViewContainerTest, RemoveView)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto view = makeOwned<CView> (CRect (0, 0, 10, 10));
	CView* view2 = new CView (CRect (0, 0, 10, 10));

	container->addView (view);
	container->addView (view2);
	container->removeView (view, false);

	EXPECT (container->isChild (view) == false)
	EXPECT (container->isChild (view2))
}

TEST_CASE (CViewContainerTest, RemoveAllViews)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto view = makeOwned<CView> (CRect (0, 0, 10, 10));
	auto view2 = makeOwned<CView> (CRect (0, 0, 10, 10));

	container->addView (view);
	container->addView (view2);
	container->removeAll (false);

	EXPECT (container->isChild (view) == false)
	EXPECT (container->isChild (view2) == false)
	EXPECT (container->hasChildren () == false)
}

TEST_CASE (CViewContainerTest, AdvanceNextFocusView)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CFrame* frame = new CFrame (CRect (0, 0, 10, 10), nullptr);
	frame->onActivate (true);
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

	EXPECT (container->advanceNextFocusView (nullptr, true) == true)
	EXPECT (frame->getFocusView () == view3)
	EXPECT (container->advanceNextFocusView (view3) == false)
	frame->setFocusView (nullptr);
	EXPECT (container->advanceNextFocusView (nullptr) == true)
	EXPECT (frame->getFocusView () == view1)
	frame->close ();
}

TEST_CASE (CViewContainerTest, AutoSizeAll)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CView* view = new CView (container->getViewSize ());
	view->setAutosizeFlags (kAutosizeAll);
	container->addView (view);
	container->setAutosizingEnabled (true);
	EXPECT (container->getAutosizingEnabled ());
	container->setViewSize (CRect (0, 0, 500, 500));
	EXPECT (view->getViewSize ().left == 0)
	EXPECT (view->getViewSize ().top == 0)
	EXPECT (view->getViewSize ().right == 500)
	EXPECT (view->getViewSize ().bottom == 500)
	container->setAutosizingEnabled (false);
	EXPECT (container->getAutosizingEnabled () == false);
}

TEST_CASE (CViewContainerTest, SizeToFit)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CRect r (10, 10, 20, 20);
	CView* view = new CView (r);
	container->addView (view);
	container->sizeToFit ();
	EXPECT (container->getViewSize ().right == 30)
	EXPECT (container->getViewSize ().bottom == 30)
}

TEST_CASE (CViewContainerTest, GetViewAt)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CRect r (10, 10, 20, 20);
	CView* view = new CView (r);
	container->addView (view);
	EXPECT (view == container->getViewAt (r.getTopLeft ()));
	EXPECT (nullptr == container->getViewAt (CPoint (0, 0)));
}

TEST_CASE (CViewContainerTest, GetViewAtDeep)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CRect r (10, 10, 20, 20);
	CViewContainer* container2 = new CViewContainer (r);
	container->addView (container2);
	CRect r2 (2, 2, 4, 4);
	CView* view = new CView (r2);
	container2->addView (view);
	EXPECT (container->getViewAt (CPoint (12, 12)) == nullptr);
	EXPECT (container->getViewAt (CPoint (12, 12), GetViewOptions (GetViewOptions::kDeep)) == view);
	EXPECT (container->getViewAt (CPoint (11, 11), GetViewOptions (GetViewOptions::kDeep)) ==
	        nullptr);
	EXPECT (container->getViewAt (
	            CPoint (11, 11),
	            GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kIncludeViewContainer)) ==
	        container2);
}

TEST_CASE (CViewContainerTest, Listener)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	TestViewContainerListener listener;
	container->registerViewContainerListener (&listener);
	auto view = new CView (CRect (0, 0, 0, 0));
	container->addView (view);
	EXPECT (listener.viewAddedCalled == true);
	container->removeView (view, false);
	EXPECT (listener.viewRemovedCalled == true);
	auto view2 = new CView (CRect (0, 0, 0, 0));
	container->addView (view);
	container->addView (view2);
	container->changeViewZOrder (view2, 0);
	EXPECT (listener.viewZOrderChangedCalled == true);
	container->setTransform (CGraphicsTransform ().translate (1., 1.));
	EXPECT (listener.transformChangedCalled == true);
	container->unregisterViewContainerListener (&listener);
}

TEST_CASE (CViewContainerTest, BackgroundColor)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	container->setBackgroundColor (kGreenCColor);
	EXPECT (container->getBackgroundColor () == kGreenCColor);
	container->setBackgroundColorDrawStyle (kDrawFilledAndStroked);
	EXPECT (container->getBackgroundColorDrawStyle () == kDrawFilledAndStroked);
	container->setBackgroundColorDrawStyle (kDrawFilled);
	EXPECT (container->getBackgroundColorDrawStyle () == kDrawFilled);
	container->setBackgroundColorDrawStyle (kDrawStroked);
	EXPECT (container->getBackgroundColorDrawStyle () == kDrawStroked);
}

TEST_CASE (CViewContainerTest, BackgroundOffset)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	container->setBackgroundOffset (CPoint (10, 10));
	EXPECT (container->getBackgroundOffset () == CPoint (10, 10));
}

TEST_CASE (CViewContainerTest, GetChildViewsOfType)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	container->addView (new TestView1 ());
	container->addView (new TestView2 ());
	std::vector<TestView1*> r;
	container->getChildViewsOfType<TestView1> (r);
	EXPECT (r.size () == 1);
	EXPECT (r[0] == container->getView (0));
	std::vector<TestView2*> r2;
	container->getChildViewsOfType<TestView2> (r2);
	EXPECT (r2.size () == 1);
	EXPECT (r2[0] == container->getView (1));
	auto c2 = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	c2->addView (container);
	r.clear ();
	c2->getChildViewsOfType<TestView1> (r);
	EXPECT (r.size () == 0);
	c2->getChildViewsOfType<TestView1> (r, true);
	EXPECT (r.size () == 1);
	c2->removeView (container, false);
}

TEST_CASE (CViewContainerTest, Iterator)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto v1 = new TestView1 ();
	auto v2 = new TestView2 ();
	container->addView (v1);
	container->addView (v2);
	ViewIterator it (container);
	EXPECT (*it == v1);
	++it;
	EXPECT (*it == v2);
	--it;
	EXPECT (*it == v1);
	auto it2 = it++;
	EXPECT (*it2 == v1);
	EXPECT (*it == v2);
}

TEST_CASE (CViewContainerTest, ReverseIterator)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto v1 = new TestView1 ();
	auto v2 = new TestView2 ();
	container->addView (v1);
	container->addView (v2);
	ReverseViewIterator it (container);
	EXPECT (*it == v2);
	++it;
	EXPECT (*it == v1);
	--it;
	EXPECT (*it == v2);
	auto it2 = it++;
	EXPECT (*it2 == v2);
	EXPECT (*it == v1);
	++it;
	EXPECT (*it == nullptr);
}

TEST_CASE (CViewContainerTest, MouseEventsInEmptyContainer)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (container, {}), EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (container, {}), EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (container, {}), EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseCancelEvent (container), EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseWheelEvent (container, {}, 1., 0.), EventConsumeState::NotHandled);
}

TEST_CASE (CViewContainerTest, MouseEvents)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

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

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (v1->mouseDownCalled);
	EXPECT_FALSE (v2->mouseDownCalled);
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (v1->mouseMovedCalled);
	EXPECT_FALSE (v2->mouseMovedCalled);
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (v1->mouseUpCalled);
	EXPECT_FALSE (v2->mouseUpCalled);

	EXPECT_EQ (dispatchMouseWheelEvent (container, {60., 10.}, 0.5, 0.),
	           EventConsumeState::Handled);
	EXPECT_FALSE (v1->onWheelCalled);
	EXPECT_TRUE (v2->onWheelCalled);
}

TEST_CASE (CViewContainerTest, MouseCancel)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto v1 = new MouseEventCheckView ();
	CRect r1 (0, 0, 50, 50);
	v1->setViewSize (r1);
	v1->setMouseableArea (r1);
	container->addView (v1);

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (v1->mouseDownCalled);
	EXPECT_EQ (dispatchMouseCancelEvent (container), EventConsumeState::Handled);
	EXPECT_TRUE (v1->mouseCancelCalled);
}

TEST_CASE (CViewContainerTest, DragEvents)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

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

	DragEventData data;
	data.drag = nullptr;
	data.pos = CPoint (10, 10);
	data.modifiers.clear ();

	auto dropTarget = container->getDropTarget ();
	dropTarget->onDragEnter (data);
	EXPECT (v1->onDragEnterCalled);
	EXPECT (v2->onDragEnterCalled == false);
	dropTarget->onDragMove (data);
	EXPECT (v1->onDragMoveCalled);
	EXPECT (v2->onDragMoveCalled == false);
	dropTarget->onDragLeave (data);
	EXPECT (v1->onDragLeaveCalled);
	EXPECT (v2->onDragLeaveCalled == false);
}

TEST_CASE (CViewContainerTest, DragMoveBetweenTwoViews)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

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

	DragEventData data;
	data.drag = nullptr;
	data.pos = CPoint (10, 10);
	data.modifiers.clear ();

	auto dropTarget = container->getDropTarget ();
	dropTarget->onDragEnter (data);
	EXPECT (v1->onDragEnterCalled);
	EXPECT (v2->onDragEnterCalled == false);
	data.pos (60, 10);
	dropTarget->onDragMove (data);
	EXPECT (v1->onDragLeaveCalled);
	EXPECT (v2->onDragEnterCalled);
}

TEST_CASE (CViewContainerTest,
           MouseDownOnTransparentViewWithoutMouseSupportHidingSubviewWithMouseSupport)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	CRect r1 (0, 0, 50, 50);
	auto v1 = new MouseEventCheckView ();
	auto v2 = new CView (r1);
	v2->setTransparency (false);
	v1->setViewSize (r1);
	v1->setMouseableArea (r1);
	container->addView (v1);
	container->addView (v2);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (v1->mouseDownCalled);
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (v1->mouseMovedCalled);
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (container, {10., 10.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (v1->mouseUpCalled);
}

TEST_CASE (CViewContainerTest, GetViewsAt)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

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
	container->getViewsAt (CPoint (0, 0), views,
	                       GetViewOptions (GetViewOptions::kIncludeViewContainer));
	EXPECT (views.size () == 3);
	views.clear ();
	container->getViewsAt (CPoint (0, 0), views, GetViewOptions (GetViewOptions::kDeep));
	EXPECT (views.size () == 3);
	views.clear ();
	container->getViewsAt (
	    CPoint (0, 0), views,
	    GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kIncludeInvisible));
	EXPECT (views.size () == 4);
}

TEST_CASE (CViewContainerTest, GetContainerAt)
{
	auto& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);

	auto c1 = new CViewContainer (CRect (0, 0, 10, 10));
	auto c2 = new CViewContainer (CRect (0, 0, 10, 10));
	auto c3 = new CViewContainer (CRect (0, 0, 10, 10));
	c2->setMouseEnabled (false);
	c3->setVisible (false);
	c2->addView (c3);
	container->addView (c1);
	container->addView (c2);
	auto res = container->getContainerAt (CPoint (0, 0), GetViewOptions (GetViewOptions::kNone));
	EXPECT (res == container);
	res = container->getContainerAt (CPoint (0, 0), GetViewOptions (GetViewOptions::kDeep));
	EXPECT (res == c2);
	res = container->getContainerAt (
	    CPoint (0, 0), GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kIncludeInvisible));
	EXPECT (res == c3);
	res = container->getContainerAt (
	    CPoint (0, 0), GetViewOptions (GetViewOptions::kDeep | GetViewOptions::kMouseEnabled));
	EXPECT (res == c1);
}

} // namespaces
