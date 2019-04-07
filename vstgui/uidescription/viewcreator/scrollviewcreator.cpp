// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "scrollviewcreator.h"

#include "../../lib/controls/cscrollbar.h"
#include "../../lib/cscrollview.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
ScrollViewCreator::ScrollViewCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr ScrollViewCreator::getViewName () const
{
	return kCScrollView;
}

//------------------------------------------------------------------------
IdStringPtr ScrollViewCreator::getBaseViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
UTF8StringPtr ScrollViewCreator::getDisplayName () const
{
	return "Scroll View";
}

//------------------------------------------------------------------------
CView* ScrollViewCreator::create (const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	return new CScrollView (CRect (0, 0, 100, 100), CRect (0, 0, 200, 200),
	                        CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
}

//------------------------------------------------------------------------
bool ScrollViewCreator::apply (CView* view, const UIAttributes& attributes,
                               const IUIDescription* description) const
{
	auto* scrollView = dynamic_cast<CScrollView*> (view);
	if (scrollView == nullptr)
		return false;

	CPoint p;
	if (attributes.getPointAttribute (kAttrContainerSize, p))
	{
		CRect r;
		r.setSize (p);
		scrollView->setContainerSize (r);
	}

	int32_t style = scrollView->getStyle ();
	applyStyleMask (attributes.getAttributeValue (kAttrHorizontalScrollbar),
	                CScrollView::kHorizontalScrollbar, style);
	applyStyleMask (attributes.getAttributeValue (kAttrVerticalScrollbar),
	                CScrollView::kVerticalScrollbar, style);
	applyStyleMask (attributes.getAttributeValue (kAttrAutoDragScrolling),
	                CScrollView::kAutoDragScrolling, style);
	const auto* attr = attributes.getAttributeValue (kAttrBordered);
	if (attr)
	{
		setBit (style, CScrollView::kDontDrawFrame, *attr != strTrue);
	}
	applyStyleMask (attributes.getAttributeValue (kAttrOverlayScrollbars),
	                CScrollView::kOverlayScrollbars, style);
	applyStyleMask (attributes.getAttributeValue (kAttrFollowFocusView),
	                CScrollView::kFollowFocusView, style);
	applyStyleMask (attributes.getAttributeValue (kAttrAutoHideScrollbars),
	                CScrollView::kAutoHideScrollbars, style);
	scrollView->setStyle (style);
	CColor color;
	CScrollbar* vscrollbar = scrollView->getVerticalScrollbar ();
	CScrollbar* hscrollbar = scrollView->getHorizontalScrollbar ();
	if (stringToColor (attributes.getAttributeValue (kAttrScrollbarBackgroundColor), color,
	                   description))
	{
		if (vscrollbar)
			vscrollbar->setBackgroundColor (color);
		if (hscrollbar)
			hscrollbar->setBackgroundColor (color);
	}
	if (stringToColor (attributes.getAttributeValue (kAttrScrollbarFrameColor), color, description))
	{
		if (vscrollbar)
			vscrollbar->setFrameColor (color);
		if (hscrollbar)
			hscrollbar->setFrameColor (color);
	}
	if (stringToColor (attributes.getAttributeValue (kAttrScrollbarScrollerColor), color,
	                   description))
	{
		if (vscrollbar)
			vscrollbar->setScrollerColor (color);
		if (hscrollbar)
			hscrollbar->setScrollerColor (color);
	}
	CCoord width;
	if (attributes.getDoubleAttribute (kAttrScrollbarWidth, width))
		scrollView->setScrollbarWidth (width);
	return true;
}

//------------------------------------------------------------------------
bool ScrollViewCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrContainerSize);
	attributeNames.emplace_back (kAttrScrollbarBackgroundColor);
	attributeNames.emplace_back (kAttrScrollbarFrameColor);
	attributeNames.emplace_back (kAttrScrollbarScrollerColor);
	attributeNames.emplace_back (kAttrHorizontalScrollbar);
	attributeNames.emplace_back (kAttrVerticalScrollbar);
	attributeNames.emplace_back (kAttrAutoHideScrollbars);
	attributeNames.emplace_back (kAttrAutoDragScrolling);
	attributeNames.emplace_back (kAttrOverlayScrollbars);
	attributeNames.emplace_back (kAttrScrollbarWidth);
	attributeNames.emplace_back (kAttrBordered);
	attributeNames.emplace_back (kAttrFollowFocusView);
	return true;
}

