// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/ccolor.h"
#include "../../../lib/cframe.h"
#include "../../../lib/events.h"
#include "../unittests.h"
#include "eventhelpers.h"
#include "platform_helper.h"
#include <vector>

namespace VSTGUI {

namespace {

class MouseObserver : public IMouseObserver
{
public:
	void reset ()
	{
		enteredViews.clear ();
		exitedViews.clear ();
	}
	void onMouseEntered (CView* view, CFrame* frame) override { enteredViews.push_back (view); }
	void onMouseExited (CView* view, CFrame* frame) override { exitedViews.push_back (view); }
	void onMouseEvent (MouseEvent&, CFrame*) override {}

	std::vector<CView*> enteredViews;
	std::vector<CView*> exitedViews;
};

bool contains (const std::vector<CView*>& c, CView* view)
{
	auto it = std::find (c.begin (), c.end (), view);
	return it != c.end ();
}

class View : public CView
{
public:
	View () : CView (CRect (0, 0, 10, 10)) {}
	bool onMouseDownCalled {false};
	bool onKeyDownCalled {false};
	bool onKeyUpCalled {false};
	CMouseEventResult onMouseDown (CPoint& p, const CButtonState& buttons) override
	{
		onMouseDownCalled = true;
		return kMouseEventHandled;
	}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	int32_t onKeyDown (VstKeyCode& key) override
	{
		onKeyDownCalled = true;
		return 1;
	}
	int32_t onKeyUp (VstKeyCode& key) override
	{
		onKeyUpCalled = true;
		return 1;
	}
#else
	void onKeyboardEvent (KeyboardEvent& event) override
	{
		if (event.type == EventType::KeyDown)
		{
			onKeyDownCalled = true;
			event.consumed = true;
		}
		else if (event.type == EventType::KeyUp)
		{
			onKeyUpCalled = true;
			event.consumed = true;
		}
	}
#endif
};

class ContainerTestingKeyboardEvents : public CViewContainer
{
public:
	bool onKeyDownCalled {false};
	bool onKeyUpCalled {false};

	ContainerTestingKeyboardEvents () : CViewContainer (CRect (0, 0, 20, 20)) {}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	int32_t onKeyDown (VstKeyCode& key) override
	{
		if (onKeyDownCalled)
			return -1;
		onKeyDownCalled = true;
		return 1;
	}
	int32_t onKeyUp (VstKeyCode& key) override
	{
		if (onKeyUpCalled)
			return -1;
		onKeyUpCalled = true;
		return 1;
	}
#else
	void onKeyboardEvent (KeyboardEvent& event) override
	{
		if (event.type == EventType::KeyDown)
		{
			onKeyDownCalled = true;
			event.consumed = true;
		}
		else if (event.type == EventType::KeyUp)
		{
			onKeyUpCalled = true;
			event.consumed = true;
		}
	}
#endif
};

class KeyboardHook : public IKeyboardHook
{
public:
	bool keyDownCalled {false};
	bool keyUpCalled {false};

	void onKeyboardEvent (KeyboardEvent& event, CFrame* frame) override
	{
		if (event.type == EventType::KeyDown)
		{
			keyDownCalled = true;
		}
		else if (event.type == EventType::KeyUp)
		{
			keyUpCalled = true;
		}
		event.consumed = true;
	}
};

class CollectInvalidRectView : public CView
{
public:
	CRect redrawRect;
	uint32_t callCount {0};

	CollectInvalidRectView () : CView (CRect (0, 0, 10, 10)) {}

	CMouseEventResult onMouseDown (CPoint& p, const CButtonState& buttons) override
	{
		invalidRect (CRect (3, 3, 8, 8));
		invalidRect (CRect (0, 0, 8, 8));
		invalidRect (CRect (1, 1, 2, 2));
		return kMouseEventHandled;
	}

	void drawRect (CDrawContext* c, const CRect& r) override
	{
		++callCount;
		redrawRect = r;
		CView::drawRect (c, r);
	}
};

} // anonymouse

TEST_CASE (CFrameTest, SetZoom)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	EXPECT (frame->setZoom (0.) == false);
	EXPECT (frame->setZoom (2.) == true);
	EXPECT (frame->getViewSize () == CRect (0, 0, 200, 200));
	EXPECT (frame->setZoom (0.5) == true);
	EXPECT (frame->getViewSize () == CRect (0, 0, 50, 50));
	EXPECT (frame->setZoom (1.0) == true);
	EXPECT (frame->getViewSize () == CRect (0, 0, 100, 100));
}

