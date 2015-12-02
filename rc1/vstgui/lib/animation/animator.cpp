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

/**
@page page_animation Animations

VSTGUI version 4 adds simple to use view animation support.

The source can be found under /lib/animation/

@section the_animator The Animator
Every @link VSTGUI::CFrame::getAnimator CFrame @endlink object can have one @link VSTGUI::Animation::Animator Animator @endlink object which runs animations at 60 Hz.

The animator is responsible for running animations.
You can add and remove animations.
Animations are identified by a view and a name.

To add an animation you just call @link VSTGUI::CView::addAnimation CView::addAnimation (name, target, timing)@endlink.
The animation will start immediately and will automatically be removed if it has finished.
If you want to stop it before it has finished you can use @link VSTGUI::CView::removeAnimation CView::removeAnimation (name)@endlink.
You can also stop all animations for a view with @link VSTGUI::CView::removeAllAnimations CView::removeAllAnimations ()@endlink.

The animator is the owner of the target and timing function objects and will destroy these objects when the animation has finished.
This means that the animator will call delete on these objects or if they are inherited from CBaseObject it will call forget() on them.

@section the_animation The Animation

An animation is made up by an @link VSTGUI::Animation::IAnimationTarget IAnimationTarget @endlink and an @link VSTGUI::Animation::ITimingFunction ITimingFunction @endlink object.

@subsection animation_target The Animation Target
The animation target is responsible for changing the view from one state to another state.

The animation target interface consists of 3 methods:
- @link VSTGUI::Animation::IAnimationTarget::animationStart animationStart (view, name) @endlink
- @link VSTGUI::Animation::IAnimationTarget::animationTick animationTick (view, name, pos) @endlink
- @link VSTGUI::Animation::IAnimationTarget::animationFinished animationFinished (view, name, wasCanceled) @endlink

All these methods have the view and the animation name as arguments to identify the animation within the target.
The animationTick method in addition has the normalized animation position as argument and the animationFinished method has a bool argument indicating if the animation was canceled.

see @link AnimationTargets included animation target classes @endlink

@subsection animation_timing The Animation Timing Function
the animation timing function maps elapsed time to a normalized position.

see @link AnimationTimingFunctions included animation timing function classes @endlink

@section simple_example Simple Usage Example
In this example the custom view animates it's alpha value when the mouse moves inside or outside the view.

@code

using namespace VSTGUI::Animation;

class MyView : public CView
{
public:
	MyView (const CRect& r) : CView (r) { setAlphaValue (0.5f); }

	CMouseEventResult onMouseEntered (CPoint &where, const CButtonState& buttons)
	{
		// this adds an animation which takes 200 ms to make a linear alpha fade from the current value to 1
		addAnimation ("AlphaValueAnimation", new AlphaValueAnimation (1.f), new LinearTimingFunction (200));
		return kMouseEventHandled;
	}
	
	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons)
	{
		// this adds an animation which takes 200 ms to make a linear alpha fade from the current value to 0.5
		addAnimation ("AlphaValueAnimation", new AlphaValueAnimation (0.5f), new LinearTimingFunction (200));
		return kMouseEventHandled;
	}

	void draw (CDrawContext* context)
	{
		// ... any drawing code here
	}
};

@endcode


*/

//------------------------------------------------------------------------
/*! @defgroup animation Animation
	see @ref page_animation
 */
//-----------------------------------------------------------------------------

#include "animator.h"
#include "ianimationtarget.h"
#include "itimingfunction.h"
#include "../cvstguitimer.h"
#include "../cview.h"
#include "../platform/iplatformframe.h"

#define DEBUG_LOG	0 // DEBUG

