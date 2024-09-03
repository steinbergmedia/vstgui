// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "crowcolumnview.h"
#include "animation/animations.h"
#include "animation/timingfunctions.h"
#include <vector>

namespace VSTGUI {
namespace Layouting {
//--------------------------------------------------------------------------------
using CRects = std::vector<CRect>;

enum class Alignment
{
	kTopLeft,
	kTopCenter,
	kTopRight,
	kMiddleLeft,
	kMiddleCenter,
	kMiddleRight,
	kBottomLeft,
	kBottomCenter,
	kBottomRight
};

enum class Style
{
	kRow,
	kColumn
};

//--------------------------------------------------------------------------------
Alignment translate (const CRowColumnView::LayoutStyle layoutStyle)
{
	switch (layoutStyle)
	{
		case CRowColumnView::LayoutStyle::kTopLeft:
			return Alignment::kTopLeft;
		case CRowColumnView::LayoutStyle::kTopCenter:
			return Alignment::kTopCenter;
		case CRowColumnView::LayoutStyle::kTopRight:
			return Alignment::kTopRight;
		case CRowColumnView::LayoutStyle::kMiddleLeft:
			return Alignment::kMiddleLeft;
		case CRowColumnView::LayoutStyle::kMiddleCenter:
			return Alignment::kMiddleCenter;
		case CRowColumnView::LayoutStyle::kMiddleRight:
			return Alignment::kMiddleRight;
		case CRowColumnView::LayoutStyle::kBottomLeft:
			return Alignment::kBottomLeft;
		case CRowColumnView::LayoutStyle::kBottomCenter:
			return Alignment::kBottomCenter;
		case CRowColumnView::LayoutStyle::kBottomRight:
			return Alignment::kBottomRight;
		default:
			return Alignment::kTopLeft;
	}
}

//--------------------------------------------------------------------------------
Style translate (const CRowColumnView::Style style)
{
	switch (style)
	{
		case CRowColumnView::Style::kRowStyle:
			return Style::kRow;
		case CRowColumnView::Style::kColumnStyle:
			return Style::kColumn;
		default:
			return Style::kRow;
	}
}

//--------------------------------------------------------------------------------
CPoint computeRectOffset (const CPoint& parent, const CPoint& rect, const Alignment alignment)
{
	CPoint offset (0., 0.);
	switch (alignment)
	{
		case Alignment::kTopLeft:
		{
			offset = CPoint (0., 0.);
			break;
		}
		case Alignment::kTopCenter:
		{
			offset = CPoint ((parent.x - rect.x) / 2., 0.);
			break;
		}
		case Alignment::kTopRight:
		{
			offset = CPoint (parent.x - rect.x, 0.);
			break;
		}
		case Alignment::kMiddleLeft:
		{
			offset = CPoint (0., (parent.y - rect.y) / 2.);
			break;
		}
		case Alignment::kMiddleCenter:
		{
			offset = CPoint ((parent.x - rect.x) / 2., (parent.y - rect.y) / 2.);
			break;
		}
		case Alignment::kMiddleRight:
		{
			offset = CPoint (parent.x - rect.x, (parent.y - rect.y) / 2.);
			break;
		}
		case Alignment::kBottomLeft:
		{
			offset = CPoint (0., parent.y - rect.y);
			break;
		}
		case Alignment::kBottomCenter:
		{
			offset = CPoint ((parent.x - rect.x) / 2., parent.y - rect.y);
			break;
		}
		case Alignment::kBottomRight:
		{
			offset = CPoint (parent.x - rect.x, parent.y - rect.y);
			break;
		}
		default:
			break;
	}

	return offset;
}

//--------------------------------------------------------------------------------
CRect computeGroupRect (const CRect& parent, const CRects& children, const Alignment alignment,
						const Style style, double spacing)
{
	CPoint maxSize;
	if (style == Style::kRow)
	{
		for (const auto& rect : children)
		{
			if (maxSize.x < rect.getWidth ())
				maxSize.x = rect.getWidth ();

			maxSize.y = maxSize.y + rect.getHeight () + spacing;
		}

		// Remove the last spacing again
		if (!children.empty ())
			maxSize.y -= spacing;
	}
	else
	{
		for (const auto& rect : children)
		{
			if (maxSize.y < rect.getHeight ())
				maxSize.y = rect.getHeight ();

			maxSize.x = maxSize.x + rect.getWidth () + spacing;
		}

		// Remove the last spacing again
		if (!children.empty ())
			maxSize.x -= spacing;
	}

	const auto offset = computeRectOffset (parent.getSize (), maxSize, alignment);
	return CRect ().setSize (maxSize).offset (offset);
}

//--------------------------------------------------------------------------------
CRect computeGroupRect (const CViewContainer& parent, const Alignment alignment, const Style style,
						const double spacing)
{
	CRects childrenViewSizes;
	parent.forEachChild (
		[&] (const CView* child) { childrenViewSizes.push_back (child->getViewSize ()); });

	return computeGroupRect (parent.getViewSize (), childrenViewSizes, alignment, style, spacing);
}

//--------------------------------------------------------------------------------
// AutoLayout Declaration
/// @brief An auto layout feature for the CRowColumnView
///
/// The AutoLayout flexibly layouts the children of a CRowColumnView. The children are grouped and
/// aligned among themselves depending on whether the style is a row (left to right) or a
/// column (top to bottom). The group is moved inside the parent to either of the nine
/// positions (top-left, top-center, top-right, middle-left, middle-center, middle-right,
/// bottom-left, bottom-center or bottom-right). If the size of the parent is changed, the layout
/// and alignment of the group is always retained.
//--------------------------------------------------------------------------------
class AutoLayout
{
public:
	//--------------------------------------------------------------------------------
	AutoLayout (const CViewContainer& parent, const Alignment alignment, const Style style,
				const double spacing)
	: alignment (alignment), style (style)
	{
		groupRect = Layouting::computeGroupRect (parent, alignment, style, spacing);
	}

