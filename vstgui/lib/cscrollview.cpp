// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cscrollview.h"
#include "cvstguitimer.h"
#include "cdrawcontext.h"
#include "cframe.h"
#include "dragging.h"
#include "controls/cscrollbar.h"
#include "events.h"
#include "algorithm.h"
#include "viewlayouter/gridlayouter.h"
#include <cmath>

/// @cond ignore
namespace VSTGUI {

// CScrollContainer is private
//-----------------------------------------------------------------------------
class CScrollContainer : public CViewContainer
//-----------------------------------------------------------------------------
{
public:
	CScrollContainer (const CRect &size, const CRect &containerSize);
	CScrollContainer (const CScrollContainer& v);
	~CScrollContainer () override = default;

	void setScrollOffset (CPoint offset, bool withRedraw = false);
	void getScrollOffset (CPoint& off) const { off = offset; }
	const CPoint& getScrollOffset () const { return offset; }

	CRect getContainerSize () const { return containerSize; }
	void setContainerSize (const CRect& cs);

	bool isDirty () const override;

	void setAutoDragScroll (bool state) { autoDragScroll = state; }

	bool attached (CView* parent) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	SharedPointer<IDropTarget> getDropTarget () override;
	void onDragMove (CPoint where);

	CLASS_METHODS(CScrollContainer, CViewContainer)
//-----------------------------------------------------------------------------
protected:
	struct DropTarget : public IDropTarget, public NonAtomicReferenceCounted
	{
		DropTarget (CScrollContainer* scrollContainer, SharedPointer<IDropTarget>&& parent)
		: scrollContainer (scrollContainer), parent (std::move (parent))
		{
		}

		DragOperation onDragEnter (DragEventData data) override
		{
			return parent->onDragEnter (data);
		}
		DragOperation onDragMove (DragEventData data) override
		{
			scrollContainer->onDragMove (data.pos);
			return parent->onDragMove (data);
		}
		void onDragLeave (DragEventData data) override
		{
			return parent->onDragLeave (data);
		}
		bool onDrop (DragEventData data) override
		{
			return parent->onDrop (data);
		}

		CScrollContainer* scrollContainer;
		SharedPointer<IDropTarget> parent;
	};

	bool getScrollValue (const CPoint& where, float& x, float& y);

	CRect containerSize;
	CPoint offset;
	bool autoDragScroll;
	bool inScrolling;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CScrollContainer::CScrollContainer (const CRect &size, const CRect &containerSize)
: CViewContainer (size)
, containerSize (containerSize)
, offset (CPoint (0, 0))
, autoDragScroll (false)
, inScrolling (false)
{
	setTransparency (true);
}

//-----------------------------------------------------------------------------
CScrollContainer::CScrollContainer (const CScrollContainer& v)
: CViewContainer (v)
, containerSize (v.containerSize)
, offset (v.offset)
, autoDragScroll (v.autoDragScroll)
, inScrolling (false)
{
}

//-----------------------------------------------------------------------------
void CScrollContainer::setContainerSize (const CRect& cs)
{
	containerSize = cs;
	setScrollOffset (offset, false);
}

//-----------------------------------------------------------------------------
void CScrollContainer::setScrollOffset (CPoint newOffset, bool redraw)
{
	newOffset.x = floor (newOffset.x + 0.5);
	newOffset.y = floor (newOffset.y + 0.5);
	if (containerSize.getWidth () >= getViewSize ().getWidth ())
	{
		if (newOffset.x < containerSize.left - (containerSize.getWidth () - getViewSize ().getWidth ()))
			newOffset.x = containerSize.left - (containerSize.getWidth () - getViewSize ().getWidth ());
	}
	else
	{
		if (newOffset.x < containerSize.left - containerSize.getWidth ())
			newOffset.x = containerSize.left - containerSize.getWidth ();
	}
	if (newOffset.x > containerSize.right)
		newOffset.x = containerSize.right;
	if (newOffset.y < containerSize.top)
		newOffset.y = containerSize.top;
	if (newOffset.y > containerSize.bottom)
		newOffset.y = containerSize.bottom;
	CPoint diff ((int32_t)(newOffset.x - offset.x), (int32_t)(offset.y - newOffset.y));
	if (diff.x == 0 && diff.y == 0)
		return;
	offset = newOffset;
	inScrolling = true;
	for (const auto& pV : getChildren ())
	{
		CRect r = pV->getViewSize ();
		CRect mr = pV->getMouseableArea ();
		r.offset (diff.x , diff.y);
		pV->setViewSize (r, false);
		mr.offset (diff.x , diff.y);
		pV->setMouseableArea (mr);
	}
	inScrolling = false;
	if (!isAttached ())
		return;

	if (getTransparency ())
	{
		invalid ();
	}
	else
	{
		CRect scrollRect (0, 0, getViewSize ().getWidth (), getViewSize ().getHeight ());
		CPoint p;
		localToFrame (p);
		scrollRect.offset (p.x, p.y);
		CRect visibleRect = getVisibleSize (CRect (0, 0, getViewSize ().getWidth (), getViewSize ().getHeight ()));
		visibleRect.offset (p.x, p.y);
		scrollRect.bound (visibleRect);

		CPoint distance (diff.x, diff.y);
		if (distance.x > 0)
			scrollRect.right -= distance.x;
		else if (distance.x < 0)
			scrollRect.left -= distance.x;
		if (distance.y > 0)
			scrollRect.bottom -= distance.y;
		else if (distance.y < 0)
			scrollRect.top -= distance.y;
		getFrame ()->scrollRect (scrollRect, distance);
	}
}

//-----------------------------------------------------------------------------
bool CScrollContainer::isDirty () const
{
	if (CView::isDirty ())
		return true;

	for (const auto& pV : getChildren ())
	{
		if (pV->isDirty () && pV->isVisible ())
		{
			CRect r = pV->getVisibleViewSize ();
			if (r.getWidth () > 0 && r.getHeight () > 0)
				return true;
			else
				pV->setDirty (false);
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CScrollContainer::getScrollValue (const CPoint& where, float& x, float& y)
{
	const CCoord kWidth = 10;
	x = 0.f;
	y = 0.f;
	if (where.x <= getViewSize ().left + kWidth)
	{
		x = (float)(where.x - (getViewSize ().left + kWidth));
	}
	else if (where.x >= getViewSize ().right - kWidth)
	{
		x = (float)(where.x - (getViewSize ().right - kWidth));
	}
	if (where.y <= getViewSize ().top + kWidth)
	{
		y = (float)(where.y - (getViewSize ().top + kWidth));
	}
	else if (where.y >= getViewSize ().bottom - kWidth)
	{
		y = (float)(where.y - (getViewSize ().bottom - kWidth));
	}
	return (x != 0.f || y != 0.f);
}

//-----------------------------------------------------------------------------
SharedPointer<IDropTarget> CScrollContainer::getDropTarget ()
{
	auto dropTarget = CViewContainer::getDropTarget ();
	if (autoDragScroll)
	{
		return makeOwned<DropTarget> (this, std::move (dropTarget));
	}
	return dropTarget;
}

//-----------------------------------------------------------------------------
void CScrollContainer::onDragMove (CPoint where)
{
	float x, y;
	if (getScrollValue (where, x, y))
	{
		if (auto* scrollView = static_cast<CScrollView*> (getParentView ()))
		{
			CRect r (getViewSize ());
			r.originize ();
			r.offset (x, y);
			scrollView->makeRectVisible (r);
		}
	}
}

//-----------------------------------------------------------------------------
bool CScrollContainer::attached (CView* parent)
{
	bool result = CViewContainer::attached (parent);
	if (getNbViews () == 1)
	{
		if (CView* view = getView (0))
		{
			const CRect& r (view->getViewSize ());
			CRect newContainerSize (containerSize);
			newContainerSize.setWidth (r.getWidth ());
			newContainerSize.setHeight (r.getHeight ());
			if (newContainerSize != containerSize)
			{
				auto* scrollView = static_cast<CScrollView*> (getParentView ());
				if (scrollView)
					scrollView->setContainerSize (newContainerSize);
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
CMessageResult CScrollContainer::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgViewSizeChanged && !inScrolling)
	{
		uint32_t numSubViews = getNbViews ();
		auto* view = static_cast<CView*> (sender);
		if (numSubViews == 1 && view && isChild (view))
		{
			const CRect& r (view->getViewSize ());
			CRect newContainerSize (containerSize);
			newContainerSize.setWidth (r.getWidth ());
			newContainerSize.setHeight (r.getHeight ());
			if (newContainerSize != containerSize)
			{
				auto* scrollView = static_cast<CScrollView*> (getParentView ());
				if (scrollView)
					scrollView->setContainerSize (newContainerSize);
			}
		}
	}
	return getParentView () ? getParentView ()->notify (sender, message) : kMessageUnknown;
}

//------------------------------------------------------------------------
struct CScrollView::Impl
{
	CScrollContainer* scrollContainer {nullptr};
	CScrollbar* vScrollbar {nullptr};
	CScrollbar* hScrollbar {nullptr};
	CView* edgeViewTop {nullptr};
	CView* edgeViewLeft {nullptr};

	CRect containerSize {};
	CCoord scrollbarWidth {};
	int32_t style {};
	int32_t activeScrollbarStyle {};

	SharedPointer<GridLayouter> layouter;

	struct Layouter : GridLayouter
	{
		using GridLayouter::GridLayouter;

		std::optional<ViewLayout> calculateLayout (const CViewContainer& view,
												   const Children& children, const CRect& newSize)
		{
			auto& scrollView = static_cast<const CScrollView&> (view);
			scrollView.preLayouting ();
			return GridLayouter::calculateLayout (view, children, newSize);
		}
	};
};

/// @endcond

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CRect& size, const CRect& containerSize, int32_t style,
						  CCoord scrollbarWidth, CBitmap* pBackground)
: CViewContainer (size)
{
	impl = std::make_unique<Impl> ();
	impl->containerSize = containerSize;
	impl->scrollbarWidth = scrollbarWidth;
	impl->style = style;

	GridLayoutProperties props;
	props.rows = 5;
	props.columns = 5;
	props.rowGap = 0;
	props.columnGap = 0;
	props.alignItems = GridLayoutProperties::AlignItems::Stretch;
	props.justifyItems = GridLayoutProperties::JustifyItems::Stretch;
	props.alignContent = GridLayoutProperties::AlignContent::Center;
	props.justifyContent = GridLayoutProperties::JustifyContent::Center;
	props.autoRows.reserve (5);
	props.autoRows.push_back (CCoord {1});					  // border
	props.autoRows.push_back (CCoord {100});				  // top-edge view
	props.autoRows.push_back (GridLayoutProperties::Auto {}); // scroll container
	props.autoRows.push_back (CCoord {25});					  // horizontal scrollbar
	props.autoRows.push_back (CCoord {1});					  // border
	props.autoColumns.reserve (5);
	props.autoColumns.push_back (CCoord {1});					 // border
	props.autoColumns.push_back (CCoord {100});					 // left-edge view
	props.autoColumns.push_back (GridLayoutProperties::Auto {}); // scroll container
	props.autoColumns.push_back (CCoord {25});					 // vertical scrollbar
	props.autoColumns.push_back (CCoord {1});					 // border
	props.gridAreas.reserve (5);
	props.gridAreas.push_back ({2, 2, 1, 1}); // scroll container
	props.gridAreas.push_back ({1, 1, 1, 3}); // top-edge view
	props.gridAreas.push_back ({2, 1, 1, 1}); // left-edge view
	props.gridAreas.push_back ({2, 3, 1, 1}); // vertical scrollbar
	props.gridAreas.push_back ({3, 1, 1, 2}); // horizontal scrollbar

	impl->layouter = makeOwned<Impl::Layouter> (props);
	setViewLayouter (impl->layouter);

	impl->vScrollbar = new CScrollbar ({}, this, kVSBTag, CScrollbar::kVertical, {});
	impl->hScrollbar = new CScrollbar ({}, this, kHSBTag, CScrollbar::kHorizontal, {});
	impl->edgeViewTop = new CView ({});
	impl->edgeViewLeft = new CView ({});
	impl->scrollContainer = new CScrollContainer ({}, impl->containerSize);

	CViewContainer::addView (impl->scrollContainer);
	CViewContainer::addView (impl->edgeViewTop);
	CViewContainer::addView (impl->edgeViewLeft);
	CViewContainer::addView (impl->vScrollbar);
	CViewContainer::addView (impl->hScrollbar);

	impl->scrollContainer->registerViewListener (this);

	if (pBackground)
		setBackground(pBackground);
	recalculateLayout ();
}

//-----------------------------------------------------------------------------
CScrollView::~CScrollView () noexcept = default;

//-----------------------------------------------------------------------------
CRect CScrollView::calculateOptimalContainerSize () const
{
	auto size = getViewSize ();
	size.originize ();
	if (!(impl->style & kDontDrawFrame))
		size.inset (1, 1);
	if (impl->edgeViewTop)
		size.top += impl->edgeViewTop->getHeight ();
	if (impl->edgeViewLeft)
		size.left += impl->edgeViewLeft->getWidth ();
	if (!(impl->style & kAutoHideScrollbars) && !(impl->style & kOverlayScrollbars))
	{
		if (impl->style & kHorizontalScrollbar)
			size.bottom -= impl->scrollbarWidth;
		if (impl->style & kVerticalScrollbar)
			size.right -= impl->scrollbarWidth;
	}
	size.originize ();
	return size;
}

//------------------------------------------------------------------------
CRect CScrollView::getVisibleClientRect () const
{
	if (!impl->scrollContainer)
		return {};
	return impl->scrollContainer->getViewSize ();
}

//------------------------------------------------------------------------
void CScrollView::preLayouting () const
{
	auto style = impl->style;
	auto& gridProps = impl->layouter->getProperties ();
	// border
	auto borderWidth = (style & kDontDrawFrame) ? 0. : 1.;
	gridProps.autoRows[0] = borderWidth;
	gridProps.autoRows[4] = borderWidth;
	gridProps.autoColumns[0] = borderWidth;
	gridProps.autoColumns[4] = borderWidth;
	// top-edge view
	gridProps.autoRows[1] = impl->edgeViewTop->getHeight ();
	// left-edge view
	gridProps.autoColumns[1] = impl->edgeViewLeft->getWidth ();

	if (style & kOverlayScrollbars)
	{
		gridProps.gridAreas[0] = {2, 2, 2, 2}; // scroll container
		impl->hScrollbar->setOverlayStyle (true);
		impl->vScrollbar->setOverlayStyle (true);
	}
	else
	{
		gridProps.gridAreas[0] = {2, 2, 1, 1}; // scroll container
		impl->hScrollbar->setOverlayStyle (false);
		impl->vScrollbar->setOverlayStyle (false);
	}

	if (style & kAutoHideScrollbars)
	{
		impl->activeScrollbarStyle = 0;
		gridProps.autoRows[3] = 0.;
		gridProps.autoColumns[3] = 0.;
		auto layout =
			impl->layouter->GridLayouter::calculateLayout (*this, getChildren (), getViewSize ());
		if (!layout)
		{
			vstgui_assert (false, "unexpected");
			return;
		}
		auto layoutData = std::any_cast<GridLayouter::LayoutData> (&layout->data);
		auto contSize = layoutData->at (0).first;
		if (style & kHorizontalScrollbar)
		{
			if (impl->containerSize.getWidth () > contSize.getWidth ())
			{
				gridProps.autoRows[3] = impl->scrollbarWidth;
				impl->activeScrollbarStyle |= kHorizontalScrollbar;
				contSize.bottom -= impl->scrollbarWidth;
			}
			if (style & kVerticalScrollbar)
			{
				if (impl->containerSize.getHeight () > contSize.getHeight ())
				{
					gridProps.autoColumns[3] = impl->scrollbarWidth;
					impl->activeScrollbarStyle |= kVerticalScrollbar;
					contSize.right -= impl->scrollbarWidth;
					if (!(impl->activeScrollbarStyle & kHorizontalScrollbar))
					{
						if (impl->containerSize.getWidth () > contSize.getWidth ())
						{
							gridProps.autoRows[3] = impl->scrollbarWidth;
							impl->activeScrollbarStyle |= kHorizontalScrollbar;
						}
					}
				}
			}
		}
		else if (style & kVerticalScrollbar)
		{
			if (impl->containerSize.getHeight () > contSize.getHeight ())
			{
				gridProps.autoColumns[3] = impl->scrollbarWidth;
				impl->activeScrollbarStyle |= kVerticalScrollbar;
			}
		}
	}
	else
	{
		// default scrollbar sizes
		gridProps.autoRows[3] = (style & kHorizontalScrollbar) ? impl->scrollbarWidth : 0.;
		gridProps.autoColumns[3] = (style & kVerticalScrollbar) ? impl->scrollbarWidth : 0.;
	}

	impl->scrollContainer->setAutoDragScroll ((getStyle () & kAutoDragScrolling) ? true : false);
}

//-----------------------------------------------------------------------------
void CScrollView::recalculateLayout ()
{
	if (!inApplyViewLayout ())
	{
		if (auto layout = calculateViewLayout (getViewSize ()))
			applyViewLayout (*layout);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::setViewSize (const CRect &rect, bool invalid)
{
	CViewContainer::setViewSize (rect, invalid);
}

//-----------------------------------------------------------------------------
void CScrollView::setAutosizeFlags (int32_t flags)
{
	CViewContainer::setAutosizeFlags (flags);
	if (impl->scrollContainer)
		impl->scrollContainer->setAutosizeFlags (flags);
}

//-----------------------------------------------------------------------------
int32_t CScrollView::getStyle () const { return impl->style; }

//-----------------------------------------------------------------------------
void CScrollView::setStyle (int32_t newStyle)
{
	if (impl->style != newStyle)
	{
		if ((impl->style & kDontDrawFrame) != (newStyle & kDontDrawFrame))
			setBackgroundColorDrawStyle ((impl->style & kDontDrawFrame) ? kDrawFilled
																		: kDrawFilledAndStroked);
		impl->style = newStyle;
		recalculateLayout ();
	}
}

//-----------------------------------------------------------------------------
int32_t CScrollView::getActiveScrollbars () const { return impl->activeScrollbarStyle; }

//-----------------------------------------------------------------------------
CCoord CScrollView::getScrollbarWidth () const { return impl->scrollbarWidth; }

//-----------------------------------------------------------------------------
void CScrollView::setScrollbarWidth (CCoord width)
{
	if (impl->scrollbarWidth != width)
	{
		impl->scrollbarWidth = width;
		recalculateLayout ();
	}
}

//-----------------------------------------------------------------------------
CScrollbar* CScrollView::getVerticalScrollbar () const { return impl->vScrollbar; }

//-----------------------------------------------------------------------------
CScrollbar* CScrollView::getHorizontalScrollbar () const { return impl->hScrollbar; }

//-----------------------------------------------------------------------------
const CRect& CScrollView::getContainerSize () const { return impl->containerSize; }

//-----------------------------------------------------------------------------
void CScrollView::setContainerSize (const CRect& cs, bool keepVisibleArea)
{
	vstgui_assert (impl->scrollContainer != nullptr);

	impl->containerSize = cs;
	impl->scrollContainer->setContainerSize (cs);
	recalculateLayout ();
	syncScrollbars (keepVisibleArea);
}

//------------------------------------------------------------------------
void CScrollView::syncVScrollbar (bool keepVisibleArea)
{
	vstgui_assert (impl->vScrollbar != nullptr && impl->scrollContainer != nullptr);
	const auto& cs = getContainerSize ();

	CRect oldScrollSize = impl->vScrollbar->getScrollSize (oldScrollSize);
	float oldValue = impl->vScrollbar->getValueNormalized ();
	impl->vScrollbar->setScrollSize (cs);
	auto csHeight = cs.getHeight ();
	auto scrollContainerHeight = impl->scrollContainer->getHeight ();
	auto newMax = static_cast<float> (csHeight - scrollContainerHeight);
	impl->vScrollbar->setMax (newMax);
	if (csHeight <= scrollContainerHeight)
		impl->vScrollbar->setValueNormalized (0.f);
	else if (impl->scrollContainer && keepVisibleArea && oldScrollSize.getHeight () != csHeight)
	{
		CRect vSize = impl->scrollContainer->getViewSize ();
		float newValue =
			static_cast<float> (oldValue * ((oldScrollSize.getHeight () - vSize.getHeight ()) /
											(csHeight - vSize.getHeight ())));
		newValue = clampNorm (newValue);
		impl->vScrollbar->setValueNormalized (newValue);
	}
	if (impl->vScrollbar->isDirty ())
	{
		valueChanged (impl->vScrollbar);
		impl->vScrollbar->onVisualChange ();
	}
}

//------------------------------------------------------------------------
void CScrollView::syncHScrollbar (bool keepVisibleArea)
{
	vstgui_assert (impl->hScrollbar != nullptr && impl->scrollContainer != nullptr);
	const auto& cs = getContainerSize ();

	CRect oldScrollSize = impl->hScrollbar->getScrollSize (oldScrollSize);
	float oldValue = impl->hScrollbar->getValueNormalized ();
	impl->hScrollbar->setScrollSize (cs);
	auto csWidth = cs.getWidth ();
	auto scrollContainerWidth = impl->scrollContainer->getWidth ();
	auto newMax = static_cast<float> (csWidth - scrollContainerWidth);
	impl->hScrollbar->setMax (newMax);
	if (csWidth <= scrollContainerWidth)
		impl->hScrollbar->setValueNormalized (0.f);
	else if (impl->scrollContainer && keepVisibleArea && oldScrollSize.getWidth () != csWidth)
	{
		CRect vSize = impl->scrollContainer->getViewSize ();
		float newValue =
			static_cast<float> (oldValue * (oldScrollSize.getWidth () -
											vSize.getWidth () / (csWidth - vSize.getWidth ())));
		newValue = clampNorm (newValue);
		impl->hScrollbar->setValueNormalized (newValue);
	}
	if (impl->hScrollbar->isDirty ())
	{
		valueChanged (impl->hScrollbar);
		impl->hScrollbar->onVisualChange ();
	}
}

//------------------------------------------------------------------------
void CScrollView::syncScrollbars (bool keepVisibleArea)
{
	syncVScrollbar (keepVisibleArea);
	syncHScrollbar (keepVisibleArea);
}

//-----------------------------------------------------------------------------
void CScrollView::makeRectVisible (const CRect& rect)
{
	CRect r (rect);
	const CPoint& scrollOffset = impl->scrollContainer->getScrollOffset ();
	CPoint newOffset (scrollOffset);
	CRect vs = impl->scrollContainer->getViewSize ();
	vs.originize ();
	CRect cs (impl->containerSize);
	cs.originize ();
	cs.setWidth (vs.getWidth ());
	cs.setHeight (vs.getHeight ());
	if (r.top >= cs.top && r.bottom <= cs.bottom && r.left >= cs.left && r.right <= cs.right)
		return;
	newOffset.x *= -1.;
	if (r.top < cs.top)
	{
		newOffset.y -= (cs.top - r.top);
	}
	else if (r.bottom > cs.bottom)
	{
		newOffset.y += (r.bottom - cs.bottom);
	}
	if (r.left < cs.left)
	{
		newOffset.x += (cs.left + r.left);
	}
	else if (r.right > cs.right && r.left != cs.left)
	{
		newOffset.x += (r.right - cs.right);
	}
	setScrollOffset (newOffset);
}

//------------------------------------------------------------------------
void CScrollView::setScrollOffset (CPoint newOffset)
{
	const CPoint& scrollOffset = impl->scrollContainer->getScrollOffset ();
	CRect vs = impl->scrollContainer->getViewSize ();
	vs.originize ();
	if (impl->vScrollbar && newOffset.y != scrollOffset.y)
	{
		if (impl->containerSize.getHeight () == vs.getHeight ())
		{
			impl->vScrollbar->setValueNormalized (0.f);
		}
		else
		{
			impl->vScrollbar->setValue (static_cast<float> (newOffset.y));
		}
		impl->vScrollbar->bounceValue ();
		impl->vScrollbar->onVisualChange ();
		impl->vScrollbar->invalid ();
		valueChanged (impl->vScrollbar);
	}
	if (impl->hScrollbar && newOffset.x != scrollOffset.x)
	{
		if (impl->containerSize.getWidth () == vs.getWidth ())
		{
			impl->hScrollbar->setValueNormalized (0.f);
		}
		else
		{
			impl->hScrollbar->setValue (static_cast<float> (newOffset.x));
		}
		impl->hScrollbar->bounceValue ();
		impl->hScrollbar->onVisualChange ();
		impl->hScrollbar->invalid ();
		valueChanged (impl->hScrollbar);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::resetScrollOffset ()
{
	if (impl->vScrollbar)
	{
		impl->vScrollbar->setValueNormalized (0);
		impl->vScrollbar->bounceValue ();
		impl->vScrollbar->onVisualChange ();
		impl->vScrollbar->invalid ();
		valueChanged (impl->vScrollbar);
	}
	if (impl->hScrollbar)
	{
		impl->hScrollbar->setValueNormalized (0);
		impl->hScrollbar->bounceValue ();
		impl->hScrollbar->onVisualChange ();
		impl->hScrollbar->invalid ();
		valueChanged (impl->hScrollbar);
	}
}

//-----------------------------------------------------------------------------
const CPoint& CScrollView::getScrollOffset () const
{
	return impl->scrollContainer->getScrollOffset ();
}

//------------------------------------------------------------------------
void CScrollView::setEdgeView (Edge edge, CView* _view)
{
	auto view = _view;
	if (view == nullptr)
		view = new CView ({});

	switch (edge)
	{
		case Edge::Top:
		{
			impl->edgeViewTop->unregisterViewListener (this);
			CViewContainer::removeView (impl->edgeViewTop);
			impl->edgeViewTop = view;
			CViewContainer::addView (view, impl->edgeViewLeft);
			break;
		}
		case Edge::Left:
		{
			impl->edgeViewLeft->unregisterViewListener (this);
			CViewContainer::removeView (impl->edgeViewLeft);
			impl->edgeViewLeft = view;
			CViewContainer::addView (view, impl->vScrollbar);
			break;
		}
	}
	recalculateLayout ();
	setContainerSize (impl->containerSize, true);
	if (_view)
		_view->registerViewListener (this);
}

//------------------------------------------------------------------------
CView* CScrollView::getEdgeView (Edge edge) const
{
	CView* result = {};
	switch (edge)
	{
		case Edge::Top:
		{
			result = impl->edgeViewTop;
			break;
		}
		case Edge::Left:
		{
			result = impl->edgeViewLeft;
			break;
		}
	}
	if (result && result->getViewSize ().isEmpty ())
		return nullptr;
	return result;
}

//-----------------------------------------------------------------------------
bool CScrollView::attached (CView* parent)
{
	setContainerSize (impl->containerSize);
	return CViewContainer::attached (parent);
}

//-----------------------------------------------------------------------------
bool CScrollView::addView (CView* pView, CView* pBefore)
{
	return impl->scrollContainer->addView (pView, pBefore);
}

//-----------------------------------------------------------------------------
bool CScrollView::removeView (CView *pView, bool withForget)
{
	return impl->scrollContainer->removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CScrollView::removeAll (bool withForget)
{
	return impl->scrollContainer->removeAll (withForget);
}

//-----------------------------------------------------------------------------
uint32_t CScrollView::getNbViews () const { return impl->scrollContainer->getNbViews (); }

//-----------------------------------------------------------------------------
CView* CScrollView::getView (uint32_t index) const
{
	return impl->scrollContainer->getView (index);
}

//-----------------------------------------------------------------------------
bool CScrollView::changeViewZOrder (CView* view, uint32_t newIndex)
{
	return impl->scrollContainer->changeViewZOrder (view, newIndex);
}

//-----------------------------------------------------------------------------
void CScrollView::setTransparency (bool val)
{
	CViewContainer::setTransparency (val);
}

//-----------------------------------------------------------------------------
void CScrollView::setBackgroundColor (const CColor& color)
{
	CViewContainer::setBackgroundColor (color);
}

//-----------------------------------------------------------------------------
void CScrollView::valueChanged (CControl *pControl)
{
	if (impl->scrollContainer)
	{
		float value = pControl->getValue ();
		int32_t tag = pControl->getTag ();
		CPoint offset;
		CRect vsize = impl->scrollContainer->getViewSize ();
		CRect csize = impl->scrollContainer->getContainerSize ();
		impl->scrollContainer->getScrollOffset (offset);

		switch (tag)
		{
			case kHSBTag:
			{
				if (csize.getWidth () > vsize.getWidth ())
				{
					offset.x = -value;
					impl->scrollContainer->setScrollOffset (offset, false);
				}
				else if (offset.x < 0)
				{
					offset.x = 0;
					impl->scrollContainer->setScrollOffset (offset, false);
				}
				break;
			}
			case kVSBTag:
			{
				if (csize.getHeight () > vsize.getHeight ())
				{
					offset.y = value;
					impl->scrollContainer->setScrollOffset (offset, false);
				}
				else if (offset.y > 0)
				{
					offset.y = 0;
					impl->scrollContainer->setScrollOffset (offset, false);
				}
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CScrollView::drawBackgroundRect (CDrawContext *pContext, const CRect& _updateRect)
{
	CRect r (getViewSize ());
	r.originize ();
	CViewContainer::drawBackgroundRect (pContext, r);
}

//-----------------------------------------------------------------------------
void CScrollView::onMouseWheelEvent (MouseWheelEvent& event)
{
	CViewContainer::onMouseWheelEvent (event);
	if (event.consumed)
		return;
	if (impl->vScrollbar && event.deltaY != 0.)
		impl->vScrollbar->onMouseWheelEvent (event);
	if (impl->hScrollbar && event.deltaX != 0.)
		impl->hScrollbar->onMouseWheelEvent (event);
}

//-----------------------------------------------------------------------------
CMessageResult CScrollView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgNewFocusView && getStyle () & kFollowFocusView)
	{
		auto* focusView = static_cast<CView*> (sender);
		if (impl->scrollContainer->isChild (focusView, true))
		{
			CRect r = focusView->getViewSize ();
			CPoint p;
			focusView->localToFrame (p);
			frameToLocal (p);
			r.offset (p.x, p.y);
			makeRectVisible (r);
		}
	}
	return CViewContainer::notify (sender, message);
}

//-----------------------------------------------------------------------------
void CScrollView::viewSizeChanged (CView* view, const CRect& oldSize)
{
	if (view == impl->edgeViewTop || view == impl->edgeViewLeft)
		recalculateLayout ();
	else if (view == impl->scrollContainer)
		syncScrollbars (true);
	else
	{
		vstgui_assert (false, "unexpected view");
	}
}

//-----------------------------------------------------------------------------
void CScrollView::viewWillDelete (CView* view)
{
	view->unregisterViewListener (this);
	if (view == impl->edgeViewTop)
		impl->edgeViewTop = nullptr;
	else if (view == impl->edgeViewLeft)
		impl->edgeViewLeft = nullptr;
}

} // VSTGUI
