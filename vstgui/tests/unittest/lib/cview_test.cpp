// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cstring.h"
#include "../../../lib/cgraphicspath.h"
#include "../../../lib/coffscreencontext.h"
#include "../../../lib/cview.h"
#include "../../../lib/cviewcontainer.h"
#include "../../../lib/dragging.h"
#include "../../../lib/events.h"
#include "../../../lib/idatapackage.h"
#include "../../../lib/iviewlistener.h"
#include "../unittests.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <array>

namespace VSTGUI {
namespace {
class View : public CView
{
public:
	View () : CView (CRect (0, 0, 10, 10)) {}
	void onIdle () override { onIdleCalled = true; }

	bool onIdleCalled {false};
};

struct ViewListener : public IViewListener
{
	void viewSizeChanged (CView* view, const CRect& oldSize) override { sizeChangedCalled = true; }
	void viewAttached (CView* view) override { attachedCalled = true; }
	void viewRemoved (CView* view) override { removedCalled = true; }
	void viewLostFocus (CView* view) override { lostFocusCalled = true; }
	void viewTookFocus (CView* view) override { tookFocusCalled = true; }
	void viewWillDelete (CView* view) override
	{
		view->unregisterViewListener (this);
		willDeleteCalled = true;
	}
	void viewOnMouseEnabled (CView* view, bool state) override {}

