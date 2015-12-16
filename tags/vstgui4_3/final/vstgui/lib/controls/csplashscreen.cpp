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

#include "csplashscreen.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cframe.h"
#include "../animation/animations.h"
#include "../animation/timingfunctions.h"

namespace VSTGUI {

/// @cond ignore
//------------------------------------------------------------------------
class CDefaultSplashScreenView : public CControl
{
public:
	CDefaultSplashScreenView (const CRect& size, IControlListener* listener, CBitmap* bitmap, const CPoint& offset) : CControl (size, listener), offset (offset) { setBackground (bitmap); }

	void draw (CDrawContext *pContext) VSTGUI_OVERRIDE_VMETHOD
	{
		if (getDrawBackground ())
			getDrawBackground ()->draw (pContext, getViewSize (), offset);
		setDirty (false);
	}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD
	{
		if (buttons.isLeftButton ())
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
CSplashScreen::CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CRect& toDisplay, const CPoint& offset)
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
CSplashScreen::CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag, CView* splashView)
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
bool CSplashScreen::hitTest (const CPoint& where, const CButtonState& buttons)
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CAnimationSplashScreen::CAnimationSplashScreen (const CRect& size, int32_t tag, CBitmap* background, CBitmap* splashBitmap)
: CSplashScreen (size, 0, tag, splashBitmap, CRect (0, 0, 0, 0))
, animationIndex (0)
, animationTime (500)
{
	CView::setBackground (background);
}

//------------------------------------------------------------------------
CAnimationSplashScreen::CAnimationSplashScreen (const CAnimationSplashScreen& splashScreen)
: CSplashScreen (splashScreen)
, animationIndex (splashScreen.animationIndex)
, animationTime (splashScreen.animationTime)
{
}

//------------------------------------------------------------------------
CAnimationSplashScreen::~CAnimationSplashScreen ()
{
}

//------------------------------------------------------------------------
void CAnimationSplashScreen::setSplashBitmap (CBitmap* bitmap)
{
	if (modalView)
	{
		modalView->setBackground (bitmap);
	}
}

//------------------------------------------------------------------------
CBitmap* CAnimationSplashScreen::getSplashBitmap () const
{
	if (modalView)
		return modalView->getBackground ();
	return 0;
}

//------------------------------------------------------------------------
void CAnimationSplashScreen::setSplashRect (const CRect& splashRect)
{
	if (modalView)
	{
		modalView->setViewSize (splashRect);
		modalView->setMouseableArea (splashRect);
	}
}

//------------------------------------------------------------------------
const CRect& CAnimationSplashScreen::getSplashRect () const
{
	if (modalView)
		return modalView->getViewSize ();
	return getViewSize ();
}

//------------------------------------------------------------------------
CMouseEventResult CAnimationSplashScreen::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	CMouseEventResult result = CSplashScreen::onMouseDown (where, buttons);
	if (modalView && value == getMax ())
	{
		createAnimation (animationIndex, animationTime, modalView, false);
	}
	return result;
}

//------------------------------------------------------------------------
void CAnimationSplashScreen::unSplash ()
{
	value = getMin ();

	if (getFrame ())
	{
		if (getFrame ()->getModalView () == modalView)
		{
			if (!createAnimation (animationIndex, animationTime, modalView, true))
			{
				if (modalView)
					modalView->invalid ();
				getFrame ()->setModalView (NULL);
				setMouseEnabled (true);
			}
		}
	}
}

//------------------------------------------------------------------------
void CAnimationSplashScreen::draw (CDrawContext *pContext)
{
	CView::draw (pContext);
	setDirty (false);
}

//------------------------------------------------------------------------
bool CAnimationSplashScreen::sizeToFit ()
{
	if (modalView && modalView->getBackground ())
	{
		CRect r = modalView->getViewSize ();
		r.setWidth (modalView->getBackground ()->getWidth ());
		r.setHeight (modalView->getBackground ()->getHeight ());
		if (getFrame ())
		{
			r.centerInside (getFrame ()->getViewSize ());
		}
		modalView->setViewSize (r);
		modalView->setMouseableArea (r);
	}
	if (getBackground ())
	{
		CRect r = getViewSize ();
		r.setWidth (getBackground ()->getWidth ());
		r.setHeight (getBackground ()->getHeight ());
		setViewSize (r);
		setMouseableArea (r);
	}
	return true;
}

//------------------------------------------------------------------------
bool CAnimationSplashScreen::createAnimation (uint32_t animationIndex, uint32_t animationTime, CView* splashView, bool removeViewAnimation)
{
	switch (animationIndex)
	{
		case 0:
		{
			if (removeViewAnimation)
			{
				splashView->setMouseEnabled (false);
				splashView->addAnimation ("AnimationSplashScreenAnimation", new Animation::AlphaValueAnimation (0.f), new Animation::PowerTimingFunction (animationTime, 2), this);
			}
			else
			{
				setMouseEnabled (false);
				splashView->setAlphaValue (0.f);
				splashView->addAnimation ("AnimationSplashScreenAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::PowerTimingFunction (animationTime, 2));
			}
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
CMessageResult CAnimationSplashScreen::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == Animation::kMsgAnimationFinished)
	{
		if (modalView)
		{
			modalView->invalid ();
			modalView->setMouseEnabled (true);
		}
		if (getFrame ())
			getFrame ()->setModalView (NULL);
		setMouseEnabled (true);
		return kMessageNotified;
	}
	return CSplashScreen::notify (sender, message);
}

} // namespace
