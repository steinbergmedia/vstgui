//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cscrollview.h"
#include "cvstguitimer.h"
#include "cdrawcontext.h"
#include "cframe.h"
#include "controls/cscrollbar.h"
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
	~CScrollContainer ();

	void setScrollOffset (CPoint offset, bool withRedraw = false);
	void getScrollOffset (CPoint& off) const { off = offset; }
	const CPoint& getScrollOffset () const { return offset; }

	CRect getContainerSize () const { return containerSize; }
	void setContainerSize (const CRect& cs);

	bool isDirty () const VSTGUI_OVERRIDE_VMETHOD;

	void onDragMove (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	void setAutoDragScroll (bool state) { autoDragScroll = state; }

	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CScrollContainer, CViewContainer)
//-----------------------------------------------------------------------------
protected:
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
, inScrolling (false)
{
}

//-----------------------------------------------------------------------------
CScrollContainer::~CScrollContainer ()
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
	inScrolling = true;
	FOREACHSUBVIEW
		CRect r = pV->getViewSize ();
		CRect mr;
		pV->getMouseableArea (mr);
		r.offset (diff.x , diff.y);
		pV->setViewSize (r, false);
		mr.offset (diff.x , diff.y);
		pV->setMouseableArea (mr);
	ENDFOREACHSUBVIEW
	inScrolling = false;
	offset = newOffset;
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

	FOREACHSUBVIEW
		if (pV->isDirty () && pV->isVisible ())
		{
			CRect r = pV->getVisibleViewSize ();
			if (r.getWidth () > 0 && r.getHeight () > 0)
				return true;
			else
				pV->setDirty (false);
		}
	ENDFOREACHSUBVIEW
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
void CScrollContainer::onDragMove (IDataPackage* drag, const CPoint& where)
{
	if (autoDragScroll)
	{
		float x, y;
		if (getScrollValue (where, x, y))
		{
			CScrollView* scrollView = static_cast<CScrollView*> (getParentView ());
			if (scrollView)
			{
				CRect r (getViewSize ());
				r.offset (x, y);
				scrollView->makeRectVisible (r);
			}
		}
	}
	return CViewContainer::onDragMove (drag, where);
}

