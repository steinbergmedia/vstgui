// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cmoviebitmap.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"

namespace VSTGUI {

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
bool CMovieBitmap::useLegacyFrameCalculation = false;
#endif

//------------------------------------------------------------------------
// CMovieBitmap
//------------------------------------------------------------------------
/**
 * CMovieBitmap constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	setHeightOfOneImage (size.getHeight ());
	setNumSubPixmaps (background ? (int32_t)(background->getHeight () / heightOfOneImage) : 0);
#endif
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
/**
 * CMovieBitmap constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of subPixmaps
 * @param heightOfOneImage height of one image in pixel
 * @param background bitmap
 * @param offset
 */
//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
}
#endif

//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CMovieBitmap& v)
: CControl (v)
, offset (v.offset)
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
#endif
}

//------------------------------------------------------------------------
void CMovieBitmap::draw (CDrawContext *pContext)
{
	if (auto bitmap = getDrawBackground ())
	{
		if (auto mfb = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			auto frameIndex = mfb->normalizedValueToFrameIndex (getValueNormalized ());
			mfb->drawFrame (pContext, frameIndex, getViewSize ().getTopLeft () + offset);
		}
		else
		{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
			CPoint where (offset.x, offset.y);
#include "../private/disabledeprecatedmessage.h"
			if (useLegacyFrameCalculation)
#include "../private/enabledeprecatedmessage.h"
			{
				where.y += heightOfOneImage *
						   (int32_t)(getValueNormalized () * (getNumSubPixmaps () - 1) + 0.5);
			}
			else
			{
				auto step = static_cast<int32_t> (std::min (
					getNumSubPixmaps () - 1.f, getValueNormalized () * getNumSubPixmaps ()));
				where.y += heightOfOneImage * step;
			}

			bitmap->draw (pContext, getViewSize (), where);
#else
			bitmap->draw (pContext, getViewSize (), offset);
#endif
		}
	}
	setDirty (false);
}

//-----------------------------------------------------------------------------------------------
bool CMovieBitmap::sizeToFit ()
{
	if (auto bitmap = getDrawBackground ())
	{
		CRect vs (getViewSize ());
		if (auto mfb = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			vs.setSize (mfb->getFrameSize ());
		}
		else
		{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
			vs.setHeight (getHeightOfOneImage ());
#else
			vs.setHeight (bitmap->getHeight ());
#endif
			vs.setWidth (bitmap->getWidth ());
		}
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

} // VSTGUI
