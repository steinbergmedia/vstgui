// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "animations.h"
#include "../cview.h"
#include "../cframe.h"
#include "../controls/ccontrol.h"
#include <cassert>
#include <cmath>

namespace VSTGUI {
namespace Animation {
//------------------------------------------------------------------------
/*! @defgroup AnimationTargets Animation Targets
 *	@ingroup animation
 */
//------------------------------------------------------------------------

/** @class AlphaValueAnimation
	see @ref page_animation Support */
//-----------------------------------------------------------------------------
AlphaValueAnimation::AlphaValueAnimation (float endValue, bool forceEndValueOnFinish)
: startValue (0.f)
, endValue (endValue)
, forceEndValueOnFinish (forceEndValueOnFinish)
{
}

//-----------------------------------------------------------------------------
void AlphaValueAnimation::animationStart (CView* view, IdStringPtr name)
{
	startValue = view->getAlphaValue ();
}

//-----------------------------------------------------------------------------
void AlphaValueAnimation::animationTick (CView* view, IdStringPtr name, float pos)
{
	float alpha = startValue + (endValue - startValue) * pos;
	view->setAlphaValue (alpha);
}

//-----------------------------------------------------------------------------
void AlphaValueAnimation::animationFinished (CView* view, IdStringPtr name, bool wasCanceled)
{
	if (!wasCanceled || forceEndValueOnFinish)
		view->setAlphaValue (endValue);
}

//-----------------------------------------------------------------------------
/** @class ViewSizeAnimation
	see @ref page_animation Support */
//-----------------------------------------------------------------------------
ViewSizeAnimation::ViewSizeAnimation (const CRect& inNewRect, bool forceEndValueOnFinish)
: newRect (inNewRect)
, forceEndValueOnFinish (forceEndValueOnFinish)
{
}

//-----------------------------------------------------------------------------
void ViewSizeAnimation::animationStart (CView* view, IdStringPtr name)
{
	startRect = view->getViewSize ();
}

//-----------------------------------------------------------------------------
void ViewSizeAnimation::animationFinished (CView* view, IdStringPtr name, bool wasCanceled)
{
	if (!wasCanceled || forceEndValueOnFinish)
	{
		if (view->getViewSize () != newRect)
		{
			view->invalid ();
			view->setViewSize (newRect);
			view->setMouseableArea (newRect);
			view->invalid ();
		}
	}
}

//-----------------------------------------------------------------------------
void ViewSizeAnimation::animationTick (CView* view, IdStringPtr name, float pos)
{
	CRect r;
	r.left = (int32_t)(startRect.left + ((newRect.left - startRect.left) * pos));
	r.right = (int32_t)(startRect.right + ((newRect.right - startRect.right) * pos));
	r.top = (int32_t)(startRect.top + ((newRect.top - startRect.top) * pos));
	r.bottom = (int32_t)(startRect.bottom + ((newRect.bottom - startRect.bottom) * pos));
	if (view->getViewSize () != r)
	{
		view->invalid ();
		view->setViewSize (r);
		view->setMouseableArea (r);
		view->invalid ();
	}
}

//-----------------------------------------------------------------------------
/** @class ExchangeViewAnimation
	see @ref page_animation Support */
//-----------------------------------------------------------------------------
ExchangeViewAnimation::ExchangeViewAnimation (CView* oldView, CView* newView, AnimationStyle style)
: newView (newView)
, viewToRemove (oldView)
, style (style)
{
	vstgui_assert (newView->isAttached () == false);
	vstgui_assert (viewToRemove->isAttached ());

	if (auto parent = viewToRemove->getParentView ()->asViewContainer ())
		parent->addView (newView);

	init ();
}

//-----------------------------------------------------------------------------
ExchangeViewAnimation::~ExchangeViewAnimation () noexcept
{
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::updateViewSize (CView* view, const CRect& rect)
{
	view->invalid ();
	view->setViewSize (rect);
	view->setMouseableArea (rect);
	view->invalid ();
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::init ()
{
	if (style == kAlphaValueFade)
	{
		oldViewAlphaValueStart = viewToRemove->getAlphaValue ();
		newViewAlphaValueEnd = newView->getAlphaValue ();
		newView->setAlphaValue (0.f);
	}
	else
	{
		destinationRect = viewToRemove->getViewSize ();
		switch (style)
		{
			case kAlphaValueFade: break;
			case kPushInFromLeft:
			{
				doPushInFromLeft (0.f);
				break;
			}
			case kPushInFromRight:
			{
				doPushInFromRight (0.f);
				break;
			}
			case kPushInFromTop:
			{
				doPushInFromTop (0.f);
				break;
			}
			case kPushInFromBottom:
			{
				doPushInFromBottom (0.f);
				break;
			}
			case kPushInOutFromLeft:
			{
				doPushInOutFromLeft (0.f);
				break;
			}
			case kPushInOutFromRight:
			{
				doPushInOutFromRight (0.f);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doAlphaFade (float pos)
{
	float alpha = oldViewAlphaValueStart - (oldViewAlphaValueStart * pos);
	viewToRemove->setAlphaValue (alpha);
	alpha = newViewAlphaValueEnd * pos;
	newView->setAlphaValue (alpha);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doPushInFromLeft (float pos)
{
	CRect viewSize (newView->getViewSize ());
	CCoord leftOrigin = destinationRect.left;
	CCoord offset = viewSize.getWidth () * (1.f - pos);
	viewSize.offset (-viewSize.left, 0);
	viewSize.offset (leftOrigin - offset, 0);
	updateViewSize (newView, viewSize);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doPushInFromRight (float pos)
{
	CRect viewSize (newView->getViewSize ());
	CCoord rightOrigin = destinationRect.left + destinationRect.getWidth ();
	CCoord offset = viewSize.getWidth () * pos;
	viewSize.offset (-viewSize.left, 0);
	viewSize.offset (rightOrigin - offset, 0);
	updateViewSize (newView, viewSize);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doPushInFromTop (float pos)
{
	CRect viewSize (newView->getViewSize ());
	CCoord topOrigin = destinationRect.top;
	CCoord offset = viewSize.getHeight () * (1.f - pos);
	viewSize.offset (0, -viewSize.top);
	viewSize.offset (0, topOrigin - offset);
	updateViewSize (newView, viewSize);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doPushInFromBottom (float pos)
{
	CRect viewSize (newView->getViewSize ());
	CCoord bottomOrigin = destinationRect.top + destinationRect.getHeight ();
	CCoord offset = viewSize.getHeight () * pos;
	viewSize.offset (0, -viewSize.top);
	viewSize.offset (0, bottomOrigin - offset);
	updateViewSize (newView, viewSize);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doPushInOutFromLeft (float pos)
{
	CRect viewSize (newView->getViewSize ());
	CCoord offset = viewSize.getWidth () * (1.f - pos);
	viewSize.offset (-viewSize.left, 0);
	viewSize.offset (destinationRect.left - offset, 0);
	updateViewSize (newView, viewSize);
	
	offset = viewToRemove->getWidth () * pos;
	viewSize = destinationRect;
	viewSize.offset (offset, 0);
	updateViewSize (viewToRemove, viewSize);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::doPushInOutFromRight (float pos)
{
	CRect viewSize (newView->getViewSize ());
	CCoord offset = viewSize.getWidth () * pos;
	viewSize.offset (-viewSize.left, 0);
	viewSize.offset ((destinationRect.left + destinationRect.getWidth ()) - offset, 0);
	updateViewSize (newView, viewSize);
	
	offset = viewToRemove->getWidth () * pos;
	viewSize = destinationRect;
	viewSize.offset (-offset, 0);
	updateViewSize (viewToRemove, viewSize);
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::animationStart (CView* view, IdStringPtr name)
{
	#if DEBUG
	CViewContainer* parent = viewToRemove->getParentView ()->asViewContainer ();
	vstgui_assert (view == parent);
	#endif
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::animationTick (CView* view, IdStringPtr name, float pos)
{
	switch (style)
	{
		case kAlphaValueFade:
		{
			doAlphaFade (pos);
			break;
		}
		case kPushInFromLeft:
		{
			doPushInFromLeft (pos);
			break;
		}
		case kPushInFromRight:
		{
			doPushInFromRight (pos);
			break;
		}
		case kPushInFromTop:
		{
			doPushInFromTop (pos);
			break;
		}
		case kPushInFromBottom:
		{
			doPushInFromBottom (pos);
			break;
		}
		case kPushInOutFromLeft:
		{
			doPushInOutFromLeft (pos);
			break;
		}
		case kPushInOutFromRight:
		{
			doPushInOutFromRight (pos);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void ExchangeViewAnimation::animationFinished (CView* view, IdStringPtr name, bool wasCanceled)
{
	if (wasCanceled)
	{
		animationTick (nullptr, nullptr, 1.f);
	}
	if (auto viewContainer = viewToRemove->getParentView ()->asViewContainer ())
	{
		viewContainer->removeView (viewToRemove);
	}
}

//-----------------------------------------------------------------------------
/** @class ControlValueAnimation
	see @ref page_animation Support */
//-----------------------------------------------------------------------------
ControlValueAnimation::ControlValueAnimation (float endValue, bool forceEndValueOnFinish)
: startValue (0.f)
, endValue (endValue)
, forceEndValueOnFinish (forceEndValueOnFinish)
{
}

//-----------------------------------------------------------------------------
void ControlValueAnimation::animationStart (CView* view, IdStringPtr name)
{
	CControl* control = dynamic_cast<CControl*> (view);
	if (control)
		startValue = control->getValue ();
}

//-----------------------------------------------------------------------------
void ControlValueAnimation::animationTick (CView* view, IdStringPtr name, float pos)
{
	CControl* control = dynamic_cast<CControl*> (view);
	if (control)
	{
		float value = startValue + (endValue - startValue) * pos;
		control->setValue (value);
	}
}

//-----------------------------------------------------------------------------
void ControlValueAnimation::animationFinished (CView* view, IdStringPtr name, bool wasCanceled)
{
	CControl* control = dynamic_cast<CControl*> (view);
	if (control)
	{
		if (!wasCanceled || forceEndValueOnFinish)
			control->setValue (endValue);
	}
}

}} // namespaces
