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

/// @endcond

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CRect &size, const CRect &containerSize, int32_t style, CCoord scrollbarWidth, CBitmap* pBackground)
: CViewContainer (size)
, sc (nullptr)
, vsb (nullptr)
, hsb (nullptr)
, containerSize (containerSize)
, scrollbarWidth (scrollbarWidth)
, style (style)
, activeScrollbarStyle (0)
{
	if (pBackground)
		setBackground(pBackground);
	recalculateSubViews ();
}

//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CScrollView& v)
: CViewContainer (v)
, containerSize (v.containerSize)
, scrollbarWidth (v.scrollbarWidth)
, style (v.style)
, activeScrollbarStyle (v.activeScrollbarStyle)
{
	CViewContainer::removeAll ();
	if (activeScrollbarStyle & kHorizontalScrollbar && v.hsb)
	{
		hsb = static_cast<CScrollbar*> (v.hsb->newCopy ());
		hsb->setListener (this);
		CViewContainer::addView (hsb, nullptr);
	}
	if (activeScrollbarStyle & kVerticalScrollbar && v.vsb)
	{
		vsb = static_cast<CScrollbar*> (v.vsb->newCopy ());
		vsb->setListener (this);
		CViewContainer::addView (vsb, nullptr);
	}
	sc = static_cast<CScrollContainer*> (v.sc->newCopy ());
	CViewContainer::addView (sc, nullptr);
}

//-----------------------------------------------------------------------------
CRect CScrollView::calculateOptimalContainerSize () const
{
	auto size = getViewSize ();
	size.originize ();
	if (!(style & kDontDrawFrame))
		size.inset (1, 1);
	if (edgeViewTop)
		size.top += edgeViewTop->getHeight ();
	if (edgeViewLeft)
		size.left += edgeViewLeft->getWidth ();
	if (!(style & kAutoHideScrollbars) && !(style & kOverlayScrollbars))
	{
		if (style & kHorizontalScrollbar)
			size.right -= scrollbarWidth;
		if (style & kVerticalScrollbar)
			size.bottom -= scrollbarWidth;
	}
	size.originize ();
	return size;
}

//------------------------------------------------------------------------
CRect CScrollView::getVisibleClientRect () const
{
	if (!sc)
		return {};
	return sc->getViewSize ();
}

