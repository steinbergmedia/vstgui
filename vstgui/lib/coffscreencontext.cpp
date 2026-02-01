// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "coffscreencontext.h"
#include "cframe.h"
#include "cbitmap.h"
#include "platform/platformfactory.h"
#include "platform/iplatformgraphicsdevice.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const SharedPointer<CBitmap>& bitmap)
: CDrawContext (CRect (0, 0, bitmap ? bitmap->getWidth () : 0, bitmap ? bitmap->getHeight () : 0))
, bitmap (bitmap)
{
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const CRect& surfaceRect)
: CDrawContext (surfaceRect)
{
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const PlatformGraphicsDeviceContextPtr device,
									  const CRect& surfaceRect,
									  const PlatformBitmapPtr& platformBitmap)
: CDrawContext (device, surfaceRect, platformBitmap ? platformBitmap->getScaleFactor () : 1.)
, bitmap (makeShared<CBitmap> (platformBitmap))
{
	vstgui_assert (platformBitmap);
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyFrom (CDrawContext& context, CRect destRect, CPoint srcOffset)
{
	if (bitmap)
		bitmap->draw (context, destRect, srcOffset);
}

//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> COffscreenContext::create (const CPoint& size, double scaleFactor)
{
	if (size.x >= 1. && size.y >= 1.)
	{
		if (auto graphicsDevice =
				getPlatformFactory ().getGraphicsDeviceFactory ().getDeviceForScreen (
					DefaultScreenIdentifier))
		{
			if (auto bitmap = getPlatformFactory ().createBitmap (size * scaleFactor))
			{
				bitmap->setScaleFactor (scaleFactor);
				if (auto context = graphicsDevice->createBitmapContext (bitmap))
				{
					CRect surfaceRect (CPoint (), size * scaleFactor);
					return makeShared<COffscreenContext> (context, surfaceRect, bitmap);
				}
			}
		}
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

//-----------------------------------------------------------------------------
SharedPointer<CBitmap> renderBitmapOffscreen (
    const CPoint& size, double scaleFactor,
    const std::function<void (CDrawContext& drawContext)> drawCallback)
{
	auto context = COffscreenContext::create (size, scaleFactor);
	if (!context)
		return nullptr;
	context->beginDraw ();
	drawCallback (*context.get ());
	context->endDraw ();
	return context->getBitmap ();
}

} // VSTGUI