TEST_CASE (CFrameTest, MouseEnterExit)
{
	MouseObserver observer;
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	frame->registerMouseObserver (&observer);
	auto v1 = new View ();
	auto v2 = new View ();
	CRect r2 (10, 10, 20, 20);
	v2->setViewSize (r2);
	v2->setMouseableArea (r2);
	frame->addView (v1);
	frame->addView (v2);
	frame->attached (frame);
	dispatchMouseEvent<MouseMoveEvent> (frame, {30., 30.});
	EXPECT (observer.enteredViews.size () == 0);
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {19., 19.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (contains (observer.enteredViews, v2));
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {9., 9.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (contains (observer.enteredViews, v1));
	EXPECT (observer.exitedViews.size () == 1);
	EXPECT (contains (observer.exitedViews, v2));
	frame->unregisterMouseObserver (&observer);
}

TEST_CASE (CFrameTest, MouseEnterExitInContainer)
{
	MouseObserver observer;
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	frame->registerMouseObserver (&observer);
	auto v1 = new View ();
	auto v2 = new View ();
	CRect r2 (10, 10, 20, 20);
	v2->setViewSize (r2);
	v2->setMouseableArea (r2);
	auto container = new CViewContainer (CRect (0, 0, 80, 80));
	auto container2 = new CViewContainer (CRect (0, 0, 50, 50));
	frame->addView (container);
	container->addView (container2);
	container2->addView (v1);
	container2->addView (v2);
	frame->attached (frame);
	dispatchMouseEvent<MouseMoveEvent> (frame, {90., 90.});
	EXPECT (observer.enteredViews.size () == 0);
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {79., 79.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (contains (observer.enteredViews, container));
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {49., 49.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (contains (observer.enteredViews, container2));
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {19., 19.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (contains (observer.enteredViews, v2));
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {18., 18.});
	EXPECT (observer.enteredViews.size () == 0);
	EXPECT (observer.exitedViews.size () == 0);
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {9., 9.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (contains (observer.enteredViews, v1));
	EXPECT (observer.exitedViews.size () == 1);
	EXPECT (contains (observer.exitedViews, v2));
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {51., 51.});
	EXPECT (observer.enteredViews.size () == 0);
	EXPECT (observer.exitedViews.size () == 2);
	EXPECT (contains (observer.exitedViews, v1));
	EXPECT (contains (observer.exitedViews, container2));
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {81., 81.});
	EXPECT (observer.enteredViews.size () == 0);
	EXPECT (observer.exitedViews.size () == 1);
	EXPECT (contains (observer.exitedViews, container));
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {9., 9.});
	EXPECT (observer.enteredViews.size () == 3);
	EXPECT (observer.exitedViews.size () == 0);
	EXPECT (contains (observer.enteredViews, container));
	EXPECT (contains (observer.enteredViews, container2));
	EXPECT (contains (observer.enteredViews, v1));
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {81., 81.});
	EXPECT (observer.enteredViews.size () == 0);
	EXPECT (observer.exitedViews.size () == 3);
	EXPECT (contains (observer.exitedViews, container));
	EXPECT (contains (observer.exitedViews, container2));
	EXPECT (contains (observer.exitedViews, v1));
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {79., 79.});
	EXPECT (observer.enteredViews.size () == 1);
	EXPECT (observer.exitedViews.size () == 0);
	EXPECT (contains (observer.enteredViews, container));
	observer.reset ();
	dispatchMouseEvent<MouseMoveEvent> (frame, {8., 8.});
	EXPECT (observer.enteredViews.size () == 2);
	EXPECT (observer.exitedViews.size () == 0);
	EXPECT (contains (observer.enteredViews, container2));
	EXPECT (contains (observer.enteredViews, v1));
	observer.reset ();

	frame->unregisterMouseObserver (&observer);
}

TEST_CASE (CFrameTest, MouseMoveInContainer)
{
	struct TestViewContainer : CViewContainer
	{
		using CViewContainer::CViewContainer;
		CPoint mouseMoveEventPos {};
		CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override
		{
			mouseMoveEventPos = where;
			return CViewContainer::onMouseMoved (where, buttons);
		}
	};
	struct TestView : CView
	{
		using CView::CView;

		CPoint mouseMoveEventPos {};
		CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override
		{
			mouseMoveEventPos = where;
			return kMouseEventNotHandled;
		}
	};

	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto container = new TestViewContainer (CRect (10, 10, 80, 80));
	frame->addView (container);

	auto testView = new TestView ({10, 10, 60, 60});
	container->addView (testView);

	frame->attached (frame);

	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (frame, {30., 30.}, MouseButton::None),
			   EventConsumeState::NotHandled);
	EXPECT_EQ (container->mouseMoveEventPos, CPoint (30., 30.));
	EXPECT_EQ (testView->mouseMoveEventPos, CPoint (20., 20.));

	container->mouseMoveEventPos = {};
	testView->mouseMoveEventPos = {};

	CGraphicsTransform tm;
	tm.scale (2., 2.);
	container->setTransform (tm);

	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (frame, {30., 30.}, MouseButton::None),
			   EventConsumeState::NotHandled);
	EXPECT_EQ (container->mouseMoveEventPos, CPoint (30., 30.));
	EXPECT_EQ (testView->mouseMoveEventPos, CPoint (10., 10.));
}

