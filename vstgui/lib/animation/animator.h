// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include <string>
#include <functional>
#include <memory>

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
/// @brief Animation runner
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class Animator : public NonAtomicReferenceCounted
{
public:
	//-----------------------------------------------------------------------------
	/// @name Adding and removing Animations
	//-----------------------------------------------------------------------------
	//@{
	VSTGUI_DEPRECATED(
	/** adds an animation.
		Animation and timingFunction is now owned by the animator.
		An already running animation for view with name will be canceled.
		If a notificationObject is supplied, it will be notified when the animation has finished @see FinishedMessage.
	*/
	void addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target, ITimingFunction* timingFunction, CBaseObject* notificationObject);)

	/** adds an animation.
		Animation and timingFunction is now owned by the animator.
		An already running animation for view with name will be canceled.
		The notification function will be called when the animation has finished or on cancelation
		of the animation if notifyOnCancel is true (new in 4.11)
	*/
	void addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target,
					   ITimingFunction* timingFunction, DoneFunction notification = nullptr,
					   bool notifyOnCancel = false);

	/** removes an animation.
		If animation has the IReference interface forget() will be called otherwise it is deleted.
		The same will be done with the timingFunction.
	*/
	void removeAnimation (CView* view, IdStringPtr name);

	/** removes all animations for view */
	void removeAnimations (CView* view);
	//@}

	/// @cond ignore

	Animator ();	// do not use this, instead use CFrame::getAnimator()
	void onTimer ();

protected:
	~Animator () noexcept override;

	struct Impl;
	std::unique_ptr<Impl> pImpl;
	/// @endcond
};

#if VSTGUI_ENABLE_DEPRECATED_METHODS
/** message sent to the notificationObject when the animation has finished, the sender parameter will be a FinishedMessage object. */
extern IdStringPtr kMsgAnimationFinished;

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
	IdStringPtr getName () const { return name.c_str (); }
	IAnimationTarget* getTarget () const { return target; }

	CLASS_METHODS_NOCOPY(FinishedMessage, CBaseObject)
protected:
	CView* view;
	const std::string& name;
	IAnimationTarget* target;
};
#endif

} // Animation
} // VSTGUI
