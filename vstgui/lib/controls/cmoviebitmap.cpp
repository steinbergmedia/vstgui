// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cmoviebitmap.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"

namespace VSTGUI {

//------------------------------------------------------------------------
// CMovieBitmap
//------------------------------------------------------------------------
/**
 * CMovieBitmap constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap
 */
//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag,
							const SharedPointer<CBitmap>& background)
: CControl (size, listener, tag, background)
{
}

//------------------------------------------------------------------------
CMovieBitmap::CMovieBitmap (const CMovieBitmap& v) : CControl (v) {}

//------------------------------------------------------------------------
void CMovieBitmap::draw (CDrawContext *pContext)
{
	if (auto bitmap = getDrawBackground ())
	{
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			auto frameIndex = getMultiFrameBitmapIndex (*mfb.get (), getValueNormalized ());
			mfb->drawFrame (pContext, frameIndex, getViewSize ().getTopLeft ());
		}
		else
		{
			bitmap->draw (pContext, getViewSize ());
		}
	}
}

//-----------------------------------------------------------------------------------------------
bool CMovieBitmap::sizeToFit ()
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
			vs.setHeight (bitmap->getHeight ());
			vs.setWidth (bitmap->getWidth ());
		}
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

} // VSTGUI
