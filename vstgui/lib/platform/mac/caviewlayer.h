// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformviewlayer.h"

#if MAC_COCOA

#include "../platform_macos.h"

namespace VSTGUI {
	
//-----------------------------------------------------------------------------
class CAViewLayer : public IPlatformViewLayer, public ICocoaViewLayer
//-----------------------------------------------------------------------------
{
public:
	CAViewLayer (CALayer* parent);
	~CAViewLayer () noexcept override;

	bool init (IPlatformViewLayerDelegate* drawDelegate);
	
	void invalidRect (const CRect& size) override;
	void setSize (const CRect& size) override;
	void setZIndex (uint32_t zIndex) override;
	void setAlpha (float alpha) override;
	void draw (CDrawContext* context, const CRect& updateRect) override;
	void onScaleFactorChanged (double newScaleFactor) override;

	CALayer* getCALayer () const override { return layer; }
//-----------------------------------------------------------------------------
protected:
	CALayer* layer;
};

} // VSTGUI

#endif // MAC_COCOA
