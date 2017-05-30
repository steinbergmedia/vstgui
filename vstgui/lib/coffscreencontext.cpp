// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "coffscreencontext.h"
#include "cframe.h"
#include "cbitmap.h"
#include "platform/iplatformframe.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CBitmap* bitmap)
: CDrawContext (CRect (0, 0, bitmap->getWidth (), bitmap->getHeight ()))
, bitmap (bitmap)
{
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const CRect& surfaceRect)
: CDrawContext (surfaceRect)
{
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset)
{
	if (bitmap)
		bitmap->draw (pContext, destRect, srcOffset);
}

//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> COffscreenContext::create (CFrame* frame, CCoord width, CCoord height, double scaleFactor)
{
	if (width >= 1. && height >= 1.)
	{
		IPlatformFrame* pf = frame ? frame->getPlatformFrame () : nullptr;
		if (pf)
			return pf->createOffscreenContext (width, height, scaleFactor);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
CCoord COffscreenContext::getWidth () const
{
	return bitmap ? bitmap->getWidth () : 0.;
}

//-----------------------------------------------------------------------------
CCoord COffscreenContext::getHeight () const
{
	return bitmap ? bitmap->getHeight () : 0.;
}

} // namespace
