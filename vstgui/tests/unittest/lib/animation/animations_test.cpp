// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/animation/animations.h"
#include "../../../../lib/controls/ccontrol.h"
#include "../../../../lib/cview.h"
#include "../../../../lib/cviewcontainer.h"
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
	void draw (CDrawContext* pContext) override {}

	CLASS_METHODS (TestControl, CControl)
};

} // anonymous

TEST_CASE (AlphaValueAnimtionTest, Animation)
{
	TestView view;
	EXPECT (view.getAlphaValue () == 1.f);
	AlphaValueAnimation a (0.f);
	a.animationStart (&view, "");
	a.animationTick (&view, "", 0.5f);
	EXPECT (view.getAlphaValue () == 0.5f);
	a.animationTick (&view, "", 1.f);
	EXPECT (view.getAlphaValue () == 0.f);
	a.animationFinished (&view, "", false);
	EXPECT (view.getAlphaValue () == 0.f);
}

//-----------------------------------------------------------------------------
TEST_CASE (ViewSizeAnimationTest, Animation)
{
	TestView view;
	EXPECT (view.getViewSize () == CRect (0, 0, 0, 0));
	ViewSizeAnimation a (CRect (10, 10, 100, 100));
	a.animationStart (&view, "");
	a.animationTick (&view, "", 0.5f);
	EXPECT (view.getViewSize () == CRect (5, 5, 50, 50));
	a.animationTick (&view, "", 1.f);
	EXPECT (view.getViewSize () == CRect (10, 10, 100, 100));
	a.animationFinished (&view, "", false);
	EXPECT (view.getViewSize () == CRect (10, 10, 100, 100));
}

TEST_CASE (ViewSizeAnimationTest, UnfinishedAnimation)
{
	TestView view;
	ViewSizeAnimation a (CRect (10, 10, 100, 100));
	a.animationStart (&view, "");
	a.animationTick (&view, "", 0.5f);
	EXPECT (view.getViewSize () == CRect (5, 5, 50, 50));
	a.animationFinished (&view, "", false);
	EXPECT (view.getViewSize () == CRect (10, 10, 100, 100));
}

//-----------------------------------------------------------------------------
TEST_CASE (ControlValueAnimationTest, Animation)
{
	TestControl control;
	EXPECT (control.getValue () == 0.f);
	ControlValueAnimation a (1.f);
	a.animationStart (&control, "");
	EXPECT (control.getValue () == 0.f);
	a.animationTick (&control, "", 0.3f);
	EXPECT (control.getValue () == 0.3f);
	a.animationTick (&control, "", 0.5f);
	EXPECT (control.getValue () == 0.5f);
	a.animationFinished (&control, "", false);
	EXPECT (control.getValue () == 1.f);
}

//-----------------------------------------------------------------------------
TEST_CASE (ExchangeViewAnimationTest, AlphaValueFade)
{
	auto parentContainer = owned (new CViewContainer (CRect (0, 0, 0, 0)));
	auto container = new CViewContainer (CRect (0, 0, 0, 0));
	container->attached (parentContainer);
	auto oldView = new TestView ();
	auto newView = new TestView ();
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kAlphaValueFade);
	a.animationStart (container, "");
	EXPECT (oldView->getAlphaValue () == 1.f);
	EXPECT (newView->getAlphaValue () == 0.f);
	a.animationTick (container, "", 0.5f);
	EXPECT (oldView->getAlphaValue () == 0.5f);
	EXPECT (newView->getAlphaValue () == 0.5f);
	a.animationTick (container, "", 1.f);
	a.animationFinished (container, "", false);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getAlphaValue () == 1.f);
	container->removed (parentContainer);
}

TEST_CASE (ExchangeViewAnimationTest, PushInFromLeft)
{
	CRect r (0, 0, 100, 100);
	auto parentContainer = owned (new CViewContainer (r));
	auto container = new CViewContainer (r);
	container->attached (parentContainer);
	auto oldView = new CView (r);
	auto newView = new CView (r);
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromLeft);
	a.animationStart (container, "");
	EXPECT (oldView->getViewSize () == r);
	EXPECT (newView->getViewSize () == CRect (-100, 0, 0, 100));
	a.animationTick (container, "", 0.5f);
	EXPECT (newView->getViewSize () == CRect (-50, 0, 50, 100));
	a.animationTick (container, "", 1.f);
	a.animationFinished (container, "", false);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getViewSize () == r);
	container->removed (parentContainer);
}

