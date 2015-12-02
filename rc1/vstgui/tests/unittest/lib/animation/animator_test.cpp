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

#include "../../../../lib/animation/animator.h"
#include "../../../../lib/animation/animations.h"
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

} // anonymous

//-----------------------------------------------------------------------------
TESTCASE(AnimatorTest,
	CFRunLoopGetMain ();

	TEST(addAnimation,
		auto a = owned (new Animator ());
		auto view = owned (new CView (CRect (0, 0, 0, 0)));
		a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (100), [] (CView*, const IdStringPtr, IAnimationTarget*) {
			CFRunLoopStop (CFRunLoopGetCurrent ());
		});
		CFRunLoopRun ();
		EXPECT(view->getAlphaValue () == 0.f);
	);

	TEST(cancelAnimation,
		auto a = owned (new Animator ());
		auto view = owned (new CView (CRect (0, 0, 0, 0)));
		a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (2000));
		CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, false);
		a->removeAnimation (view, "Test");
		EXPECT(view->getAlphaValue () != 0.f);
	);

	TEST(removeAnimationInCallback,
		auto a = owned (new Animator ());
		auto view = owned (new CView (CRect (0, 0, 0, 0)));
		a->addAnimation (view, "Test", new RemoveAnimationInCallback (a), new LinearTimingFunction (100), [] (CView*, const IdStringPtr, IAnimationTarget*) {
			CFRunLoopStop (CFRunLoopGetCurrent ());
		});
		CFRunLoopRun ();
	);
	
	TEST(animationMessage,
		auto a = owned (new Animator ());
		auto view = owned (new CView (CRect (0, 0, 0, 0)));
		MessageReceiver recevier;
		a->addAnimation (view, "Test", new AlphaValueAnimation (0.f), new LinearTimingFunction (2000), &recevier);
		CFRunLoopRun ();
		EXPECT(recevier.messageReceived == true)
	);
);

} // VSTGUI

#endif // MAC
