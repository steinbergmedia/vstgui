//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

	void drawRect (CDrawContext *pContext, const CRect& r) VSTGUI_OVERRIDE_VMETHOD;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
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
	return 0;
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
			separators.push_back (separatorView);
		else
			viewIndex++;
		r = view->getViewSize ();
		if (separatorView == 0 && viewIndex == 2)
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
		CViewContainer::addView (separator, 0);
	}
	pView->setViewSize (viewSize);
	pView->setMouseableArea (viewSize);
	return CViewContainer::addView (pView, 0);
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
			it++;
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
			if (sepView == 0)
			{
				CRect r ((*it)->getViewSize ());
				controller->storeViewSize (index, style == kHorizontal ? r.getWidth () : r.getHeight (), this);
				index++;
			}
			it++;
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
	uint32_t sepIndex = 0;
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
		newSize.makeIntegral ();

		CRect constrainSize (getViewSize ());
		constrainSize.originize ();

		CView* prevSeparator = sepIndex > 2 ? getView (sepIndex - 2) : 0;
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
	ISplitViewSeparatorDrawer* drawer = 0;
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
		it++;
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
	ISplitViewSeparatorDrawer* drawer = splitView ? splitView->getDrawer () : 0;
	if (drawer)
	{
		drawer->drawSplitViewSeparator (pContext, getViewSize (), flags, index, splitView);
	}
	CViewContainer::drawRect (pContext, r);
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (hitTestSubViews (where, buttons))
	{
		return CViewContainer::onMouseDown (where, buttons);
	}
	if (buttons.isLeftButton ())
	{
		flags |= ISplitViewSeparatorDrawer::kMouseDown;
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
	if (mouseDownView)
		return CViewContainer::onMouseUp (where, buttons);
	if (flags & ISplitViewSeparatorDrawer::kMouseDown)
	{
		flags &= ~ISplitViewSeparatorDrawer::kMouseDown;
		invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (mouseDownView)
		return CViewContainer::onMouseMoved (where, buttons);
	if (flags & ISplitViewSeparatorDrawer::kMouseDown)
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
	else if (!(flags & ISplitViewSeparatorDrawer::kMouseOver))
	{
		if (!hitTestSubViews (where, buttons) && hitTest (where))
			onMouseEntered (where, buttons);
	}
	else if (flags & ISplitViewSeparatorDrawer::kMouseOver)
	{
		if (hitTestSubViews (where, buttons))
			onMouseExited (where, buttons);
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSplitViewSeparatorView::onMouseEntered (CPoint& where, const CButtonState& buttons)
{
	if (hitTestSubViews (where, buttons))
		return kMouseEventHandled;
	flags |= ISplitViewSeparatorDrawer::kMouseOver;
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
	flags &= ~ISplitViewSeparatorDrawer::kMouseOver;
	invalid ();
	getFrame ()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
bool CSplitViewSeparatorView::removed (CView* parent)
{
	if (flags & ISplitViewSeparatorDrawer::kMouseOver && getFrame ())
		getFrame ()->setCursor (kCursorDefault);
	return CViewContainer::removed (parent);
}

} // namespace
