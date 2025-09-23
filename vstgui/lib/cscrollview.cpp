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
	CScrollContainer* sc {nullptr};
	CScrollbar* vsb {nullptr};
	CScrollbar* hsb {nullptr};

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
	if (impl->activeScrollbarStyle & kHorizontalScrollbar && v.impl->hsb)
	{
		impl->hsb = static_cast<CScrollbar*> (v.impl->hsb->newCopy ());
		impl->hsb->setListener (this);
		CViewContainer::addView (impl->hsb, nullptr);
	}
	if (impl->activeScrollbarStyle & kVerticalScrollbar && v.impl->vsb)
	{
		impl->vsb = static_cast<CScrollbar*> (v.impl->vsb->newCopy ());
		impl->vsb->setListener (this);
		CViewContainer::addView (impl->vsb, nullptr);
	}
	impl->sc = static_cast<CScrollContainer*> (v.impl->sc->newCopy ());
	CViewContainer::addView (impl->sc, nullptr);
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
	if (!impl->sc)
		return {};
	return impl->sc->getViewSize ();
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
			if (impl->hsb && (impl->vsb && impl->vsb->isVisible () == false))
				impl->hsb->invalid ();
			sbr.right -= (impl->scrollbarWidth - 1);
		}
		if (impl->edgeViewLeft && impl->style & kOverlayScrollbars)
			sbr.left = impl->edgeViewLeft->getViewSize ().right;
		if (impl->hsb)
		{
			impl->hsb->setViewSize (sbr, true);
			impl->hsb->setMouseableArea (sbr);
			impl->hsb->setVisible (true);
		}
		else
		{
			impl->hsb =
				new CScrollbar (sbr, this, kHSBTag, CScrollbar::kHorizontal, impl->containerSize);
			impl->hsb->setAutosizeFlags (kAutosizeLeft | kAutosizeRight | kAutosizeBottom);
			CViewContainer::addView (impl->hsb, nullptr);
			impl->hsb->registerViewListener (this);
		}
		if (!(impl->style & kOverlayScrollbars))
			scsize.bottom = sbr.top;
		impl->hsb->setOverlayStyle ((impl->style & kOverlayScrollbars) ? true : false);
	}
	else if (impl->hsb)
	{
		impl->hsb->setVisible (false);
	}
	if (impl->activeScrollbarStyle & kVerticalScrollbar)
	{
		CRect sbr (getViewSize ());
		sbr.originize ();
		sbr.left = sbr.right - impl->scrollbarWidth;
		if (impl->activeScrollbarStyle & kHorizontalScrollbar)
		{
			if (impl->vsb && (impl->hsb && impl->hsb->isVisible () == false))
				impl->vsb->invalid ();
			sbr.bottom -= (impl->scrollbarWidth - 1);
		}
		if (impl->edgeViewTop)
			sbr.top = impl->edgeViewTop->getViewSize ().bottom;
		if (impl->vsb)
		{
			impl->vsb->setViewSize (sbr, true);
			impl->vsb->setMouseableArea (sbr);
			impl->vsb->setVisible (true);
		}
		else
		{
			impl->vsb =
				new CScrollbar (sbr, this, kVSBTag, CScrollbar::kVertical, impl->containerSize);
			impl->vsb->setAutosizeFlags (kAutosizeTop | kAutosizeRight | kAutosizeBottom);
			CViewContainer::addView (impl->vsb, nullptr);
			impl->vsb->registerViewListener (this);
		}
		if (!(impl->style & kOverlayScrollbars))
			scsize.right = sbr.left;
		impl->vsb->setOverlayStyle ((impl->style & kOverlayScrollbars) ? true : false);
	}
	else if (impl->vsb)
	{
		impl->vsb->setVisible (false);
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

	if (!impl->sc)
	{
		impl->sc = new CScrollContainer (scsize, impl->containerSize);
		impl->sc->setAutosizeFlags (kAutosizeAll);
		CViewContainer::addView (impl->sc, CViewContainer::getView (0));
	}
	else
	{
		impl->sc->setViewSize (scsize, true);
		impl->sc->setMouseableArea (scsize);
	}
	impl->sc->setAutoDragScroll ((impl->style & kAutoDragScrolling) ? true : false);
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
	if (impl->sc)
		impl->sc->setAutosizeFlags (flags);
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
CScrollbar* CScrollView::getVerticalScrollbar () const { return impl->vsb; }

//-----------------------------------------------------------------------------
CScrollbar* CScrollView::getHorizontalScrollbar () const { return impl->hsb; }

//-----------------------------------------------------------------------------
const CRect& CScrollView::getContainerSize () const { return impl->containerSize; }

//-----------------------------------------------------------------------------
void CScrollView::setContainerSize (const CRect& cs, bool keepVisibleArea)
{
	vstgui_assert (impl->sc != nullptr);

	CRect oldSize (impl->containerSize);
	impl->containerSize = cs;
	impl->sc->setContainerSize (cs);
	recalculateSubViews ();
	if (impl->vsb)
	{
		CRect oldScrollSize = impl->vsb->getScrollSize (oldScrollSize);
		float oldValue = impl->vsb->getValueNormalized ();
		impl->vsb->setScrollSize (cs);
		impl->vsb->setMax (
			static_cast<float> (cs.getHeight () - impl->sc->getViewSize ().getHeight ()));
		if (cs.getHeight () <= impl->sc->getViewSize ().getHeight ())
			impl->vsb->setValueNormalized (0.f);
		else if (impl->sc && keepVisibleArea && oldScrollSize.getHeight () != cs.getHeight ())
		{
			CRect vSize = impl->sc->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getHeight () - vSize.getHeight ()) / ((float)cs.getHeight () - vSize.getHeight ())));
			newValue = clampNorm (newValue);
			impl->vsb->setValueNormalized (newValue);
		}
		if (oldSize != impl->containerSize)
			impl->vsb->onVisualChange ();
		valueChanged (impl->vsb);
	}
	if (impl->hsb)
	{
		CRect oldScrollSize = impl->hsb->getScrollSize (oldScrollSize);
		float oldValue = impl->hsb->getValueNormalized ();
		impl->hsb->setScrollSize (cs);
		impl->hsb->setMax (
			static_cast<float> (cs.getWidth () - impl->sc->getViewSize ().getWidth ()));
		if (cs.getWidth () <= impl->sc->getViewSize ().getWidth ())
			impl->hsb->setValueNormalized (0.f);
		else if (impl->sc && keepVisibleArea && oldScrollSize.getWidth () != cs.getWidth ())
		{
			CRect vSize = impl->sc->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getWidth () - vSize.getWidth ()) / ((float)cs.getWidth () - vSize.getWidth ())));
			newValue = clampNorm (newValue);
			impl->hsb->setValueNormalized (newValue);
		}
		if (oldSize != impl->containerSize)
			impl->hsb->onVisualChange ();
		valueChanged (impl->hsb);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::makeRectVisible (const CRect& rect)
{
	CRect r (rect);
	const CPoint& scrollOffset = impl->sc->getScrollOffset ();
	CPoint newOffset (scrollOffset);
	CRect vs = impl->sc->getViewSize ();
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
	if (impl->vsb && newOffset.y != scrollOffset.y)
	{
		if (impl->containerSize.getHeight () == vs.getHeight ())
		{
			impl->vsb->setValueNormalized (0.f);
		}
		else
		{
			impl->vsb->setValue (static_cast<float> (newOffset.y));
			//			auto newValue = (newOffset.y - vs.top) / (impl->containerSize.getHeight () -
			// vs.getHeight ()); 			impl->vsb->setValue (newValue * impl->vsb->getMax ());
			// impl->vsb->setValueNormalized (static_cast<float> (newValue));
		}
		impl->vsb->bounceValue ();
		impl->vsb->onVisualChange ();
		impl->vsb->invalid ();
		valueChanged (impl->vsb);
	}
	if (impl->hsb && newOffset.x != scrollOffset.x)
	{
		if (impl->containerSize.getWidth () == vs.getWidth ())
		{
			impl->hsb->setValueNormalized (0.f);
		}
		else
		{
			impl->hsb->setValue (static_cast<float> (newOffset.x));
			//			auto newValue = (newOffset.x - vs.left) / (impl->containerSize.getWidth () -
			// vs.getWidth ()); 			impl->hsb->setValueNormalized (-static_cast<float>
			// (newValue));
		}
		impl->hsb->bounceValue ();
		impl->hsb->onVisualChange ();
		impl->hsb->invalid ();
		valueChanged (impl->hsb);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::resetScrollOffset ()
{
	if (impl->vsb)
	{
		impl->vsb->setValueNormalized (0);
		impl->vsb->bounceValue ();
		impl->vsb->onVisualChange ();
		impl->vsb->invalid ();
		valueChanged (impl->vsb);
	}
	if (impl->hsb)
	{
		impl->hsb->setValueNormalized (0);
		impl->hsb->bounceValue ();
		impl->hsb->onVisualChange ();
		impl->hsb->invalid ();
		valueChanged (impl->hsb);
	}
}

//-----------------------------------------------------------------------------
const CPoint& CScrollView::getScrollOffset () const { return impl->sc->getScrollOffset (); }

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
				CViewContainer::addView (view, impl->sc);
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
				CViewContainer::addView (view, impl->sc);
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
	return impl->sc->addView (pView, pBefore);
}

//-----------------------------------------------------------------------------
bool CScrollView::removeView (CView *pView, bool withForget)
{
	return impl->sc->removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CScrollView::removeAll (bool withForget) { return impl->sc->removeAll (withForget); }

//-----------------------------------------------------------------------------
uint32_t CScrollView::getNbViews () const { return impl->sc->getNbViews (); }

//-----------------------------------------------------------------------------
CView* CScrollView::getView (uint32_t index) const { return impl->sc->getView (index); }

//-----------------------------------------------------------------------------
bool CScrollView::changeViewZOrder (CView* view, uint32_t newIndex)
{
	return impl->sc->changeViewZOrder (view, newIndex);
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
	if (impl->sc)
	{
		float value = pControl->getValue ();
		int32_t tag = pControl->getTag ();
		CPoint offset;
		CRect vsize = impl->sc->getViewSize ();
		CRect csize = impl->sc->getContainerSize ();
		impl->sc->getScrollOffset (offset);

		switch (tag)
		{
			case kHSBTag:
			{
				if (csize.getWidth () > vsize.getWidth ())
				{
					offset.x = -value;
					impl->sc->setScrollOffset (offset, false);
				}
				else if (offset.x < 0)
				{
					offset.x = 0;
					impl->sc->setScrollOffset (offset, false);
				}
				break;
			}
			case kVSBTag:
			{
				if (csize.getHeight () > vsize.getHeight ())
				{
					offset.y = value;
					impl->sc->setScrollOffset (offset, false);
				}
				else if (offset.y > 0)
				{
					offset.y = 0;
					impl->sc->setScrollOffset (offset, false);
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
	if (impl->vsb && event.deltaY != 0.)
		impl->vsb->onMouseWheelEvent (event);
	if (impl->hsb && event.deltaX != 0.)
		impl->hsb->onMouseWheelEvent (event);
}

//-----------------------------------------------------------------------------
CMessageResult CScrollView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgNewFocusView && getStyle () & kFollowFocusView)
	{
		auto* focusView = static_cast<CView*> (sender);
		if (impl->sc->isChild (focusView, true))
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
	if (view == impl->hsb)
	{
		impl->hsb->setScrollSize (impl->containerSize);
		impl->hsb->onVisualChange ();
	}
	else if (view == impl->vsb)
	{
		impl->vsb->setScrollSize (impl->containerSize);
		impl->vsb->onVisualChange ();
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
	if (view == impl->hsb || view == impl->vsb || view == impl->edgeViewTop ||
		view == impl->edgeViewLeft)
		view->unregisterViewListener (this);
}

} // VSTGUI
