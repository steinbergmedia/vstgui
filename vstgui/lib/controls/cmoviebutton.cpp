// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cmoviebutton.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../events.h"

namespace VSTGUI {

//------------------------------------------------------------------------
// CMovieButton
//------------------------------------------------------------------------
/**
 * CMovieButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap
 */
//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CRect& size, IControlListener* listener, int32_t tag,
							const SharedPointer<CBitmap>& background)
: CControl (size, listener, tag, background), buttonState (getValue ())
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
void CMovieButton::draw (CDrawContext& context)
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
			CPoint where {};
			bitmap->draw (context, getViewSize (), where);
		}
	}
	buttonState = getValue ();
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = getValue ();
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
		endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		bool changed = false;
		if (where.x >= getViewSize ().left &&
				where.y >= getViewSize ().top  &&
				where.x <= getViewSize ().right &&
				where.y <= getViewSize ().bottom)
			changed = setValue ((fEntryState == getMax ()) ? getMin () : getMax ());
		else
			changed = setValue (fEntryState);

		if (changed)
			valueChanged ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseCancel ()
{
	if (isEditing ())
	{
		if (setValue (fEntryState))
			valueChanged ();
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void CMovieButton::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown || event.modifiers.empty () == false)
		return;
	if (event.virt == VirtualKey::Return)
	{
		setValue ((getValue () == getMax ()) ? getMin () : getMax ());
		beginEdit ();
		valueChanged ();
		endEdit ();
		event.consumed = true;
	}
}

//-----------------------------------------------------------------------------------------------
bool CMovieButton::sizeToFit ()
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
