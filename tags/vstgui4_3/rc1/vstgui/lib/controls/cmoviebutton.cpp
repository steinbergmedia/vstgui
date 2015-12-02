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

#include "cmoviebutton.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"

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
CMovieButton::~CMovieButton ()
{}

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
			valueChanged ();
		if (isDirty ())
			invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
int32_t CMovieButton::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
	{
		value = (value == getMax ()) ? getMin () : getMax ();
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		return 1;
	}
	return -1;
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

} // namespace
