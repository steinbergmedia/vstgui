// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "csplitview.h"
#include "cframe.h"
#include "cdrawcontext.h"
#include "../uidescription/icontroller.h"
#include <list>

namespace VSTGUI {

/// @cond ignore
//-----------------------------------------------------------------------------
class CSplitViewSeparatorView : public CViewContainer
{
public:
	CSplitViewSeparatorView (const CRect& size, CSplitView::Style style, int32_t index);

	void drawRect (CDrawContext *pContext, const CRect& r) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;

	CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) override;
	bool hitTestSubViews (const CPoint& where, const CButtonState& buttons = -1) override;

	bool removed (CView* parent) override;
protected:
	CPoint lastMousePos;
	CRect startSize;
	CSplitView::Style style;
	int32_t index;
	int32_t flags;
};
/// @endcond

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static ISplitViewController* getSplitViewController (const CView* view)
{
	IController* controller = getViewController (view, true);
	if (controller)
	{
		return dynamic_cast<ISplitViewController*> (controller);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSplitView::CSplitView (const CRect& size, Style style, CCoord separatorWidth, ISplitViewSeparatorDrawer* separatorDrawer)
: CViewContainer (size)
, style (style)
, resizeMethod (kResizeLastView)
, separatorWidth (separatorWidth)
, separatorDrawer (separatorDrawer)
{
}

//-----------------------------------------------------------------------------
void CSplitView::setStyle (Style s)
{
	if (style != s)
	{
		style = s;
		// TODO: rearrange views
	}
}

//-----------------------------------------------------------------------------
void CSplitView::setResizeMethod (ResizeMethod method)
{
	resizeMethod = method;
}

//-----------------------------------------------------------------------------
void CSplitView::setSeparatorWidth (CCoord width)
{
	if (width != separatorWidth)
	{
		ReverseViewIterator it (this);
		while (*it)
		{
			CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (*it);
			if (separatorView)
			{
				CRect r (separatorView->getViewSize ());
				if (style == kHorizontal)
					r.setWidth (width);
				else
					r.setHeight (width);
				requestNewSeparatorSize (separatorView, r);
			}
			++it;
		}
		separatorWidth = width;
	}
}

//-----------------------------------------------------------------------------
void CSplitView::resizeFirstView (CPoint diff)
{
	CRect r;
	std::list<CSplitViewSeparatorView*> separators;
	ViewIterator it (this);
	if (*it)
	{
		CView* view = *it;
		r = view->getViewSize ();
		r.right += diff.x;
		r.bottom += diff.y;
		view->setViewSize (r);
		view->setMouseableArea (r);
		++it;
	}
	while (*it)
	{
		CView* view = *it;
		CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view);
		if (separatorView)
			separators.emplace_back (separatorView);
		r = view->getViewSize ();
		if (style == kHorizontal)
		{
			r.offset (diff.x, 0);
			r.bottom += diff.y;
		}
		else
		{
			r.offset (0, diff.y);
			r.right += diff.x;
		}
		view->setViewSize (r);
		view->setMouseableArea (r);
		++it;
	}
	for (auto& seperatorView : separators)
	{
		CRect r (seperatorView->getViewSize ());
		requestNewSeparatorSize (seperatorView, r);
	}
}

//-----------------------------------------------------------------------------
void CSplitView::resizeSecondView (CPoint diff)
{
	CRect r;
	std::list<CSplitViewSeparatorView*> separators;
	ViewIterator it (this);
	int32_t viewIndex = 0;
	while (*it)
	{
		CView* view = *it;
		CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view);
		if (separatorView)
			separators.emplace_back (separatorView);
		else
			viewIndex++;
		r = view->getViewSize ();
		if (separatorView == nullptr && viewIndex == 2)
		{
			r.right += diff.x;
			r.bottom += diff.y;
		}
		else if (viewIndex == 1)
		{
			if (style == kHorizontal)
			{
				r.bottom += diff.y;
			}
			else
			{
				r.right += diff.x;
			}
		}
		else if (viewIndex > 1)
		{
			if (style == kHorizontal)
			{
				r.offset (diff.x, 0);
				r.bottom += diff.y;
			}
			else
			{
				r.offset (0, diff.y);
				r.right += diff.x;
			}
		}
		view->setViewSize (r);
		view->setMouseableArea (r);
		++it;
	}
	for (auto& seperatorView : separators)
	{
		CRect r (seperatorView->getViewSize ());
		requestNewSeparatorSize (seperatorView, r);
	}
}

