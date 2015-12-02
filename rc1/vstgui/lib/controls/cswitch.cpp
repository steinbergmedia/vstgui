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

#include "cswitch.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cvstguitimer.h"

namespace VSTGUI {

//------------------------------------------------------------------------
// CVerticalSwitch
//------------------------------------------------------------------------
/*! @class CVerticalSwitch
Define a switch with a given number of positions, the current position is defined by the position
of the last click on this object (the object is divided in its height by the number of position).
Each position has its subbitmap, each subbitmap is stacked in the given handle bitmap.
By clicking Alt+Left Mouse the default value is used.
*/
//------------------------------------------------------------------------
/**
 * CVerticalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the switch bitmap
 * @param offset unused 
 */
//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.getHeight ();
	setNumSubPixmaps (background ? (int32_t)(background->getHeight () / heightOfOneImage) : 0);

	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CVerticalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param iMaxPositions TODO
 * @param background the switch bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CVerticalSwitch& v)
: CControl (v)
, offset (v.offset)
{
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CVerticalSwitch::~CVerticalSwitch ()
{}

//------------------------------------------------------------------------
void CVerticalSwitch::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		float norm = (value - getMin ()) / (getMax () - getMin ());
		// source position in bitmap
		CPoint where (0, heightOfOneImage * ((int32_t)(norm * (getNumSubPixmaps () - 1) + 0.5f)));

		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = (double)heightOfOneImage / (double)getNumSubPixmaps ();

	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CVerticalSwitch::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		float norm = (int32_t)((where.y - getViewSize ().top) / coef) / (float)(getNumSubPixmaps () - 1);
		value = getMin () + norm * (getMax () - getMin ());
		if (value > getMax ())
			value = getMax ();
		else if (value < getMin ())
			value = getMin ();

		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
int32_t CVerticalSwitch::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0)
	{
		float norm = (value - getMin ()) / (getMax () - getMin ());
		int32_t currentIndex = (int32_t)(norm * (getNumSubPixmaps () - 1) + 0.5f);
		if (keyCode.virt == VKEY_UP && currentIndex > 0)
		{
			currentIndex--;
			norm = (float)currentIndex / (float)(getNumSubPixmaps () - 1);
			value = (getMax () - getMin ()) * norm + getMin ();
		}
		if (keyCode.virt == VKEY_DOWN && currentIndex < (getNumSubPixmaps () - 1))
		{
			currentIndex++;
			norm = (float)currentIndex / (float)(getNumSubPixmaps () - 1);
			value = (getMax () - getMin ()) * norm + getMin ();
		}
		if (isDirty ())
		{
			invalid ();
			beginEdit ();
			valueChanged ();
			endEdit ();
			return 1;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------------------------
bool CVerticalSwitch::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getHeightOfOneImage ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
// CHorizontalSwitch
//------------------------------------------------------------------------
/*! @class CHorizontalSwitch
Same as the CVerticalSwitch but horizontal.
*/
//------------------------------------------------------------------------
/**
 * CHorizontalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap of the switch
 * @param offset unused
 */
//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.getWidth ();
	setNumSubPixmaps (background ? (int32_t)(background->getWidth () / heightOfOneImage) : 0);

	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CHorizontalSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage height of one sub bitmap
 * @param iMaxPositions ignored
 * @param background the switch bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CHorizontalSwitch& v)
: CControl (v)
, offset (v.offset)
{
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CHorizontalSwitch::~CHorizontalSwitch ()
{}

//------------------------------------------------------------------------
void CHorizontalSwitch::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		float norm = (value - getMin ()) / (getMax () - getMin ());
		// source position in bitmap
		CPoint where (0, heightOfOneImage * ((int32_t)(norm * (getNumSubPixmaps () - 1) + 0.5f)));

		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = (double)getDrawBackground ()->getWidth () / (double)getNumSubPixmaps ();

	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CHorizontalSwitch::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		float norm = (int32_t)((where.x - getViewSize ().left) / coef) / (float)(getNumSubPixmaps () - 1);
		value = getMin () + norm * (getMax () - getMin ());
		if (value > getMax ())
			value = getMax ();
		else if (value < getMin ())
			value = getMin ();

		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
int32_t CHorizontalSwitch::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0)
	{
		float norm = getValueNormalized ();
		int32_t currentIndex = (int32_t)(norm * (getNumSubPixmaps () - 1) + 0.5f);
		if (keyCode.virt == VKEY_LEFT && currentIndex > 0)
		{
			currentIndex--;
			norm = (float)currentIndex / (float)(getNumSubPixmaps () - 1);
			value = (getMax () - getMin ()) * norm + getMin ();
		}
		if (keyCode.virt == VKEY_RIGHT && currentIndex < (getNumSubPixmaps () - 1))
		{
			currentIndex++;
			norm = (float)currentIndex / (float)(getNumSubPixmaps () - 1);
			value = (getMax () - getMin ()) * norm + getMin ();
		}
		if (isDirty ())
		{
			invalid ();
			beginEdit ();
			valueChanged ();
			endEdit ();
			return 1;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------------------------
bool CHorizontalSwitch::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getHeightOfOneImage ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
// CRockerSwitch
//------------------------------------------------------------------------
/*! @class CRockerSwitch
Define a rocker switch with 3 states using 3 subbitmaps.
One click on its leftside, then the first subbitmap is displayed.
One click on its rightside, then the third subbitmap is displayed.
When the mouse button is relaxed, the second subbitmap is framed. */
//------------------------------------------------------------------------
/**
 * CRockerSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap with 3 stacked images of the rocker switch
 * @param offset
 * @param style
 */
//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset, const int32_t style)
: CControl (size, listener, tag, background)
, offset (offset)
, style (style)
, resetValueTimer (0)
{
	setNumSubPixmaps (3);
	setHeightOfOneImage (size.getHeight ());
	setWantsFocus (true);
	setMin (-1.f);
	setMax (1.f);
	setValue ((getMax () - getMin ()) / 2.f + getMin ());
}

//------------------------------------------------------------------------
/**
 * CRockerSwitch constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param heightOfOneImage height of one image in pixel
 * @param background bitmap with 3 stacked images of the rocker switch
 * @param offset
 * @param style
 */
//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset, const int32_t style)
: CControl (size, listener, tag, background)
, offset (offset)
, style (style)
, resetValueTimer (0)
{
	setNumSubPixmaps (3);
	setHeightOfOneImage (heightOfOneImage);
	setWantsFocus (true);
	setMin (-1.f);
	setMax (1.f);
	setValue ((getMax () - getMin ()) / 2.f + getMin ());
}

//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRockerSwitch& v)
: CControl (v)
, offset (v.offset)
, style (v.style)
, resetValueTimer (0)
{
	setHeightOfOneImage (v.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CRockerSwitch::~CRockerSwitch ()
{
	if (resetValueTimer)
		resetValueTimer->forget ();
}

//------------------------------------------------------------------------
void CRockerSwitch::draw (CDrawContext *pContext)
{
	CPoint where (offset.x, offset.y);

	if (value == getMax ())
		where.y += 2 * heightOfOneImage;
	else if (value == (getMax () - getMin ()) / 2.f + getMin ())
		where.y += heightOfOneImage;

	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	value = (getMax () - getMin ()) / 2.f + getMin ();
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		CCoord  width_2  = getViewSize ().getWidth () / 2;
		CCoord  height_2 = getViewSize ().getHeight () / 2;

		if (style & kHorizontal) 
		{
			if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
				where.x <= (getViewSize ().left + width_2) && where.y <= getViewSize ().bottom)
				value = getMin ();
			else if (where.x >= (getViewSize ().left + width_2) && where.y >= getViewSize ().top  &&
				where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
				value = getMax ();
			else
				value = fEntryState;
		}
		else
		{
			if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
				where.x <= getViewSize ().right && where.y <= (getViewSize ().top + height_2))
				value = getMin ();
			else if (where.x >= getViewSize ().left && where.y >= (getViewSize ().top + height_2) &&
				where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
				value = getMax ();
			else
				value = fEntryState;
		}

		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
int32_t CRockerSwitch::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0)
	{
		if (style & kHorizontal && (keyCode.virt == VKEY_LEFT || keyCode.virt == VKEY_RIGHT))
		{
			value = keyCode.virt == VKEY_LEFT ? getMin () : getMax ();
			invalid ();
			beginEdit ();
			valueChanged ();
			return 1;
		}
		if (style & kVertical && (keyCode.virt == VKEY_UP || keyCode.virt == VKEY_DOWN))
		{
			value = keyCode.virt == VKEY_UP ? getMin () : getMax ();
			invalid ();
			beginEdit ();
			valueChanged ();
			return 1;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
int32_t CRockerSwitch::onKeyUp (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0)
	{
		if (keyCode.virt == VKEY_LEFT || keyCode.virt == VKEY_RIGHT)
		{
			value = (getMax () - getMin ()) / 2.f + getMin ();
			invalid ();
			valueChanged ();
			endEdit ();

			return 1;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
bool CRockerSwitch::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	if (distance > 0)
		value = getMin ();
	else
		value = getMax ();

	if (isDirty ())
	{
		invalid ();
		if (!isEditing ())
			beginEdit ();
		valueChanged ();
	}

	if (resetValueTimer == 0)
		resetValueTimer = new CVSTGUITimer (this, 200);
	resetValueTimer->stop ();
	resetValueTimer->start ();

	return true;
}

//------------------------------------------------------------------------
CMessageResult CRockerSwitch::notify (CBaseObject* sender, IdStringPtr message)
{
	if (sender == resetValueTimer)
	{
		float newValue = (getMax () - getMin ()) / 2.f + getMin ();
		if (value != newValue)
		{
			value = newValue;
			if (!isEditing ())
				beginEdit ();
			valueChanged ();
			endEdit ();
			setDirty (true);
		}
		resetValueTimer->forget ();
		resetValueTimer = 0;
		return kMessageNotified;
	}
	return CControl::notify (sender, message);
}

//-----------------------------------------------------------------------------------------------
bool CRockerSwitch::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getHeightOfOneImage ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

} // namespace
