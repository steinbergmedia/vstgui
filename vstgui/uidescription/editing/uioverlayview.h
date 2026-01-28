// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cviewcontainer.h"
#include "../../lib/iviewlistener.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIOverlayView : public CView, public ViewListenerAdapter
//----------------------------------------------------------------------------------------------------
{
public:
	UIOverlayView (const SharedPointer<CViewContainer>& view);
	~UIOverlayView () override;

	bool attached (const SharedPointer<CViewContainer>& parent) override;
	void viewSizeChanged (CView* view, const CRect& oldSize) override;

protected:
	SharedPointer<CViewContainer> getTargetView () const { return targetView; }

private:
	SharedPointer<CViewContainer> targetView;
	SharedPointer<CView> targetViewParent;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