//-----------------------------------------------------------------------------
void CSplitView::resizeLastView (CPoint diff)
{
	CRect r;
	std::list<CSplitViewSeparatorView*> separators;
	ReverseViewIterator it (this);
	if (*it)
	{
		CView* view = *it;
		r = view->getViewSize ();
		r.right += diff.x;
		r.bottom += diff.y;
		view->setViewSize (r);
		view->setMouseableArea (r);
		++it;
	}
	while (*it)
	{
		CView* view = *it;
		CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view);
		if (separatorView)
			separators.emplace_back (separatorView);
		r = view->getViewSize ();
		if (style == kHorizontal)
		{
			r.bottom += diff.y;
		}
		else
		{
			r.right += diff.x;
		}
		view->setViewSize (r);
		view->setMouseableArea (r);
		++it;
	}
	for (auto& seperatorView : separators)
	{
		CRect r (seperatorView->getViewSize ());
		requestNewSeparatorSize (seperatorView, r);
	}
}

//-----------------------------------------------------------------------------
void CSplitView::resizeViewsEqual (CPoint diff)
{
	uint32_t numViews = getNbViews ();
	if (numViews == 0)
		return;

	uint32_t numSeparators = numViews / 2;
	numViews -= numSeparators;

	CPoint offset;
	if (style == kHorizontal)
	{
		diff.x /= numViews;
		offset.y = diff.y;
	}
	else
	{
		diff.y /= numViews;
		offset.x = diff.x;
	}

	CRect r;
	ViewIterator it (this);
	std::list<CSplitViewSeparatorView*> separators;
	while (*it)
	{
		CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (*it);
		if (separatorView)
		{
			separators.emplace_back (separatorView);
			CRect r2 = separatorView->getViewSize ();
			if (style == kHorizontal)
			{
				r2.offset (offset.x, 0);
				r2.bottom += offset.y;
			}
			else
			{
				r2.offset (0, offset.y);
				r2.right += offset.x;
			}
			separatorView->setViewSize (r2);
			separatorView->setMouseableArea (r2);
		}
		else
		{
			CView* view = *it;
			r = view->getViewSize ();
			if (style == kHorizontal)
			{
				r.offset (offset.x, 0);
				r.right += diff.x;
				r.bottom += diff.y;
				offset.x += diff.x;
			}
			else
			{
				r.offset (0, offset.y);
				r.right += diff.x;
				r.bottom += diff.y;
				offset.y += diff.y;
			}
			view->setViewSize (r);
			view->setMouseableArea (r);
		}
		++it;
	}
	for (auto& seperatorView : separators)
	{
		CRect r (seperatorView->getViewSize ());
		requestNewSeparatorSize (seperatorView, r);
	}
}

