// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/animation/animations.h"
#include "../../../../lib/animation/animator.h"
#include "../../../../lib/animation/timingfunctions.h"
#include "../../../../lib/cview.h"
#include "../../unittests.h"

#if MAC

#include <CoreFoundation/CoreFoundation.h>

namespace VSTGUI {
using namespace Animation;

namespace {

struct RemoveAnimationInCallback : public IAnimationTarget
{
	RemoveAnimationInCallback (Animator* animator) : animator (animator) {}

	Animator* animator;

	void animationStart (CView* view, IdStringPtr name) override {}
	void animationTick (CView* view, IdStringPtr name, float pos) override
	{
		animator->removeAnimations (view);
	}
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override {}
};

} // anonymous

//-----------------------------------------------------------------------------
TEST_CASE (AnimatorTest, AddAnimation)
{
	auto a = owned (new Animator ());
	auto view = owned (new CView (CRect (0, 0, 0, 0)));
	a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (100),
	                 [] (CView*, const IdStringPtr, IAnimationTarget*) {
		                 CFRunLoopStop (CFRunLoopGetCurrent ());
	                 });
	CFRunLoopRun ();
	EXPECT (view->getAlphaValue () == 0.f);
}

TEST_CASE (AnimatorTest, CancelAnimation)
{
	auto a = owned (new Animator ());
	auto view = owned (new CView (CRect (0, 0, 0, 0)));
	a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (2000));
	CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, false);
	a->removeAnimation (view, "Test");
	EXPECT (view->getAlphaValue () != 0.f);
}

TEST_CASE (AnimatorTest, CancelAnimationWithCallback)
{
	auto a = owned (new Animator ());
	auto view = owned (new CView (CRect (0, 0, 0, 0)));
	bool cancelDoneFunctionCalled = false;
	auto doneFunc = [&] (auto, auto, auto) { cancelDoneFunctionCalled = true; };
	a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (2000),
					 doneFunc, false);
	CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, false);
	a->removeAnimation (view, "Test");
	EXPECT_FALSE (cancelDoneFunctionCalled);
	EXPECT (view->getAlphaValue () != 0.f);
	a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (2000),
					 doneFunc, true);
	CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, false);
	a->removeAnimation (view, "Test");
	EXPECT_TRUE (cancelDoneFunctionCalled);
}

TEST_CASE (AnimatorTest, RemoveAnimationInCallback)
{
	auto a = owned (new Animator ());
	auto view = owned (new CView (CRect (0, 0, 0, 0)));
	a->addAnimation (view, "Test", new RemoveAnimationInCallback (a),
	                 new LinearTimingFunction (100),
	                 [] (CView*, const IdStringPtr, IAnimationTarget*) {
		                 CFRunLoopStop (CFRunLoopGetCurrent ());
	                 });
	CFRunLoopRun ();
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
#include "../../../../lib/private/disabledeprecatedmessage.h"

struct MessageReceiver : public CBaseObject
{
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override
	{
		messageReceived = true;
		CFRunLoopStop (CFRunLoopGetCurrent ());
		return kMessageNotified;
	}

	bool messageReceived {false};
};

TEST_CASE (AnimatorTest, AnimationMessage)
{
	auto a = owned (new Animator ());
	auto view = owned (new CView (CRect (0, 0, 0, 0)));
	MessageReceiver recevier;
	a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (100),
	                 &recevier);
	CFRunLoopRun ();
	EXPECT (recevier.messageReceived == true)
}
#include "../../../../lib/private/enabledeprecatedmessage.h"
#endif

} // VSTGUI

#endif // MAC
