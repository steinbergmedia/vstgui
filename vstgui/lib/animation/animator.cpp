// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
#include "../dispatchlist.h"
#include "../platform/platformfactory.h"
#include <list>

#define DEBUG_LOG	0 // DEBUG

namespace VSTGUI {
namespace Animation {

///@cond ignore
namespace Detail {

//-----------------------------------------------------------------------------
class Timer : public NonAtomicReferenceCounted
{
public:
	static void addAnimator (Animator* animator)
	{
		getInstance ()->animators.emplace_back (animator);
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
				gInstance->toRemove.emplace_back (animator);
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
					gInstance = nullptr;
				}
			}
		}
	}
	
protected:
	static Timer* getInstance ()
	{
		if (gInstance == nullptr)
			gInstance = new Timer;
		return gInstance;
	}

	Timer ()
	: inTimer (false)
	{
#if DEBUG_LOG
		DebugPrint ("Animation timer started\n");
#endif
		timer = new CVSTGUITimer ([this] (CVSTGUITimer*) {
			onTimer ();
		}, 1000/60); // 60 Hz
	}
	
	~Timer () noexcept override
	{
#if DEBUG_LOG
		DebugPrint ("Animation timer stopped\n");
#endif
		timer->forget ();
		gInstance = nullptr;
	}
	
	void onTimer ()
	{
		inTimer = true;
		auto guard = shared (this);
#if DEBUG_LOG
		DebugPrint ("Current Animators : %d\n", animators.size ());
#endif
		for (auto& animator : animators)
			animator->onTimer ();
		inTimer = false;
		for (auto& animator : toRemove)
			removeAnimator (animator);
		toRemove.clear ();
	}

	CVSTGUITimer* timer;
	
	using Animators = std::list<Animator*>;
	Animators animators;
	Animators toRemove;
	bool inTimer;
	static Timer* gInstance;
};
Timer* Timer::gInstance = nullptr;

//-----------------------------------------------------------------------------
class Animation : public NonAtomicReferenceCounted
{
public:
	Animation (CView* view, const std::string& name, IAnimationTarget* at, ITimingFunction* t,
			   DoneFunction&& notification, bool notifyOnCancel);
	~Animation () noexcept override;
	
	std::string name;
	SharedPointer<CView> view;
	IAnimationTarget* animationTarget;
	ITimingFunction* timingFunction;
	DoneFunction notification;
	uint64_t startTime {0};
	float lastPos {-1.};
	bool done {false};
	bool notifyOnCancel;
};

//-----------------------------------------------------------------------------
Animation::Animation (CView* view, const std::string& name, IAnimationTarget* at,
					  ITimingFunction* t, DoneFunction&& notification, bool notifyOnCancel)
: name (name)
, view (view)
, animationTarget (at)
, timingFunction (t)
, notification (std::move (notification))
, notifyOnCancel (notifyOnCancel)
{
}

//-----------------------------------------------------------------------------
Animation::~Animation () noexcept
{
	if (notification)
		notification (view, name.c_str (), animationTarget);
	if (auto obj = dynamic_cast<IReference*> (animationTarget))
		obj->forget ();
	else
		delete animationTarget;
	if (auto obj = dynamic_cast<IReference*> (timingFunction))
		obj->forget ();
	else
		delete timingFunction;
}

} // Detail

//-----------------------------------------------------------------------------
struct Animator::Impl
{
	DispatchList<SharedPointer<Detail::Animation>> animations;
};
///@endcond

//-----------------------------------------------------------------------------
Animator::Animator ()
{
	pImpl = std::unique_ptr<Impl> (new Impl ());
}

//-----------------------------------------------------------------------------
Animator::~Animator () noexcept
{
	Detail::Timer::removeAnimator (this);
}

//-----------------------------------------------------------------------------
void Animator::addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target,
							 ITimingFunction* timingFunction, DoneFunction notification,
							 bool notifyOnCancel)
{
	if (pImpl->animations.empty ())
		Detail::Timer::addAnimator (this);
	removeAnimation (view, name);
	pImpl->animations.add (makeOwned<Detail::Animation> (view, name, target, timingFunction,
														 std::move (notification), notifyOnCancel));
#if DEBUG_LOG
	DebugPrint ("new animation added: %p - %s\n", view, name);
#endif
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void Animator::addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target, ITimingFunction* timingFunction, CBaseObject* notificationObject)
{
	DoneFunction notification;
	if (notificationObject)
	{
		SharedPointer<CBaseObject> nObj (notificationObject);
		notification = [nObj] (CView* view, const IdStringPtr name, IAnimationTarget* target) {
			FinishedMessage fmsg (view, name, target);
			nObj->notify (&fmsg, kMsgAnimationFinished);
		};
	}
	addAnimation (view, name, target, timingFunction, std::move (notification));
}
#endif

//-----------------------------------------------------------------------------
void Animator::removeAnimation (CView* view, IdStringPtr name)
{
	pImpl->animations.forEach ([&] (const SharedPointer<Detail::Animation>& animation) {
		if (animation->view == view && animation->name == name)
		{
#if DEBUG_LOG
			DebugPrint ("animation removed: %p - %s\n", view, name);
#endif
			if (animation->done == false)
			{
				animation->done = true;
				animation->animationTarget->animationFinished (view, name, true);
			}
			if (!animation->notifyOnCancel)
				animation->notification = nullptr;
			pImpl->animations.remove (animation);
		}
	});
}

//-----------------------------------------------------------------------------
void Animator::removeAnimations (CView* view)
{
	pImpl->animations.forEach ([&] (const SharedPointer<Detail::Animation>& animation) {
		if (animation->view == view)
		{
#if DEBUG_LOG
			DebugPrint ("animation removed: %p - %s\n", view, animation->name.data ());
#endif
			if (animation->done == false)
			{
				animation->done = true;
				animation->animationTarget->animationFinished (view, animation->name.data (), true);
			}
			pImpl->animations.remove (animation);
		}
	});
}

//-----------------------------------------------------------------------------
void Animator::onTimer ()
{
	auto selfGuard = shared (this);
	auto currentTicks = getPlatformFactory ().getTicks ();
	pImpl->animations.forEach ([&] (SharedPointer<Detail::Animation>& animation) {
		if (animation->startTime == 0)
		{
#if DEBUG_LOG
			DebugPrint ("animation start: %p - %s\n", animation->view.cast<CView>(), animation->name.data ());
#endif
			animation->animationTarget->animationStart (animation->view, animation->name.data ());
			animation->startTime = currentTicks;
		}
		uint32_t time = static_cast<uint32_t> (currentTicks - animation->startTime);
		float pos = animation->timingFunction->getPosition (time);
		if (pos != animation->lastPos)
		{
			animation->animationTarget->animationTick (animation->view, animation->name.data (), pos);
			animation->lastPos = pos;
		}
		if (animation->timingFunction->isDone (time))
		{
			animation->done = true;
			animation->animationTarget->animationFinished (animation->view, animation->name.data (), false);
#if DEBUG_LOG
			DebugPrint ("animation finished: %p - %s\n", animation->view.cast<CView>(), animation->name.data ());
#endif
			pImpl->animations.remove (animation);
		}
	});
	if (pImpl->animations.empty ())
		Detail::Timer::removeAnimator (this);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
IdStringPtr kMsgAnimationFinished = "kMsgAnimationFinished";
#endif

}} // namespaces
