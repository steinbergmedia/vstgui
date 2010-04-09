//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

/**
@page animation Animation

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

An animation is made up of an @link VSTGUI::Animation::IAnimationTarget IAnimationTarget @endlink and an @link VSTGUI::Animation::ITimingFunction ITimingFunction @endlink object.

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

	CMouseEventResult onMouseEntered (CPoint &where, const long& buttons)
	{
		// this adds an animation which takes 200 ms to make a linear alpha fade from the current value to 1
		addAnimation ("AlphaValueAnimation", new AlphaValueAnimation (1.f), new LinearTimingFunction (200));
		return kMouseEventHandled;
	}
	
	CMouseEventResult onMouseExited (CPoint &where, const long& buttons)
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
 */
//-----------------------------------------------------------------------------

#include "animator.h"
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
				if (gInstance->animators.size () == 0)
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
	
	CMessageResult notify (CBaseObject* sender, const char* message)
	{
		if (message == CVSTGUITimer::kMsgTimer)
		{
			inTimer = true;
			CBaseObjectGuard guard (this);
			#if DEBUG_LOG
			DebugPrint ("Current Animators : %d\n", animators.size ());
			#endif
			std::list<Animator*>::iterator it = animators.begin ();
			while (it != animators.end ())
			{
				(*it)->notify (sender, message);
				it++;
			}
			inTimer = false;
			it = toRemove.begin ();
			while (it != toRemove.end ())
			{
				removeAnimator (*it);
				it++;
			}
			toRemove.clear ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}

	CVSTGUITimer* timer;
	std::list<Animator*>animators;
	std::list<Animator*>toRemove;
	bool inTimer;
	static Timer* gInstance;
};
Timer* Timer::gInstance = 0;
///@endcond

//-----------------------------------------------------------------------------
Animator::Animator ()
: inTimer (false)
{
}

//-----------------------------------------------------------------------------
Animator::~Animator ()
{
	Timer::removeAnimator (this);
	if (animations.size () > 0)
	{
		std::list<Animation*>::iterator it = animations.begin ();
		while (it != animations.end ())
		{
			(*it)->forget ();
			it++;
		}
	}
}

//-----------------------------------------------------------------------------
void Animator::addAnimation (CView* view, const char* name, IAnimationTarget* target, ITimingFunction* timingFunction)
{
	if (animations.size () == 0)
		Timer::addAnimator (this);
	removeAnimation (view, name); // cancel animation with same view and name
	animations.push_back (new Animation (view, name, target, timingFunction));
	#if DEBUG_LOG
	DebugPrint ("new animation added: %p - %s\n", view, name);
	#endif
}

//-----------------------------------------------------------------------------
void Animator::removeAnimation (CView* view, const char* name)
{
	std::list<Animation*>::iterator it = animations.begin ();
	while (it != animations.end ())
	{
		if ((*it)->view == view && (*it)->name == name)
		{
			#if DEBUG_LOG
			DebugPrint ("animation removed: %p - %s\n", view, name);
			#endif
			(*it)->target->animationFinished (view, name, true);
			removeAnimation (*it);
			break;
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
void Animator::removeAnimations (CView* view)
{
	std::list<Animation*>::iterator it = animations.begin ();
	while (it != animations.end ())
	{
		if ((*it)->view == view)
		{
			(*it)->target->animationFinished ((*it)->view, (*it)->name.c_str (), true);
			removeAnimation (*it);
		}
		it++;
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
		a->forget ();
		animations.remove (a);
	}
}

//-----------------------------------------------------------------------------
CMessageResult Animator::notify (CBaseObject* sender, const char* message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		CBaseObjectGuard selfGuard (this);
		inTimer = true;
		unsigned long currentTicks = IPlatformFrame::getTicks ();
		std::list<Animation*>::iterator it = animations.begin ();
		while (it != animations.end ())
		{
			Animation* a = (*it);
			CBaseObjectGuard guard (a);
			if (a->startTime == 0)
			{
				#if DEBUG_LOG
				DebugPrint ("animation start: %p - %s\n", a->view, a->name.c_str ());
				#endif
				a->target->animationStart (a->view, a->name.c_str ());
				a->startTime = currentTicks;
			}
			unsigned long time = currentTicks - a->startTime;
			float pos = a->timingFunction->getPosition (time);
			if (pos != a->lastPos)
			{
				a->target->animationTick (a->view, a->name.c_str (), pos);
				a->lastPos = pos;
			}
			if (a->timingFunction->isDone (time))
			{
				a->target->animationFinished (a->view, a->name.c_str (), false);
				#if DEBUG_LOG
				DebugPrint ("animation finished: %p - %s\n", a->view, a->name.c_str ());
				#endif
				removeAnimation (a);
			}
			it++;
		}
		inTimer = false;
		std::list<Animation*>::const_iterator cit = toRemove.begin ();
		while (cit != toRemove.end ())
		{
			removeAnimation (*cit);
			cit++;
		}
		toRemove.clear ();
		if (animations.size () == 0)
			Timer::removeAnimator (this);
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
Animator::Animation::Animation (CView* view, const std::string& name, IAnimationTarget* at, ITimingFunction* t)
: view (view)
, name (name)
, target (at)
, timingFunction (t)
, startTime (0)
, lastPos (-1)
{
	view->remember ();
}

//-----------------------------------------------------------------------------
Animator::Animation::~Animation ()
{
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
	view->forget ();
}

}} // namespaces
