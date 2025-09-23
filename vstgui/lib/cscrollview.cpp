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
#include "viewlayouter/noviewlayouter.h"
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
	bool recalculateSubViewsRecursionGard {false};
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

	setViewLayouter (makeOwned<NoViewLayouter> ());
	if (pBackground)
		setBackground(pBackground);
	recalculateSubViews ();
}

//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CScrollView& v) : CViewContainer (v)
{
	impl = std::make_unique<Impl> (*v.impl);

	setViewLayouter (makeOwned<NoViewLayouter> ());
	CViewContainer::removeAll ();
	if (impl->activeScrollbarStyle & kHorizontalScrollbar && v.impl->hScrollbar)
	{
		impl->hScrollbar = static_cast<CScrollbar*> (v.impl->hScrollbar->newCopy ());
		impl->hScrollbar->setListener (this);
		CViewContainer::addView (impl->hScrollbar, nullptr);
	}
	if (impl->activeScrollbarStyle & kVerticalScrollbar && v.impl->vScrollbar)
	{
		impl->vScrollbar = static_cast<CScrollbar*> (v.impl->vScrollbar->newCopy ());
		impl->vScrollbar->setListener (this);
		CViewContainer::addView (impl->vScrollbar, nullptr);
	}
	impl->scrollContainer = static_cast<CScrollContainer*> (v.impl->scrollContainer->newCopy ());
	CViewContainer::addView (impl->scrollContainer, nullptr);
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

//-----------------------------------------------------------------------------
void CScrollView::recalculateSubViews ()
{
	if (impl->recalculateSubViewsRecursionGard)
		return;
	impl->recalculateSubViewsRecursionGard = true;
	CRect scsize (impl->containerSize.left, impl->containerSize.top, getViewSize ().getWidth (),
				  getViewSize ().getHeight ());
	if (!(impl->style & kDontDrawFrame))
	{
		scsize.left++; scsize.top++;
		scsize.right-=1; scsize.bottom--;
	}
	if (impl->style & kAutoHideScrollbars)
	{
		impl->activeScrollbarStyle = 0;
		CRect r (scsize);
		if (impl->edgeViewTop)
			r.top += impl->edgeViewTop->getHeight ();
		if (impl->edgeViewLeft)
			r.left += impl->edgeViewLeft->getWidth ();
		if (impl->style & kHorizontalScrollbar)
		{
			if (impl->style & kVerticalScrollbar &&
				r.getHeight () < impl->containerSize.getHeight ())
			{
				impl->activeScrollbarStyle |= kVerticalScrollbar;
				if (!(impl->style & kOverlayScrollbars))
					r.right -= impl->scrollbarWidth;
			}
			impl->activeScrollbarStyle |=
				impl->containerSize.getWidth () <= r.getWidth () ? 0 : kHorizontalScrollbar;
			if (!(impl->style & kOverlayScrollbars))
				r.bottom -= impl->scrollbarWidth;
			if (impl->activeScrollbarStyle == kHorizontalScrollbar &&
				impl->style & kVerticalScrollbar &&
				r.getHeight () < impl->containerSize.getHeight ())
			{
				impl->activeScrollbarStyle |= kVerticalScrollbar;
			}
		}
		else if (impl->style & kVerticalScrollbar)
		{
			impl->activeScrollbarStyle |=
				impl->containerSize.getHeight () <= r.getHeight () ? 0 : kVerticalScrollbar;
		}
	}
	else
	{
		impl->activeScrollbarStyle =
			(impl->style & kHorizontalScrollbar) | (impl->style & kVerticalScrollbar);
	}

	if (impl->activeScrollbarStyle & kHorizontalScrollbar)
	{
		CRect sbr (getViewSize ());
		sbr.originize ();
		sbr.top = sbr.bottom - impl->scrollbarWidth;
		if (impl->activeScrollbarStyle & kVerticalScrollbar)
		{
			if (impl->hScrollbar && (impl->vScrollbar && impl->vScrollbar->isVisible () == false))
				impl->hScrollbar->invalid ();
			sbr.right -= (impl->scrollbarWidth - 1);
		}
		if (impl->edgeViewLeft && impl->style & kOverlayScrollbars)
			sbr.left = impl->edgeViewLeft->getViewSize ().right;
		if (impl->hScrollbar)
		{
			impl->hScrollbar->setViewSize (sbr, true);
			impl->hScrollbar->setMouseableArea (sbr);
			impl->hScrollbar->setVisible (true);
		}
		else
		{
			impl->hScrollbar =
				new CScrollbar (sbr, this, kHSBTag, CScrollbar::kHorizontal, impl->containerSize);
			impl->hScrollbar->setAutosizeFlags (kAutosizeLeft | kAutosizeRight | kAutosizeBottom);
			CViewContainer::addView (impl->hScrollbar, nullptr);
			impl->hScrollbar->registerViewListener (this);
		}
		if (!(impl->style & kOverlayScrollbars))
			scsize.bottom = sbr.top;
		impl->hScrollbar->setOverlayStyle ((impl->style & kOverlayScrollbars) ? true : false);
	}
	else if (impl->hScrollbar)
	{
		impl->hScrollbar->setVisible (false);
	}
	if (impl->activeScrollbarStyle & kVerticalScrollbar)
	{
		CRect sbr (getViewSize ());
		sbr.originize ();
		sbr.left = sbr.right - impl->scrollbarWidth;
		if (impl->activeScrollbarStyle & kHorizontalScrollbar)
		{
			if (impl->vScrollbar && (impl->hScrollbar && impl->hScrollbar->isVisible () == false))
				impl->vScrollbar->invalid ();
			sbr.bottom -= (impl->scrollbarWidth - 1);
		}
		if (impl->edgeViewTop)
			sbr.top = impl->edgeViewTop->getViewSize ().bottom;
		if (impl->vScrollbar)
		{
			impl->vScrollbar->setViewSize (sbr, true);
			impl->vScrollbar->setMouseableArea (sbr);
			impl->vScrollbar->setVisible (true);
		}
		else
		{
			impl->vScrollbar =
				new CScrollbar (sbr, this, kVSBTag, CScrollbar::kVertical, impl->containerSize);
			impl->vScrollbar->setAutosizeFlags (kAutosizeTop | kAutosizeRight | kAutosizeBottom);
			CViewContainer::addView (impl->vScrollbar, nullptr);
			impl->vScrollbar->registerViewListener (this);
		}
		if (!(impl->style & kOverlayScrollbars))
			scsize.right = sbr.left;
		impl->vScrollbar->setOverlayStyle ((impl->style & kOverlayScrollbars) ? true : false);
	}
	else if (impl->vScrollbar)
	{
		impl->vScrollbar->setVisible (false);
	}

	if (impl->edgeViewTop)
	{
		auto evls = impl->edgeViewTop->getViewSize ();
		evls.originize ();
		evls.offset (scsize.getTopLeft ());
		scsize.top += evls.getHeight ();
		if (impl->style & kOverlayScrollbars)
			evls.right = scsize.right;
		else
			evls.right = getViewSize ().getWidth ();
		impl->edgeViewTop->setViewSize (evls);
	}
	if (impl->edgeViewLeft)
	{
		auto evls = impl->edgeViewLeft->getViewSize ();
		evls.originize ();
		evls.offset (scsize.getTopLeft ());
		if (impl->edgeViewTop)
			evls.top = impl->edgeViewTop->getViewSize ().bottom;
		scsize.left += evls.getWidth ();
		evls.bottom = scsize.bottom;
		impl->edgeViewLeft->setViewSize (evls);
	}

	if (!impl->scrollContainer)
	{
		impl->scrollContainer = new CScrollContainer (scsize, impl->containerSize);
		impl->scrollContainer->setAutosizeFlags (kAutosizeAll);
		CViewContainer::addView (impl->scrollContainer, CViewContainer::getView (0));
	}
	else
	{
		impl->scrollContainer->setViewSize (scsize, true);
		impl->scrollContainer->setMouseableArea (scsize);
	}
	impl->scrollContainer->setAutoDragScroll ((impl->style & kAutoDragScrolling) ? true : false);
	impl->recalculateSubViewsRecursionGard = false;
}

//-----------------------------------------------------------------------------
void CScrollView::setViewSize (const CRect &rect, bool invalid)
{
	bool autoHideScrollbars = (impl->style & kAutoHideScrollbars) != 0;
	impl->style &= ~kAutoHideScrollbars;
	CViewContainer::setViewSize (rect, invalid);
	if (autoHideScrollbars)
		impl->style |= kAutoHideScrollbars;
	setContainerSize (impl->containerSize, true);
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
		recalculateSubViews ();
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
		recalculateSubViews ();
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

	CRect oldSize (impl->containerSize);
	impl->containerSize = cs;
	impl->scrollContainer->setContainerSize (cs);
	recalculateSubViews ();
	if (impl->vScrollbar)
	{
		CRect oldScrollSize = impl->vScrollbar->getScrollSize (oldScrollSize);
		float oldValue = impl->vScrollbar->getValueNormalized ();
		impl->vScrollbar->setScrollSize (cs);
		impl->vScrollbar->setMax (static_cast<float> (
			cs.getHeight () - impl->scrollContainer->getViewSize ().getHeight ()));
		if (cs.getHeight () <= impl->scrollContainer->getViewSize ().getHeight ())
			impl->vScrollbar->setValueNormalized (0.f);
		else if (impl->scrollContainer && keepVisibleArea &&
				 oldScrollSize.getHeight () != cs.getHeight ())
		{
			CRect vSize = impl->scrollContainer->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getHeight () - vSize.getHeight ()) / ((float)cs.getHeight () - vSize.getHeight ())));
			newValue = clampNorm (newValue);
			impl->vScrollbar->setValueNormalized (newValue);
		}
		if (oldSize != impl->containerSize)
			impl->vScrollbar->onVisualChange ();
		valueChanged (impl->vScrollbar);
	}
	if (impl->hScrollbar)
	{
		CRect oldScrollSize = impl->hScrollbar->getScrollSize (oldScrollSize);
		float oldValue = impl->hScrollbar->getValueNormalized ();
		impl->hScrollbar->setScrollSize (cs);
		impl->hScrollbar->setMax (static_cast<float> (
			cs.getWidth () - impl->scrollContainer->getViewSize ().getWidth ()));
		if (cs.getWidth () <= impl->scrollContainer->getViewSize ().getWidth ())
			impl->hScrollbar->setValueNormalized (0.f);
		else if (impl->scrollContainer && keepVisibleArea &&
				 oldScrollSize.getWidth () != cs.getWidth ())
		{
			CRect vSize = impl->scrollContainer->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getWidth () - vSize.getWidth ()) / ((float)cs.getWidth () - vSize.getWidth ())));
			newValue = clampNorm (newValue);
			impl->hScrollbar->setValueNormalized (newValue);
		}
		if (oldSize != impl->containerSize)
			impl->hScrollbar->onVisualChange ();
		valueChanged (impl->hScrollbar);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::makeRectVisible (const CRect& rect)
{
	CRect r (rect);
	const CPoint& scrollOffset = impl->scrollContainer->getScrollOffset ();
	CPoint newOffset (scrollOffset);
	CRect vs = impl->scrollContainer->getViewSize ();
	vs.originize ();
#if 0
	if (!(impl->style & kDontDrawFrame))
	{
		vs.left--; //vs.top--;
		vs.right++; //vs.bottom++;
	}
#endif
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
	if (impl->vScrollbar && newOffset.y != scrollOffset.y)
	{
		if (impl->containerSize.getHeight () == vs.getHeight ())
		{
			impl->vScrollbar->setValueNormalized (0.f);
		}
		else
		{
			impl->vScrollbar->setValue (static_cast<float> (newOffset.y));
			//			auto newValue = (newOffset.y - vs.top) / (impl->containerSize.getHeight () -
			// vs.getHeight ()); 			impl->vsb->setValue (newValue * impl->vsb->getMax ());
			// impl->vsb->setValueNormalized (static_cast<float> (newValue));
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
			//			auto newValue = (newOffset.x - vs.left) / (impl->containerSize.getWidth () -
			// vs.getWidth ()); 			impl->hsb->setValueNormalized (-static_cast<float>
			// (newValue));
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
void CScrollView::setEdgeView (Edge edge, CView* view)
{
	switch (edge)
	{
		case Edge::Top:
		{
			if (impl->edgeViewTop)
			{
				impl->edgeViewTop->unregisterViewListener (this);
				CViewContainer::removeView (impl->edgeViewTop);
			}
			impl->edgeViewTop = view;
			if (view)
			{
				auto vs = view->getViewSize ();
				if (vs.getWidth () < getVisibleClientRect ().getWidth ())
				{
					vs.setWidth (getVisibleClientRect ().getWidth ());
					view->setViewSize (vs);
				}
				view->setAutosizeFlags (kAutosizeTop | kAutosizeLeft | kAutosizeRight);
				CViewContainer::addView (view, impl->scrollContainer);
			}
			break;
		}
		case Edge::Left:
		{
			if (impl->edgeViewLeft)
			{
				impl->edgeViewLeft->unregisterViewListener (this);
				CViewContainer::removeView (impl->edgeViewLeft);
			}
			impl->edgeViewLeft = view;
			if (view)
			{
				auto vs = view->getViewSize ();
				if (vs.getHeight () < getVisibleClientRect ().getHeight ())
				{
					vs.setHeight (getVisibleClientRect ().getHeight ());
					view->setViewSize (vs);
				}
				view->setAutosizeFlags (kAutosizeLeft | kAutosizeTop | kAutosizeBottom);
				CViewContainer::addView (view, impl->scrollContainer);
			}
		}
	}
	recalculateSubViews ();
	setContainerSize (impl->containerSize, true);
	if (view)
		view->registerViewListener (this);
}

//------------------------------------------------------------------------
CView* CScrollView::getEdgeView (Edge edge) const
{
	switch (edge)
	{
		case Edge::Top:
		{
			return impl->edgeViewTop;
		}
		case Edge::Left:
		{
			return impl->edgeViewLeft;
		}
	}
	return nullptr;
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
	if (view == impl->hScrollbar)
	{
		impl->hScrollbar->setScrollSize (impl->containerSize);
		impl->hScrollbar->onVisualChange ();
	}
	else if (view == impl->vScrollbar)
	{
		impl->vScrollbar->setScrollSize (impl->containerSize);
		impl->vScrollbar->onVisualChange ();
	}
	else if (view == impl->edgeViewTop)
	{
		recalculateSubViews ();
	}
	else if (view == impl->edgeViewLeft)
	{
		recalculateSubViews ();
	}
}

//-----------------------------------------------------------------------------
void CScrollView::viewWillDelete (CView* view)
{
	if (view == impl->hScrollbar || view == impl->vScrollbar || view == impl->edgeViewTop ||
		view == impl->edgeViewLeft)
		view->unregisterViewListener (this);
}

} // VSTGUI
