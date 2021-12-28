// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cswitch.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cvstguitimer.h"
#include "../events.h"

namespace VSTGUI {

bool CSwitchBase::useLegacyIndexCalculation = false;

//------------------------------------------------------------------------
CSwitchBase::CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag,
                          CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background), offset (offset)
{
	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSwitchBase::CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag,
                          int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions,
                          CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background), offset (offset)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSwitchBase::CSwitchBase (const CSwitchBase& other) : CControl (other), offset (other.offset)
{
	setNumSubPixmaps (other.subPixmaps);
	setHeightOfOneImage (other.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
void CSwitchBase::draw (CDrawContext* pContext)
{
	if (getDrawBackground ())
	{
		float norm = getValueNormalized ();
		if (inverseBitmap)
			norm = 1.f - norm;
		// source position in bitmap
		CPoint where (0, heightOfOneImage * normalizedToIndex (norm));

		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
bool CSwitchBase::sizeToFit ()
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
CMouseEventResult CSwitchBase::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	coef = calculateCoef ();

	beginEdit ();

	mouseStartValue = getValue ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSwitchBase::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
		endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSwitchBase::onMouseCancel ()
{
	if (isEditing ())
	{
		value = mouseStartValue;
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSwitchBase::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		float norm = calcNormFromPoint (where);
		if (inverseBitmap)
			norm = 1.f - norm;
		value = getMin () + norm * (getMax () - getMin ());
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
void CSwitchBase::setInverseBitmap (bool state)
{
	if (inverseBitmap != state)
	{
		inverseBitmap = state;
		invalid ();
	}
}

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
CVerticalSwitch::CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
                                  CBitmap* background, const CPoint& offset)
: CSwitchBase (size, listener, tag, background, offset)
{
	heightOfOneImage = size.getHeight ();
	setNumSubPixmaps (
	    background ? static_cast<int32_t> (background->getHeight () / heightOfOneImage) : 0);
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
CVerticalSwitch::CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
                                  int32_t subPixmaps, CCoord heightOfOneImage,
                                  int32_t iMaxPositions, CBitmap* background, const CPoint& offset)
: CSwitchBase (size, listener, tag, subPixmaps, heightOfOneImage, iMaxPositions, background, offset)
{
}

//------------------------------------------------------------------------
CVerticalSwitch::CVerticalSwitch (const CVerticalSwitch& v)
: CSwitchBase (v)
{
}

//------------------------------------------------------------------------
double CVerticalSwitch::calculateCoef () const
{
	return static_cast<double> (heightOfOneImage) / static_cast<double> (getNumSubPixmaps ());
}

//------------------------------------------------------------------------
float CVerticalSwitch::calcNormFromPoint (const CPoint& where) const
{
	return static_cast<int32_t> ((where.y - getViewSize ().top) / getCoef ()) /
	       static_cast<float> (getNumSubPixmaps () - 1);
}

//------------------------------------------------------------------------
void CVerticalSwitch::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown || event.modifiers.empty () == false)
		return;
	float norm = getValueNormalized ();
	int32_t currentIndex = normalizedToIndex (norm);
	if (event.virt == VirtualKey::Up && currentIndex > 0)
	{
		--currentIndex;
		norm = indexToNormalized (currentIndex);
		value = (getMax () - getMin ()) * norm + getMin ();
		bounceValue ();
	}
	if (event.virt == VirtualKey::Down && currentIndex < (getNumSubPixmaps () - 1))
	{
		++currentIndex;
		norm = indexToNormalized (currentIndex);
		value = (getMax () - getMin ()) * norm + getMin ();
		bounceValue ();
	}
	if (isDirty ())
	{
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		event.consumed = true;
	}
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
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
                                      CBitmap* background, const CPoint& offset)
: CSwitchBase (size, listener, tag, background, offset)
{
	heightOfOneImage = size.getWidth ();
	setNumSubPixmaps (background ? (int32_t) (background->getWidth () / heightOfOneImage) : 0);
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
CHorizontalSwitch::CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
                                      int32_t subPixmaps, CCoord heightOfOneImage,
                                      int32_t iMaxPositions, CBitmap* background,
                                      const CPoint& offset)
: CSwitchBase (size, listener, tag, subPixmaps, heightOfOneImage, iMaxPositions, background, offset)
{
}

//------------------------------------------------------------------------
CHorizontalSwitch::CHorizontalSwitch (const CHorizontalSwitch& v)
: CSwitchBase (v)
{
}

//------------------------------------------------------------------------
double CHorizontalSwitch::calculateCoef () const
{
	return getDrawBackground ()->getWidth () / static_cast<double> (getNumSubPixmaps ());
}

//------------------------------------------------------------------------
float CHorizontalSwitch::calcNormFromPoint (const CPoint& where) const
{
	return static_cast<int32_t> ((where.x - getViewSize ().left) / getCoef ()) /
	       static_cast<float> (getNumSubPixmaps () - 1);
}

void CHorizontalSwitch::onKeyboardEvent(KeyboardEvent &event)
{
	if (event.type != EventType::KeyDown || event.modifiers.empty () == false)
		return;
	float norm = getValueNormalized ();
	int32_t currentIndex = normalizedToIndex (norm);
	if (event.virt == VirtualKey::Left && currentIndex > 0)
	{
		--currentIndex;
		norm = indexToNormalized (currentIndex);
		value = (getMax () - getMin ()) * norm + getMin ();
		bounceValue ();
	}
	if (event.virt == VirtualKey::Right && currentIndex < (getNumSubPixmaps () - 1))
	{
		++currentIndex;
		norm = indexToNormalized (currentIndex);
		value = (getMax () - getMin ()) * norm + getMin ();
		bounceValue ();
	}
	if (isDirty ())
	{
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		event.consumed = true;
	}
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
, resetValueTimer (nullptr)
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
, resetValueTimer (nullptr)
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
, resetValueTimer (nullptr)
{
	setHeightOfOneImage (v.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CRockerSwitch::~CRockerSwitch () noexcept
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
	mouseStartValue = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		value = (getMax () - getMin ()) / 2.f + getMin ();
		if (isDirty ())
			invalid ();
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseCancel ()
{
	if (isEditing ())
	{
		value = mouseStartValue;
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		endEdit ();
	}
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
				value = mouseStartValue;
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
				value = mouseStartValue;
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
void CRockerSwitch::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.modifiers.empty () == false)
		return;
	if (event.type == EventType::KeyDown)
	{
		if (style & kHorizontal &&
		    (event.virt == VirtualKey::Left || event.virt == VirtualKey::Right))
		{
			value = event.virt == VirtualKey::Left ? getMin () : getMax ();
			invalid ();
			beginEdit ();
			valueChanged ();
			event.consumed = true;
		}
		if (style & kVertical && (event.virt == VirtualKey::Up || event.virt == VirtualKey::Down))
		{
			value = event.virt == VirtualKey::Up ? getMin () : getMax ();
			invalid ();
			beginEdit ();
			valueChanged ();
			event.consumed = true;
		}
	}
	else if (event.type == EventType::KeyUp)
	{
		if ((style & kHorizontal &&
		     (event.virt == VirtualKey::Left || event.virt == VirtualKey::Right)) ||
		    (style & kVertical && (event.virt == VirtualKey::Up || event.virt == VirtualKey::Down)))
		{
			value = (getMax () - getMin ()) / 2.f + getMin ();
			invalid ();
			valueChanged ();
			endEdit ();
			event.consumed = true;
		}
	}
}

//------------------------------------------------------------------------
void CRockerSwitch::onMouseWheelEvent (MouseWheelEvent& event)
{
	auto distance = event.deltaY;
	if (distance == 0.)
		return;

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

	if (resetValueTimer == nullptr)
		resetValueTimer = new CVSTGUITimer (this, 200);
	resetValueTimer->stop ();
	resetValueTimer->start ();

	event.consumed = true;
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
		resetValueTimer = nullptr;
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

} // VSTGUI