//-----------------------------------------------------------------------------
void CSplitView::setViewSize (const CRect& rect, bool invalid)
{
	CPoint diff (rect.getWidth () - getWidth (), rect.getHeight () - getHeight ());
	CView::setViewSize (rect, invalid);
	if (diff.x == 0 && diff.y == 0)
		return;

	switch (getResizeMethod ())
	{
		case kResizeFirstView:
		{
			resizeFirstView (diff);
			break;
		}
		case kResizeSecondView:
		{
			resizeSecondView (diff);
			break;
		}
		case kResizeLastView:
		{
			resizeLastView (diff);
			break;
		}
		case kResizeAllViews:
		{
			resizeViewsEqual (diff);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
bool CSplitView::addView (CView* pView)
{
	CRect viewSize (pView->getViewSize ());
	viewSize.originize ();
	if (style == kHorizontal)
		viewSize.setHeight (getHeight ());
	else
		viewSize.setWidth (getWidth ());
	ReverseViewIterator it (this);
	if (*it)
	{
		CView* lastView = *it;
		CRect r (lastView->getViewSize ());
		if (style == kHorizontal)
		{
			r.left = r.right;
			r.right += getSeparatorWidth ();
			viewSize.offset (r.right, 0);
		}
		else
		{
			r.top = r.bottom;
			r.bottom += getSeparatorWidth ();
			viewSize.offset (0, r.bottom);
		}
		CSplitViewSeparatorView* separator = new CSplitViewSeparatorView (r, getStyle (), (getNbViews () - 1) / 2);
		CViewContainer::addView (separator, nullptr);
	}
	pView->setViewSize (viewSize);
	pView->setMouseableArea (viewSize);
	return CViewContainer::addView (pView, nullptr);
}

//-----------------------------------------------------------------------------
bool CSplitView::addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled)
{
	return addView (pView);
}

//-----------------------------------------------------------------------------
bool CSplitView::addView (CView* pView, CView* pBefore)
{
	return addView (pView);
}

//-----------------------------------------------------------------------------
bool CSplitView::removeView (CView* pView, bool withForget)
{
	ReverseViewIterator it (this);
	while (*it)
	{
		if (*it == pView)
		{
			++it;
			if (*it)
			{
				CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it);
				if (sepView)
				{
					CViewContainer::removeView (sepView, true);
				}
			}
			else
			{
				CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*> (getView (1));
				if (sepView)
				{
					CViewContainer::removeView (sepView, true);
				}
			}
			break;
		}
		++it;
	}
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CSplitView::removeAll (bool withForget)
{
	return CViewContainer::removeAll (withForget);
}

//-----------------------------------------------------------------------------
bool CSplitView::sizeToFit ()
{
	return false;
}

//-----------------------------------------------------------------------------
void CSplitView::storeViewSizes ()
{
	ISplitViewController* controller = getSplitViewController (this);
	if (controller)
	{
		int32_t index = 0;
		ViewIterator it (this);
		while (*it)
		{
			CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it);
			if (sepView == nullptr)
			{
				CRect r ((*it)->getViewSize ());
				controller->storeViewSize (index, style == kHorizontal ? r.getWidth () : r.getHeight (), this);
				index++;
			}
			++it;
		}
	}
}

//-----------------------------------------------------------------------------
bool CSplitView::removed (CView* parent)
{
	storeViewSizes ();
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
bool CSplitView::attached (CView* parent)
{
	bool result = CViewContainer::attached (parent);
	ISplitViewController* controller = getSplitViewController (this);
	if (controller)
	{
		CRect r;
		CPoint offset;
		int32_t index = 0;
		ViewIterator it (this);
		while (*it)
		{
			CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it);
			if (sepView)
			{
				r = sepView->getViewSize ();
				r.offset (offset.x, offset.y);
				sepView->setViewSize (r);
				sepView->setMouseableArea (r);
			}
			else
			{
				CView* view = *it;
				r = view->getViewSize ();
				r.offset (offset.x, offset.y);
				CCoord size;
				if (controller->restoreViewSize (index, size, this))
				{
					if (style == kHorizontal)
					{
						offset.x += size - r.getWidth ();
						r.setWidth (size);
					}
					else
					{
						offset.y += size - r.getHeight ();
						r.setHeight (size);
					}
				}
				view->setViewSize (r);
				view->setMouseableArea (r);
				index++;
			}			
			++it;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CSplitView::requestNewSeparatorSize (CSplitViewSeparatorView* separatorView, const CRect& _newSize)
{
	bool result = false;
	ViewIterator it (this);
	uint32_t sepIndex = 0;
	CView* view1 = nullptr;
	CView* view2 = nullptr;
	while (*it)
	{
		if (*it == separatorView)
		{
			++it;
			view2 = *it;
			break;
		}
		else
		{
			view1 = *it;
		}
		++it;
		sepIndex++;
	}
	if (view1 && view2)
	{
		CRect newSize (_newSize);
		newSize.makeIntegral ();

		CRect constrainSize (getViewSize ());
		constrainSize.originize ();

		CView* prevSeparator = sepIndex > 2 ? getView (sepIndex - 2) : nullptr;
		CView* nextSeparator = getView (sepIndex + 2);
		if (prevSeparator)
		{
			if (style == kHorizontal)
				constrainSize.left = prevSeparator->getViewSize ().right;
			else
				constrainSize.top = prevSeparator->getViewSize ().bottom;
		}
		if (nextSeparator)
		{
			if (style == kHorizontal)
				constrainSize.right = nextSeparator->getViewSize ().left;
			else
				constrainSize.bottom = nextSeparator->getViewSize ().top;
		}

		CCoord view1MinWidth = -1.;
		CCoord view1MaxWidth = -1.;
		CCoord view2MinWidth = -1.;
		CCoord view2MaxWidth = -1.;
		ISplitViewController* controller = getSplitViewController (this);
		if (controller)
		{
			if (controller->getSplitViewSizeConstraint (sepIndex / 2, view1MinWidth, view1MaxWidth, this) && view1MinWidth >= 0.)
			{
				if (style == kHorizontal)
					constrainSize.left += view1MinWidth;
				else
					constrainSize.top += view1MinWidth;
			}
			if (controller->getSplitViewSizeConstraint (sepIndex / 2 + 1, view2MinWidth, view2MaxWidth, this) && view2MinWidth >= 0.)
			{
				if (style == kHorizontal)
					constrainSize.right -= view2MinWidth;
				else
					constrainSize.bottom -= view2MinWidth;
			}
		}

		if (style == kHorizontal)
		{
			if (newSize.left < constrainSize.left)
				newSize.offset (constrainSize.left - newSize.left, 0);
			else if (newSize.right > constrainSize.right)
				newSize.offset (constrainSize.right - newSize.right, 0);
		}
		else
		{
			if (newSize.top < constrainSize.top)
				newSize.offset (0, constrainSize.top - newSize.top);
			else if (newSize.bottom > constrainSize.bottom)
				newSize.offset (0, constrainSize.bottom - newSize.bottom);
		}

		CRect r1 (view1->getViewSize ());
		CRect r2 (view2->getViewSize ());
		if (style == kHorizontal)
		{
			r1.right = newSize.left;
			r2.left = newSize.right;
		}
		else
		{
			r1.bottom = newSize.top;
			r2.top = newSize.bottom;
		}

	// TODO: if one of the view is too small or too wide, we could check to move another separator together with this one
		if (view1MaxWidth >= 0.)
		{
			if (style == kHorizontal)
			{
				if (r1.getWidth () > view1MaxWidth)
				{
					return false;
				}
			}
			else
			{
				if (r1.getHeight () > view1MaxWidth)
				{
					return false;
				}
			}
		}
		if (view1MinWidth >= 0.)
		{
			if (style == kHorizontal)
			{
				if (r1.getWidth () < view1MinWidth)
				{
					return false;
				}
			}
			else
			{
				if (r1.getHeight () < view1MinWidth)
				{
					return false;
				}
			}
		}

		if (view2MaxWidth >= 0.)
		{
			if (style == kHorizontal)
			{
				if (r2.getWidth () > view2MaxWidth)
				{
					return false;
				}
			}
			else
			{
				if (r2.getHeight () > view2MaxWidth)
				{
					return false;
				}
			}
		}
		if (view2MinWidth >= 0.)
		{
			if (style == kHorizontal)
			{
				if (r2.getWidth () < view2MinWidth)
				{
					return false;
				}
			}
			else
			{
				if (r2.getHeight () < view2MinWidth)
				{
					return false;
				}
			}
		}

		if (view1->getViewSize () != r1)
		{
			view1->setViewSize (r1);
			view1->setMouseableArea (r1);
			view1->invalid ();
		}
		if (view2->getViewSize () != r2)
		{
			view2->setViewSize (r2);
			view2->setMouseableArea (r2);
			view2->invalid ();
		}
		if (separatorView->getViewSize () != newSize)
		{
			separatorView->setViewSize (newSize);
			separatorView->setMouseableArea (newSize);
			separatorView->invalid ();
			result = true;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
ISplitViewSeparatorDrawer* CSplitView::getDrawer ()
{
	ISplitViewSeparatorDrawer* drawer = nullptr;
	ISplitViewController* controller = getSplitViewController (this);
	if (controller)
		drawer = controller->getSplitViewSeparatorDrawer (this);
	return drawer ? drawer : separatorDrawer;
}

//-----------------------------------------------------------------------------
bool CSplitView::addViewToSeparator (int32_t sepIndex, CView* view)
{
	ViewIterator it (this);
	while (*it)
	{
		CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*>(*it);
		if (sepView)
		{
			if (sepIndex == 0)
			{
				sepView->addView (view);
				return true;
			}
			sepIndex--;
		}
		++it;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSplitViewSeparatorView::CSplitViewSeparatorView (const CRect& size, CSplitView::Style style, int32_t index)
: CViewContainer (size)
, style (style)
, index (index)
, flags (0)
{
	setTransparency (true);
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::drawRect (CDrawContext *pContext, const CRect& r)
{
	CSplitView* splitView = static_cast<CSplitView*> (getParentView ());
	ISplitViewSeparatorDrawer* drawer = splitView ? splitView->getDrawer () : nullptr;
	if (drawer)
	{
		drawer->drawSplitViewSeparator (pContext, getViewSize (), flags, index, splitView);
	}
	CViewContainer::drawRect (pContext, r);
}

//------------------------------------------------------------------------
bool CSplitViewSeparatorView::hitTestSubViews (const CPoint& where, const CButtonState& buttons)
{
	return hitTest (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (CViewContainer::hitTestSubViews (where, buttons))
	{
		return CViewContainer::onMouseDown (where, buttons);
	}
	if (buttons.isLeftButton ())
	{
		setBit (flags, ISplitViewSeparatorDrawer::kMouseDown, true);
		lastMousePos = where;
		startSize = getViewSize ();
		invalid ();
		return onMouseMoved (where, buttons);
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (getMouseDownView ())
		return CViewContainer::onMouseUp (where, buttons);
	if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseDown))
	{
		setBit (flags, ISplitViewSeparatorDrawer::kMouseDown, false);
		invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (getMouseDownView ())
		return CViewContainer::onMouseMoved (where, buttons);
	if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseDown))
	{
		if (where != lastMousePos)
		{
			CRect newSize (startSize);
			if (style == CSplitView::kHorizontal)
				newSize.offset (where.x - lastMousePos.x, 0);
			else
				newSize.offset (0, where.y - lastMousePos.y);
			CSplitView* splitView = static_cast<CSplitView*> (getParentView ());
			splitView->requestNewSeparatorSize (this, newSize);
		}
	}
	else if (!hasBit (flags, ISplitViewSeparatorDrawer::kMouseOver))
	{
		if (!CViewContainer::hitTestSubViews (where, buttons) && hitTest (where))
			onMouseEntered (where, buttons);
	}
	else if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseOver))
	{
		if (CViewContainer::hitTestSubViews (where, buttons))
			onMouseExited (where, buttons);
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseEntered (CPoint& where, const CButtonState& buttons)
{
	if (CViewContainer::hitTestSubViews (where, buttons))
		return kMouseEventHandled;
	setBit (flags, ISplitViewSeparatorDrawer::kMouseOver, true);
	invalid ();
	if (style == CSplitView::kHorizontal)
		getFrame ()->setCursor (kCursorHSize);
	else
		getFrame ()->setCursor (kCursorVSize);
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	setBit (flags, ISplitViewSeparatorDrawer::kMouseOver, false);
	invalid ();
	getFrame ()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
bool CSplitViewSeparatorView::removed (CView* parent)
{
	if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseOver) && getFrame ())
		getFrame ()->setCursor (kCursorDefault);
	return CViewContainer::removed (parent);
}

} // namespace