TEST_CASE (ExchangeViewAnimationTest, PushInFromRight)
{
	CRect r (0, 0, 100, 100);
	auto parentContainer = owned (new CViewContainer (r));
	auto container = new CViewContainer (r);
	container->attached (parentContainer);
	auto oldView = new CView (r);
	auto newView = new CView (r);
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromRight);
	a.animationStart (container, "");
	EXPECT (oldView->getViewSize () == r);
	EXPECT (newView->getViewSize () == CRect (100, 0, 200, 100));
	a.animationTick (container, "", 0.5f);
	EXPECT (newView->getViewSize () == CRect (50, 0, 150, 100));
	a.animationTick (container, "", 1.f);
	a.animationFinished (container, "", false);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getViewSize () == r);
	container->removed (parentContainer);
}

TEST_CASE (ExchangeViewAnimationTest, PushInFromTop)
{
	CRect r (0, 0, 100, 100);
	auto parentContainer = owned (new CViewContainer (r));
	auto container = new CViewContainer (r);
	container->attached (parentContainer);
	auto oldView = new CView (r);
	auto newView = new CView (r);
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromTop);
	a.animationStart (container, "");
	EXPECT (oldView->getViewSize () == r);
	EXPECT (newView->getViewSize () == CRect (0, -100, 100, 0));
	a.animationTick (container, "", 0.5f);
	EXPECT (newView->getViewSize () == CRect (0, -50, 100, 50));
	a.animationTick (container, "", 1.f);
	a.animationFinished (container, "", false);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getViewSize () == r);
	container->removed (parentContainer);
}

TEST_CASE (ExchangeViewAnimationTest, PushInFromBottom)
{
	CRect r (0, 0, 100, 100);
	auto parentContainer = owned (new CViewContainer (r));
	auto container = new CViewContainer (r);
	container->attached (parentContainer);
	auto oldView = new CView (r);
	auto newView = new CView (r);
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInFromBottom);
	a.animationStart (container, "");
	EXPECT (oldView->getViewSize () == r);
	EXPECT (newView->getViewSize () == CRect (0, 100, 100, 200));
	a.animationTick (container, "", 0.5f);
	EXPECT (newView->getViewSize () == CRect (0, 50, 100, 150));
	a.animationTick (container, "", 1.f);
	a.animationFinished (container, "", false);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getViewSize () == r);
	container->removed (parentContainer);
}

TEST_CASE (ExchangeViewAnimationTest, PushInOutFromLeft)
{
	CRect r (0, 0, 100, 100);
	auto parentContainer = owned (new CViewContainer (r));
	auto container = new CViewContainer (r);
	container->attached (parentContainer);
	auto oldView = new CView (r);
	auto newView = new CView (r);
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInOutFromLeft);
	a.animationStart (container, "");
	EXPECT (oldView->getViewSize () == r);
	EXPECT (newView->getViewSize () == CRect (-100, 0, 0, 100));
	a.animationTick (container, "", 0.5f);
	EXPECT (oldView->getViewSize () == CRect (50, 0, 150, 100));
	EXPECT (newView->getViewSize () == CRect (-50, 0, 50, 100));
	a.animationTick (container, "", 1.f);
	a.animationFinished (container, "", false);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getViewSize () == r);
	container->removed (parentContainer);
}

TEST_CASE (ExchangeViewAnimationTest, PushInOutFromRight)
{
	CRect r (0, 0, 100, 100);
	auto parentContainer = owned (new CViewContainer (r));
	auto container = new CViewContainer (r);
	container->attached (parentContainer);
	auto oldView = new CView (r);
	auto newView = new CView (r);
	container->addView (oldView);
	ExchangeViewAnimation a (oldView, newView, ExchangeViewAnimation::kPushInOutFromRight);
	a.animationStart (container, "");
	EXPECT (oldView->getViewSize () == r);
	EXPECT (newView->getViewSize () == CRect (100, 0, 200, 100));
	a.animationTick (container, "", 0.5f);
	EXPECT (oldView->getViewSize () == CRect (-50, 0, 50, 100));
	EXPECT (newView->getViewSize () == CRect (50, 0, 150, 100));
	a.animationFinished (container, "", true);
	EXPECT (oldView->isAttached () == false);
	EXPECT (newView->getViewSize () == r);
	container->removed (parentContainer);
}

} // VSTGUI
