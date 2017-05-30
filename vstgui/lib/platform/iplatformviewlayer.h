// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformviewlayer__
#define __iplatformviewlayer__

#include "../vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformViewLayerDelegate
{
public:
	virtual ~IPlatformViewLayerDelegate () noexcept = default;

	virtual void drawViewLayer (CDrawContext* context, const CRect& dirtyRect) = 0; // dirtyRect is zero based
};

//-----------------------------------------------------------------------------
class IPlatformViewLayer : public AtomicReferenceCounted
{
public:
	virtual void invalidRect (const CRect& size) = 0; ///< size must be zero based
	virtual void setSize (const CRect& size) = 0; ///< size is relative to platformParent
	virtual void setZIndex (uint32_t zIndex) = 0;
	virtual void setAlpha (float alpha) = 0;
	virtual void draw (CDrawContext* context, const CRect& updateRect) = 0;
};

}

#endif // __iplatformviewlayer__
