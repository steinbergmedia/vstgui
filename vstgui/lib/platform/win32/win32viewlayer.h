// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformviewlayer.h"
#include "../../cinvalidrectlist.h"
#include "win32directcomposition.h"
#include "wintimer.h"
#include <limits>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class Win32ViewLayer : public IPlatformViewLayer, public IPlatformTimerCallback
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
	void fire () override;
	void drawInvalidRects ();

	DirectComposition::VisualPtr visual;
	IPlatformViewLayerDelegate* delegate {nullptr};
	CRect viewSize;
	uint64_t lastDrawTime {0u};
	CInvalidRectList invalidRectList;
	SharedPointer<WinTimer> timer;
};

//------------------------------------------------------------------------
} // VSTGUI
