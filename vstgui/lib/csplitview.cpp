// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "csplitview.h"
#include "cframe.h"
#include "cdrawcontext.h"
#include "events.h"
#include "viewlayouter/baseviewlayouter.h"
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

	void onMouseDownEvent (MouseDownEvent& event) override;
	void onMouseUpEvent (MouseUpEvent& event) override;
	void onMouseMoveEvent (MouseMoveEvent& event) override;

	void mouseMoved (MouseEvent& event);

	void onMouseEnterEvent (MouseEnterEvent& event) override;
	void onMouseExitEvent (MouseExitEvent& event) override;
	bool hitTestSubViews (const CPoint& where, const Event& event) override;

	bool removed (CView* parent) override;
protected:
	CPoint lastMousePos;
	CRect startSize;
	CSplitView::Style style;
	int32_t index;
	int32_t flags;
};

//-----------------------------------------------------------------------------
struct SplitViewLayouter final : BaseViewLayouter,
								 NonAtomicReferenceCounted
{
	std::optional<ViewLayout> resizeFirstView (const CSplitView& splitView,
											   const Children& children, const CRect& newSize,
											   CPoint diff)
	{
		LayoutData layoutData;
		layoutData.reserve (children.size ());

		auto it = children.begin ();
		if (it != children.end ())
		{
			CView* view = *it;
			auto r = view->getViewSize ();
			r.right += diff.x;
			r.bottom += diff.y;
			std::optional<ViewLayout> childLayout;
			if (auto childContainer = view->asViewContainer ())
				childLayout = childContainer->calculateViewLayout (r);
			layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
			++it;
		}
		for (; it != children.end (); ++it)
		{
			CView* view = *it;
			auto r = view->getViewSize ();
			if (splitView.getStyle () == CSplitView::kHorizontal)
			{
				r.offset (diff.x, 0);
				r.bottom += diff.y;
			}
			else
			{
				r.offset (0, diff.y);
				r.right += diff.x;
			}
			std::optional<ViewLayout> childLayout;
			if (auto childContainer = view->asViewContainer ())
				childLayout = childContainer->calculateViewLayout (r);
			layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
		}
		return {{newSize, std::move (layoutData)}};
	}

	std::optional<ViewLayout> resizeSecondView (const CSplitView& splitView,
												const Children& children, const CRect& newSize,
												CPoint diff)
	{
		LayoutData layoutData;
		layoutData.reserve (children.size ());

		int32_t viewIndex = 0;
		for (auto it = children.begin (); it != children.end (); ++it)
		{
			CView* view = *it;
			auto* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view);
			if (!separatorView)
				viewIndex++;
			auto r = view->getViewSize ();
			if (separatorView == nullptr && viewIndex == 2)
			{
				r.right += diff.x;
				r.bottom += diff.y;
			}
			else if (viewIndex == 1)
			{
				if (splitView.getStyle () == CSplitView::kHorizontal)
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
				if (splitView.getStyle () == CSplitView::kHorizontal)
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
			std::optional<ViewLayout> childLayout;
			if (auto childContainer = view->asViewContainer ())
				childLayout = childContainer->calculateViewLayout (r);
			layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
		}
		return {{newSize, std::move (layoutData)}};
	}

	std::optional<ViewLayout> resizeLastView (const CSplitView& splitView, const Children& children,
											  const CRect& newSize, CPoint diff)
	{
		LayoutData layoutData;
		layoutData.reserve (children.size ());

		auto it = children.rbegin ();
		if (it != children.rend ())
		{
			CView* view = *it;
			auto r = view->getViewSize ();
			r.right += diff.x;
			r.bottom += diff.y;
			std::optional<ViewLayout> childLayout;
			if (auto childContainer = view->asViewContainer ())
				childLayout = childContainer->calculateViewLayout (r);
			layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
			++it;
		}
		for (; it != children.rend (); ++it)
		{
			CView* view = *it;
			auto r = view->getViewSize ();
			if (splitView.getStyle () == CSplitView::kHorizontal)
			{
				r.bottom += diff.y;
			}
			else
			{
				r.right += diff.x;
			}
			std::optional<ViewLayout> childLayout;
			if (auto childContainer = view->asViewContainer ())
				childLayout = childContainer->calculateViewLayout (r);
			layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
		}
		return {{newSize, std::move (layoutData)}};
	}

	std::optional<ViewLayout> resizeViewsEqual (const CSplitView& splitView,
												const Children& children, const CRect& newSize,
												CPoint diff)
	{
		LayoutData layoutData;
		if (children.empty ())
			return {{newSize, std::move (layoutData)}};

		auto numViews = children.size ();
		layoutData.reserve (numViews);

		auto numSeparators = numViews / 2u;
		numViews -= numSeparators;

		CPoint offset;
		if (splitView.getStyle () == CSplitView::kHorizontal)
		{
			diff.x /= numViews;
			offset.y = diff.y;
		}
		else
		{
			diff.y /= numViews;
			offset.x = diff.x;
		}

		for (const auto& view : children)
		{
			if (auto* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view.get ()))
			{
				auto r = separatorView->getViewSize ();
				if (splitView.getStyle () == CSplitView::kHorizontal)
				{
					r.offset (offset.x, 0);
					r.bottom += offset.y;
				}
				else
				{
					r.offset (0, offset.y);
					r.right += offset.x;
				}
				std::optional<ViewLayout> childLayout;
				if (auto childContainer = view->asViewContainer ())
					childLayout = childContainer->calculateViewLayout (r);
				layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
			}
			else
			{
				auto r = view->getViewSize ();
				if (splitView.getStyle () == CSplitView::kHorizontal)
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
				std::optional<ViewLayout> childLayout;
				if (auto childContainer = view->asViewContainer ())
					childLayout = childContainer->calculateViewLayout (r);
				layoutData.push_back ({view->getRuntimeID (), r, r, std::move (childLayout)});
			}
		}
		return {{newSize, std::move (layoutData)}};
	}

	std::optional<ViewLayout> calculateLayout (const CViewContainer& view, const Children& children,
											   const CRect& newSize) override
	{
		auto& splitView = static_cast<const CSplitView&> (view);
		auto oldSize = view.getViewSize ();
		CPoint diff (newSize.getWidth () - oldSize.getWidth (),
					 newSize.getHeight () - oldSize.getHeight ());
		if (diff.x == 0. && diff.y == 0.)
			return {{newSize, LayoutData {}}};

		switch (splitView.getResizeMethod ())
		{
			case CSplitView::kResizeFirstView:
			{
				return resizeFirstView (splitView, children, newSize, diff);
			}
			case CSplitView::kResizeSecondView:
			{
				return resizeSecondView (splitView, children, newSize, diff);
			}
			case CSplitView::kResizeLastView:
			{
				return resizeLastView (splitView, children, newSize, diff);
			}
			case CSplitView::kResizeAllViews:
			{
				return resizeViewsEqual (splitView, children, newSize, diff);
			}
		}
		return {};
	}

	bool applyLayout (CViewContainer& view, const Children& children,
					  const ViewLayout& layout) override
	{
		if (!BaseViewLayouter::applyLayout (view, children, layout))
			return false;

		auto& splitView = static_cast<CSplitView&> (view);
		std::vector<CSplitViewSeparatorView*> separators;
		view.getChildViewsOfType<CSplitViewSeparatorView> (separators);
		std::for_each (separators.begin (), separators.end (),
					   [&] (CSplitViewSeparatorView* separatorView) {
						   CRect r (separatorView->getViewSize ());
						   splitView.requestNewSeparatorSize (separatorView, r);
					   });
		return true;
	}
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
CSplitView::CSplitView (const CRect& size, Style style, CCoord separatorWidth,
						ISplitViewSeparatorDrawer* separatorDrawer)
