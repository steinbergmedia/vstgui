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

#include "animator.h"
#include "../cvstguitimer.h"
#include "../cview.h"
#include "../platform/iplatformframe.h"

namespace VSTGUI {
namespace Animation {

///@ cond ignore
//-----------------------------------------------------------------------------
class Timer : public CBaseObject
{
public:
	static void addAnimator (Animator* animator)
	{
		getInstance ()->animators.push_back (animator);
	}
	static void removeAnimator (Animator* animator)
	{
		if (getInstance ()->inTimer)
			gInstance->toRemove.push_back (animator);
		else
		{
			gInstance->animators.remove (animator);
			if (gInstance->animators.size () == 0)
			{
				gInstance->forget ();
				gInstance = 0;
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
		timer = new CVSTGUITimer (this, 1000/60); // 60 Hz
		timer->start ();
	}
	
	~Timer ()
	{
		timer->forget ();
	}
	
	CMessageResult notify (CBaseObject* sender, const char* message)
	{
		if (message == CVSTGUITimer::kMsgTimer)
		{
			inTimer = true;
			CBaseObjectGuard guard (this);
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
///@ endcond

//-----------------------------------------------------------------------------
Animator::Animator ()
: inTimer (false)
{
}

//-----------------------------------------------------------------------------
Animator::~Animator ()
{
	if (animations.size () > 0)
	{
		Timer::removeAnimator (this);
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
	removeAnimation (view, name); // cancel animation with same view and name
	if (animations.size () == 0)
		Timer::addAnimator (this);
	animations.push_back (new Animation (view, name, target, timingFunction));
}

//-----------------------------------------------------------------------------
void Animator::removeAnimation (CView* view, const char* name)
{
	std::list<Animation*>::iterator it = animations.begin ();
	while (it != animations.end ())
	{
		if ((*it)->view == view && (*it)->name == name)
		{
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
			removeAnimation (*it);
		it++;
	}
}

//-----------------------------------------------------------------------------
void Animator::removeAnimation (Animation* a)
{
	if (inTimer)
	{
		toRemove.push_back (a);
	}
	else
	{
		a->forget ();
		animations.remove (a);
		if (animations.size () == 0)
			Timer::removeAnimator (this);
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
				a->target->animationStart (a->view, a->name.c_str ());
				a->startTime = currentTicks;
			}
			unsigned long time = currentTicks - a->startTime;
			a->target->animationTick (a->view, a->name.c_str (), a->timingFunction->getPosition (time));
			if (a->timingFunction->isDone (time))
			{
				a->target->animationFinished (a->view, a->name.c_str ());
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