namespace VSTGUI {
namespace Animation {

///@cond ignore
//-----------------------------------------------------------------------------
class Timer : public CBaseObject
{
public:
	static void addAnimator (Animator* animator)
	{
		getInstance ()->animators.push_back (animator);
		#if DEBUG_LOG
		DebugPrint ("Animator added: %p\n", animator);
		#endif
	}
	static void removeAnimator (Animator* animator)
	{
		if (gInstance)
		{
			if (getInstance ()->inTimer)
			{
				gInstance->toRemove.push_back (animator);
			}
			else
			{
				#if DEBUG_LOG
				DebugPrint ("Animator removed: %p\n", animator);
				#endif
				gInstance->animators.remove (animator);
				if (gInstance->animators.empty ())
				{
					gInstance->forget ();
					gInstance = 0;
				}
			}
		}
	}
	
protected:
	static Timer* getInstance ()
	{
		if (gInstance == 0)
			gInstance = new Timer;
		return gInstance;
	}

	Timer ()
	: inTimer (false)
	{
		#if DEBUG_LOG
		DebugPrint ("Animation timer started\n");
		#endif
		timer = new CVSTGUITimer (this, 1000/60); // 60 Hz
		timer->start ();
	}
	
	~Timer ()
	{
		#if DEBUG_LOG
		DebugPrint ("Animation timer stopped\n");
		#endif
		timer->forget ();
		gInstance = 0;
	}
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD
	{
		if (message == CVSTGUITimer::kMsgTimer)
		{
			inTimer = true;
			CBaseObjectGuard guard (this);
			#if DEBUG_LOG
			DebugPrint ("Current Animators : %d\n", animators.size ());
			#endif
			Animators::iterator it = animators.begin ();
			while (it != animators.end ())
			{
				Animator* animator = *it++;
				animator->notify (sender, message);
			}
			inTimer = false;
			it = toRemove.begin ();
			while (it != toRemove.end ())
			{
				Animator* animator = *it++;
				removeAnimator (animator);
			}
			toRemove.clear ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}

	CVSTGUITimer* timer;
	
	typedef std::list<Animator*> Animators;
	Animators animators;
	Animators toRemove;
	bool inTimer;
	static Timer* gInstance;
};
Timer* Timer::gInstance = 0;
///@endcond

/** @class Animator
	see @ref page_animation Support */
//-----------------------------------------------------------------------------
Animator::Animator ()
: inTimer (false)
{
}

//-----------------------------------------------------------------------------
Animator::~Animator ()
{
	Timer::removeAnimator (this);
}

#if VSTGUI_HAS_FUNCTIONAL
//-----------------------------------------------------------------------------
void Animator::addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target, ITimingFunction* timingFunction, NotificationFunction notification)
{
	if (animations.empty ())
		Timer::addAnimator (this);
	removeAnimation (view, name); // cancel animation with same view and name
	SharedPointer<Animation> anim = owned (new Animation (view, name, target, timingFunction, std::move (notification)));
	animations.push_back (anim);
	#if DEBUG_LOG
	DebugPrint ("new animation added: %p - %s\n", view, name);
	#endif
}
#endif

//-----------------------------------------------------------------------------
void Animator::addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target, ITimingFunction* timingFunction, CBaseObject* notificationObject)
{
#if VSTGUI_HAS_FUNCTIONAL
	NotificationFunction notification;
	if (notificationObject)
	{
		SharedPointer<CBaseObject> nObj (notificationObject);
		notification = [nObj] (CView* view, const IdStringPtr name, IAnimationTarget* target) {
			FinishedMessage fmsg (view, name, target);
			nObj->notify (&fmsg, kMsgAnimationFinished);
		};
	}
	addAnimation (view, name, target, timingFunction, std::move (notification));
#else
	if (animations.empty ())
		Timer::addAnimator (this);
	removeAnimation (view, name); // cancel animation with same view and name
	SharedPointer<Animation> anim = owned (new Animation (view, name, target, timingFunction, notificationObject));
	animations.push_back (anim);
	#if DEBUG_LOG
	DebugPrint ("new animation added: %p - %s\n", view, name);
	#endif
#endif
}

//-----------------------------------------------------------------------------
void Animator::removeAnimation (CView* view, IdStringPtr name)
{
	AnimationList::iterator it = animations.begin ();
	while (it != animations.end ())
	{
		Animation* animation = *it++;
		if (animation->view == view && animation->name == name)
		{
			#if DEBUG_LOG
			DebugPrint ("animation removed: %p - %s\n", view, name);
			#endif
			if (animation->done == false)
			{
				animation->done = true;
				animation->target->animationFinished (view, name, true);
			}
			removeAnimation (animation);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void Animator::removeAnimations (CView* view)
{
	AnimationList::iterator it = animations.begin ();
	while (it != animations.end ())
	{
		Animation* animation = *it++;
		if (animation->view == view)
		{
			if (animation->done == false)
			{
				animation->done = true;
				animation->target->animationFinished (animation->view, animation->name.c_str (), true);
			}
			removeAnimation (animation);
		}
	}
}

//-----------------------------------------------------------------------------
void Animator::removeAnimation (Animation* a)
{
	if (inTimer)
	{
		toRemove.push_back (a);
		toRemove.unique ();
	}
	else
	{
		animations.remove (a);
	}
}

//-----------------------------------------------------------------------------
CMessageResult Animator::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		CBaseObjectGuard selfGuard (this);
		inTimer = true;
		uint32_t currentTicks = IPlatformFrame::getTicks ();
		for (AnimationList::iterator it = animations.begin (), end = animations.end (); it != end;)
		{
			SharedPointer<Animation> a (*it);
			if (a->startTime == 0)
			{
			#if DEBUG_LOG
				DebugPrint ("animation start: %p - %s\n", a->view.cast<CView>(), a->name.c_str ());
			#endif
				a->target->animationStart (a->view, a->name.c_str ());
				a->startTime = currentTicks;
			}
			uint32_t time = currentTicks - a->startTime;
			float pos = a->timingFunction->getPosition (time);
			if (pos != a->lastPos)
			{
				a->target->animationTick (a->view, a->name.c_str (), pos);
				a->lastPos = pos;
			}
			if (a->timingFunction->isDone (time))
			{
				a->done = true;
				a->target->animationFinished (a->view, a->name.c_str (), false);
			#if DEBUG_LOG
				DebugPrint ("animation finished: %p - %s\n", a->view.cast<CView>(), a->name.c_str ());
			#endif
				it = animations.erase (it);
			}
			else
				++it;
		}
		inTimer = false;
		AnimationList::const_iterator cit = toRemove.begin ();
		while (cit != toRemove.end ())
		{
			Animation* a = *cit++;
			removeAnimation (a);
		}
		toRemove.clear ();
		if (animations.empty ())
			Timer::removeAnimator (this);
		return kMessageNotified;
	}
	return kMessageUnknown;
}

IdStringPtr kMsgAnimationFinished = "kMsgAnimationFinished";

#if VSTGUI_HAS_FUNCTIONAL
//-----------------------------------------------------------------------------
Animator::Animation::Animation (CView* view, const std::string& name, IAnimationTarget* at, ITimingFunction* t, NotificationFunction notification)
: view (view)
, name (name)
, target (at)
, timingFunction (t)
, notification (std::move (notification))
, startTime (0)
, lastPos (-1)
, done (false)
{
}
#else
//-----------------------------------------------------------------------------
Animator::Animation::Animation (CView* view, const std::string& name, IAnimationTarget* at, ITimingFunction* t, CBaseObject* notificationObject)
: view (view)
, name (name)
, target (at)
, timingFunction (t)
, notificationObject (notificationObject)
, startTime (0)
, lastPos (-1)
, done (false)
{
}
#endif

//-----------------------------------------------------------------------------
Animator::Animation::~Animation ()
{
#if VSTGUI_HAS_FUNCTIONAL
	if (notification)
		notification (view, name.c_str (), target);
#else
	if (notificationObject)
	{
		FinishedMessage fmsg (view, name, target);
		notificationObject->notify (&fmsg, kMsgAnimationFinished);
	}
#endif
	CBaseObject* obj = dynamic_cast<CBaseObject*> (target);
	if (obj)
		obj->forget ();
	else
		delete target;
	obj = dynamic_cast<CBaseObject*> (timingFunction);
	if (obj)
		obj->forget ();
	else
		delete timingFunction;
}

}} // namespaces