	/** moves the child rect to the calculated position (inside the group and inside the parent) */
	auto moveRect (CRect& viewSize) -> CRect&
	{
		// Offset the viewSize inside the groupRect...
		const CPoint offset =
			Layouting::computeRectOffset (groupRect.getSize (), viewSize.getSize (), alignment);
		if (style == Style::kRow)
			viewSize.offset (offset.x, 0.);
		else
			viewSize.offset (0., offset.y);

		//...and offset by topLeft of the groupRect afterwards, in order to align with the
		// 'real' parent.
		return viewSize.offset (groupRect.getTopLeft ());
	}

	//--------------------------------------------------------------------------------
private:
	const Alignment alignment = Alignment::kTopLeft;
	const Style style = Style::kRow;
	CRect groupRect;
};

//--------------------------------------------------------------------------------

}

//--------------------------------------------------------------------------------
CRowColumnView::CRowColumnView (const CRect& size, Style style, LayoutStyle layoutStyle,
								CCoord spacing, const CRect& margin)
: CAutoLayoutContainerView (size)
, style (style)
, layoutStyle (layoutStyle)
, spacing (spacing)
, margin (margin)
, flags (0)
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
void CRowColumnView::setLayoutStyle (LayoutStyle inLayoutStyle)
{
	if (inLayoutStyle != layoutStyle)
	{
		layoutStyle = inLayoutStyle;
		if (isAttached ())
			layoutViews ();
	}
}

//--------------------------------------------------------------------------------
bool CRowColumnView::isAnimateViewResizing () const { return hasBit (flags, kAnimateViewResizing); }

//--------------------------------------------------------------------------------
void CRowColumnView::setAnimateViewResizing (bool state)
{
	setBit (flags, kAnimateViewResizing, state);
}

//--------------------------------------------------------------------------------
bool CRowColumnView::hideClippedSubviews () const { return hasBit (flags, kHideClippedSubViews); }

//--------------------------------------------------------------------------------
void CRowColumnView::setHideClippedSubviews (bool state)
{
	setBit (flags, kHideClippedSubViews, state);
}

//--------------------------------------------------------------------------------
void CRowColumnView::resizeSubView (CView* view, const CRect& newSize)
{
	if (view->getViewSize () != newSize)
	{
		if (isAttached () && isAnimateViewResizing () && viewResizeAnimationTime > 0)
		{
			view->addAnimation ("CRowColumnResizing",
								new Animation::ViewSizeAnimation (newSize, false),
								new Animation::LinearTimingFunction (viewResizeAnimationTime));
		}
		else
		{
			view->setViewSize (newSize);
			view->setMouseableArea (newSize);
		}
	}
}

//--------------------------------------------------------------------------------
void CRowColumnView::getMaxChildViewSize (CPoint& maxSize)
{
	forEachChild ([&] (CView* view) {
		const CRect& viewSize = view->getViewSize ();
		if (viewSize.getWidth () > maxSize.x)
			maxSize.x = viewSize.getWidth ();
		if (viewSize.getHeight () > maxSize.y)
			maxSize.y = viewSize.getHeight ();
	});
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

	Layouting::AutoLayout layout (*this, Layouting::translate (layoutStyle),
								  Layouting::translate (style), spacing);

	CPoint location = margin.getTopLeft ();
	forEachChild ([&] (CView* view) {
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
			case kTopLeft:
			case kTopCenter:
			case kTopRight:
			case kMiddleLeft:
			case kMiddleCenter:
			case kMiddleRight:
			case kBottomLeft:
			case kBottomCenter:
			case kBottomRight:
			{
				layout.moveRect (viewSize);
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
	});
}

//--------------------------------------------------------------------------------
void CRowColumnView::layoutViews ()
{
	if (layoutGuard == false)
	{
		layoutGuard = true;
		layoutViewsEqualSize ();
		if (hideClippedSubviews ())
		{
			forEachChild ([&] (CView* view) {
				if (view->getVisibleViewSize () != view->getViewSize ())
					view->setVisible (false);
				else
					view->setVisible (true);
			});
		}
		layoutGuard = false;
	}
}

//--------------------------------------------------------------------------------
bool CRowColumnView::sizeToFit ()
{
	if (!getChildren ().empty ())
	{
		CRect viewSize = getViewSize ();
		CPoint maxSize;
		if (style == kRowStyle)
		{
			forEachChild ([&] (CView* view) {
				const CRect& size = view->getViewSize ();
				if (size.getWidth () > maxSize.x)
					maxSize.x = size.getWidth ();
				maxSize.y += size.getHeight () + spacing;
			});
		}
		else
		{
			forEachChild ([&] (CView* view) {
				CRect size = view->getViewSize ();
				maxSize.x += size.getWidth () + spacing;
				if (size.bottom > maxSize.y)
					maxSize.y = size.getHeight ();
			});
		}
		viewSize.setWidth (maxSize.x + margin.left + margin.right);
		viewSize.setHeight (maxSize.y + margin.top + margin.bottom);
		if (viewSize != getViewSize ())
		{
			invalid ();
			auto autosizeState = getAutosizingEnabled ();
			setAutosizingEnabled (false);
			CViewContainer::setViewSize (viewSize);
			CViewContainer::setMouseableArea (viewSize);
			setAutosizingEnabled (autosizeState);
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
CAutoLayoutContainerView::CAutoLayoutContainerView (const CRect& size) : CViewContainer (size) {}

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

} // VSTGUI