//-----------------------------------------------------------------------------
bool CScrollContainer::attached (CView* parent)
{
	bool result = CViewContainer::attached (parent);
	if (getNbViews () == 1)
	{
		CView* view = getView (0);
		if (view)
		{
			CRect r (view->getViewSize ());
			CRect newContainerSize (containerSize);
			newContainerSize.setWidth (r.getWidth ());
			newContainerSize.setHeight (r.getHeight ());
			if (newContainerSize != containerSize)
			{
				CScrollView* scrollView = (CScrollView*)getParentView ();
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
		CView* view = static_cast<CView*> (sender);
		if (numSubViews == 1 && view && isChild (view))
		{
			CRect r (view->getViewSize ());
			CRect newContainerSize (containerSize);
			newContainerSize.setWidth (r.getWidth ());
			newContainerSize.setHeight (r.getHeight ());
			if (newContainerSize != containerSize)
			{
				CScrollView* scrollView = (CScrollView*)getParentView ();
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
, sc (0)
, vsb (0)
, hsb (0)
, containerSize (containerSize)
, scrollbarWidth (scrollbarWidth)
, style (style)
, activeScrollbarStyle (0)
{
	if (pBackground)
		setBackground(pBackground);
	recalculateSubViews ();
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CRect &size, const CRect &containerSize, CFrame* pParent, int32_t style, CCoord scrollbarWidth, CBitmap* pBackground)
: CViewContainer (size)
, sc (0)
, vsb (0)
, hsb (0)
, containerSize (containerSize)
, scrollbarWidth (scrollbarWidth)
, style (style)
, activeScrollbarStyle (0)
{
	setBackground (pBackground);
	recalculateSubViews ();
}
#endif

//-----------------------------------------------------------------------------
CScrollView::CScrollView (const CScrollView& v)
: CViewContainer (v)
, containerSize (v.containerSize)
, style (v.style)
, activeScrollbarStyle (v.activeScrollbarStyle)
, scrollbarWidth (v.scrollbarWidth)
{
	CViewContainer::removeAll ();
	if (activeScrollbarStyle & kHorizontalScrollbar && v.hsb)
	{
		hsb = (CScrollbar*)v.hsb->newCopy ();
		hsb->setListener (this);
		CViewContainer::addView (hsb, 0);
	}
	if (activeScrollbarStyle & kVerticalScrollbar && v.vsb)
	{
		vsb = (CScrollbar*)v.vsb->newCopy ();
		vsb->setListener (this);
		CViewContainer::addView (vsb, 0);
	}
	sc = (CScrollContainer*)v.sc->newCopy ();
	CViewContainer::addView (sc, 0);
}

//-----------------------------------------------------------------------------
CScrollView::~CScrollView ()
{
}

//-----------------------------------------------------------------------------
void CScrollView::recalculateSubViews ()
{
	CRect scsize (0, 0, getViewSize ().getWidth (), getViewSize ().getHeight ());
	if (!(style & kDontDrawFrame))
	{
		scsize.left++; scsize.top++;
		scsize.right-=1; scsize.bottom--;
	}
	if (style & kAutoHideScrollbars)
	{
		activeScrollbarStyle = 0;
		CRect r (scsize);
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
			CViewContainer::addView (hsb, 0);
		}
		if (!(style & kOverlayScrollbars))
			scsize.bottom = sbr.top;
		hsb->setOverlayStyle (style & kOverlayScrollbars ? true : false);
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
			CViewContainer::addView (vsb, 0);
		}
		if (!(style & kOverlayScrollbars))
			scsize.right = sbr.left;
		vsb->setOverlayStyle (style & kOverlayScrollbars ? true : false);
	}
	else if (vsb)
	{
		vsb->setVisible (false);
	}

	if (!sc)
	{
		sc = new CScrollContainer (scsize, containerSize);
		sc->setAutosizeFlags (kAutosizeAll);
		CViewContainer::addView (sc, 0);
	}
	else
	{
		sc->setViewSize (scsize, true);
		sc->setMouseableArea (scsize);
	}
	if (style & kOverlayScrollbars)
		CViewContainer::changeViewZOrder (sc, 0);
	sc->setAutoDragScroll (style & kAutoDragScrolling ? true : false);
}

//-----------------------------------------------------------------------------
void CScrollView::setViewSize (const CRect &rect, bool invalid)
{
	CViewContainer::setViewSize (rect, invalid);
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
	CRect oldSize (containerSize);
	containerSize = cs;
	if (sc)
	{
		sc->setContainerSize (cs);
	}
	if (style & kAutoHideScrollbars)
		recalculateSubViews ();
	if (vsb)
	{
		CRect oldScrollSize = vsb->getScrollSize (oldScrollSize);
		float oldValue = vsb->getValue ();
		vsb->setScrollSize (cs);
		if (cs.getHeight () <= getViewSize ().getHeight ())
			vsb->setValue (0);
		else if (sc && keepVisibleArea && oldScrollSize.getHeight () != cs.getHeight ())
		{
			CRect vSize = sc->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getHeight () - vSize.getHeight ()) / ((float)cs.getHeight () - vSize.getHeight ())));
			if (newValue > 1.f)
				newValue = 1.f;
			else if (newValue < 0.f)
				newValue = 0.f;
			vsb->setValue (newValue);
		}
		valueChanged (vsb);
	}
	if (hsb)
	{
		CRect oldScrollSize = hsb->getScrollSize (oldScrollSize);
		float oldValue = hsb->getValue ();
		hsb->setScrollSize (cs);
		if (cs.getWidth () <= getViewSize ().getWidth ())
			hsb->setValue (0);
		else if (sc && keepVisibleArea && oldScrollSize.getWidth () != cs.getWidth ())
		{
			CRect vSize = sc->getViewSize ();
			float newValue = (float)(oldValue * ((float)(oldScrollSize.getWidth () - vSize.getWidth ()) / ((float)cs.getWidth () - vSize.getWidth ())));
			if (newValue > 1.f)
				newValue = 1.f;
			else if (newValue < 0.f)
				newValue = 0.f;
			hsb->setValue (newValue);
		}
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
	if (!(style & kDontDrawFrame))
	{
		vs.left--; //vs.top--;
		vs.right++; //vs.bottom++;
	}
	CRect cs (containerSize);
	cs.offset (-cs.left, -cs.top);
	cs.setWidth (vs.getWidth ());
	cs.setHeight (vs.getHeight ());
	if (r.top >= cs.top && r.bottom <= cs.bottom && r.left >= cs.left && r.right <= cs.right)
		return;
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
		newOffset.x -= (cs.left + r.left);
	}
	else if (r.right > cs.right && r.left != cs.left)
	{
		newOffset.x += (cs.right - r.right);
	}
	if (vsb && newOffset.y != scrollOffset.y)
	{
		if (containerSize.getHeight () == vs.getHeight ())
			vsb->setValue (0.f);
		else
			vsb->setValue ((float)(newOffset.y - vs.top) / (float)(containerSize.getHeight () - vs.getHeight ()));
		vsb->bounceValue ();
		vsb->onVisualChange ();
		vsb->invalid ();
		valueChanged (vsb);
	}
	if (hsb && newOffset.x != scrollOffset.x)
	{
		if (containerSize.getWidth () == vs.getWidth ())
			hsb->setValue (0.f);
		else
			hsb->setValue (-(float)(newOffset.x - vs.left) / (float)(containerSize.getWidth () - vs.getWidth ()));
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
		vsb->setValue (0);
		vsb->bounceValue ();
		vsb->onVisualChange ();
		vsb->invalid ();
		valueChanged (vsb);
	}
	if (hsb)
	{
		hsb->setValue (0);
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

//-----------------------------------------------------------------------------
bool CScrollView::addView (CView *pView)
{
	return sc->addView (pView);
}

//-----------------------------------------------------------------------------
bool CScrollView::addView (CView *pView, const CRect &mouseableArea, bool mouseEnabled)
{
	return sc->addView (pView, mouseableArea, mouseEnabled);
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
	return sc->removeAll ();
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
					offset.x = (int32_t) (csize.left - (csize.getWidth () - vsize.getWidth ()) * value);
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
					offset.y = (int32_t) (csize.top + (csize.getHeight () - vsize.getHeight ()) * value);
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
	if ((backgroundColor.alpha != 255 && getTransparency ()) || !getTransparency ())
	{
		pContext->setDrawMode (kAliasing);
		pContext->setFillColor (backgroundColor);
		pContext->drawRect (_updateRect, kDrawFilled);
	}
	if (!(style & kDontDrawFrame))
	{
		pContext->setDrawMode (kAliasing);
		pContext->setFrameColor (backgroundColor);
		pContext->setLineWidth (1);
		pContext->drawRect (r);
	}
}

//-----------------------------------------------------------------------------
bool CScrollView::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	bool result = CViewContainer::onWheel (where, axis, distance, buttons);
	if (!result)
	{
		if (vsb && axis == kMouseWheelAxisY)
			result = vsb->onWheel (where, axis, distance, buttons);
		else if (hsb && axis == kMouseWheelAxisX)
			result = hsb->onWheel (where, axis, distance, buttons);
	}
	return result;
}

//-----------------------------------------------------------------------------
CMessageResult CScrollView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgNewFocusView && getStyle () & kFollowFocusView)
	{
		CView* focusView = (CView*)sender;
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

} // namespace

