// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cautoanimation.h"
#include "../algorithm.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"

namespace VSTGUI {
//------------------------------------------------------------------------
// CAutoAnimation
//------------------------------------------------------------------------
/*! @class CAutoAnimation
An auto-animation control contains a given number of subbitmaps which can be displayed in loop.
Two functions allows to get the previous or the next subbitmap (these functions increase or decrease
the current value of this control). Use a CMultiFrameBitmap for its background bitmap.
*/
// displays bitmaps within a (child-) window
//------------------------------------------------------------------------
/**
 * CAutoAnimation constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag,
								const SharedPointer<CBitmap>& background)
: CControl (size, listener, tag, background)
{
}

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CAutoAnimation& v) : CControl (v) {}

//------------------------------------------------------------------------
bool CAutoAnimation::isWindowOpened () const { return bWindowOpened; }

//------------------------------------------------------------------------
void CAutoAnimation::draw (CDrawContext& context)
{
	if (isWindowOpened ())
	{
		if (auto bitmap = getDrawBackground ())
		{
			if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
			{
				auto frameIndex = getMultiFrameBitmapIndex (*mfb.get (), getValueNormalized ());
				mfb->drawFrame (context, frameIndex, getViewSize ().getTopLeft ());
			}
			else
			{
				CView::draw (context);
			}
		}
	}
}

//------------------------------------------------------------------------
CMouseEventResult CAutoAnimation::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		if (!isWindowOpened ())
		{
			setValue (0);
			openWindow ();
			invalid ();
			valueChanged ();
		}
		else
		{                                                                       
			// stop info animation
			setValue (0); // draw first pic of bitmap
			invalid ();
			closeWindow ();
			valueChanged ();
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool CAutoAnimation::attached (CViewContainer& parent)
{
	if (CControl::attached (parent))
	{
		if (animationFrameTime > 0 && isWindowOpened ())
			startTimer ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CAutoAnimation::removed (CViewContainer& parent)
{
	timer = nullptr;
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CAutoAnimation::startTimer ()
{
	if (animationFrameTime > 0)
	{
		timer = makeShared<CVSTGUITimer> (
			[this] (auto*) {
				nextPixmap ();
				invalid ();
			},
			animationFrameTime, true);
	}
}

//------------------------------------------------------------------------
void CAutoAnimation::openWindow ()
{
	bWindowOpened = true;
	if (isAttached ())
		startTimer ();
}

//------------------------------------------------------------------------
void CAutoAnimation::closeWindow ()
{
	bWindowOpened = false;
	timer = nullptr;
}

//------------------------------------------------------------------------
void CAutoAnimation::updateMinMaxFromBackground ()
{
	if (auto bitmap = getDrawBackground ())
	{
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			auto numFrames = getMultiFrameBitmapRangeLength (*mfb.get ());
			setMin (0.f);
			setMax (numFrames);
		}
	}
}

//------------------------------------------------------------------------
void CAutoAnimation::setBackground (const SharedPointer<CBitmap>& background)
{
	CControl::setBackground (background);
	updateMinMaxFromBackground ();
}

//------------------------------------------------------------------------
void CAutoAnimation::nextPixmap ()
{
	if (auto bitmap = getDrawBackground ())
	{
		if (bitmap.cast<CMultiFrameBitmap> ())
		{
			if (getValue () == getMax ())
				setValue (getMin ());
			else
				setValue (getValue () + 1.f);
			return;
		}
	}
}

//------------------------------------------------------------------------
void CAutoAnimation::previousPixmap ()
{
	if (auto bitmap = getDrawBackground ())
	{
		if (bitmap.cast<CMultiFrameBitmap> ())
		{
			if (getValue () == getMin ())
				setValue (getMax ());
			else
				setValue (getValue () - 1.f);
			return;
		}
	}
}

//------------------------------------------------------------------------
void CAutoAnimation::setAnimationTime (uint32_t animationTime)
{
	animationFrameTime = animationTime;
	if (timer)
		startTimer ();
}

//------------------------------------------------------------------------
uint32_t CAutoAnimation::getAnimationTime () const { return animationFrameTime; }

} // VSTGUI