TEST_CASE (CFrameTest, RemoveViewWhileMouseInside)
{
	MouseObserver observer;
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	frame->registerMouseObserver (&observer);
	auto v1 = new View ();
	frame->addView (v1);
	frame->attached (frame);
	dispatchMouseEvent<MouseMoveEvent> (frame, {5., 5.});
	EXPECT (contains (observer.enteredViews, v1));
	observer.reset ();
	frame->removeView (v1);
	EXPECT (contains (observer.exitedViews, v1));
	frame->unregisterMouseObserver (&observer);
}

TEST_CASE (CFrameTest, FocusSettings)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	EXPECT (frame->getFocusColor () == kRedCColor);
	EXPECT (frame->getFocusWidth () == 2.);
	EXPECT (frame->focusDrawingEnabled () == false);
	frame->setFocusColor (kWhiteCColor);
	EXPECT (frame->getFocusColor () == kWhiteCColor);
	frame->setFocusColor (kGreenCColor);
	EXPECT (frame->getFocusColor () == kGreenCColor);
	frame->setFocusWidth (5.);
	EXPECT (frame->getFocusWidth () == 5.);
	frame->setFocusWidth (8.);
	EXPECT (frame->getFocusWidth () == 8.);
	frame->setFocusDrawingEnabled (true);
	EXPECT (frame->focusDrawingEnabled () == true);
	frame->setFocusDrawingEnabled (false);
	EXPECT (frame->focusDrawingEnabled () == false);
}

TEST_CASE (CFrameTest, SetModalView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = shared (new View ());
	EXPECT (frame->getModalView () == nullptr);
	auto session = frame->beginModalViewSession (view);
	EXPECT (session);
	EXPECT (frame->getModalView () == view);
	auto container = shared (new CViewContainer (CRect (0, 0, 0, 0)));
	auto session2 = frame->beginModalViewSession (container);
	EXPECT (session2)
	EXPECT (frame->getModalView () == container);
	EXPECT (frame->endModalViewSession (*session) == false);
	EXPECT (frame->endModalViewSession (*session2) == true);
	EXPECT (frame->getModalView () == view);
	EXPECT (frame->endModalViewSession (*session) == true);
	EXPECT (frame->getModalView () == nullptr);
}

TEST_CASE (CFrameTest, KeyDownEvent)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = new View ();
	frame->addView (view);
	frame->attached (frame);
	frame->onActivate (true);
	KeyboardEvent event;
	event.type = EventType::KeyDown;
	frame->dispatchEvent (event);
	EXPECT (event.consumed == false);
	frame->setFocusView (view);
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	event.consumed = false;
	EXPECT (view->onKeyDownCalled);
	frame->removeAll ();
	auto container = new ContainerTestingKeyboardEvents ();
	auto view2 = new CView (CRect (0, 0, 10, 10));
	container->addView (view2);
	frame->addView (container);
	frame->setFocusView (view2);
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	event.consumed = false;
	EXPECT (container->onKeyDownCalled);
	frame->setFocusView (nullptr);
	view2->setWantsFocus (true);
	EXPECT (frame->getFocusView () == nullptr);
	event.virt = VirtualKey::Tab;
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	event.consumed = false;
	EXPECT (frame->getFocusView () == view2);
	auto view3 = shared (new View ());
	auto modalSession = frame->beginModalViewSession (view3);
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (view3->onKeyDownCalled);
	frame->endModalViewSession (*modalSession);
}

TEST_CASE (CFrameTest, KeyUpEvent)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = new View ();
	frame->addView (view);
	frame->attached (frame);
	frame->onActivate (true);
	KeyboardEvent event;
	event.type = EventType::KeyUp;
	frame->dispatchEvent (event);
	EXPECT (event.consumed == false);
	frame->setFocusView (view);
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	event.consumed = false;
	EXPECT (view->onKeyUpCalled);
	frame->removeAll ();
	auto container = new ContainerTestingKeyboardEvents ();
	auto view2 = new CView (CRect (0, 0, 10, 10));
	container->addView (view2);
	frame->addView (container);
	frame->setFocusView (view2);
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	event.consumed = false;
	EXPECT (container->onKeyUpCalled);
	auto view3 = shared (new View ());
	auto modalSession = frame->beginModalViewSession (view3);
	frame->dispatchEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (view3->onKeyUpCalled);
	frame->endModalViewSession (*modalSession);
}

