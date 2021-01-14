// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "coffscreencontext.h"
#include "cframe.h"
#include "cbitmap.h"
#include "platform/platformfactory.h"

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

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> COffscreenContext::create (CFrame* frame, CCoord width, CCoord height, double scaleFactor)
{
	return create ({width, height}, scaleFactor);
}
#endif

//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> COffscreenContext::create (const CPoint& size, double scaleFactor)
{
	if (size.x >= 1. && size.y >= 1.)
		return getPlatformFactory ().createOffscreenContext (size, scaleFactor);
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

//-----------------------------------------------------------------------------
SharedPointer<CBitmap> renderBitmapOffscreen (
    const CPoint& size, double scaleFactor,
    const std::function<void (CDrawContext& drawContext)> drawCallback)
{
	auto context = COffscreenContext::create (size, scaleFactor);
	if (!context)
		return nullptr;
	context->beginDraw ();
	drawCallback (*context);
	context->endDraw ();
	return shared (context->getBitmap ());
}

} // VSTGUI
