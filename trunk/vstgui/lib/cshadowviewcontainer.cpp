#include "cshadowviewcontainer.h"
#include "coffscreencontext.h"
#include "cbitmapfilter.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CShadowViewContainer::CShadowViewContainer (const CRect& size)
: CViewContainer (size)
, shadowInvalid (true)
, shadowIntensity (0.3f)
, shadowBlurSize (4)
{
}

//-----------------------------------------------------------------------------
CShadowViewContainer::CShadowViewContainer (const CShadowViewContainer& copy)
: CViewContainer (copy)
, shadowInvalid (copy.shadowInvalid)
, shadowIntensity (copy.shadowIntensity)
, shadowBlurSize (copy.shadowBlurSize)
{
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setShadowOffset (const CPoint& offset)
{
	if (shadowOffset != offset)
	{
		shadowOffset = offset;
		setBackgroundOffset (offset);
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setShadowIntensity (float intensity)
{
	if (shadowIntensity != intensity)
	{
		shadowIntensity = intensity;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setShadowBlurSize (uint32_t size)
{
	if (shadowBlurSize != size)
	{
		shadowBlurSize = size;
		invalidateShadow ();
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::invalidateShadow ()
{
	shadowInvalid = true;
}

//-----------------------------------------------------------------------------
CMessageResult CShadowViewContainer::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgViewSizeChanged)
		invalidateShadow ();
	return CViewContainer::notify(sender, message);
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (shadowInvalid && getWidth () > 0. && getHeight () > 0.)
	{
		OwningPointer<COffscreenContext> offscreenContext = COffscreenContext::create (getFrame (), getWidth (), getHeight ());
		if (offscreenContext)
		{
			offscreenContext->beginDraw ();
			offscreenContext->setOffset (CPoint (-getViewSize ().left, -getViewSize ().top));
			CViewContainer::draw (offscreenContext);
			offscreenContext->endDraw ();
			CBitmap* bitmap = offscreenContext->getBitmap ();
			
			if (bitmap)
			{
				OwningPointer<BitmapFilter::IFilter> setColorFilter = BitmapFilter::Factory::getInstance().createFilter (BitmapFilter::Standard::kSetColor);
				if (setColorFilter)
				{
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, bitmap);
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputColor, kBlackCColor);
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kIgnoreAlphaColorValue, (int32_t)1);
					if (setColorFilter->run ())
					{
						SharedPointer<CBaseObject> background = setColorFilter->getProperty(BitmapFilter::Standard::Property::kOutputBitmap).getObject ();
						if (background)
						{
							OwningPointer<BitmapFilter::IFilter> boxBlurFilter = BitmapFilter::Factory::getInstance().createFilter (BitmapFilter::Standard::kBoxBlur);
							if (boxBlurFilter)
							{
								boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, (CBaseObject*)background);
								boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kRadius, (int32_t)shadowBlurSize);
								if (boxBlurFilter->run (true))
								{
									CBaseObject* b = background;
									setBackground (dynamic_cast<CBitmap*> (b));
									shadowInvalid = false;
								}
							}
						}
					}
				}

				CCoord save[4];
				modifyDrawContext (save, pContext);
				CRect clientRect (getViewSize ());
				clientRect.originize ();
				drawBackgroundRect (pContext, clientRect);
				restoreDrawContext (pContext, save);
				bitmap->draw (pContext, getViewSize ());
				setDirty (false);
			}
		}
	}
	else
	{
		CViewContainer::drawRect (pContext, updateRect);
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect)
{
	if (shadowInvalid == false)
	{
		float tmp = pContext->getGlobalAlpha ();
		pContext->setGlobalAlpha (tmp * shadowIntensity);
		CViewContainer::drawBackgroundRect (pContext, _updateRect);
		pContext->setGlobalAlpha (tmp);
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setViewSize (const CRect& rect, bool invalid)
{
	if (getViewSize () != rect)
	{
		bool diffSize = (getWidth () != rect.getWidth () || getHeight () != rect.getHeight ());
		CViewContainer::setViewSize (rect, invalid);
		if (diffSize)
			invalidateShadow ();
	}
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::addView (CView* pView)
{
	if (CViewContainer::addView (pView))
	{
		invalidateShadow ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled)
{
	if (CViewContainer::addView (pView, mouseableArea, mouseEnabled))
	{
		invalidateShadow ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::addView (CView* pView, CView* pBefore)
{
	if (CViewContainer::addView (pView, pBefore))
	{
		invalidateShadow ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::removeView (CView* pView, bool withForget)
{
	if (CViewContainer::removeView (pView, withForget))
	{
		invalidateShadow ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::changeViewZOrder (CView* view, int32_t newIndex)
{
	if (CViewContainer::changeViewZOrder (view, newIndex))
	{
		invalidateShadow ();
		return true;
	}
	return false;
}

} // namespace
