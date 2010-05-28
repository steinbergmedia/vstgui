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
//! @brief Animation target interface
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
//! @brief Animation timing function interface
//-----------------------------------------------------------------------------
class ITimingFunction
{
public:
	virtual ~ITimingFunction () {}

	virtual float getPosition (uint32_t milliseconds) = 0;
	virtual bool isDone (uint32_t milliseconds) = 0;
};

//-----------------------------------------------------------------------------
//! @brief Animation runner
//-----------------------------------------------------------------------------
class Animator : public CBaseObject
{
public:
	Animator ();	///< do not use this, instead use CFrame::getAnimator()

	//-----------------------------------------------------------------------------
	/// @name Adding and removing Animations
	//-----------------------------------------------------------------------------
	//@{
	/** adds an animation. animation and timingFunction is now owned by the animator. An already running animation for view with name will be canceled. */
	void addAnimation (CView* view, IdStringPtr name, IAnimationTarget* target, ITimingFunction* timingFunction);
	/** removes an animation. if animation is a CBaseObject forget() will be called otherwise it is deleted. The same will be done with the timingFunction. */
	void removeAnimation (CView* view, IdStringPtr name);

	/** removes all animations for view */
	void removeAnimations (CView* view);
	//@}

	/// @cond ignore

	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	CLASS_METHODS_NOCOPY(Animator, CBaseObject)
protected:

	~Animator ();

	class Animation : public CBaseObject
	{
	public:
		Animation (CView* view, const std::string& name, IAnimationTarget* at, ITimingFunction* t);
		~Animation ();

		std::string name;
		CView* view;
		IAnimationTarget* target;
		ITimingFunction* timingFunction;
		uint32_t startTime;
		float lastPos;
	};

	void removeAnimation (Animation* a);

	std::list<Animation*> animations;
	std::list<Animation*> toRemove;
	bool inTimer;
	/// @endcond
};

}} // namespaces

#endif // __animator__
