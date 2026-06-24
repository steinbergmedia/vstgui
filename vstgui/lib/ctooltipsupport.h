// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cpoint.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CTooltipSupport Declaration
//! Generic Tooltip Support class
//-----------------------------------------------------------------------------
class CTooltipSupport : public CBaseObject
{
public:
	CTooltipSupport (const CFrame& frame, uint32_t delay = 1000);

	void onMouseEntered (CView& view);
	void onMouseExited (CView& view);
	void onMouseMoved (const CPoint& where);
	void onMouseDown (const CPoint& where);

	void hideTooltip ();

protected:
	VSTGUI_SHAREDPTR_FRIEND (CTooltipSupport)
	~CTooltipSupport () noexcept override;
	bool showTooltip ();

	enum {
		kHidden,
		kVisible,
		kHiding,
		kShowing,
		kForceVisible
	};

	SharedPointer<CVSTGUITimer> timer;
	const PlatformFramePtr& frame;
	SharedPointer<CView> currentView;

	uint32_t delay;
	int32_t state;
	CPoint lastMouseMove;
};

} // VSTGUI
