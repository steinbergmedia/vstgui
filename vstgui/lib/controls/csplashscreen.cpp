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

#include "csplashscreen.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cframe.h"

namespace VSTGUI {

/// @cond ignore
//------------------------------------------------------------------------
class CDefaultSplashScreenView : public CControl
{
public:
	CDefaultSplashScreenView (const CRect& size, CControlListener* listener, CBitmap* bitmap, const CPoint& offset) : CControl (size, listener), offset (offset) { setBackground (bitmap); }

	void draw (CDrawContext *pContext)
	{
		pBackground->draw (pContext, size, offset);
		setDirty (false);
	}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	{
		if (buttons & kLButton)
		{
			valueChanged ();
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
		return kMouseEventNotHandled;
	}
	CLASS_METHODS(CDefaultSplashScreenView, CControl)
protected:
	CPoint offset;
};
/// @endcond

//------------------------------------------------------------------------
// CSplashScreen
//------------------------------------------------------------------------
/*! @class CSplashScreen
One click on its activated region and its bitmap or view is displayed, in this state the other controls can not be used,
and another click on the displayed area will leave the modal mode.
*/
//------------------------------------------------------------------------
/**
 * CSplashScreen constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param toDisplay the region where to display the bitmap
 * @param offset offset of background bitmap
 */
//------------------------------------------------------------------------
CSplashScreen::CSplashScreen (const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background, const CRect& toDisplay, const CPoint& offset)
: CControl (size, listener, tag, background)
, toDisplay (toDisplay)
, offset (offset)
, modalView (0)
{
	modalView = new CDefaultSplashScreenView (toDisplay, this, background, offset);
}

//------------------------------------------------------------------------
/**
 * CSplashScreen constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param splashView the view to show
 */
//------------------------------------------------------------------------
CSplashScreen::CSplashScreen (const CRect& size, CControlListener* listener, int32_t tag, CView* splashView)
: CControl (size, listener, tag)
, modalView (splashView)
{
}

//------------------------------------------------------------------------
CSplashScreen::CSplashScreen (const CSplashScreen& v)
: CControl (v)
, toDisplay (v.toDisplay)
, keepSize (v.keepSize)
, offset (v.offset)
{
	modalView = (CView*)v.modalView->newCopy ();
}

//------------------------------------------------------------------------
CSplashScreen::~CSplashScreen ()
{
	if (modalView)
		modalView->forget ();
}

//------------------------------------------------------------------------
void CSplashScreen::draw (CDrawContext *pContext)
{
	setDirty (false);
}

//------------------------------------------------------------------------
bool CSplashScreen::hitTest (const CPoint& where, const CButtonState buttons)
{
	bool result = CView::hitTest (where, buttons);
	if (result && !(buttons & kLButton))
		return false;
	return result;
}

//------------------------------------------------------------------------
CMouseEventResult CSplashScreen::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		value = (value == getMax ()) ? getMin () : getMax ();
		if (value == getMax ())
		{
			if (modalView && getFrame () && getFrame ()->setModalView (modalView))
			{
				CControl::valueChanged ();
			}
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CSplashScreen::valueChanged (CControl *pControl)
{
	if (pControl == modalView)
	{
		unSplash ();
		CControl::valueChanged ();
	}
}

//------------------------------------------------------------------------
void CSplashScreen::unSplash ()
{
	value = getMin ();

	if (getFrame ())
	{
		if (getFrame ()->getModalView () == modalView)
		{
			if (modalView)
				modalView->invalid ();
			getFrame ()->setModalView (NULL);
		}
	}
}

} // namespace
