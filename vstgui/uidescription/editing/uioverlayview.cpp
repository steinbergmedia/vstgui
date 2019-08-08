// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uioverlayview.h"

#if VSTGUI_LIVE_EDITING

//------------------------------------------------------------------------
namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIOverlayView::UIOverlayView (CViewContainer* view)
: CView ({}), targetView (view), targetViewParent (view->getParentView ())
{
	setMouseEnabled (false);
	targetViewParent->registerViewListener (this);
	targetView->registerViewListener (this);
}

//----------------------------------------------------------------------------------------------------
UIOverlayView::~UIOverlayView ()
{
	targetViewParent->unregisterViewListener (this);
	targetView->unregisterViewListener (this);
}

//----------------------------------------------------------------------------------------------------
bool UIOverlayView::attached (CView* parent)
{
	auto result = CView::attached (parent);
	viewSizeChanged (targetViewParent, {});
	return result;
}

//----------------------------------------------------------------------------------------------------
void UIOverlayView::viewSizeChanged (CView* view, const CRect& oldSize)
{
	if (view == targetView)
		invalid ();
	CRect r = targetView->getVisibleViewSize ();
	CPoint p;
	targetViewParent->localToFrame (p);
	r.offset (p.x, p.y);
	if (getViewSize () != r)
	{
		setViewSize (r);
		invalid ();
	}
}

//------------------------------------------------------------------------
} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
