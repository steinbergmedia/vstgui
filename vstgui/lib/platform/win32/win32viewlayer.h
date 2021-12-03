// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformviewlayer.h"
#include "win32directcomposition.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class Win32ViewLayer : public IPlatformViewLayer
{
public:
	Win32ViewLayer (const DirectComposition::VisualPtr& visual, IPlatformViewLayerDelegate* inDelegate);
	~Win32ViewLayer () noexcept;

	void invalidRect (const CRect& size) override;
	void setSize (const CRect& size) override;
	void setZIndex (uint32_t zIndex) override;
	void setAlpha (float alpha) override;
	void draw (CDrawContext* context, const CRect& updateRect) override;
	void onScaleFactorChanged (double newScaleFactor) override;

	const DirectComposition::VisualPtr& getVisual () const;
private:
	DirectComposition::VisualPtr visual;
	IPlatformViewLayerDelegate* delegate {nullptr};
	CRect viewSize;
};

//------------------------------------------------------------------------
} // VSTGUI
