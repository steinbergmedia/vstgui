//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __animator__
#define __animator__

#include "../vstguibase.h"
#include <list>
#include <string>

namespace VSTGUI {
class CVSTGUITimer;
class CView;

namespace Animation {

//-----------------------------------------------------------------------------
/// @brief Animation target interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IAnimationTarget
{
public:
	virtual ~IAnimationTarget () {}

	virtual void animationStart (CView* view, IdStringPtr name) = 0;						///< animation starts
	virtual void animationTick (CView* view, IdStringPtr name, float pos) = 0;				///< pos is a normalized value between zero and one
	virtual void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) = 0;	///< animation ended
};

//-----------------------------------------------------------------------------
/// @brief Animation timing function interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class ITimingFunction
{
public:
	virtual ~ITimingFunction () {}

	virtual float getPosition (uint32_t milliseconds) = 0;
	virtual bool isDone (uint32_t milliseconds) = 0;
};

//-----------------------------------------------------------------------------
/// @brief Animation runner
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class Animator : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// @name Adding and removing Animations
	//-----------------------------------------------------------------------------
	//@{
	/** adds an animation.
		Animation and timingFunction is now owned by the animator.
		An already running animation for view with name will be canceled.
		If a notificationObject is supplied, it will be notified when the animation has finished @see FinishedMessage.
	*/
	void addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target, ITimingFunction* timingFunction, CBaseObject* notificationObject = 0);

	/** removes an animation.
		If animation is a CBaseObject forget() will be called otherwise it is deleted.
		The same will be done with the timingFunction.
	*/
	void removeAnimation (CView* view, IdStringPtr name);

	/** removes all animations for view */
	void removeAnimations (CView* view);
	//@}

	/// @cond ignore

	Animator ();	// do not use this, instead use CFrame::getAnimator()
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS_NOCOPY(Animator, CBaseObject)
protected:

	~Animator ();

	class Animation : public CBaseObject
	{
	public:
		Animation (CView* view, const std::string& name, IAnimationTarget* at, ITimingFunction* t, CBaseObject* notificationObject);
		~Animation ();

		std::string name;
		SharedPointer<CView> view;
		IAnimationTarget* target;
		ITimingFunction* timingFunction;
		SharedPointer<CBaseObject> notificationObject;
		uint32_t startTime;
		float lastPos;
		bool done;
	};

	void removeAnimation (Animation* a);

	typedef std::list<SharedPointer<Animation> > AnimationList;
	AnimationList animations;
	AnimationList toRemove;
	bool inTimer;
	/// @endcond
};

extern IdStringPtr kMsgAnimationFinished;	///< message sent to the notificationObject when the animation has finished, the sender parameter will be a FinishedMessage object.

//-----------------------------------------------------------------------------
/// @brief Animation Finished Message Object
///
/// The FinishedMessage will be sent to the notificationObject when the animation has finished
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class FinishedMessage : public CBaseObject
{
public:
	FinishedMessage (CView* view, const std::string& name, IAnimationTarget* target) : view (view), name (name), target (target) {}
	
	CView* getView () const { return view; }
	const IdStringPtr getName () const { return name.c_str (); }
	IAnimationTarget* getTarget () const { return target; }

	CLASS_METHODS_NOCOPY(FinishedMessage, CBaseObject)
protected:
	CView* view;
	const std::string& name;
	IAnimationTarget* target;
};

}} // namespaces

#endif // __animator__
