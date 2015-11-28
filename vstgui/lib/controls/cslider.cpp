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

#include "cslider.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include <cmath>

namespace VSTGUI {

bool CSlider::kAlwaysUseZoomFactor = false;

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
CSlider::CSlider (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CControl (rect, listener, tag, background)
, offset (offset)
, pHandle (handle)
, style (style)
, minPos (iMinPos)
, mode (kFreeClickMode)
, drawStyle (0)
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

	widthControl  = getViewSize ().getWidth ();
	heightControl = getViewSize ().getHeight ();

	if (style & kHorizontal)
	{
		minPos = iMinPos - getViewSize ().left;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
	}
	else
	{
		minPos = iMinPos - getViewSize ().top;
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
CSlider::CSlider (const CRect &rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t _rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CControl (rect, listener, tag, background)
, offset (offset)
, pHandle (handle) 
, style (style)
, mode (kFreeClickMode)
, minPos (0)
, drawStyle (0)
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

	widthControl  = getViewSize ().getWidth ();
	heightControl = getViewSize ().getHeight ();
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
, drawStyle (v.drawStyle)
, mode (v.mode)
, backColor (v.backColor)
, frameColor (v.frameColor)
, valueColor (v.valueColor)
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
void CSlider::setStyle (int32_t _style)
{
	style =_style;
}

//------------------------------------------------------------------------
void CSlider::setViewSize (const CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	if (style & kHorizontal)
	{
		minPos = rect.left - getViewSize ().left;
		rangeHandle = rect.getWidth () - (widthOfSlider + offsetHandle.x * 2);
	}
	else
	{
		minPos = rect.top - getViewSize ().top;
		rangeHandle = rect.getHeight () - (heightOfSlider + offsetHandle.y * 2);
	}
	
	widthControl  = rect.getWidth ();
	heightControl = rect.getHeight ();

	setOffsetHandle (offsetHandle);
}

//------------------------------------------------------------------------
bool CSlider::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getDrawBackground ()->getHeight ());
		setViewSize (vs, true);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CSlider::setOffsetHandle (const CPoint &val)
{
	offsetHandle = val;

	if (style & kHorizontal)
	{
		minTmp = offsetHandle.x + minPos;
		maxTmp = minTmp + rangeHandle + widthOfSlider;
	}
	else
	{
		minTmp = offsetHandle.y + minPos;
		maxTmp = minTmp + rangeHandle + heightOfSlider;
	}
}

//------------------------------------------------------------------------
void CSlider::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	// draw background
	if (getDrawBackground ())
	{
		CRect rect (0, 0, widthControl, heightControl);
		rect.offset (getViewSize ().left, getViewSize ().top);
		getDrawBackground ()->draw (drawContext, rect, offset);
	}
	
	if (drawStyle != 0)
	{
		CRect r (getViewSize ());

		pContext->setDrawMode (kAliasing);
		pContext->setLineStyle (kLineSolid);
		pContext->setLineWidth (1.);
		if (drawStyle & kDrawFrame || drawStyle & kDrawBack)
		{
			pContext->setFrameColor (frameColor);
			pContext->setFillColor (backColor);
			CDrawStyle d = kDrawFilled;
			if (drawStyle & kDrawFrame && drawStyle & kDrawBack)
				d = kDrawFilledAndStroked;
			else if (drawStyle & kDrawFrame)
				d = kDrawStroked;
			pContext->drawRect (r, d);
		}
		pContext->setDrawMode (kAliasing);
		if (drawStyle & kDrawValue)
		{
			if (drawStyle & kDrawFrame)
				r.inset (1., 1.);
			float drawValue = getValueNormalized ();
			if (drawStyle & kDrawValueFromCenter)
			{
				if (drawStyle & kDrawInverted)
					drawValue = 1.f - drawValue;
				if (getStyle () & kHorizontal)
				{
					CCoord width = r.getWidth ();
					r.right = r.left + r.getWidth () * drawValue;
					r.left += width / 2.;
					r.normalize ();
				}
				else
				{
					CCoord height = r.getHeight ();
					r.bottom = r.top + r.getHeight () * drawValue;
					r.top += height / 2.;
					r.normalize ();
				}
			}
			else
			{
				if (getStyle () & kHorizontal)
				{
					if (drawStyle & kDrawInverted)
						r.left = r.right - r.getWidth () * drawValue;
					else
						r.right = r.left + r.getWidth () * drawValue;
				}
				else
				{
					if (drawStyle & kDrawInverted)
						r.bottom = r.top + r.getHeight () * drawValue;
					else
						r.top = r.bottom - r.getHeight () * drawValue;
				}
			}
			r.normalize ();
			if (r.getWidth () >= 0.5 && r.getHeight () >= 0.5)
			{
				pContext->setFillColor (valueColor);
				pContext->drawRect (r, kDrawFilled);
			}
		}
	}
	
	if (pHandle)
	{
		float normValue = getValueNormalized ();
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;
		
		// calc new coords of slider
		CRect rectNew;
		if (style & kHorizontal)
		{
			rectNew.top    = offsetHandle.y;
			rectNew.bottom = rectNew.top + heightOfSlider;	

			rectNew.left   = offsetHandle.x + floor (normValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
		}
		else
		{
			rectNew.left   = offsetHandle.x;
			rectNew.right  = rectNew.left + widthOfSlider;	

			rectNew.top    = offsetHandle.y + floor (normValue * rangeHandle);
			rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

			rectNew.bottom = rectNew.top + heightOfSlider;
			rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
		}
		rectNew.offset (getViewSize ().left, getViewSize ().top);

		// draw slider at new position
		pHandle->draw (drawContext, rectNew);
	}

	setDirty (false);
}

//------------------------------------------------------------------------
float CSlider::calculateDelta (const CPoint& where, CRect* handleRect) const
{
	CCoord result;
	if (style & kHorizontal)
		result = getViewSize ().left + offsetHandle.x;
	else
		result = getViewSize ().top + offsetHandle.y;
	if (getMode () != kFreeClickMode)
	{
		float normValue = getValueNormalized ();
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;
		CCoord actualPos;
		CRect rect;
		
		actualPos = result + (int32_t)(normValue * rangeHandle);

		if (style & kHorizontal)
		{
			if (handleRect)
			{
				handleRect->left   = actualPos;
				handleRect->top    = getViewSize ().top  + offsetHandle.y;
				handleRect->right  = handleRect->left + widthOfSlider;
				handleRect->bottom = handleRect->top  + heightOfSlider;
			}
			result += where.x - actualPos;
		}
		else
		{
			if (handleRect)
			{
				handleRect->left   = getViewSize ().left  + offsetHandle.x;
				handleRect->top    = actualPos;
				handleRect->right  = handleRect->left + widthOfSlider;
				handleRect->bottom = handleRect->top  + heightOfSlider;
			}
			result += where.y - actualPos;
		}
	}
	else
	{
		if (style & kHorizontal)
			result += widthOfSlider / 2 - 1;
		else
			result += heightOfSlider / 2 - 1;
	}
	return (float)result;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	CRect handleRect;
	delta = calculateDelta (where, getMode () != kFreeClickMode ? &handleRect : 0);
	if (getMode () == kTouchMode && !handleRect.pointInside (where))
		return kMouseEventNotHandled;

	oldVal    = getMin () - 1;
	oldButton = buttons;

	if ((getMode () == kRelativeTouchMode && handleRect.pointInside (where)) || getMode () != kRelativeTouchMode)
	{
		if (checkDefaultValue (buttons))
		{
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
	}
	startVal = getValue ();
	beginEdit ();
	mouseStartPoint = where;
	if (buttons & kZoomModifier)
		return kMouseEventHandled;
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseCancel ()
{
	value = startVal;
	if (isDirty ())
		valueChanged ();
	oldButton = 0;
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	oldButton = 0;
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseMoved (CPoint& where, const CButtonState& _buttons)
{
	if (isEditing ())
	{
		CButtonState buttons (_buttons);
		if (kAlwaysUseZoomFactor)
			buttons |= kZoomModifier;
		if (buttons & kLButton)
		{
			if (kAlwaysUseZoomFactor)
			{
				CCoord distance = fabs ((style & kHorizontal) ? where.y - mouseStartPoint.y : where.x - mouseStartPoint.x);
				float newZoomFactor = 1.f;
				if (distance > ((style & kHorizontal) ? getHeight () : getWidth ()))
				{
					newZoomFactor = (float)(distance / ((style & kHorizontal) ? getHeight () : getWidth ()));
					newZoomFactor = static_cast<int32_t>(newZoomFactor * 10.f) / 10.f;
				}
				if (zoomFactor != newZoomFactor)
				{
					zoomFactor = newZoomFactor;
					oldVal = (value - getMin ()) / getRange ();
					delta = calculateDelta (where);
				}
			}
			
			if (oldVal == getMin () - 1)
				oldVal = (value - getMin ()) / getRange ();
				
			if ((oldButton != buttons) && (buttons & kZoomModifier))
			{
				oldVal = (value - getMin ()) / getRange ();
				oldButton = buttons;
			}
			else if (!(buttons & kZoomModifier))
				oldVal = (value - getMin ()) / getRange ();

			float normValue;
			if (style & kHorizontal)
				normValue = (float)(where.x - delta) / (float)rangeHandle;
			else
				normValue = (float)(where.y - delta) / (float)rangeHandle;

			if (style & kRight || style & kBottom)
				normValue = 1.f - normValue;

			if (buttons & kZoomModifier)
				normValue = oldVal + ((normValue - oldVal) / zoomFactor);

			setValueNormalized (normValue);
				
			if (isDirty ())
			{
				valueChanged ();
				invalid ();
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
static bool styleIsInverseStyle (int32_t style)
{
	if ((style & kVertical) && (style & kTop))
		return true;
	if ((style & kHorizontal) && (style & kRight))
		return true;
	return false;
}

//------------------------------------------------------------------------
bool CSlider::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	float _distance = distance;
	if (styleIsInverseStyle (style))
		_distance *= -1.f;
	float normValue = getValueNormalized ();
	if (buttons & kZoomModifier)
		normValue += 0.1f * _distance * wheelInc;
	else
		normValue += _distance * wheelInc;

	setValueNormalized (normValue);

	if (isDirty ())
	{
		invalid ();
		
		// begin of edit parameter
		beginEdit ();
	
		valueChanged ();
	
		// end of edit parameter
		endEdit ();
	}

	return true;
}

//------------------------------------------------------------------------
int32_t CSlider::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_UP :
		case VKEY_RIGHT :
		case VKEY_DOWN :
		case VKEY_LEFT :
		{
			float distance = 1.f;
			bool isInverse = styleIsInverseStyle (style);
			if ((keyCode.virt == VKEY_DOWN && !isInverse) 
			 || (keyCode.virt == VKEY_UP && isInverse)
			 || (keyCode.virt == VKEY_LEFT && !isInverse)
			 || (keyCode.virt == VKEY_RIGHT && isInverse))
			{
				distance = -distance;
			}

			float normValue = getValueNormalized ();
			if (mapVstKeyModifier (keyCode.modifier) & kZoomModifier)
				normValue += 0.1f * distance * wheelInc;
			else
				normValue += distance * wheelInc;

			setValueNormalized (normValue);

			if (isDirty ())
			{
				invalid ();

				// begin of edit parameter
				beginEdit ();
			
				valueChanged ();
			
				// end of edit parameter
				endEdit ();
			}
			return 1;
		}
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
		setViewSize (getViewSize (), true);
	}
}

//------------------------------------------------------------------------
void CSlider::setDrawStyle (int32_t style)
{
	if (style != drawStyle)
	{
		drawStyle = style;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setFrameColor (CColor color)
{
	if (color != frameColor)
	{
		frameColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setBackColor (CColor color)
{
	if (color != backColor)
	{
		backColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setValueColor (CColor color)
{
	if (color != valueColor)
	{
		valueColor = color;
		invalid ();
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
CVerticalSlider::CVerticalSlider (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
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
CVerticalSlider::CVerticalSlider (const CRect &rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
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
CHorizontalSlider::CHorizontalSlider (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
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
CHorizontalSlider::CHorizontalSlider (const CRect &rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CHorizontalSlider& slider)
: CSlider (slider)
{}

} // namespace
