//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "csplitview.h"
#include "cframe.h"
#include "cdrawcontext.h"
#include "../uidescription/uidescription.h"
#include <list>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CSplitViewSeparatorView : public CView
{
public:
	CSplitViewSeparatorView (const CRect& size, CSplitView::Style style, ISplitViewSeparatorDrawer* drawer);

	void draw (CDrawContext *pContext);

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);

	CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);

	bool removed (CView* parent);
protected:
	enum {
		kMouseOver = 1 << 0,
		kMouseDown = 1 << 1,
	};
	ISplitViewSeparatorDrawer* drawer;
	CPoint lastMousePos;
	CRect startSize;
	CSplitView::Style style;
	int32_t flags;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline ISplitViewController* getSplitViewController (CView* view)
{
	IController* controller = getViewController (view, true);
	if (controller)
	{
		return dynamic_cast<ISplitViewController*> (controller);
	}
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSplitView::CSplitView (const CRect& size, Style style, CCoord separatorWidth, ISplitViewSeparatorDrawer* separatorDrawer)
: CViewContainer (size, 0)
, style (style)
, resizeMethod (kResizeLastView)
, separatorWidth (separatorWidth)
, separatorDrawer (separatorDrawer)
{
}

//-----------------------------------------------------------------------------
CSplitView::~CSplitView ()
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
			it++;
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
		it++;
	}
	while (*it)
	{
		CView* view = *it;
		CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view);
		if (separatorView)
			separators.push_back (separatorView);
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
		it++;
	}
	for (std::list<CSplitViewSeparatorView*>::const_iterator sit = separators.begin (); sit != separators.end (); sit++)
	{
		CRect r ((*sit)->getViewSize ());
		requestNewSeparatorSize (*sit, r);
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
		it++;
	}
	while (*it)
	{
		CView* view = *it;
		CSplitViewSeparatorView* separatorView = dynamic_cast<CSplitViewSeparatorView*> (view);
		if (separatorView)
			separators.push_back (separatorView);
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
		it++;
	}
	for (std::list<CSplitViewSeparatorView*>::const_iterator sit = separators.begin (); sit != separators.end (); sit++)
	{
		CRect r ((*sit)->getViewSize ());
		requestNewSeparatorSize (*sit, r);
	}
}

//-----------------------------------------------------------------------------
void CSplitView::resizeViewsEqual (CPoint diff)
{
	int32_t numViews = getNbViews ();
	int32_t numSeparators = numViews / 2;
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
			separators.push_back (separatorView);
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
		it++;
	}
	for (std::list<CSplitViewSeparatorView*>::const_iterator sit = separators.begin (); sit != separators.end (); sit++)
	{
		CRect r ((*sit)->getViewSize ());
		requestNewSeparatorSize (*sit, r);
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
		ISplitViewController* controller = getSplitViewController (this);
		CSplitViewSeparatorView* separator = new CSplitViewSeparatorView (r, getStyle (), controller ? controller->getSplitViewSeparatorDrawer () : separatorDrawer);
		CViewContainer::addView (separator);
	}
	pView->setViewSize (viewSize);
	pView->setMouseableArea (viewSize);
	return CViewContainer::addView (pView);
}

//-----------------------------------------------------------------------------
bool CSplitView::addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled)
{
	return addView (pView);
}

//-----------------------------------------------------------------------------
bool CSplitView::addView (CView* pView, CView* pBefore)
{
	return false;
}

//-----------------------------------------------------------------------------
bool CSplitView::removeView (CView* pView, bool withForget)
{
	ReverseViewIterator it (this);
	while (*it)
	{
		if (*it == pView)
		{
			it++;
			if (*it)
			{
				CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it);
				if (sepView)
				{
					CViewContainer::removeView (sepView, true);
				}
			}
			break;
		}
		it++;
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
bool CSplitView::removed (CView* parent)
{
	ISplitViewController* controller = getSplitViewController (this);
	if (controller)
	{
		int32_t index = 0;
		ViewIterator it (this);
		while (*it)
		{
			CSplitViewSeparatorView* sepView = dynamic_cast<CSplitViewSeparatorView*> (*it);
			if (sepView == 0)
			{
				CRect r ((*it)->getViewSize ());
				controller->storeViewSize (index, style == kHorizontal ? r.getWidth () : r.getHeight (), this);
				index++;
			}
			it++;
		}
	}
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
			it++;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CSplitView::requestNewSeparatorSize (CSplitViewSeparatorView* separatorView, const CRect& _newSize)
{
	bool result = false;
	ViewIterator it (this);
	int32_t sepIndex = 0;
	CView* view1 = 0;
	CView* view2 = 0;
	while (*it)
	{
		if (*it == separatorView)
		{
			it++;
			view2 = *it;
			break;
		}
		else
		{
			view1 = *it;
		}
		it++;
		sepIndex++;
	}
	if (view1 && view2)
	{
		CRect newSize (_newSize);
		CRect constrainSize (getViewSize ());
		constrainSize.originize ();

		CView* prevSeparator = getView (sepIndex - 2);
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSplitViewSeparatorView::CSplitViewSeparatorView (const CRect& size, CSplitView::Style style, ISplitViewSeparatorDrawer* drawer)
: CView (size)
, style (style)
, drawer (drawer)
, flags (0)
{
}

//-----------------------------------------------------------------------------
void CSplitViewSeparatorView::draw (CDrawContext *pContext)
{
	if (drawer)
	{
		drawer->drawSplitViewSeparator (pContext, getViewSize (), flags != 0 ? ISplitViewSeparatorDrawer::kMouseOver : 0, (CSplitView*)getParentView ());
	}
	else
	{
		CColor c (255, 255, 255, 100);
		pContext->setFrameColor (c);
		pContext->setDrawMode (kAliasing);
		if (style == CSplitView::kHorizontal)
		{
			CPoint p (getViewSize ().left + getWidth () / 2 - 1, getViewSize ().top + 3);
			pContext->moveTo (p);
			p.y = getViewSize ().bottom - 2;
			pContext->lineTo (p);
		}
		else
		{
			CPoint p (getViewSize ().left + 2, getViewSize ().top + getHeight () / 2 + 1);
			pContext->moveTo (p);
			p.x = getViewSize ().right - 3;
			pContext->lineTo (p);
		}
	}
	setDirty (false);
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		flags |= kMouseDown;
		lastMousePos = where;
		startSize = getViewSize ();
		return onMouseMoved (where, buttons);
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (flags & kMouseDown)
	{
		flags &= ~kMouseDown;
		invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (flags & kMouseDown)
	{
		if (where != lastMousePos)
		{
			CRect newSize (startSize);
			if (style == CSplitView::kHorizontal)
				newSize.offset (where.x - lastMousePos.x, 0);
			else
				newSize.offset (0, where.y - lastMousePos.y);
			CSplitView* splitView = (CSplitView*)getParentView ();
			splitView->requestNewSeparatorSize (this, newSize);
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseEntered (CPoint& where, const CButtonState& buttons)
{
	flags |= kMouseOver;
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
	flags &= ~kMouseOver;
	invalid ();
	getFrame ()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
bool CSplitViewSeparatorView::removed (CView* parent)
{
	if (flags & kMouseOver && getFrame ())
		getFrame ()->setCursor (kCursorDefault);
	return CView::removed (parent);
}

} // namespace
