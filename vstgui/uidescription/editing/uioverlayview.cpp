// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uioverlayview.h"

#if VSTGUI_LIVE_EDITING

//------------------------------------------------------------------------
namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIOverlayView::UIOverlayView (const SharedPointer<CViewContainer>& view)
: CView ({}), targetView (view), targetViewParent (shared (view->getParentView ()))
{
	setMouseEnabled (false);
}

//----------------------------------------------------------------------------------------------------
UIOverlayView::~UIOverlayView ()
{
	if (auto tvp = targetViewParent.lock ())
		tvp->unregisterViewListener (this);
	if (auto tv = targetView.lock ())
		tv->unregisterViewListener (this);
}

//------------------------------------------------------------------------
void UIOverlayView::viewWillDelete (CView& view)
{
	if (auto tvp = targetViewParent.lock ())
	{
		tvp->unregisterViewListener (this);
		targetViewParent.reset ();
	}
	if (auto tv = targetView.lock ())
	{
		tv->unregisterViewListener (this);
		targetView.reset ();
	}
}

//----------------------------------------------------------------------------------------------------
bool UIOverlayView::attached (CViewContainer& parent)
{
	auto result = CView::attached (parent);
	if (auto tv = targetView.lock ())
		tv->registerViewListener (this);
	if (auto tvp = targetViewParent.lock ())
	{
		tvp->registerViewListener (this);
		viewSizeChanged (*tvp.get (), {});
	}
	return result;
}

//------------------------------------------------------------------------
bool UIOverlayView::removed (CViewContainer& parent)
{
	if (auto tvp = targetViewParent.lock ())
		tvp->unregisterViewListener (this);
	if (auto tv = targetView.lock ())
		tv->unregisterViewListener (this);
	return CView::removed (parent);
}

//----------------------------------------------------------------------------------------------------
void UIOverlayView::viewSizeChanged (CView& view, const CRect& oldSize)
{
	auto tv = targetView.lock ();
	if (!tv)
		return;
	if (&view == tv.get ())
		invalid ();

	CRect r = tv->getVisibleViewSize ();
	CPoint p;
	tv->getParentView ()->localToFrame (p);
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