TEST_CASE (CFrameTest, AdvanceNextFocusView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = new View ();
	frame->attached (frame);
	frame->addView (view);
	frame->onActivate (true);
	EXPECT (frame->getFocusView () == nullptr);
	view->setWantsFocus (true);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view);
	frame->removeAll ();

	auto container = new CViewContainer ({0., 0., 20., 20.});
	auto view2 = new View ();
	container->addView (view2);
	frame->addView (container);

	EXPECT (frame->getFocusView () == nullptr);
	view2->setWantsFocus (true);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view2);
	auto container2 = new CViewContainer ({0., 0., 20., 20.});
	auto view3 = new View ();
	container2->addView (view3);
	container->addView (container2);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == nullptr);
	view3->setWantsFocus (true);
	frame->setFocusView (view2);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view3);
	auto view4 = new View ();
	view4->setWantsFocus (true);
	container2->addView (view4);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view4);
	auto container3 = new CViewContainer ({0., 0., 20., 20.});
	auto view5 = new View ();
	view5->setWantsFocus (true);
	container3->addView (view5);
	container->addView (container3);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view5);
	auto view6 = new View ();
	view6->setWantsFocus (true);
	frame->addView (view6);
	frame->advanceNextFocusView (nullptr);
	EXPECT (frame->getFocusView () == view6);
}

TEST_CASE (CFrameTest, AdvanceNextFocusViewInModalView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = shared (new View ());
	frame->attached (frame);
	auto modalSession = frame->beginModalViewSession (view);
	EXPECT (frame->getFocusView () == nullptr);
	frame->onActivate (true);
	view->setWantsFocus (true);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view);
	frame->endModalViewSession (*modalSession);
	auto container = shared (new CViewContainer ({0., 0., 20., 20.}));
	auto view2 = new View ();
	container->addView (view2);
	modalSession = frame->beginModalViewSession (container);
	EXPECT (frame->getFocusView () == nullptr);
	view2->setWantsFocus (true);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view2);
	auto container2 = new CViewContainer ({0., 0., 20., 20.});
	auto view3 = new View ();
	container2->addView (view3);
	container->addView (container2);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view2);
	view3->setWantsFocus (true);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view3);
	auto view4 = new View ();
	view4->setWantsFocus (true);
	container2->addView (view4);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view4);
	auto container3 = new CViewContainer ({0., 0., 20., 20.});
	auto view5 = new View ();
	view5->setWantsFocus (true);
	container3->addView (view5);
	container->addView (container3);
	frame->advanceNextFocusView (frame->getFocusView ());
	EXPECT (frame->getFocusView () == view5);

	frame->endModalViewSession (*modalSession);
}

TEST_CASE (CFrameTest, GetViewAtModalView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto container = new CViewContainer ({0., 0., 20., 20.});
	auto view = new View ();
	container->addView (view);
	frame->attached (frame);
	auto modalSession = frame->beginModalViewSession (container);
	EXPECT (frame->getViewAt (CPoint (1, 1)) == container);
	EXPECT (frame->getViewAt (CPoint (1, 1), GetViewOptions (GetViewOptions::kDeep)) == view);
	EXPECT (frame->getViewAt (CPoint (90, 90)) == nullptr);
	frame->endModalViewSession (*modalSession);
}

TEST_CASE (CFrameTest, GetContainerAtModalView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto container = new CViewContainer ({0., 0., 20., 20.});
	CRect r (0, 0, 50, 50);
	container->setViewSize (r);
	container->setMouseableArea (r);
	auto container2 = new CViewContainer ({0., 0., 20., 20.});
	container->addView (container2);
	frame->attached (frame);
	EXPECT (frame->getContainerAt (CPoint (1, 1)) == frame);
	auto modalSession = frame->beginModalViewSession (container);
	EXPECT (frame->getContainerAt (CPoint (1, 1), GetViewOptions (GetViewOptions::kNone)) ==
	        container);
	EXPECT (frame->getContainerAt (CPoint (1, 1), GetViewOptions (GetViewOptions::kDeep)) ==
	        container2);
	EXPECT (frame->getContainerAt (CPoint (80, 80), GetViewOptions (GetViewOptions::kDeep)) ==
	        nullptr);

	frame->endModalViewSession (*modalSession);
}

