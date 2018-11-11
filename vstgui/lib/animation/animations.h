// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include "ianimationtarget.h"
#include "../crect.h"

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
/// @brief animates the alpha value of the view
/// @ingroup AnimationTargets
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class AlphaValueAnimation : public IAnimationTarget, public NonAtomicReferenceCounted
{
public:
	AlphaValueAnimation (float endValue, bool forceEndValueOnFinish = false);

	void animationStart (CView* view, IdStringPtr name) override;
	void animationTick (CView* view, IdStringPtr name, float pos) override;
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override;
protected:
	float startValue;
	float endValue;
	bool forceEndValueOnFinish;
};

//-----------------------------------------------------------------------------
/// @brief animates the view size of the view
/// @ingroup AnimationTargets
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class ViewSizeAnimation : public IAnimationTarget, public NonAtomicReferenceCounted
{
public:
	ViewSizeAnimation (const CRect& newRect, bool forceEndValueOnFinish = false);

	void animationStart (CView* view, IdStringPtr name) override;
	void animationTick (CView* view, IdStringPtr name, float pos) override;
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override;
protected:
	CRect startRect;
	CRect newRect;
	bool forceEndValueOnFinish;
};

//-----------------------------------------------------------------------------
/// @brief exchange a view by another view with an animation
/// @ingroup AnimationTargets
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class ExchangeViewAnimation : public IAnimationTarget, public NonAtomicReferenceCounted
{
public:
	enum AnimationStyle {
		kAlphaValueFade = 0,
		kPushInFromLeft,
		kPushInFromRight,
		kPushInFromTop,
		kPushInFromBottom,
		kPushInOutFromLeft,
		kPushInOutFromRight
	};

	/** oldView must be a subview of the animation view */
	ExchangeViewAnimation (CView* oldView, CView* newView, AnimationStyle style = kAlphaValueFade);
	~ExchangeViewAnimation () noexcept override;

	void animationStart (CView* view, IdStringPtr name) override;
	void animationTick (CView* view, IdStringPtr name, float pos) override;
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override;
protected:

	void init ();
	void doAlphaFade (float pos);
	void doPushInFromLeft (float pos);
	void doPushInFromRight (float pos);
	void doPushInFromTop (float pos);
	void doPushInFromBottom (float pos);
	void doPushInOutFromLeft (float pos);
	void doPushInOutFromRight (float pos);

	void updateViewSize (CView* view, const CRect& rect);

	SharedPointer<CView> newView;
	SharedPointer<CView> viewToRemove;
	AnimationStyle style;
	float newViewAlphaValueEnd;
	float oldViewAlphaValueStart;
	CRect destinationRect;
};

//-----------------------------------------------------------------------------
/// @brief animates the value of a CControl
/// @ingroup AnimationTargets
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class ControlValueAnimation : public IAnimationTarget, public NonAtomicReferenceCounted
{
public:
	ControlValueAnimation (float endValue, bool forceEndValueOnFinish = false);

	void animationStart (CView* view, IdStringPtr name) override;
	void animationTick (CView* view, IdStringPtr name, float pos) override;
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override;
protected:
	float startValue;
	float endValue;
	bool forceEndValueOnFinish;
};

} // Animation
} // VSTGUI