//-----------------------------------------------------------------------------
void CScrollView::recalculateSubViews ()
{
	if (recalculateSubViewsRecursionGard)
		return;
	recalculateSubViewsRecursionGard = true;
	CRect scsize (containerSize.left, containerSize.top, getViewSize ().getWidth (), getViewSize ().getHeight ());
	if (!(style & kDontDrawFrame))
	{
		scsize.left++; scsize.top++;
		scsize.right-=1; scsize.bottom--;
	}
	if (style & kAutoHideScrollbars)
	{
		activeScrollbarStyle = 0;
		CRect r (scsize);
		if (edgeViewTop)
			r.top += edgeViewTop->getHeight ();
		if (edgeViewLeft)
			r.left += edgeViewLeft->getWidth ();
		if (style & kHorizontalScrollbar)
		{
			if (style & kVerticalScrollbar && r.getHeight () < containerSize.getHeight ())
			{
				activeScrollbarStyle |= kVerticalScrollbar;
				if (!(style & kOverlayScrollbars))
					r.right -= scrollbarWidth;
			}
			activeScrollbarStyle |= containerSize.getWidth () <= r.getWidth () ? 0 : kHorizontalScrollbar;
			if (!(style & kOverlayScrollbars))
				r.bottom -= scrollbarWidth;
			if (activeScrollbarStyle == kHorizontalScrollbar && style & kVerticalScrollbar && r.getHeight () < containerSize.getHeight ())
			{
				activeScrollbarStyle |= kVerticalScrollbar;
			}
		}
		else if (style & kVerticalScrollbar)
		{
			activeScrollbarStyle |= containerSize.getHeight () <= r.getHeight () ? 0 : kVerticalScrollbar;
		}
	}
	else
	{
		activeScrollbarStyle = (style & kHorizontalScrollbar) | (style & kVerticalScrollbar);
	}
	
	if (activeScrollbarStyle & kHorizontalScrollbar)
	{
		CRect sbr (getViewSize ());
		sbr.originize ();
		sbr.top = sbr.bottom - scrollbarWidth;
		if (activeScrollbarStyle & kVerticalScrollbar)
		{
			if (hsb && (vsb && vsb->isVisible () == false))
				hsb->invalid ();
			sbr.right -= (scrollbarWidth - 1);
		}
		if (edgeViewLeft && style & kOverlayScrollbars)
			sbr.left = edgeViewLeft->getViewSize ().right;
		if (hsb)
		{
			hsb->setViewSize (sbr, true);
			hsb->setMouseableArea (sbr);
			hsb->setVisible (true);
		}
		else
		{
			hsb = new CScrollbar (sbr, this, kHSBTag, CScrollbar::kHorizontal, containerSize);
			hsb->setAutosizeFlags (kAutosizeLeft | kAutosizeRight | kAutosizeBottom);
			CViewContainer::addView (hsb, nullptr);
			hsb->registerViewListener (this);
		}
		if (!(style & kOverlayScrollbars))
			scsize.bottom = sbr.top;
		hsb->setOverlayStyle ((style & kOverlayScrollbars) ? true : false);
	}
	else if (hsb)
	{
		hsb->setVisible (false);
	}
	if (activeScrollbarStyle & kVerticalScrollbar)
	{
		CRect sbr (getViewSize ());
		sbr.originize ();
		sbr.left = sbr.right - scrollbarWidth;
		if (activeScrollbarStyle & kHorizontalScrollbar)
		{
			if (vsb && (hsb  && hsb->isVisible () == false))
				vsb->invalid ();
			sbr.bottom -= (scrollbarWidth - 1);
		}
		if (edgeViewTop)
			sbr.top = edgeViewTop->getViewSize ().bottom;
		if (vsb)
		{
			vsb->setViewSize (sbr, true);
			vsb->setMouseableArea (sbr);
			vsb->setVisible (true);
		}
		else
		{
			vsb = new CScrollbar (sbr, this, kVSBTag, CScrollbar::kVertical, containerSize);
			vsb->setAutosizeFlags (kAutosizeTop | kAutosizeRight | kAutosizeBottom);
			CViewContainer::addView (vsb, nullptr);
			vsb->registerViewListener (this);
		}
		if (!(style & kOverlayScrollbars))
			scsize.right = sbr.left;
		vsb->setOverlayStyle ((style & kOverlayScrollbars) ? true : false);
	}
	else if (vsb)
	{
		vsb->setVisible (false);
	}

	if (edgeViewTop)
	{
		auto evls = edgeViewTop->getViewSize ();
		evls.originize ();
		evls.offset (scsize.getTopLeft ());
		scsize.top += evls.getHeight ();
		if (style & kOverlayScrollbars)
			evls.right = scsize.right;
		else
			evls.right = getViewSize ().getWidth ();
		edgeViewTop->setViewSize (evls);
	}
	if (edgeViewLeft)
	{
		auto evls = edgeViewLeft->getViewSize ();
		evls.originize ();
		evls.offset (scsize.getTopLeft ());
		if (edgeViewTop)
			evls.top = edgeViewTop->getViewSize ().bottom;
		scsize.left += evls.getWidth ();
		evls.bottom = scsize.bottom;
		edgeViewLeft->setViewSize (evls);
	}

	if (!sc)
	{
		sc = new CScrollContainer (scsize, containerSize);
		sc->setAutosizeFlags (kAutosizeAll);
		CViewContainer::addView (sc, CViewContainer::getView (0));
	}
	else
	{
		sc->setViewSize (scsize, true);
		sc->setMouseableArea (scsize);
	}
	sc->setAutoDragScroll ((style & kAutoDragScrolling) ? true : false);
	recalculateSubViewsRecursionGard = false;
}

//-----------------------------------------------------------------------------
void CScrollView::setViewSize (const CRect &rect, bool invalid)
{
	bool autoHideScrollbars = (style & kAutoHideScrollbars) != 0;
	style &= ~ kAutoHideScrollbars;
	CViewContainer::setViewSize (rect, invalid);
	if (autoHideScrollbars)
		style |= kAutoHideScrollbars;
	setContainerSize (containerSize, true);
}

//-----------------------------------------------------------------------------
void CScrollView::setAutosizeFlags (int32_t flags)
{
	CViewContainer::setAutosizeFlags (flags);
	if (sc)
		sc->setAutosizeFlags (flags);
}

