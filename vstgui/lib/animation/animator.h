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
	/** adds an animation.
		Animation and timingFunction is now owned by the animator.
		An already running animation for view with name will be canceled.
		The notification function will be called when the animation has finished or on cancellation
		of the animation if notifyOnCancel is true (new in 4.11)
	*/
	void addAnimation (const SharedPointer<CView>& view, IdStringPtr name,
					   const SharedPointer<IAnimationTarget>& target,
					   const SharedPointer<ITimingFunction>& timingFunction,
					   DoneFunction notification = nullptr, bool notifyOnCancel = false);

	/** removes an animation.
		If animation has the IReference interface forget() will be called otherwise it is deleted.
		The same will be done with the timingFunction.
	*/
	void removeAnimation (const SharedPointer<CView>& view, IdStringPtr name);

	/** removes all animations for view */
	void removeAnimations (const SharedPointer<CView>& view);
	//@}

	/// @cond ignore

	Animator ();	// do not use this, instead use CFrame::getAnimator()
	void onTimer ();

protected:
	VSTGUI_SHAREDPTR_FRIEND (Animator)
	~Animator () noexcept override;

	struct Impl;
	std::unique_ptr<Impl> pImpl;
	/// @endcond
};

} // Animation
} // VSTGUI