	bool sizeChangedCalled {false};
	bool attachedCalled {false};
	bool removedCalled {false};
	bool lostFocusCalled {false};
	bool tookFocusCalled {false};
	bool willDeleteCalled {false};
};

} // anonymous

TEST_CASE (CViewTest, VisibleState)
{
	auto v = owned (new View ());
	EXPECT (v->isVisible () == true);
	v->setVisible (false);
	EXPECT (v->isVisible () == false);
	v->setVisible (true);
	EXPECT (v->isVisible () == true);
	v->setAlphaValue (0.f);
	EXPECT (v->isVisible () == false);
}

TEST_CASE (CViewTest, TransparencyState)
{
	auto v = owned (new View ());
	EXPECT (v->getTransparency () == false);
	v->setTransparency (true);
	EXPECT (v->getTransparency () == true);
	v->setTransparency (false);
	EXPECT (v->getTransparency () == false);
}

TEST_CASE (CViewTest, FocusState)
{
	auto v = owned (new View ());
	EXPECT (v->wantsFocus () == false);
	v->setWantsFocus (true);
	EXPECT (v->wantsFocus () == true);
	v->setWantsFocus (false);
	EXPECT (v->wantsFocus () == false);
}

TEST_CASE (CViewTest, IdleState)
{
	auto v = owned (new View ());
	EXPECT (v->wantsIdle () == false);
	v->setWantsIdle (true);
	EXPECT (v->wantsIdle () == true);
	v->setWantsIdle (false);
	EXPECT (v->wantsIdle () == false);
}

TEST_CASE (CViewTest, MouseEnabledState)
{
	auto v = owned (new View ());
	EXPECT (v->getMouseEnabled () == true);
	v->setMouseEnabled (false);
	EXPECT (v->getMouseEnabled () == false);
	v->setMouseEnabled (true);
	EXPECT (v->getMouseEnabled () == true);
}

TEST_CASE (CViewTest, AutosizeFlags)
{
	auto v = owned (new View ());
	EXPECT (v->getAutosizeFlags () == kAutosizeNone);
	v->setAutosizeFlags (kAutosizeLeft);
	EXPECT (v->getAutosizeFlags () == kAutosizeLeft);
	v->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	EXPECT (v->getAutosizeFlags () == (kAutosizeLeft | kAutosizeTop));
}

TEST_CASE (CViewTest, Attributes)
{
	auto v = owned (new View ());
	uint32_t outSize;
	void* outData = nullptr;
	EXPECT (v->getAttribute (0, 10, outData, outSize) == false);
	EXPECT (v->removeAttribute (0) == false);
	uint64_t myAttr = 500;
	EXPECT (v->setAttribute (0, 0, &myAttr) == false);
	EXPECT (v->setAttribute (0, sizeof (myAttr), nullptr) == false);
	EXPECT (v->setAttribute ('myAt', sizeof (myAttr), &myAttr) == true);
	myAttr = 10;
	EXPECT (v->getAttributeSize ('myAt', outSize) == true);
	EXPECT (outSize == sizeof (myAttr));
	EXPECT (v->getAttribute ('myAt', sizeof (myAttr), &myAttr, outSize) == true);
	EXPECT (myAttr == 500);
	myAttr = 100;
	EXPECT (v->setAttribute ('myAt', sizeof (myAttr), &myAttr) == true);
	myAttr = 102;
	EXPECT (v->getAttribute ('myAt', sizeof (myAttr), &myAttr, outSize) == true);
	EXPECT (myAttr == 100);
	EXPECT (v->removeAttribute ('myAt') == true);
	EXPECT (v->getAttribute ('myAt', sizeof (myAttr), &myAttr, outSize) == false);
}

TEST_CASE (CViewTest, ResizeAttribute)
{
	auto v = owned (new View ());
	uint32_t outSize;
	uint8_t firstData = 8;
	EXPECT (v->setAttribute (0, sizeof (firstData), &firstData));
	firstData = 0;
	EXPECT (v->getAttribute (0, sizeof (firstData), &firstData, outSize));
	EXPECT (firstData == 8);
	uint32_t secondData = 32;
	EXPECT (v->setAttribute (0, sizeof (secondData), &secondData));
	secondData = 0;
	EXPECT (v->getAttribute (0, sizeof (firstData), &firstData, outSize) == false);
	EXPECT (v->getAttribute (0, sizeof (secondData), &secondData, outSize));
	EXPECT (secondData == 32);
}

TEST_CASE (CViewTest, ViewListener)
{
	ViewListener listener;
	{
		auto v = new View ();
		v->registerViewListener (&listener);
		v->setViewSize (CRect (1, 2, 3, 4));
		auto container1 = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container2 = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		container2->addView (v);
		container2->attached (container1);
		v->takeFocus ();
		v->looseFocus ();
		container2->removeView (v);
		container2->removed (container1);
	}
	EXPECT (listener.sizeChangedCalled);
	EXPECT (listener.attachedCalled);
	EXPECT (listener.removedCalled);
	EXPECT (listener.tookFocusCalled);
	EXPECT (listener.lostFocusCalled);
	EXPECT (listener.willDeleteCalled);
}

TEST_CASE (CViewTest, CoordCalculations)
{
	auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
	container->attached (parent);
	auto v = new View ();
	container->addView (v);
	CPoint p (0, 0);
	v->localToFrame (p);
	EXPECT (p.x == 50 && p.y == 50);
	p (52, 53);
	v->frameToLocal (p);
	EXPECT (p.x == 2 && p.y == 3);
	container->removed (parent);
}

TEST_CASE (CViewTest, VisibleViewSize)
{
	auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
	container->attached (parent);
	auto v = new View ();
	v->setViewSize (CRect (20, 20, 150, 150));
	container->addView (v);
	auto visible = v->getVisibleViewSize ();
	EXPECT (visible == CRect (20, 20, 50, 50));
	container->removeView (v);
	container->removed (parent);
}

TEST_CASE (CViewTest, GlobalTransform)
{
	auto container1 = owned (new CViewContainer (CRect (0, 0, 10, 10)));
	auto container2 = new CViewContainer (CRect (0, 0, 10, 10));
	container1->setTransform (CGraphicsTransform ().translate (10, 20));
	container2->setTransform (CGraphicsTransform ().translate (15, 35));
	container1->addView (container2);
	auto v = new View ();
	container2->addView (v);
	container2->attached (container1);
	auto transform = v->getGlobalTransform ();
	EXPECT (transform.dx == 25 && transform.dy == 55);

	CPoint p (0, 0);
	v->translateToGlobal (p);
	EXPECT (p.x == 25 && p.y == 55);
	v->translateToLocal (p);
	EXPECT (p.x == 0 && p.y == 0);

	auto p2 = v->translateToGlobal (CRect (0, 0, 1, 1));
	EXPECT (p2 == CRect (25, 55, 26, 56));
	p2 = v->translateToLocal (CRect (25, 55, 26, 56));
	EXPECT (p2 == CRect (0, 0, 1, 1));

	container2->removed (container1);
}

TEST_CASE (CViewTest, HitTest)
{
	auto v = owned (new View ());
	v->setMouseableArea (CRect (20, 20, 40, 40));
	EXPECT (v->hitTest (CPoint (5, 5)) == false);
	EXPECT (v->hitTest (CPoint (20, 20)) == true);
	EXPECT (v->hitTest (CPoint (40, 40)) == false);
}

TEST_CASE (CViewTest, DefaultHandling)
{
	auto v = makeOwned<View> ();
	KeyboardEvent keyEvent;
	v->dispatchEvent (keyEvent);
	EXPECT (keyEvent.consumed == false);
	keyEvent.type = EventType::KeyUp;
	v->dispatchEvent (keyEvent);
	EXPECT (keyEvent.consumed == false);
	MouseWheelEvent event;
	v->onMouseWheelEvent (event);
	EXPECT (event.consumed == false);
	CPoint p (0, 0);
	EXPECT (v->onMouseDown (p, kLButton) == kMouseEventNotImplemented);
	EXPECT (v->onMouseUp (p, kLButton) == kMouseEventNotImplemented);
	EXPECT (v->onMouseMoved (p, kLButton) == kMouseEventNotImplemented);
	EXPECT (v->onMouseCancel () == kMouseEventNotImplemented);
	EXPECT (v->onMouseEntered (p, kLButton) == kMouseEventNotImplemented);
	EXPECT (v->onMouseExited (p, kLButton) == kMouseEventNotImplemented);
	EXPECT (v->notify (nullptr, nullptr) == kMessageUnknown);

	EXPECT (v->doDrag (DragDescription (nullptr)) == false);

	EXPECT (v->getDropTarget () == nullptr);
	EXPECT (v->getEditor () == nullptr);
	EXPECT (v->isDirty () == false);
	EXPECT (v->sizeToFit () == false);
	EXPECT (v->getBackground () == nullptr);
	EXPECT (v->getDisabledBackground () == nullptr);
	EXPECT (v->getDrawBackground () == nullptr);
	EXPECT (v->checkUpdate (CRect (0, 0, 5, 5)) == true);
	EXPECT (v->getWidth () == 10);
	EXPECT (v->getHeight () == 10);
	EXPECT (v->getViewSize () == v->getMouseableArea ());
}

TEST_CASE (CViewTest, PathHitTest)
{
	auto v = makeOwned<View> ();
	v->setViewSize ({0., 0., 100., 100.});
	{
		auto drawContext = COffscreenContext::create ({100., 100.});
		auto path = owned (drawContext->createGraphicsPath ());
		path->addRect ({10., 10., 80., 80.});
		v->setHitTestPath (path);
	}
	EXPECT_FALSE (v->hitTest ({5., 5.}, noEvent ()));
	EXPECT_TRUE (v->hitTest ({15., 15.}, noEvent ()));
}

namespace {

class TestView : public CView
{
public:
	TestView () : CView (CRect (0, 0, 10, 10))
	{
		std::fill (called.begin (), called.end (), false);
	}

