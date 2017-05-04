// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __caviewlayer__
#define __caviewlayer__

#include "../iplatformviewlayer.h"

#if MAC_COCOA

#ifdef __OBJC__
@class CALayer, VSTGUI_CALayer;
#else
struct CALayer;
struct VSTGUI_CALayer;
#endif

namespace VSTGUI {
	
//-----------------------------------------------------------------------------
class CAViewLayer : public IPlatformViewLayer
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

	CALayer* getLayer () const { return layer; }
//-----------------------------------------------------------------------------
protected:
	CALayer* layer;
};

} // namespace

#endif // MAC_COCOA

#endif // __caviewlayer__
