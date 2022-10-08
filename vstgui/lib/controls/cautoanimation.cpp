// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cautoanimation.h"
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
 * @param offset unused
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
, bWindowOpened (false)
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
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
, bWindowOpened (false)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	totalHeightOfBitmap = heightOfOneImage * getNumSubPixmaps ();
	setMin (0.f);
	setMax ((float)(totalHeightOfBitmap - (heightOfOneImage + 1.)));
}
#endif

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CAutoAnimation& v)
: CControl (v)
, offset (v.offset)
#if VSTGUI_ENABLE_DEPRECATED_METHODS
, totalHeightOfBitmap (v.totalHeightOfBitmap)
#endif
, bWindowOpened (v.bWindowOpened)
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
#endif
}

//------------------------------------------------------------------------
void CAutoAnimation::draw (CDrawContext *pContext)
{
	if (isWindowOpened ())
	{
		if (auto bitmap = getDrawBackground ())
		{
			if (auto frameBitmap = dynamic_cast<CMultiFrameBitmap*> (bitmap))
			{
				auto frameIndex = getValueNormalized () * frameBitmap->getNumFrames ();
				frameBitmap->drawFrame (pContext, frameIndex, getViewSize ().getTopLeft ());
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
void CAutoAnimation::openWindow ()
{
	bWindowOpened = true;
}

//------------------------------------------------------------------------
void CAutoAnimation::closeWindow ()
{
	bWindowOpened = false;
}

//------------------------------------------------------------------------
void CAutoAnimation::updateMinMaxFromBackground ()
{
	if (auto bitmap = getDrawBackground ())
	{
		if (auto frameBitmap = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			setMin (0.f);
			setMax (frameBitmap->getHeight ());
#if VSTGUI_ENABLE_DEPRECATED_METHODS
			heightOfOneImage = frameBitmap->getFrameSize ().y;
			totalHeightOfBitmap = heightOfOneImage * frameBitmap->getNumFrames ();
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
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	value += (float)heightOfOneImage;
	if (value >= (totalHeightOfBitmap - heightOfOneImage))
		value = 0;
#else
	if (auto bitmap = getDrawBackground ())
	{
		if (auto frameBitmap = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			value += frameBitmap->getFrameSize ().y;
			if (value >= frameBitmap->getHeight ())
				value = 0.f;
		}
	}
#endif
}

//------------------------------------------------------------------------
void CAutoAnimation::previousPixmap ()
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	value -= (float)heightOfOneImage;
	if (value < 0.f)
		value = (float)(totalHeightOfBitmap - heightOfOneImage - 1);
#else
	if (auto bitmap = getDrawBackground ())
	{
		if (auto frameBitmap = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			value -= frameBitmap->getFrameSize ().y;
			if (value < 0.f)
				value = frameBitmap->getHeight ();
		}
	}
#endif
}

} // VSTGUI
