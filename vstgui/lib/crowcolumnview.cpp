// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "crowcolumnview.h"
#include "animation/animations.h"
#include "animation/timingfunctions.h"

namespace VSTGUI {

//--------------------------------------------------------------------------------
CRowColumnView::CRowColumnView (const CRect& size, Style style, LayoutStyle layoutStyle, CCoord spacing, const CRect& margin)
: CAutoLayoutContainerView (size)
, style (style)
, layoutStyle (layoutStyle)
, spacing (spacing)
, margin (margin)
, flags (0)
, layoutGuard (false)
, viewResizeAnimationTime (200)
{
}

//--------------------------------------------------------------------------------
void CRowColumnView::setStyle (Style newStyle)
{
	if (newStyle != style)
	{
		style = newStyle;
		if (isAttached ())
			layoutViews ();
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::setSpacing (CCoord newSpacing)
{
	if (newSpacing != spacing)
	{
		spacing = newSpacing;
		if (isAttached ())
			layoutViews ();
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::setMargin (const CRect& newMargin)
{
	if (newMargin != margin)
	{
		margin = newMargin;
		if (isAttached ())
			layoutViews ();
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::setLayoutStyle (LayoutStyle inLayoutStyle)
{
	if (inLayoutStyle != layoutStyle)
	{
		layoutStyle = inLayoutStyle;
		if (isAttached ())
			layoutViews ();
	}
}

//--------------------------------------------------------------------------------
bool CRowColumnView::isAnimateViewResizing () const
{
	return hasBit (flags, kAnimateViewResizing);
}

//--------------------------------------------------------------------------------
void CRowColumnView::setAnimateViewResizing (bool state)
{
	setBit (flags, kAnimateViewResizing, state);
}

//--------------------------------------------------------------------------------
bool CRowColumnView::hideClippedSubviews () const
{
	return hasBit (flags, kHideClippedSubViews);
}

//--------------------------------------------------------------------------------
void CRowColumnView::setHideClippedSubviews (bool state)
{
	setBit (flags, kHideClippedSubViews, state);
}

//--------------------------------------------------------------------------------
void CRowColumnView::resizeSubView (CView* view, const CRect& newSize)
{
	if (view->getViewSize () != newSize)
	{
		if (isAttached () && isAnimateViewResizing () && viewResizeAnimationTime > 0)
		{
			view->addAnimation ("CRowColumnResizing", new Animation::ViewSizeAnimation (newSize, false), new Animation::LinearTimingFunction (viewResizeAnimationTime));
		}
		else
		{
			view->setViewSize (newSize);
			view->setMouseableArea (newSize);
		}
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::getMaxChildViewSize (CPoint& maxSize)
{
	forEachChild ([&] (CView* view) {
		const CRect& viewSize = view->getViewSize ();
		if (viewSize.getWidth () > maxSize.x)
			maxSize.x = viewSize.getWidth ();
		if (viewSize.getHeight () > maxSize.y)
			maxSize.y = viewSize.getHeight ();
	});
}

//--------------------------------------------------------------------------------
void CRowColumnView::layoutViewsEqualSize ()
{
	CPoint maxSize;
	getMaxChildViewSize (maxSize);
	if (style == kRowStyle)
		maxSize.x = getViewSize ().getWidth () - (margin.right + margin.left);
	else
		maxSize.y = getViewSize ().getHeight () - (margin.top + margin.bottom);
	CPoint location = margin.getTopLeft ();
	forEachChild ([&] (CView* view) {
		CRect viewSize = view->getViewSize ();
		viewSize.originize ();
		viewSize.offset (location.x, location.y);
		switch (layoutStyle)
		{
			case kStretchEqualy:
			{
				viewSize.setWidth (maxSize.x);
				viewSize.setHeight (maxSize.y);
				break;
			}
			case kCenterEqualy:
			{
				CCoord diffX = (maxSize.x - viewSize.getWidth ()) / 2.;
				CCoord diffY = (maxSize.y - viewSize.getHeight ()) / 2.;
				viewSize.offset (diffX, diffY);
				break;
			}
			case kRightBottomEqualy:
			{
				CCoord diffX = maxSize.x - viewSize.getWidth ();
				CCoord diffY = maxSize.y - viewSize.getHeight ();
				viewSize.offset (diffX, diffY);
				break;
			}
			default:
				break;
		}
		resizeSubView (view, viewSize);
		if (style == kRowStyle)
		{
			location.y += spacing;
			location.y += viewSize.getHeight ();
		}
		else
		{
			location.x += spacing;
			location.x += viewSize.getWidth ();
		}
	});
}

//--------------------------------------------------------------------------------
void CRowColumnView::layoutViews ()
{
	if (layoutGuard == false)
	{
		layoutGuard = true;
		layoutViewsEqualSize ();
		if (hideClippedSubviews ())
		{
			forEachChild ([&] (CView* view) {
				if (view->getVisibleViewSize () != view->getViewSize ())
					view->setVisible (false);
				else
					view->setVisible (true);
			});
		}
		layoutGuard = false;
	}
}

//--------------------------------------------------------------------------------
bool CRowColumnView::sizeToFit ()
{
	if (!getChildren ().empty ())
	{
		CRect viewSize = getViewSize ();
		CPoint maxSize;
		if (style == kRowStyle)
		{
			forEachChild ([&] (CView* view) {
				const CRect& size = view->getViewSize ();
				if (size.getWidth () > maxSize.x)
					maxSize.x = size.getWidth ();
				maxSize.y += size.getHeight () + spacing;
			});
		}
		else
		{
			forEachChild ([&] (CView* view) {
				CRect size = view->getViewSize ();
				maxSize.x += size.getWidth () + spacing;
				if (size.bottom > maxSize.y)
					maxSize.y = size.getHeight ();
			});
		}
		viewSize.setWidth (maxSize.x + margin.left + margin.right);
		viewSize.setHeight (maxSize.y + margin.top + margin.bottom);
		if (viewSize != getViewSize ())
		{
			invalid ();
			CViewContainer::setViewSize (viewSize);
			CViewContainer::setMouseableArea (viewSize);
			invalid ();
		}
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------
CMessageResult CRowColumnView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgViewSizeChanged)
	{
		if (isAttached ())
			layoutViews ();
	}
	return CViewContainer::notify (sender, message);
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
CAutoLayoutContainerView::CAutoLayoutContainerView (const CRect& size)
: CViewContainer (size)
{
}

//--------------------------------------------------------------------------------
bool CAutoLayoutContainerView::attached (CView* parent)
{
	if (!isAttached ())
	{
		layoutViews ();
		return CViewContainer::attached (parent);
	}
	return false;
}

//--------------------------------------------------------------------------------
void CAutoLayoutContainerView::setViewSize (const CRect& rect, bool invalid)
{
	CViewContainer::setViewSize (rect, invalid);
	if (isAttached ())
		layoutViews ();
}

//--------------------------------------------------------------------------------
bool CAutoLayoutContainerView::addView (CView* pView, CView* pBefore)
{
	if (CViewContainer::addView (pView, pBefore))
	{
		if (isAttached ())
			layoutViews ();
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------
bool CAutoLayoutContainerView::removeView (CView* pView, bool withForget)
{
	if (CViewContainer::removeView (pView, withForget))
	{
		if (isAttached ())
			layoutViews ();
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------
bool CAutoLayoutContainerView::changeViewZOrder (CView* view, uint32_t newIndex)
{
	if (CViewContainer::changeViewZOrder (view, newIndex))
	{
		if (isAttached ())
			layoutViews ();
		return true;
	}
	return false;
}

} // VSTGUI

