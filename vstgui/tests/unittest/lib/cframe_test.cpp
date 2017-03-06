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
#include "../unittests.h"
#include "platform_helper.h"

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
	void onMouseEntered (CView* view, CFrame* frame) override
	{
		enteredViews.push_back (view);
	}
	void onMouseExited (CView* view, CFrame* frame) override
	{
		exitedViews.push_back (view);
	}
	
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
	bool onKeyDownCalled {false};
	bool onKeyUpCalled {false};
	bool onMouseDownCalled {false};
	CMouseEventResult onMouseDown (CPoint& p, const CButtonState& buttons) override
	{
		onMouseDownCalled = true;
		return kMouseEventHandled;
	}
	
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
};

class Container : public CViewContainer
{
public:
	Container () : CViewContainer (CRect (0, 0, 20, 20)) {}
	bool onKeyDownCalled {false};
	bool onKeyUpCalled {false};
	
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
};

class KeyboardHook : public IKeyboardHook
{
public:
	bool keyDownCalled {false};
	bool keyUpCalled {false};

	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) override
	{
		keyDownCalled = true;
		return -1;
	}
	
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) override
	{
		keyUpCalled = true;
		return -1;
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

TESTCASE(CFrameTest,

	TEST(setZoom,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		EXPECT (frame->setZoom (0.) == false);
		EXPECT (frame->setZoom (2.) == true);
		EXPECT (frame->getViewSize () == CRect (0, 0, 200, 200));
		EXPECT (frame->setZoom (0.5) == true);
		EXPECT (frame->getViewSize () == CRect (0, 0, 50, 50));
		EXPECT (frame->setZoom (1.0) == true);
		EXPECT (frame->getViewSize () == CRect (0, 0, 100, 100));
	);

	TEST(MouseEnterExit,
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
		CPoint p (30, 30);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size() == 0);
		EXPECT (observer.exitedViews.size() == 0);
		observer.reset();
		p (19, 19);
		frame->onMouseMoved (p, 0);
		EXPECT(observer.enteredViews.size () == 1);
		EXPECT(contains (observer.enteredViews, v2));
		EXPECT(observer.exitedViews.size () == 0);
		observer.reset();
		p (9, 9);
		frame->onMouseMoved (p, 0);
		EXPECT(observer.enteredViews.size () == 1);
		EXPECT(contains (observer.enteredViews, v1));
		EXPECT(observer.exitedViews.size () == 1);
		EXPECT(contains (observer.exitedViews, v2));
		frame->unregisterMouseObserver (&observer);
	);

	TEST(MouseEnterExitInContainer,
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
		CPoint p (90, 90);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 0);
		EXPECT (observer.exitedViews.size () == 0);
		observer.reset ();
		p (79, 79);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 1);
		EXPECT (contains (observer.enteredViews, container));
		EXPECT (observer.exitedViews.size () == 0);
		observer.reset ();
		p (49, 49);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 1);
		EXPECT (contains (observer.enteredViews, container2));
		EXPECT (observer.exitedViews.size () == 0);
		observer.reset ();
		p (19, 19);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 1);
		EXPECT (contains (observer.enteredViews, v2));
		EXPECT (observer.exitedViews.size () == 0);
		observer.reset ();
		p (18, 18);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 0);
		EXPECT (observer.exitedViews.size () == 0);
		observer.reset ();
		p (9, 9);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 1);
		EXPECT (contains (observer.enteredViews, v1));
		EXPECT (observer.exitedViews.size () == 1);
		EXPECT (contains (observer.exitedViews, v2));
		observer.reset ();
		p (51, 51);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 0);
		EXPECT (observer.exitedViews.size () == 2);
		EXPECT (contains (observer.exitedViews, v1));
		EXPECT (contains (observer.exitedViews, container2));
		observer.reset ();
		p (81, 81);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 0);
		EXPECT (observer.exitedViews.size () == 1);
		EXPECT (contains (observer.exitedViews, container));
		observer.reset ();
		p (9, 9);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 3);
		EXPECT (observer.exitedViews.size () == 0);
		EXPECT (contains (observer.enteredViews, container));
		EXPECT (contains (observer.enteredViews, container2));
		EXPECT (contains (observer.enteredViews, v1));
		observer.reset ();
		p (81, 81);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 0);
		EXPECT (observer.exitedViews.size () == 3);
		EXPECT (contains (observer.exitedViews, container));
		EXPECT (contains (observer.exitedViews, container2));
		EXPECT (contains (observer.exitedViews, v1));
		observer.reset ();
		p (79, 79);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 1);
		EXPECT (observer.exitedViews.size () == 0);
		EXPECT (contains (observer.enteredViews, container));
		observer.reset ();
		p (8, 8);
		frame->onMouseMoved (p, 0);
		EXPECT (observer.enteredViews.size () == 2);
		EXPECT (observer.exitedViews.size () == 0);
		EXPECT (contains (observer.enteredViews, container2));
		EXPECT (contains (observer.enteredViews, v1));
		observer.reset ();

		frame->unregisterMouseObserver (&observer);
	);
	
	TEST(removeViewWhileMouseInside,
		MouseObserver observer;
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		frame->registerMouseObserver (&observer);
		auto v1 = new View ();
		frame->addView(v1);
		frame->attached (frame);
		CPoint p (5, 5);
		frame->onMouseMoved (p, 0);
		EXPECT (contains (observer.enteredViews, v1));
		observer.reset ();
		frame->removeView (v1);
		EXPECT (contains (observer.exitedViews, v1));
		frame->unregisterMouseObserver (&observer);
	);
	
	TEST(focusSettings,
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
	);

	TEST(setModalView,
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
	);
	
	TEST(onKeyDown,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto view = new View ();
		frame->addView (view);
		frame->attached (frame);
		VstKeyCode key {};
		EXPECT (frame->onKeyDown (key) == -1);
		frame->setFocusView (view);
		EXPECT (frame->onKeyDown (key) == 1);
		EXPECT (view->onKeyDownCalled);
		frame->removeAll ();
		auto container = new Container ();
		auto view2 = new CView (CRect (0, 0, 10, 10));
		container->addView (view2);
		frame->addView (container);
		frame->setFocusView (view2);
		EXPECT (frame->onKeyDown (key) == 1);
		EXPECT (container->onKeyDownCalled);
		frame->setFocusView (nullptr);
		view2->setWantsFocus (true);
		EXPECT (frame->getFocusView () == nullptr);
		key.virt = VKEY_TAB;
		EXPECT (frame->onKeyDown (key) == 1);
		EXPECT (frame->getFocusView() == view2);
		auto view3 = owned (new View ());
		frame->setModalView (view3);
		EXPECT (frame->onKeyDown (key) == 1);
		EXPECT (view3->onKeyDownCalled);
		frame->setModalView (nullptr);
	);

	TEST(onKeyUp,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto view = new View ();
		frame->addView (view);
		frame->attached (frame);
		VstKeyCode key {};
		EXPECT (frame->onKeyUp (key) == -1);
		frame->setFocusView (view);
		EXPECT (frame->onKeyUp (key) == 1);
		EXPECT (view->onKeyUpCalled);
		frame->removeAll ();
		auto container = new Container ();
		auto view2 = new CView (CRect (0, 0, 10, 10));
		container->addView (view2);
		frame->addView (container);
		frame->setFocusView (view2);
		EXPECT (frame->onKeyUp (key) == 1);
		EXPECT (container->onKeyUpCalled);
		auto view3 = owned (new View ());
		frame->setModalView (view3);
		EXPECT (frame->onKeyUp (key) == 1);
		EXPECT (view3->onKeyUpCalled);
		frame->setModalView (nullptr);
	);
	
	TEST(advanceNextFocusView,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto view = new View ();
		frame->attached (frame);
		frame->addView (view);
		EXPECT (frame->getFocusView () == nullptr);
		view->setWantsFocus (true);
		frame->advanceNextFocusView (frame->getFocusView ());
		EXPECT (frame->getFocusView () == view);
		frame->removeAll ();
		
		auto container = new Container ();
		auto view2 = new View ();
		container->addView (view2);
		frame->addView (container);

		EXPECT (frame->getFocusView () == nullptr);
		view2->setWantsFocus (true);
		frame->advanceNextFocusView (frame->getFocusView ());
		EXPECT (frame->getFocusView () == view2);
		auto container2 = new Container ();
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
		auto container3 = new Container ();
		auto view5 = new View ();
		view5->setWantsFocus(true);
		container3->addView (view5);
		container->addView (container3);
		frame->advanceNextFocusView (frame->getFocusView ());
		EXPECT (frame->getFocusView () == view5);
		auto view6 = new View ();
		view6->setWantsFocus (true);
		frame->addView (view6);
		frame->advanceNextFocusView (nullptr);
		EXPECT (frame->getFocusView () == view6);
	);
	
	TEST(advanceNextFocusViewInModalView,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto view = owned (new View ());
		frame->attached (frame);
		frame->setModalView (view);
		EXPECT (frame->getFocusView () == nullptr);
		view->setWantsFocus (true);
		frame->advanceNextFocusView (frame->getFocusView ());
		EXPECT (frame->getFocusView () == view);
		frame->setModalView (nullptr);
		auto container = owned (new Container ());
		auto view2 = new View ();
		container->addView (view2);
		frame->setModalView (container);
		EXPECT (frame->getFocusView () == nullptr);
		view2->setWantsFocus (true);
		frame->advanceNextFocusView (frame->getFocusView ());
		EXPECT (frame->getFocusView () == view2);
		auto container2 = new Container ();
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
		auto container3 = new Container ();
		auto view5 = new View ();
		view5->setWantsFocus(true);
		container3->addView (view5);
		container->addView (container3);
		frame->advanceNextFocusView (frame->getFocusView ());
		EXPECT (frame->getFocusView () == view5);

		frame->setModalView (nullptr);
	);

	TEST(getViewAtModalView,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto container = owned (new Container);
		auto view = new View ();
		container->addView (view);
		frame->attached (frame);
		frame->setModalView (container);
		EXPECT (frame->getViewAt (CPoint (1, 1)) == container);
		EXPECT (frame->getViewAt (CPoint (1, 1), GetViewOptions (GetViewOptions::kDeep)) == view);
		EXPECT (frame->getViewAt (CPoint (90, 90)) == nullptr);
		frame->setModalView (nullptr);
	);
	
	TEST(getContainerAtModalView,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto container = owned (new Container ());
		CRect r (0, 0, 50, 50);
		container->setViewSize (r);
		container->setMouseableArea (r);
		auto container2 = new Container ();
		container->addView (container2);
		frame->attached (frame);
		EXPECT (frame->getContainerAt (CPoint (1, 1)) == frame);
		frame->setModalView (container);
		EXPECT (frame->getContainerAt (CPoint (1, 1), GetViewOptions (GetViewOptions::kNone)) == container);
		EXPECT (frame->getContainerAt (CPoint (1, 1), GetViewOptions (GetViewOptions::kDeep)) == container2);
		EXPECT (frame->getContainerAt (CPoint (80, 80), GetViewOptions (GetViewOptions::kDeep)) == nullptr);
		
		frame->setModalView (nullptr);
	);
	
	TEST(mouseDownModalView,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto container = owned (new Container ());
		auto view1 = new View ();
		container->addView (view1);
		frame->attached (frame);
		frame->setModalView (container);

		CPoint p (80, 80);
		EXPECT (frame->onMouseDown (p, 0) == kMouseEventNotHandled);
		EXPECT (view1->onMouseDownCalled == false);
		p (1, 1);
		EXPECT (frame->onMouseDown (p, 0) == kMouseEventHandled);
		EXPECT (view1->onMouseDownCalled);

		frame->setModalView (nullptr);
	);
	
	TEST(activate,
		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		auto view = new View ();
		view->setWantsFocus (true);
		auto view2 = new View ();
		view2->setWantsFocus (true);
		frame->addView (view);
		frame->addView (view2);
		frame->attached (frame);
		EXPECT(frame->getFocusView () == nullptr);
		frame->onActivate (false);
		EXPECT(frame->getFocusView () == nullptr);
		frame->onActivate (true);
		EXPECT(frame->getFocusView () == view);
		frame->setFocusView (view2);
		frame->onActivate (false);
		EXPECT(frame->getFocusView () == nullptr);
		frame->onActivate (true);
		EXPECT(frame->getFocusView () == view2);
	);

	TEST(keyboardHook,
		KeyboardHook hook;
		EXPECT (hook.keyDownCalled == false);

		auto frame = owned (new CFrame (CRect (0, 0, 100, 100), nullptr));
		frame->attached (frame);
		frame->registerKeyboardHook (&hook);

		VstKeyCode key;
		frame->onKeyDown (key);
		EXPECT (hook.keyDownCalled);
		EXPECT (hook.keyUpCalled == false);
		frame->onKeyUp (key);
		EXPECT (hook.keyUpCalled);
		
		frame->unregisterKeyboardHook (&hook);
	);
	
	TEST(open,
		auto platformHandle = UnitTest::PlatformParentHandle::create ();
		EXPECT(platformHandle);
		auto frame = new CFrame (CRect (0, 0, 100, 100), nullptr);
		EXPECT (frame->open (nullptr) == false);
		EXPECT (frame->open (platformHandle->getHandle (), platformHandle->getType ()));
		frame->close ();
	);
	
	TEST(setPosition,
		auto platformHandle = UnitTest::PlatformParentHandle::create ();
		EXPECT(platformHandle);
		auto frame = new CFrame (CRect (0, 0, 100, 100), nullptr);
		frame->open (platformHandle->getHandle (), platformHandle->getType ());
		CPoint p;
		EXPECT (frame->setPosition (10, 10));
		CRect r;
		frame->getSize (r);
		EXPECT (r == CRect (10, 10, 110, 110));
		frame->close ();
	);
	
	TEST(collectInvalidRectsOnMouseDown,
		auto platformHandle = UnitTest::PlatformParentHandle::create ();
		EXPECT(platformHandle);
		auto frame = new CFrame (CRect (0, 0, 100, 100), nullptr);
		auto view = new CollectInvalidRectView ();
		frame->addView (view);
		frame->open (platformHandle->getHandle (), platformHandle->getType ());
		platformHandle->forceRedraw ();
		EXPECT (view->callCount == 1);
		EXPECT (view->redrawRect == view->getViewSize ());

		auto platformFrameCallback = dynamic_cast<IPlatformFrameCallback*>(frame);
		CPoint p (0, 0);
		platformFrameCallback->platformOnMouseDown (p, 0);
		platformHandle->forceRedraw ();
		EXPECT (view->redrawRect == CRect (0, 0, 8, 8));
		EXPECT (view->callCount == 2);
		frame->close ();
	);
);

} // VSTGUI