: CViewContainer (size)
, style (style)
, resizeMethod (kResizeLastView)
, separatorWidth (separatorWidth)
, separatorDrawer (separatorDrawer)
{
	setViewLayouter (makeOwned<SplitViewLayouter> ());
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
void CSplitView::setResizeMethod (ResizeMethod method) { resizeMethod = method; }

//-----------------------------------------------------------------------------
void CSplitView::setSeparatorWidth (CCoord width)
{
	if (width != separatorWidth)
	{
		ReverseViewIterator it (this);
		while (*it)
		{
			if (auto* separatorView = dynamic_cast<CSplitViewSeparatorView*> (*it))
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
void CSplitView::setViewSize (const CRect& rect, bool invalid)
{
	CViewContainer::setViewSize (rect, invalid);

	if (!inApplyViewLayout ())
	{
		std::vector<CSplitViewSeparatorView*> separators;
		getChildViewsOfType<CSplitViewSeparatorView> (separators);
		std::for_each (separators.begin (), separators.end (),
					   [&] (CSplitViewSeparatorView* separatorView) {
						   CRect r (separatorView->getViewSize ());
						   requestNewSeparatorSize (separatorView, r);
					   });
	}
}

//-----------------------------------------------------------------------------
bool CSplitView::addView (CView* pView, CView* pBefore)
{
	vstgui_assert (pBefore == nullptr);
	
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
		auto* separator = new CSplitViewSeparatorView (r, getStyle (), (getNbViews () - 1) / 2);
		CViewContainer::addView (separator, nullptr);
	}
	pView->setViewSize (viewSize);
	pView->setMouseableArea (viewSize);
	return CViewContainer::addView (pView, nullptr);
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
				if (auto* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it))
				{
					CViewContainer::removeView (sepView, true);
				}
			}
			else
			{
				if (auto* sepView = dynamic_cast<CSplitViewSeparatorView*> (getView (1)))
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
			auto* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it);
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
			if (auto* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it))
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
		if (auto* sepView = dynamic_cast<CSplitViewSeparatorView*>(*it))
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
	auto* splitView = static_cast<CSplitView*> (getParentView ());
	ISplitViewSeparatorDrawer* drawer = splitView ? splitView->getDrawer () : nullptr;
	if (drawer)
	{
		drawer->drawSplitViewSeparator (pContext, getViewSize (), flags, index, splitView);
	}
	CViewContainer::drawRect (pContext, r);
}

