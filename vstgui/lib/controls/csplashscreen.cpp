// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	CDefaultSplashScreenView (const CRect& size, IControlListener* listener, CBitmap* bitmap,
	                          const CPoint& offset)
	: CControl (size, listener), offset (offset)
	{
		setBackground (bitmap);
	}

	void draw (CDrawContext *pContext) override
	{
		if (getDrawBackground ())
			getDrawBackground ()->draw (pContext, getViewSize (), offset);
		setDirty (false);
	}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override
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
CSplashScreen::CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag,
                              CBitmap* background, const CRect& toDisplay, const CPoint& offset)
: CControl (size, listener, tag, background), toDisplay (toDisplay), offset (offset)
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
	modalView = static_cast<CView*> (v.modalView->newCopy ());
}

//------------------------------------------------------------------------
CSplashScreen::~CSplashScreen () noexcept
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
		if (value == getMax () && !modalViewSessionID && modalView)
		{
			if (auto frame = getFrame ())
			{
				if (modalView)
				{
					if ((modalViewSessionID = frame->beginModalViewSession (modalView)))
					{
						modalView->remember ();
						CControl::valueChanged ();
					}
				}
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

	if (auto frame = getFrame ())
	{
		if (modalViewSessionID)
		{
			if (modalView)
				modalView->invalid ();
			frame->endModalViewSession (*modalViewSessionID);
			modalViewSessionID = {};
		}
	}
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CAnimationSplashScreen::CAnimationSplashScreen (const CRect& size, int32_t tag, CBitmap* background, CBitmap* splashBitmap)
: CSplashScreen (size, nullptr, tag, splashBitmap, CRect (0, 0, 0, 0))
{
	CView::setBackground (background);
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
	return nullptr;
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

	if (auto frame = getFrame ())
	{
		if (frame->getModalView () == modalView)
		{
			if (!createAnimation (animationIndex, animationTime, modalView, true))
			{
				if (modalView)
					modalView->invalid ();
				if (modalViewSessionID)
				{
					frame->endModalViewSession (*modalViewSessionID);
					modalViewSessionID = {};
				}
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
bool CAnimationSplashScreen::createAnimation (uint32_t animIndex, uint32_t animTime,
                                              CView* splashView, bool removeViewAnimation)
{
	if (!isAttached ())
		return false;
	switch (animIndex)
	{
		case 0:
		{
			if (removeViewAnimation)
			{
				splashView->setMouseEnabled (false);
				splashView->addAnimation (
				    "AnimationSplashScreenAnimation", new Animation::AlphaValueAnimation (0.f),
				    new Animation::PowerTimingFunction (animTime, 2),
				    [this] (CView*, const IdStringPtr, Animation::IAnimationTarget*) {
					    if (modalView)
					    {
						    modalView->invalid ();
						    modalView->setMouseEnabled (true);
					    }
					    if (modalViewSessionID)
					    {
						    if (auto frame = getFrame ())
							    frame->endModalViewSession (*modalViewSessionID);
						    modalViewSessionID = {};
					    }
					    setMouseEnabled (true);
				    });
			}
			else
			{
				setMouseEnabled (false);
				splashView->setAlphaValue (0.f);
				splashView->addAnimation ("AnimationSplashScreenAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::PowerTimingFunction (animTime, 2));
			}
			return true;
		}
	}
	return false;
}

} // VSTGUI
