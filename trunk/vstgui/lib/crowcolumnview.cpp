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
, animateViewResizing (false)
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
void CRowColumnView::setLayoutStyle (LayoutStyle style)
{
	if (style != layoutStyle)
	{
		layoutStyle = style;
		if (isAttached ())
			layoutViews ();
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::resizeSubView (CView* view, const CRect& newSize)
{
	if (view->getViewSize () != newSize)
	{
		if (isAttached () && animateViewResizing && viewResizeAnimationTime > 0)
		{
			view->addAnimation ("CRowColumnResizing", new Animation::ViewSizeAnimation (newSize, false), new Animation::LinearTimingFunction (viewResizeAnimationTime));
		}
		else
		{
			view->setMouseableArea (newSize);
			view->setViewSize (newSize);
		}
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::getMaxChildViewSize (CPoint& maxSize)
{
	ViewIterator it (this);
	while (*it)
	{
		CView* view = *it;
		CRect viewSize = view->getViewSize ();
		if (viewSize.getWidth () > maxSize.x)
			maxSize.x = viewSize.getWidth ();
		if (viewSize.getHeight () > maxSize.y)
			maxSize.y = viewSize.getHeight ();
		it++;
	}
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
	ViewIterator it (this);
	while (*it)
	{
		CView* view = *it;
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
		it++;
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::layoutViews ()
{
	if (layoutGuard == false)
	{
		layoutGuard = true;
		layoutViewsEqualSize ();
		layoutGuard = false;
	}
}

//--------------------------------------------------------------------------------
bool CRowColumnView::sizeToFit ()
{
	if (children.size () > 0)
	{
		CRect viewSize = getViewSize ();
		CPoint maxSize;
		ViewIterator it (this);
		if (style == kRowStyle)
		{
			while (*it)
			{
				CRect size = (*it)->getViewSize ();
				if (size.getWidth () > maxSize.x)
					maxSize.x = size.getWidth ();
				maxSize.y += size.getHeight () + spacing;
				it++;
			}
		}
		else
		{
			while (*it)
			{
				CRect size = (*it)->getViewSize ();
				maxSize.x += size.getWidth () + spacing;
				if (size.bottom > maxSize.y)
					maxSize.y = size.getHeight ();
				it++;
			}
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
bool CAutoLayoutContainerView::addView (CView* pView)
{
	if (CViewContainer::addView (pView, 0))
	{
		if (isAttached ())
			layoutViews ();
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------
bool CAutoLayoutContainerView::addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled)
{
	if (CViewContainer::addView (pView, mouseableArea, mouseEnabled))
	{
		if (isAttached ())
			layoutViews ();
		return true;
	}
	return false;
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

} // namespace