//------------------------------------------------------------------------
bool CSplitViewSeparatorView::hitTestSubViews (const CPoint& where, const Event& event)
{
	return hitTest (where, event);
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::onMouseDownEvent (MouseDownEvent& event)
{
	if (CViewContainer::hitTestSubViews (event.mousePosition, event))
	{
		CViewContainer::onMouseDownEvent (event);
	}
	else if (event.buttonState.isLeft ())
	{
		setBit (flags, ISplitViewSeparatorDrawer::kMouseDown, true);
		lastMousePos = event.mousePosition;
		startSize = getViewSize ();
		invalid ();
		mouseMoved (event);
	}
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::onMouseUpEvent (MouseUpEvent& event)
{
	if (getMouseDownView ())
		CViewContainer::onMouseUpEvent (event);
	else if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseDown))
	{
		setBit (flags, ISplitViewSeparatorDrawer::kMouseDown, false);
		invalid ();
		event.consumed = true;
	}
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::onMouseMoveEvent (MouseMoveEvent& event)
{
	if (getMouseDownView ())
		CViewContainer::onMouseMoveEvent (event);
	else
		mouseMoved (event);
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::mouseMoved (MouseEvent& event)
{
	if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseDown))
	{
		if (event.mousePosition != lastMousePos)
		{
			CRect newSize (startSize);
			if (style == CSplitView::kHorizontal)
				newSize.offset (event.mousePosition.x - lastMousePos.x, 0);
			else
				newSize.offset (0, event.mousePosition.y - lastMousePos.y);
			auto* splitView = static_cast<CSplitView*> (getParentView ());
			if (splitView)
				splitView->requestNewSeparatorSize (this, newSize);
		}
		event.consumed = true;
	}
	else if (!hasBit (flags, ISplitViewSeparatorDrawer::kMouseOver))
	{
		if (!CViewContainer::hitTestSubViews (event.mousePosition, event) && hitTest (event.mousePosition, event))
		{
			MouseEnterEvent enterEvent (event.mousePosition, event.buttonState, event.modifiers);
			onMouseEnterEvent (enterEvent);
			if (enterEvent.consumed)
				event.consumed = true;
		}
	}
	else if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseOver))
	{
		if (CViewContainer::hitTestSubViews (event.mousePosition, event))
		{
			MouseExitEvent exitEvent (event.mousePosition, event.buttonState, event.modifiers);
			onMouseExitEvent (exitEvent);
			if (exitEvent.consumed)
				event.consumed = true;
		}
	}
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::onMouseEnterEvent (MouseEnterEvent& event)
{
	if (CViewContainer::hitTestSubViews (event.mousePosition, event))
	{
		event.consumed = true;
		return;
	}
	setBit (flags, ISplitViewSeparatorDrawer::kMouseOver, true);
	invalid ();
	if (style == CSplitView::kHorizontal)
		getFrame ()->setCursor (kCursorHSize);
	else
		getFrame ()->setCursor (kCursorVSize);
	event.consumed = true;
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::onMouseExitEvent (MouseExitEvent& event)
{
	setBit (flags, ISplitViewSeparatorDrawer::kMouseOver, false);
	invalid ();
	getFrame ()->setCursor (kCursorDefault);
	event.consumed = true;
}

//-----------------------------------------------------------------------------
bool CSplitViewSeparatorView::removed (CView* parent)
{
	if (hasBit (flags, ISplitViewSeparatorDrawer::kMouseOver) && getFrame ())
		getFrame ()->setCursor (kCursorDefault);
	return CViewContainer::removed (parent);
}

} // VSTGUI
