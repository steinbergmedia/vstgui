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
class Win32ViewLayer
: public IPlatformViewLayer
, public IPlatformTimerCallback
{
public:
	using DestroyCallback = std::function<void (Win32ViewLayer*)>;
	Win32ViewLayer (const DirectComposition::VisualPtr& visual,
					IPlatformViewLayerDelegate* inDelegate, DestroyCallback&& destroyCallback);
	~Win32ViewLayer () noexcept;

	void invalidRect (const CRect& size) override;
	void setSize (const CRect& size) override;
	void setZIndex (uint32_t zIndex) override;
	void setAlpha (float alpha) override;
	void onScaleFactorChanged (double newScaleFactor) override;

	bool drawInvalidRects ();
	const DirectComposition::VisualPtr& getVisual () const;
	const CRect& getViewSize () const { return viewSize; }

private:
	void fire () override;

	DirectComposition::VisualPtr visual;
	DestroyCallback destroyCallback;
	IPlatformViewLayerDelegate* delegate {nullptr};
	CRect viewSize;
	uint64_t lastDrawTime {0u};
	CInvalidRectList invalidRectList;
	SharedPointer<WinTimer> timer;
};

//------------------------------------------------------------------------
} // VSTGUI
