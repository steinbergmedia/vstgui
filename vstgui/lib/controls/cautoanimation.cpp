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
								CBitmap* background)
: CControl (size, listener, tag, background)
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	heightOfOneImage = size.getHeight ();
	setNumSubPixmaps (background ? (int32_t)(background->getHeight () / heightOfOneImage) : 0);

	totalHeightOfBitmap = heightOfOneImage * getNumSubPixmaps ();
#else
#endif
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
/**
 * CAutoAnimation constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag,
								CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background), offset (offset)
{
	heightOfOneImage = size.getHeight ();
	setNumSubPixmaps (background ? (int32_t)(background->getHeight () / heightOfOneImage) : 0);
	totalHeightOfBitmap = heightOfOneImage * getNumSubPixmaps ();
}

//------------------------------------------------------------------------
/**
 * CAutoAnimation constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag,
								int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background,
								const CPoint& offset)
: CControl (size, listener, tag, background), offset (offset)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	totalHeightOfBitmap = heightOfOneImage * getNumSubPixmaps ();
	setMin (0.f);
	setMax ((float)(totalHeightOfBitmap - (heightOfOneImage + 1.)));
}

//------------------------------------------------------------------------
void CAutoAnimation::setBitmapOffset (const CPoint& off)
{
	offset = off;
	invalid ();
}

//------------------------------------------------------------------------
CPoint CAutoAnimation::getBitmapOffset () const { return offset; }

#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CAutoAnimation& v)
: CControl (v)
#if VSTGUI_ENABLE_DEPRECATED_METHODS
, offset (v.offset)
, totalHeightOfBitmap (v.totalHeightOfBitmap)
#endif
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
#endif
}

//------------------------------------------------------------------------
bool CAutoAnimation::isWindowOpened () const { return bWindowOpened; }

//------------------------------------------------------------------------
void CAutoAnimation::draw (CDrawContext *pContext)
{
	if (isWindowOpened ())
	{
		if (auto bitmap = getDrawBackground ())
		{
			if (auto mfb = dynamic_cast<CMultiFrameBitmap*> (bitmap))
			{
				auto frameIndex = getMultiFrameBitmapIndex (*mfb, getValueNormalized ());
				mfb->drawFrame (pContext, frameIndex, getViewSize ().getTopLeft ());
			}
			else
			{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
				CPoint where;
				where.y = (int32_t)value + offset.y;
				where.x = offset.x;
				bitmap->draw (pContext, getViewSize (), where);
#else
				CView::draw (pContext);
#endif
			}
		}
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CAutoAnimation::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		if (!isWindowOpened ())
		{	
			value = 0;
			openWindow ();
			invalid ();
			valueChanged ();
		}
		else
		{                                                                       
			// stop info animation
			value = 0; // draw first pic of bitmap
			invalid ();
			closeWindow ();
			valueChanged ();
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool CAutoAnimation::attached (CView* parent)
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
bool CAutoAnimation::removed (CView* parent)
{
	timer = nullptr;
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CAutoAnimation::startTimer ()
{
	if (animationFrameTime > 0)
	{
		timer = makeOwned<CVSTGUITimer> (
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
		if (auto mfb = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			auto numFrames = getMultiFrameBitmapRangeLength (*mfb);
			setMin (0.f);
			setMax (numFrames);
#if VSTGUI_ENABLE_DEPRECATED_METHODS
			heightOfOneImage = mfb->getFrameSize ().y;
			totalHeightOfBitmap = heightOfOneImage * numFrames;
#endif
		}
	}
}

//------------------------------------------------------------------------
void CAutoAnimation::setBackground (CBitmap* background)
{
	CControl::setBackground (background);
	updateMinMaxFromBackground ();
}

//------------------------------------------------------------------------
void CAutoAnimation::nextPixmap ()
{
	if (auto bitmap = getDrawBackground ())
	{
		if (dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			if (getValue () == getMax ())
				setValue (getMin ());
			else
				setValue (getValue () + 1.f);
			return;
		}
	}
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	value += (float)heightOfOneImage;
	if (value >= (totalHeightOfBitmap - heightOfOneImage))
		value = 0;
#endif
}

//------------------------------------------------------------------------
void CAutoAnimation::previousPixmap ()
{
	if (auto bitmap = getDrawBackground ())
	{
		if (dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			if (getValue () == getMin ())
				setValue (getMax ());
			else
				setValue (getValue () - 1.f);
			return;
		}
	}
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	value -= (float)heightOfOneImage;
	if (value < 0.f)
		value = (float)(totalHeightOfBitmap - heightOfOneImage - 1);
#endif
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