	void onMouseDownEvent (MouseDownEvent& event) override
	{
		called[e2p (EventType::MouseDown)] = true;
	}
	void onMouseMoveEvent (MouseMoveEvent& event) override
	{
		called[e2p (EventType::MouseMove)] = true;
	}
	void onMouseUpEvent (MouseUpEvent& event) override { called[e2p (EventType::MouseUp)] = true; }
	void onMouseCancelEvent (MouseCancelEvent& event) override
	{
		called[e2p (EventType::MouseCancel)] = true;
	}
	void onMouseEnterEvent (MouseEnterEvent& event) override
	{
		called[e2p (EventType::MouseEnter)] = true;
	}
	void onMouseExitEvent (MouseExitEvent& event) override
	{
		called[e2p (EventType::MouseExit)] = true;
	}
	void onMouseWheelEvent (MouseWheelEvent& event) override
	{
		called[e2p (EventType::MouseWheel)] = true;
	}
	void onZoomGestureEvent (ZoomGestureEvent& event) override
	{
		called[e2p (EventType::ZoomGesture)] = true;
	}
	void onKeyboardEvent (KeyboardEvent& event) override { called[e2p (EventType::KeyUp)] = true; }

	bool eventCalled (EventType t) const { return called[e2p (t)]; }

private:
	static constexpr size_t e2p (EventType t) { return static_cast<size_t> (t); }
	std::array<bool, static_cast<size_t> (EventType::KeyDown)> called;
};

struct TestViewEventHandler : IViewEventListener
{
	using Func = std::function<void (CView*, Event&)>;
	TestViewEventHandler (Func&& func) : func (std::move (func)) {}
	void viewOnEvent (CView* view, Event& event) override { func (view, event); }

	Func func;
};

} // anonymous

TEST_CASE (CViewTest, ViewEventListenerMouseDownEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseDownEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseDown));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseDown));
}

TEST_CASE (CViewTest, ViewEventListenerMouseMoveEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseMoveEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseMove));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseMove));
}

TEST_CASE (CViewTest, ViewEventListenerMouseUpEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseUpEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseUp));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseUp));
}

TEST_CASE (CViewTest, ViewEventListenerMouseCancelEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseCancelEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseCancel));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseCancel));
}