//------------------------------------------------------------------------
auto ScrollViewCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrContainerSize)
		return kPointType;
	if (attributeName == kAttrScrollbarBackgroundColor)
		return kColorType;
	if (attributeName == kAttrScrollbarFrameColor)
		return kColorType;
	if (attributeName == kAttrScrollbarScrollerColor)
		return kColorType;
	if (attributeName == kAttrHorizontalScrollbar)
		return kBooleanType;
	if (attributeName == kAttrVerticalScrollbar)
		return kBooleanType;
	if (attributeName == kAttrAutoHideScrollbars)
		return kBooleanType;
	if (attributeName == kAttrAutoDragScrolling)
		return kBooleanType;
	if (attributeName == kAttrOverlayScrollbars)
		return kBooleanType;
	if (attributeName == kAttrScrollbarWidth)
		return kIntegerType;
	if (attributeName == kAttrBordered)
		return kBooleanType;
	if (attributeName == kAttrFollowFocusView)
		return kBooleanType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool ScrollViewCreator::getAttributeValue (CView* view, const string& attributeName,
                                           string& stringValue, const IUIDescription* desc) const
{
	auto* sc = dynamic_cast<CScrollView*> (view);
	if (sc == nullptr)
		return false;
	if (attributeName == kAttrContainerSize)
	{
		stringValue = UIAttributes::pointToString (sc->getContainerSize ().getSize ());
		return true;
	}
	if (attributeName == kAttrScrollbarWidth)
	{
		stringValue = UIAttributes::doubleToString (sc->getScrollbarWidth ());
		return true;
	}
	CScrollbar* scrollbar = sc->getVerticalScrollbar ();
	if (!scrollbar)
		scrollbar = sc->getHorizontalScrollbar ();
	if (scrollbar)
	{
		if (attributeName == kAttrScrollbarBackgroundColor)
		{
			colorToString (scrollbar->getBackgroundColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrScrollbarFrameColor)
		{
			colorToString (scrollbar->getFrameColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrScrollbarScrollerColor)
		{
			colorToString (scrollbar->getScrollerColor (), stringValue, desc);
			return true;
		}
	}
	if (attributeName == kAttrHorizontalScrollbar)
	{
		stringValue = sc->getStyle () & CScrollView::kHorizontalScrollbar ? strTrue : strFalse;
		return true;
	}
	if (attributeName == kAttrVerticalScrollbar)
	{
		stringValue = sc->getStyle () & CScrollView::kVerticalScrollbar ? strTrue : strFalse;
		return true;
	}
	if (attributeName == kAttrAutoHideScrollbars)
	{
		stringValue = sc->getStyle () & CScrollView::kAutoHideScrollbars ? strTrue : strFalse;
		return true;
	}
	if (attributeName == kAttrAutoDragScrolling)
	{
		stringValue = sc->getStyle () & CScrollView::kAutoDragScrolling ? strTrue : strFalse;
		return true;
	}
	if (attributeName == kAttrBordered)
	{
		stringValue = sc->getStyle () & CScrollView::kDontDrawFrame ? strFalse : strTrue;
		return true;
	}
	if (attributeName == kAttrOverlayScrollbars)
	{
		stringValue = sc->getStyle () & CScrollView::kOverlayScrollbars ? strTrue : strFalse;
		return true;
	}
	if (attributeName == kAttrFollowFocusView)
	{
		stringValue = sc->getStyle () & CScrollView::kFollowFocusView ? strTrue : strFalse;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
