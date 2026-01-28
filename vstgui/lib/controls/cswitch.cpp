// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cswitch.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cvstguitimer.h"
#include "../events.h"
#include "../algorithm.h"

namespace VSTGUI {

//------------------------------------------------------------------------
CSwitchBase::CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag,
						  const SharedPointer<CBitmap>& background)
: CControl (size, listener, tag, background)
{
	setDefaultValue (0.f);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSwitchBase::CSwitchBase (const CSwitchBase& other) : CControl (other) { setWantsFocus (true); }

//------------------------------------------------------------------------
int32_t CSwitchBase::normalizedToIndex (float norm) const
{
	if (auto mfb = getDrawBackground ().cast<CMultiFrameBitmap> ())
	{
		return getMultiFrameBitmapIndex (*mfb.get (), norm);
	}
	return 0;
}

//------------------------------------------------------------------------
float CSwitchBase::indexToNormalized (int32_t index) const
{
	if (auto mfb = getDrawBackground ().cast<CMultiFrameBitmap> ())
	{
		return getNormValueFromMultiFrameBitmapIndex (*mfb.get (), static_cast<uint16_t> (index));
	}
	return 0.f;
}

//------------------------------------------------------------------------
void CSwitchBase::draw (CDrawContext* pContext)
{
	if (auto bitmap = getDrawBackground ())
	{
		float norm = getValueNormalized ();
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			auto frameIndex = getMultiFrameBitmapIndex (*mfb.get (), norm);
			if (inverseBitmap)
				frameIndex = getInverseIndex (*mfb.get (), frameIndex);
			mfb->drawFrame (pContext, frameIndex, getViewSize ().getTopLeft ());
		}
		else
		{
			bitmap->draw (pContext, getViewSize ());
		}
	}
}

//------------------------------------------------------------------------
bool CSwitchBase::sizeToFit ()
{
	if (auto bitmap = getDrawBackground ())
	{
		CRect vs (getViewSize ());
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			vs.setSize (mfb->getFrameSize ());
		}
		else
		{
			vs.setWidth (bitmap->getWidth ());
			vs.setHeight (bitmap->getHeight ());
		}
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
		if (setValue (mouseStartValue))
			valueChanged ();
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
		if (setValueNormalized (norm))
			valueChanged ();
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
Use a CMultiFrameBitmap for its background bitmap.
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
								  const SharedPointer<CBitmap>& background)
: CSwitchBase (size, listener, tag, background)
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
	if (auto mfb = getDrawBackground ().cast<CMultiFrameBitmap> ())
	{
		return mfb->getFrameSize ().y /
			   static_cast<double> (getMultiFrameBitmapRangeLength (*mfb.get ()));
	}
	return 1.;
}

//------------------------------------------------------------------------
float CVerticalSwitch::calcNormFromPoint (const CPoint& where) const
{
	if (auto mfb = getDrawBackground ().cast<CMultiFrameBitmap> ())
	{
		return static_cast<int32_t> ((where.y - getViewSize ().top) / getCoef ()) /
			   static_cast<float> (getMultiFrameBitmapRangeLength (*mfb.get ()) - 1);
	}
	return 0.f;
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
	}
	if (event.virt == VirtualKey::Down && norm < 1.f)
	{
		++currentIndex;
		norm = indexToNormalized (currentIndex);
	}
	if (setValueNormalized (norm))
	{
		beginEdit ();
		valueChanged ();
		endEdit ();
	}
	event.consumed = true;
}

//------------------------------------------------------------------------
// CHorizontalSwitch
//------------------------------------------------------------------------
/*! @class CHorizontalSwitch
Same as the CVerticalSwitch but horizontal.
Use a CMultiFrameBitmap for its background bitmap.
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
									  const SharedPointer<CBitmap>& background)
: CSwitchBase (size, listener, tag, background)
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
	if (auto mfb = getDrawBackground ().cast<CMultiFrameBitmap> ())
	{
		return mfb->getFrameSize ().x /
			   static_cast<double> (getMultiFrameBitmapRangeLength (*mfb.get ()));
	}
	return 1.;
}

//------------------------------------------------------------------------
float CHorizontalSwitch::calcNormFromPoint (const CPoint& where) const
{
	if (auto mfb = getDrawBackground ().cast<CMultiFrameBitmap> ())
	{
		return static_cast<int32_t> ((where.x - getViewSize ().left) / getCoef ()) /
			   static_cast<float> (getMultiFrameBitmapRangeLength (*mfb.get ()) - 1);
	}
	return 0.f;
}

//------------------------------------------------------------------------
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
	}
	if (event.virt == VirtualKey::Right && norm < 1.f)
	{
		++currentIndex;
		norm = indexToNormalized (currentIndex);
	}
	if (setValueNormalized (norm))
	{
		beginEdit ();
		valueChanged ();
		endEdit ();
	}
	event.consumed = true;
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
 * @param style
 */
