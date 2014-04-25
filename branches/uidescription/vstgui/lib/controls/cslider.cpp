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

#include "cslider.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
// CSlider
//------------------------------------------------------------------------
/*! @class CSlider
Define a slider with a given background and handle.
The range of variation of the handle should be defined.
By default the handler is drawn with transparency.
By clicking Alt+Left Mouse the default value is used.
*/
//------------------------------------------------------------------------
/**
 * CSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
 */
//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CControl (rect, listener, tag, background)
, offset (offset)
, pHandle (handle)
, style (style)
, minPos (iMinPos)
, bFreeClick (true)
{
	setDrawTransparentHandle (true);

	if (pHandle)
	{
		pHandle->remember ();
		widthOfSlider  = pHandle->getWidth ();
		heightOfSlider = pHandle->getHeight ();
	}
	else
	{
		widthOfSlider  = 1; 
		heightOfSlider = 1;
	}

	widthControl  = size.width ();
	heightControl = size.height ();

	if (style & kHorizontal)
	{
		minPos = iMinPos - size.left;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
	}
	else
	{
		minPos = iMinPos - size.top;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
	}

	CPoint p (0, 0);
	setOffsetHandle (p);

	zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param _rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
 */
//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long _rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CControl (rect, listener, tag, background)
, offset (offset)
, pHandle (handle) 
, style (style)
, minPos (0)
, bFreeClick (true)
{
	setDrawTransparentHandle (true);

	if (pHandle)
	{
		pHandle->remember ();
		widthOfSlider  = pHandle->getWidth ();
		heightOfSlider = pHandle->getHeight ();
	}
	else
	{
		widthOfSlider  = 1; 
		heightOfSlider = 1;
	}

	widthControl  = size.width ();
	heightControl = size.height ();
	if (style & kHorizontal)
		rangeHandle = _rangeHandle - widthOfSlider;
	else
		rangeHandle = _rangeHandle - heightOfSlider;

	setOffsetHandle (offsetHandle);
	
	zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSlider::CSlider (const CSlider& v)
: CControl (v)
, offset (v.offset)
, offsetHandle (v.offsetHandle)
, pHandle (v.pHandle)
, style (v.style)
, widthOfSlider (v.widthOfSlider)
, heightOfSlider (v.heightOfSlider)
, rangeHandle (v.rangeHandle)
, minTmp (v.minTmp)
, maxTmp (v.maxTmp)
, minPos (v.minPos)
, widthControl (v.widthControl)
, heightControl (v.heightControl)
, zoomFactor (v.zoomFactor)
, bDrawTransparentEnabled (v.bDrawTransparentEnabled)
, bFreeClick (v.bFreeClick)
{
	if (pHandle)
		pHandle->remember ();
}

//------------------------------------------------------------------------
CSlider::~CSlider ()
{
	if (pHandle)
		pHandle->forget ();
}

//------------------------------------------------------------------------
void CSlider::setStyle (long _style)
{
	style =_style;
}

//------------------------------------------------------------------------
void CSlider::setViewSize (CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	if (style & kHorizontal)
	{
		minPos = rect.left - size.left;
		rangeHandle = rect.getWidth () - (widthOfSlider + offsetHandle.h*2);
	}
	else
	{
		minPos = rect.top - size.top;
		rangeHandle = rect.getHeight () - (heightOfSlider + offsetHandle.v * 2);
	}
	
	widthControl  = rect.width ();
	heightControl = rect.height ();

	setOffsetHandle (offsetHandle);
}

//------------------------------------------------------------------------
void CSlider::setOffsetHandle (const CPoint &val)
{
	offsetHandle = val;

	if (style & kHorizontal)
	{
		minTmp = offsetHandle.h + minPos;
		maxTmp = minTmp + rangeHandle + widthOfSlider;
	}
	else
	{
		minTmp = offsetHandle.v + minPos;
		maxTmp = minTmp + rangeHandle + heightOfSlider;
	}
}

//------------------------------------------------------------------------
void CSlider::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	// draw background
	if (pBackground)
	{
		CRect rect (0, 0, widthControl, heightControl);
		rect.offset (size.left, size.top);
		pBackground->draw (drawContext, rect, offset);
	}
	
	if (pHandle)
	{
		float normValue = (value - getMin ()) / (getMax () - getMin ());
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;
		
		// calc new coords of slider
		CRect rectNew;
		if (style & kHorizontal)
		{
			rectNew.top    = offsetHandle.v;
			rectNew.bottom = rectNew.top + heightOfSlider;	

			rectNew.left   = offsetHandle.h + floor (normValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
		}
		else
		{
			rectNew.left   = offsetHandle.h;
			rectNew.right  = rectNew.left + widthOfSlider;	

			rectNew.top    = offsetHandle.v + floor (normValue * rangeHandle);
			rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

			rectNew.bottom = rectNew.top + heightOfSlider;
			rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
		}
		rectNew.offset (size.left, size.top);

		// draw slider at new position
		pHandle->draw (drawContext, rectNew);
	}

	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if (style & kHorizontal)
		delta = size.left + offsetHandle.h;
	else
		delta = size.top + offsetHandle.v;
	if (!bFreeClick)
	{
		float normValue = (value - getMin ()) / (getMax () - getMin ());
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;
		CCoord actualPos;
		CRect rect;

		if (style & kHorizontal)
		{
			actualPos = offsetHandle.h + (int)(normValue * rangeHandle) + size.left;

			rect.left   = actualPos;
			rect.top    = size.top  + offsetHandle.v;
			rect.right  = rect.left + widthOfSlider;
			rect.bottom = rect.top  + heightOfSlider;

			if (!where.isInside (rect))
				return kMouseEventNotHandled;
			else
				delta += where.h - actualPos;
		}
		else
		{
			actualPos = offsetHandle.v + (int)(normValue * rangeHandle) + size.top;
		
			rect.left   = size.left  + offsetHandle.h;
			rect.top    = actualPos;
			rect.right  = rect.left + widthOfSlider;
			rect.bottom = rect.top  + heightOfSlider;

			if (!where.isInside (rect))
				return kMouseEventNotHandled;
			else
				delta += where.v - actualPos;
		}
	} 
	else
	{
		if (style & kHorizontal)
			delta += widthOfSlider / 2 - 1;
		else
			delta += heightOfSlider / 2 - 1;
	}
	
	oldVal    = getMin ()-1;
	oldButton = buttons;

	beginEdit ();
	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	if (buttons & kShift)
		return kMouseEventHandled;
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseUp (CPoint& where, const long& buttons)
{
	oldButton = 0;
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseMoved (CPoint& where, const long& buttons)
{
	if (oldButton && buttons & kLButton)
	{
		if (oldVal == getMin () - 1)
			oldVal = (value - getMin ()) / (getMax () - getMin ());
			
		if ((oldButton != buttons) && (buttons & kShift))
		{
			oldVal = (value - getMin ()) / (getMax () - getMin ());
			oldButton = buttons;
		}
		else if (!(buttons & kShift))
			oldVal = (value - getMin ()) / (getMax () - getMin ());

		float normValue = getMin ();
		if (style & kHorizontal)
			normValue = (float)(where.h - delta) / (float)rangeHandle;
		else
			normValue = (float)(where.v - delta) / (float)rangeHandle;

		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;

		if (buttons & kShift)
			normValue = oldVal + ((normValue - oldVal) / zoomFactor);

		value = getMin () + ((getMax () - getMin ()) * normValue);
			
		bounceValue ();
    	    
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
bool CSlider::onWheel (const CPoint& where, const float &distance, const long &buttons)
{
	if (!bMouseEnabled)
		return false;

	float normValue = (value - getMin ()) / (getMax () - getMin ());
	if (buttons & kShift)
		normValue += 0.1f * distance * wheelInc;
	else
		normValue += distance * wheelInc;

	value = getMin () + ((getMax () - getMin ()) * normValue);

	bounceValue ();

	if (isDirty ())
	{
		// begin of edit parameter
		beginEdit ();
	
		valueChanged ();
	
		// end of edit parameter
		endEdit ();
	}

	return true;
}

//------------------------------------------------------------------------
long CSlider::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
	case VKEY_UP :
	case VKEY_RIGHT :
	case VKEY_DOWN :
	case VKEY_LEFT :
		{
			float distance = 1.f;
			if (keyCode.virt == VKEY_DOWN || keyCode.virt == VKEY_LEFT)
				distance = -distance;

			float normValue = (value - getMin ()) / (getMax () - getMin ());
			if (keyCode.modifier & MODIFIER_SHIFT)
				normValue += 0.1f * distance * wheelInc;
			else
				normValue += distance * wheelInc;

			value = getMin () + ((getMax () - getMin ()) * normValue);

			bounceValue ();

			if (isDirty ())
			{
				// begin of edit parameter
				beginEdit ();
			
				valueChanged ();
			
				// end of edit parameter
				endEdit ();
			}
		} return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
void CSlider::setHandle (CBitmap *_pHandle)
{
	if (pHandle)
		pHandle->forget ();
	pHandle = _pHandle;
	if (pHandle)
	{
		pHandle->remember ();
		widthOfSlider  = pHandle->getWidth ();
		heightOfSlider = pHandle->getHeight ();
	}
}


//------------------------------------------------------------------------
// CVerticalSlider
//------------------------------------------------------------------------
/*! @class CVerticalSlider
This is the vertical slider. See CSlider.
*/
//------------------------------------------------------------------------
/**
 * CVerticalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
/**
 * CVerticalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CVerticalSlider& slider)
: CSlider (slider)
{}

//------------------------------------------------------------------------
// CHorizontalSlider
//------------------------------------------------------------------------
/*! @class CHorizontalSlider
This is the horizontal slider. See CSlider.
*/
//------------------------------------------------------------------------
/**
 * CHorizontalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
/**
 * CHorizontalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CHorizontalSlider& slider)
: CSlider (slider)
{}

} // namespace