TEST_CASE (CViewTest, ViewEventListenerMouseEnterEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseEnterEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseEnter));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseEnter));
}

TEST_CASE (CViewTest, ViewEventListenerMouseExitEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseExitEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseExit));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseExit));
}

TEST_CASE (CViewTest, ViewEventListenerMouseWheelEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	MouseWheelEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::MouseWheel));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::MouseWheel));
}

TEST_CASE (CViewTest, ViewEventListenerZoomGestureEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	ZoomGestureEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::ZoomGesture));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::ZoomGesture));
}

TEST_CASE (CViewTest, ViewEventListenerKeyEvent)
{
	auto v = makeOwned<TestView> ();
	TestViewEventHandler listener ([] (CView*, Event& event) { event.consumed = true; });
	v->registerViewEventListener (&listener);
	KeyboardEvent event;
	v->dispatchEvent (event);
	EXPECT_FALSE (v->eventCalled (EventType::KeyUp));
	v->unregisterViewEventListener (&listener);
	event.consumed.reset ();
	v->dispatchEvent (event);
	EXPECT_TRUE (v->eventCalled (EventType::KeyUp));
}

#if MAC // TODO: Make test work on other platforms too.

TEST_CASE (CViewTest, IdleAfterAttached)
{
	auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
	container->attached (parent);
	auto v = new View ();
	container->addView (v);
	v->setWantsIdle (true);
	CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, false);
	EXPECT (v->onIdleCalled == true);
	v->setWantsIdle (false);
	v->onIdleCalled = false;
	CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, false);
	EXPECT (v->onIdleCalled == false);
	container->removeView (v);
	container->removed (parent);
}

TEST_CASE (CViewTest, IdleBeforeAttached)
{
	auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
	auto v = new View ();
	container->addView (v);
	v->setWantsIdle (true);
	container->attached (parent);
	CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, true);
	EXPECT (v->onIdleCalled == true);
	container->removeView (v);
	container->removed (parent);
}

#endif

struct DataPackage : IDataPackage
{
	UTF8String str;
	UTF8String path;
	int8_t binary[3];

	uint32_t getCount () const override { return 3; }

	uint32_t getDataSize (uint32_t index) const override
	{
		if (index == 0)
			return static_cast<uint32_t> (str.length ());
		else if (index == 1)
			return static_cast<uint32_t> (path.length ());
		else if (index == 2)
			return 3;
		return 0;
	}

	Type getDataType (uint32_t index) const override
	{
		if (index == 0)
			return kText;
		else if (index == 1)
			return kFilePath;
		else if (index == 2)
			return kBinary;
		return kError;
	}

	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override
	{
		type = kError;
		if (index == 0)
		{
			buffer = str.data ();
			type = kText;
		}
		else if (index == 1)
		{
			buffer = path.data ();
			type = kFilePath;
		}
		else if (index == 2)
		{
			buffer = binary;
			type = kBinary;
		}
		return getDataSize (index);
	}
};

TEST_CASE (CDragContainerHelperTest, Count)
{
	DataPackage package;
	CDragContainerHelper helper (&package);
	EXPECT (helper.getCount () == 3);
}

TEST_CASE (CDragContainerHelper, GetType)
{
	DataPackage package;
	CDragContainerHelper helper (&package);
	EXPECT (helper.getType (0) == CDragContainerHelper::kUnicodeText);
	EXPECT (helper.getType (1) == CDragContainerHelper::kFile);
	EXPECT (helper.getType (2) == CDragContainerHelper::kUnknown);
	EXPECT (helper.getType (3) == CDragContainerHelper::kError);
}

TEST_CASE (CDragContainerHelper, Iteration)
{
	DataPackage package;
	package.str = "Test";
	package.path = "/var/tmp/test";
	CDragContainerHelper helper (&package);
	int32_t size;
	int32_t type;
	auto res = helper.first (size, type);
	EXPECT (res == package.str.data ());
	EXPECT (size == static_cast<int32_t> (package.str.length ()));
	EXPECT (type == CDragContainerHelper::kUnicodeText);
	res = helper.next (size, type);
	EXPECT (res == package.path.data ());
	EXPECT (size == static_cast<int32_t> (package.path.length ()));
	EXPECT (type == CDragContainerHelper::kFile);
	res = helper.next (size, type);
	EXPECT (res == package.binary);
	EXPECT (size == 3);
	EXPECT (type == CDragContainerHelper::kUnknown);
	res = helper.next (size, type);
	EXPECT (res == nullptr);
	EXPECT (size == 0);
	EXPECT (type == CDragContainerHelper::kError);
}

} // VSTGUI