//-----------------------------------------------------------------------------
void CScrollView::setStyle (int32_t newStyle)
{
	if (style != newStyle)
	{
		if ((style & kDontDrawFrame) != (newStyle & kDontDrawFrame))
			setBackgroundColorDrawStyle ((style & kDontDrawFrame) ? kDrawFilled : kDrawFilledAndStroked);
		style = newStyle;
		recalculateSubViews ();
	}
}

//-----------------------------------------------------------------------------
void CScrollView::setScrollbarWidth (CCoord width)
{
	if (scrollbarWidth != width)
	{
		scrollbarWidth = width;
		recalculateSubViews ();
	}
}

//-----------------------------------------------------------------------------
void CScrollView::setContainerSize (const CRect& cs, bool keepVisibleArea)
{
	vstgui_assert (sc != nullptr);

	CRect oldSize (containerSize);
	containerSize = cs;
	sc->setContainerSize (cs);
	if (style & kAutoHideScrollbars)
		recalculateSubViews ();
	if (vsb)
	{
		CRect oldScrollSize = vsb->getScrollSize (oldScrollSize);
		float oldValue = vsb->getValueNormalized ();
		vsb->setScrollSize (cs);
		vsb->setMax (cs.getHeight () - sc->getViewSize ().getHeight ());
		if (cs.getHeight () <= sc->getViewSize ().getHeight ())
			vsb->setValueNormalized (0.f);
		else if (sc && keepVisibleArea && oldScrollSize.getHeight () != cs.getHeight ())
		{
			CRect vSize = sc->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getHeight () - vSize.getHeight ()) / ((float)cs.getHeight () - vSize.getHeight ())));
			newValue = clampNorm (newValue);
			vsb->setValueNormalized (newValue);
		}
		if (oldSize != containerSize)
			vsb->onVisualChange ();
		valueChanged (vsb);
	}
	if (hsb)
	{
		CRect oldScrollSize = hsb->getScrollSize (oldScrollSize);
		float oldValue = hsb->getValueNormalized ();
		hsb->setScrollSize (cs);
		hsb->setMax (cs.getWidth () - sc->getViewSize ().getWidth ());
		if (cs.getWidth () <= sc->getViewSize ().getWidth ())
			hsb->setValueNormalized (0.f);
		else if (sc && keepVisibleArea && oldScrollSize.getWidth () != cs.getWidth ())
		{
			CRect vSize = sc->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getWidth () - vSize.getWidth ()) / ((float)cs.getWidth () - vSize.getWidth ())));
			newValue = clampNorm (newValue);
			hsb->setValueNormalized (newValue);
		}
		if (oldSize != containerSize)
			hsb->onVisualChange ();
		valueChanged (hsb);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::makeRectVisible (const CRect& rect)
{
	CRect r (rect);
	const CPoint& scrollOffset = sc->getScrollOffset ();
	CPoint newOffset (scrollOffset);
	CRect vs = sc->getViewSize ();
	vs.originize ();
#if 0
	if (!(style & kDontDrawFrame))
	{
		vs.left--; //vs.top--;
		vs.right++; //vs.bottom++;
	}
#endif
	CRect cs (containerSize);
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
	if (vsb && newOffset.y != scrollOffset.y)
	{
		if (containerSize.getHeight () == vs.getHeight ())
		{
			vsb->setValueNormalized (0.f);
		}
		else
		{
			vsb->setValue (newOffset.y);
			//			auto newValue = (newOffset.y - vs.top) / (containerSize.getHeight () -
			//vs.getHeight ()); 			vsb->setValue (newValue * vsb->getMax ()); 			vsb->setValueNormalized
			//(static_cast<float> (newValue));
		}
		vsb->bounceValue ();
		vsb->onVisualChange ();
		vsb->invalid ();
		valueChanged (vsb);
	}
	if (hsb && newOffset.x != scrollOffset.x)
	{
		if (containerSize.getWidth () == vs.getWidth ())
		{
			hsb->setValueNormalized (0.f);
		}
		else
		{
			hsb->setValue (newOffset.x);
			//			auto newValue = (newOffset.x - vs.left) / (containerSize.getWidth () -
			//vs.getWidth ()); 			hsb->setValueNormalized (-static_cast<float> (newValue));
		}
		hsb->bounceValue ();
		hsb->onVisualChange ();
		hsb->invalid ();
		valueChanged (hsb);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::resetScrollOffset ()
{
	if (vsb)
	{
		vsb->setValueNormalized (0);
		vsb->bounceValue ();
		vsb->onVisualChange ();
		vsb->invalid ();
		valueChanged (vsb);
	}
	if (hsb)
	{
		hsb->setValueNormalized (0);
		hsb->bounceValue ();
		hsb->onVisualChange ();
		hsb->invalid ();
		valueChanged (hsb);
	}
}

//-----------------------------------------------------------------------------
const CPoint& CScrollView::getScrollOffset () const
{
	return sc->getScrollOffset ();
}

//------------------------------------------------------------------------
void CScrollView::setEdgeView (Edge edge, CView* view)
{
	switch (edge)
	{
		case Edge::Top:
		{
			if (edgeViewTop)
			{
				edgeViewTop->unregisterViewListener (this);
				CViewContainer::removeView (edgeViewTop);
			}
			edgeViewTop = view;
			if (view)
			{
				auto vs = view->getViewSize ();
				if (vs.getWidth () < getVisibleClientRect ().getWidth ())
					vs.setWidth (getVisibleClientRect ().getWidth ());
				CViewContainer::addView (view, sc);
			}
			break;
		}
		case Edge::Left:
		{
			if (edgeViewLeft)
			{
				edgeViewLeft->unregisterViewListener (this);
				CViewContainer::removeView (edgeViewLeft);
			}
			edgeViewLeft = view;
			if (view)
			{
				auto vs = view->getViewSize ();
				if (vs.getHeight () < getVisibleClientRect ().getHeight ())
					vs.setHeight (getVisibleClientRect ().getHeight ());
				CViewContainer::addView (view, sc);
			}
		}
	}
	setContainerSize (containerSize, true);
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
			return edgeViewTop;
		}
		case Edge::Left:
		{
			return edgeViewLeft;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
bool CScrollView::addView (CView* pView, CView* pBefore)
{
	return sc->addView (pView, pBefore);
}

//-----------------------------------------------------------------------------
bool CScrollView::removeView (CView *pView, bool withForget)
{
	return sc->removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CScrollView::removeAll (bool withForget)
{
	return sc->removeAll (withForget);
}

//-----------------------------------------------------------------------------
uint32_t CScrollView::getNbViews () const
{
	return sc->getNbViews ();
}

//-----------------------------------------------------------------------------
CView* CScrollView::getView (uint32_t index) const
{
	return sc->getView (index);
}

//-----------------------------------------------------------------------------
bool CScrollView::changeViewZOrder (CView* view, uint32_t newIndex)
{
	return sc->changeViewZOrder (view, newIndex);
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
	if (sc)
	{
		float value = pControl->getValue ();
		int32_t tag = pControl->getTag ();
		CPoint offset;
		CRect vsize = sc->getViewSize ();
		CRect csize = sc->getContainerSize ();
		sc->getScrollOffset (offset);

		switch (tag)
		{
			case kHSBTag:
			{
				if (csize.getWidth () > vsize.getWidth ())
				{
					offset.x = -value;
					sc->setScrollOffset (offset, false);
				}
				else if (offset.x < 0)
				{
					offset.x = 0;
					sc->setScrollOffset (offset, false);
				}
				break;
			}
			case kVSBTag:
			{
				if (csize.getHeight () > vsize.getHeight ())
				{
					offset.y = value;
					sc->setScrollOffset (offset, false);
				}
				else if (offset.y > 0)
				{
					offset.y = 0;
					sc->setScrollOffset (offset, false);
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
	if (vsb && event.deltaY != 0.)
		vsb->onMouseWheelEvent (event);
	if (hsb && event.deltaX != 0.)
		hsb->onMouseWheelEvent (event);
}

//-----------------------------------------------------------------------------
CMessageResult CScrollView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgNewFocusView && getStyle () & kFollowFocusView)
	{
		auto* focusView = static_cast<CView*> (sender);
		if (sc->isChild (focusView, true))
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
	if (view == hsb)
	{
		hsb->setScrollSize (containerSize);
		hsb->onVisualChange ();
	}
	else if (view == vsb)
	{
		vsb->setScrollSize (containerSize);
		vsb->onVisualChange ();
	}
	else if (view == edgeViewTop)
	{
		setContainerSize (containerSize, true);
	}
	else if (view == edgeViewLeft)
	{
		setContainerSize (containerSize, true);
	}
}

//-----------------------------------------------------------------------------
void CScrollView::viewWillDelete (CView* view)
{
	if (view == hsb || view == vsb || view == edgeViewTop || view == edgeViewLeft)
		view->unregisterViewListener (this);
}

} // VSTGUI