TEST_CASE (CFrameTest, MouseDownModalView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto container = new CViewContainer ({0., 0., 20., 20.});
	auto view1 = new View ();
	container->addView (view1);
	frame->attached (frame);
	auto modalSession = frame->beginModalViewSession (container);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (frame, {80., 80.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (view1->onMouseDownCalled);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (frame, {1., 1.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (view1->onMouseDownCalled);

	frame->endModalViewSession (*modalSession);
}

TEST_CASE (CFrameTest, Activate)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = new View ();
	view->setWantsFocus (true);
	auto view2 = new View ();
	view2->setWantsFocus (true);
	frame->addView (view);
	frame->addView (view2);
	frame->attached (frame);
	EXPECT (frame->getFocusView () == nullptr);
	frame->onActivate (false);
	EXPECT (frame->getFocusView () == nullptr);
	frame->onActivate (true);
	EXPECT (frame->getFocusView () == view);
	frame->setFocusView (view2);
	frame->onActivate (false);
	EXPECT (frame->getFocusView () == nullptr);
	frame->onActivate (true);
	EXPECT (frame->getFocusView () == view2);
}

TEST_CASE (CFrameTest, KeyboardHook)
{
	KeyboardHook hook;
	EXPECT (hook.keyDownCalled == false);

	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	frame->attached (frame);
	frame->registerKeyboardHook (&hook);
	KeyboardEvent event;
	event.type = EventType::KeyDown;
	frame->dispatchEvent (event);
	EXPECT (hook.keyDownCalled);
	EXPECT (hook.keyUpCalled == false);
	event.type = EventType::KeyUp;
	frame->dispatchEvent (event);
	EXPECT (hook.keyUpCalled);

	frame->unregisterKeyboardHook (&hook);
}

TEST_CASE (CFrameTest, Open)
{
	auto platformHandle = UnitTest::PlatformParentHandle::create ();
	EXPECT (platformHandle);
	auto frame = new CFrame (CRect (0, 0, 100, 100), nullptr);
	EXPECT (frame->open (nullptr) == false);
	EXPECT (frame->open (platformHandle->getHandle (), platformHandle->getType ()));
	frame->close ();
}

TEST_CASE (CFrameTest, SetPosition)
{
	auto platformHandle = UnitTest::PlatformParentHandle::create ();
	EXPECT (platformHandle);
	auto frame = new CFrame (CRect (0, 0, 100, 100), nullptr);
	frame->open (platformHandle->getHandle (), platformHandle->getType ());
	EXPECT (frame->setPosition (10, 10));
	CRect r;
	frame->getSize (r);
	EXPECT (r == CRect (10, 10, 110, 110));
	frame->close ();
}

#if 0
TEST_CASE (CFrameTest, CollectInvalidRectsOnMouseDown)
{
	// It is expected that this test failes on Mac OS X 10.11 because of OS changes
	auto platformHandle = UnitTest::PlatformParentHandle::create ();
	auto frame = new CFrame (CRect (0, 0, 100, 100), nullptr);
	auto view = new CollectInvalidRectView ();
	frame->addView (view);
	frame->open (platformHandle->getHandle (), platformHandle->getType ());
	platformHandle->forceRedraw ();
	EXPECT (view->callCount == 1);
	EXPECT (view->redrawRect == view->getViewSize ());

	auto platformFrameCallback = dynamic_cast<IPlatformFrameCallback*> (frame);
	MouseDownEvent downEvent (CPoint (), MouseButton::Left);
	platformFrameCallback->platformOnEvent (downEvent);
	platformHandle->forceRedraw ();
	EXPECT (view->redrawRect == CRect (0, 0, 8, 8));
	EXPECT (view->callCount == 2);
	frame->close ();
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
#include "../../../lib/private/disabledeprecatedmessage.h"

TEST_CASE (CFameLegacyTest, SetModalView)
{
	auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
	auto view = owned (new View ());
	EXPECT (frame->getModalView () == nullptr);
	EXPECT (frame->setModalView (view));
	EXPECT (frame->getModalView () == view);
	auto container = owned (new CViewContainer (CRect (0, 0, 0, 0)));
	EXPECT (frame->setModalView (container) == false);
	EXPECT (frame->setModalView (nullptr));
	EXPECT (frame->setModalView (container));
	EXPECT (frame->getModalView () == container);
	EXPECT (frame->setModalView (nullptr));
	EXPECT (frame->getModalView () == nullptr);
}

#include "../../../lib/private/enabledeprecatedmessage.h"
#endif

} // VSTGUI
