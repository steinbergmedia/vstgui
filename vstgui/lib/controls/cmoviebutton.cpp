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
 * @param offset
 */
//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background), offset (offset), buttonState (value)
{
	heightOfOneImage = size.getHeight ();
	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CMovieButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param heightOfOneImage height of one image in pixel
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
, buttonState (value)
{
	setHeightOfOneImage (heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CMovieButton::CMovieButton (const CMovieButton& v)
: CControl (v)
, offset (v.offset)
, buttonState (v.buttonState)
{
	setHeightOfOneImage (v.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
void CMovieButton::draw (CDrawContext *pContext)
{
	CPoint where;

	where.x = 0;

	if (value == getMax ())
		where.y = heightOfOneImage;
	else
		where.y = 0;

	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	buttonState = value;

	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CMovieButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
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
		if (where.x >= getViewSize ().left &&
				where.y >= getViewSize ().top  &&
				where.x <= getViewSize ().right &&
				where.y <= getViewSize ().bottom)
			value = (fEntryState == getMax ()) ? getMin () : getMax ();
		else
			value = fEntryState;
	
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
CMouseEventResult CMovieButton::onMouseCancel ()
{
	if (isEditing ())
	{
		value = fEntryState;
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
void CMovieButton::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown || event.modifiers.empty () == false)
		return;
	if (event.virt == VirtualKey::Return)
	{
		value = (value == getMax ()) ? getMin () : getMax ();
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		event.consumed = true;
	}
}

//-----------------------------------------------------------------------------------------------
bool CMovieButton::sizeToFit ()
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
