//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#include "cautoanimation.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"

namespace VSTGUI {

//------------------------------------------------------------------------
// CAutoAnimation
//------------------------------------------------------------------------
/*! @class CAutoAnimation
An auto-animation control contains a given number of subbitmaps which can be displayed in loop.
Two functions allows to get the previous or the next subbitmap (these functions increase or decrease the current value of this control).
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

//------------------------------------------------------------------------
CAutoAnimation::CAutoAnimation (const CAutoAnimation& v)
: CControl (v)
, offset (v.offset)
, totalHeightOfBitmap (v.totalHeightOfBitmap)
, bWindowOpened (v.bWindowOpened)
{
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
}

//------------------------------------------------------------------------
CAutoAnimation::~CAutoAnimation ()
{}

//------------------------------------------------------------------------
void CAutoAnimation::draw (CDrawContext *pContext)
{
	if (isWindowOpened ())
	{	
		CPoint where;
		where.y = (int32_t)value + offset.y;
		where.x = offset.x;
		
		if (getDrawBackground ())
		{
			getDrawBackground ()->draw (pContext, getViewSize (), where);
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
void CAutoAnimation::nextPixmap ()
{
	value += (float)heightOfOneImage;
	if (value >= (totalHeightOfBitmap - heightOfOneImage))
		value = 0;
}

//------------------------------------------------------------------------
void CAutoAnimation::previousPixmap ()
{
	value -= (float)heightOfOneImage;
	if (value < 0.f)
		value = (float)(totalHeightOfBitmap - heightOfOneImage - 1);
}

} // namespace