//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag,
							  const SharedPointer<CBitmap>& background, const int32_t style)
: CControl (size, listener, tag, background), style (style), resetValueTimer (nullptr)
{
	setWantsFocus (true);
	setMin (-1.f);
	setMax (1.f);
	setValue ((getMax () - getMin ()) / 2.f + getMin ());
}

//------------------------------------------------------------------------
CRockerSwitch::CRockerSwitch (const CRockerSwitch& v)
: CControl (v), style (v.style), resetValueTimer (nullptr)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CRockerSwitch::~CRockerSwitch () noexcept {}

//------------------------------------------------------------------------
void CRockerSwitch::draw (CDrawContext *pContext)
{
	if (auto bitmap = getDrawBackground ())
	{
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			uint16_t frameIndex = getMultiFrameBitmapIndex (*mfb.get (), getValueNormalized ());
			mfb->drawFrame (pContext, frameIndex, getViewSize ().getTopLeft ());
		}
		else
			bitmap->draw (pContext, getViewSize ());
	}
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	mouseStartValue = getValue ();
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		setValue ((getMax () - getMin ()) / 2.f + getMin ());
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CRockerSwitch::onMouseCancel ()
{
	if (isEditing ())
	{
		if (setValue (mouseStartValue))
			valueChanged ();
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
		bool changed = false;
		if (style & kHorizontal)
		{
			if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
				where.x <= (getViewSize ().left + width_2) && where.y <= getViewSize ().bottom)
				changed = setValue (getMin ());
			else if (where.x >= (getViewSize ().left + width_2) && where.y >= getViewSize ().top  &&
				where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
				changed = setValue (getMax ());
			else
				changed = setValue (mouseStartValue);
		}
		else
		{
			if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
				where.x <= getViewSize ().right && where.y <= (getViewSize ().top + height_2))
				changed = setValue (getMin ());
			else if (where.x >= getViewSize ().left && where.y >= (getViewSize ().top + height_2) &&
				where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
				changed = setValue (getMax ());
			else
				changed = setValue (mouseStartValue);
		}

		if (changed)
			valueChanged ();
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
			setValue (event.virt == VirtualKey::Left ? getMin () : getMax ());
			beginEdit ();
			valueChanged ();
			event.consumed = true;
		}
		if (style & kVertical && (event.virt == VirtualKey::Up || event.virt == VirtualKey::Down))
		{
			setValue (event.virt == VirtualKey::Up ? getMin () : getMax ());
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
			setValue ((getMax () - getMin ()) / 2.f + getMin ());
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

	if (setValue (distance > 0 ? getMin () : getMax ()))
	{
		if (!isEditing ())
			beginEdit ();
		valueChanged ();
	}

	if (resetValueTimer == nullptr)
	{
		resetValueTimer = makeOwned<CVSTGUITimer> (
			[this] (auto&&) {
				float newValue = (getMax () - getMin ()) / 2.f + getMin ();
				if (getValue () != newValue)
				{
					setValue (newValue);
					if (!isEditing ())
						beginEdit ();
					valueChanged ();
					endEdit ();
				}
				resetValueTimer.reset ();
			},
			200);
	}
	else
	{
		resetValueTimer->stop ();
		resetValueTimer->start ();
	}

	event.consumed = true;
}

//-----------------------------------------------------------------------------------------------
bool CRockerSwitch::sizeToFit ()
{
	if (auto bitmap = getDrawBackground ())
	{
		CRect vs (getViewSize ());
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			vs.setSize (mfb->getFrameSize ());
		}
		else
		{
			vs.setWidth (bitmap->getWidth ());
			vs.setHeight (bitmap->getHeight ());
		}
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

} // VSTGUI
