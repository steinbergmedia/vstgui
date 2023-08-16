// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformViewLayerDelegate
{
public:
	virtual ~IPlatformViewLayerDelegate () noexcept = default;

	/** rects are in client coordinates (top-left is 0, 0) */
	virtual void drawViewLayerRects (const PlatformGraphicsDeviceContextPtr& context,
									 double scaleFactor, const std::vector<CRect>& rects) = 0;
};

//-----------------------------------------------------------------------------
class IPlatformViewLayer : public AtomicReferenceCounted
{
public:
	/** size must be zero based */
	virtual void invalidRect (const CRect& size) = 0;
	/** size is relative to platformParent */
	virtual void setSize (const CRect& size) = 0;
	virtual void setZIndex (uint32_t zIndex) = 0;
	virtual void setAlpha (float alpha) = 0;
	virtual void onScaleFactorChanged (double newScaleFactor) = 0;
};

} // VSTGUI
