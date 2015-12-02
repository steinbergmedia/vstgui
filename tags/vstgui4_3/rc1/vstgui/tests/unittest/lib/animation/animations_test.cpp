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

#include "../../../../lib/animation/animations.h"
#include "../../../../lib/cview.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../../../lib/controls/ccontrol.h"
#include "../../unittests.h"

namespace VSTGUI {
using namespace Animation;

namespace {

//-----------------------------------------------------------------------------
class TestView : public CView
{
public:
	TestView () : CView (CRect (0, 0, 0, 0)) {}
};

//-----------------------------------------------------------------------------
class TestControl : public CControl
{
public:
	TestControl () : CControl (CRect (0, 0, 0, 0)) {}
	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD {}

	CLASS_METHODS(TestControl, CControl)
};

} // anonymous

//-----------------------------------------------------------------------------
TESTCASE(AlphaValueAnimationTest,

	TEST(animation,
		TestView view;
		EXPECT(view.getAlphaValue () == 1.f);
		AlphaValueAnimation a (0.f);
		a.animationStart (&view, "");
		a.animationTick (&view, "", 0.5f);
		EXPECT(view.getAlphaValue () == 0.5f);
		a.animationTick (&view, "", 1.f);
		EXPECT(view.getAlphaValue () == 0.f);
		a.animationFinished (&view, "", false);
		EXPECT(view.getAlphaValue () == 0.f);
	);

);

//-----------------------------------------------------------------------------
TESTCASE(ViewSizeAnimationTest,

	TEST(animation,
		TestView view;
		EXPECT(view.getViewSize () == CRect (0, 0, 0, 0));
		ViewSizeAnimation a (CRect (10, 10, 100, 100));
		a.animationStart (&view, "");
		a.animationTick (&view, "", 0.5f);
		EXPECT(view.getViewSize () == CRect (5, 5, 50, 50));
		a.animationTick (&view, "", 1.f);
		EXPECT(view.getViewSize () == CRect (10, 10, 100, 100));
		a.animationFinished (&view, "", false);
		EXPECT(view.getViewSize () == CRect (10, 10, 100, 100));
	);

	TEST(unfinishedAnimation,
		TestView view;
		ViewSizeAnimation a (CRect (10, 10, 100, 100));
		a.animationStart (&view, "");
		a.animationTick (&view, "", 0.5f);
		EXPECT(view.getViewSize () == CRect (5, 5, 50, 50));
		a.animationFinished (&view, "", false);
		EXPECT(view.getViewSize () == CRect (10, 10, 100, 100));
	);

);

//-----------------------------------------------------------------------------
TESTCASE(ControlValueAnimationTest,
	
	TEST(animation,
		TestControl control;
		EXPECT(control.getValue () == 0.f);
		ControlValueAnimation a (1.f);
		a.animationStart (&control, "");
		EXPECT(control.getValue () == 0.f);
		a.animationTick (&control, "", 0.3f);
		EXPECT(control.getValue () == 0.3f);
		a.animationTick (&control, "", 0.5f);
		EXPECT(control.getValue () == 0.5f);
		a.animationFinished(&control, "", false);
		EXPECT(control.getValue () == 1.f);
	);
);

//-----------------------------------------------------------------------------
TESTCASE(ExchangeViewAnimationTest,

	TEST(alphaValueFade,
		auto parentContainer = owned (new CViewContainer (CRect (0, 0, 0, 0)));
		auto container = new CViewContainer (CRect (0, 0, 0, 0));
		container->attached (parentContainer);
		auto oldView = new TestView ();
		auto newView = new TestView ();
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kAlphaValueFade);
		a.animationStart (container, "");
		EXPECT(oldView->getAlphaValue () == 1.f);
		EXPECT(newView->getAlphaValue () == 0.f);
		a.animationTick(container, "", 0.5f);
		EXPECT(oldView->getAlphaValue () == 0.5f);
		EXPECT(newView->getAlphaValue () == 0.5f);
		a.animationTick(container, "", 1.f);
		a.animationFinished(container, "", false);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getAlphaValue () == 1.f);
		container->removed (parentContainer);
	);

	TEST(pushInFromLeft,
		CRect r (0, 0, 100, 100);
		auto parentContainer = owned (new CViewContainer (r));
		auto container = new CViewContainer (r);
		container->attached (parentContainer);
		auto oldView = new CView (r);
		auto newView = new CView (r);
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromLeft);
		a.animationStart (container, "");
		EXPECT(oldView->getViewSize () == r);
		EXPECT(newView->getViewSize () == CRect (-100, 0, 0, 100));
		a.animationTick(container, "", 0.5f);
		EXPECT(newView->getViewSize () == CRect (-50, 0, 50, 100));
		a.animationTick(container, "", 1.f);
		a.animationFinished(container, "", false);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getViewSize () == r);
		container->removed (parentContainer);
	);

	TEST(pushInFromRight,
		CRect r (0, 0, 100, 100);
		auto parentContainer = owned (new CViewContainer (r));
		auto container = new CViewContainer (r);
		container->attached (parentContainer);
		auto oldView = new CView (r);
		auto newView = new CView (r);
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromRight);
		a.animationStart (container, "");
		EXPECT(oldView->getViewSize () == r);
		EXPECT(newView->getViewSize () == CRect (100, 0, 200, 100));
		a.animationTick(container, "", 0.5f);
		EXPECT(newView->getViewSize () == CRect (50, 0, 150, 100));
		a.animationTick(container, "", 1.f);
		a.animationFinished(container, "", false);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getViewSize () == r);
		container->removed (parentContainer);
	);

	TEST(pushInFromTop,
		CRect r (0, 0, 100, 100);
		auto parentContainer = owned (new CViewContainer (r));
		auto container = new CViewContainer (r);
		container->attached (parentContainer);
		auto oldView = new CView (r);
		auto newView = new CView (r);
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromTop);
		a.animationStart (container, "");
		EXPECT(oldView->getViewSize () == r);
		EXPECT(newView->getViewSize () == CRect (0, -100, 100, 0));
		a.animationTick(container, "", 0.5f);
		EXPECT(newView->getViewSize () == CRect (0, -50, 100, 50));
		a.animationTick(container, "", 1.f);
		a.animationFinished(container, "", false);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getViewSize () == r);
		container->removed (parentContainer);
	);

	TEST(pushInFromBottom,
		CRect r (0, 0, 100, 100);
		auto parentContainer = owned (new CViewContainer (r));
		auto container = new CViewContainer (r);
		container->attached (parentContainer);
		auto oldView = new CView (r);
		auto newView = new CView (r);
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromBottom);
		a.animationStart (container, "");
		EXPECT(oldView->getViewSize () == r);
		EXPECT(newView->getViewSize () == CRect (0, 100, 100, 200));
		a.animationTick(container, "", 0.5f);
		EXPECT(newView->getViewSize () == CRect (0, 50, 100, 150));
		a.animationTick(container, "", 1.f);
		a.animationFinished(container, "", false);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getViewSize () == r);
		container->removed (parentContainer);
	);

	TEST(pushInOutFromLeft,
		CRect r (0, 0, 100, 100);
		auto parentContainer = owned (new CViewContainer (r));
		auto container = new CViewContainer (r);
		container->attached (parentContainer);
		auto oldView = new CView (r);
		auto newView = new CView (r);
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInOutFromLeft);
		a.animationStart (container, "");
		EXPECT(oldView->getViewSize () == r);
		EXPECT(newView->getViewSize () == CRect (-100, 0, 0, 100));
		a.animationTick(container, "", 0.5f);
		EXPECT(oldView->getViewSize () == CRect (50, 0, 150, 100));
		EXPECT(newView->getViewSize () == CRect (-50, 0, 50, 100));
		a.animationTick(container, "", 1.f);
		a.animationFinished(container, "", false);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getViewSize () == r);
		container->removed (parentContainer);
	);

	TEST(pushInOutFromRight,
		CRect r (0, 0, 100, 100);
		auto parentContainer = owned (new CViewContainer (r));
		auto container = new CViewContainer (r);
		container->attached (parentContainer);
		auto oldView = new CView (r);
		auto newView = new CView (r);
		container->addView (oldView);
		ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInOutFromRight);
		a.animationStart (container, "");
		EXPECT(oldView->getViewSize () == r);
		EXPECT(newView->getViewSize () == CRect (100, 0, 200, 100));
		a.animationTick(container, "", 0.5f);
		EXPECT(oldView->getViewSize () == CRect (-50, 0, 50, 100));
		EXPECT(newView->getViewSize () == CRect (50, 0, 150, 100));
		a.animationFinished(container, "", true);
		EXPECT(oldView->isAttached () == false);
		EXPECT(newView->getViewSize () == r);
		container->removed (parentContainer);
	);

);

} // VSTGUI
