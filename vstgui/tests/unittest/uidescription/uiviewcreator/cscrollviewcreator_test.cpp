// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cscrollbar.h"
#include "../../../../lib/cscrollview.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CScrollViewContainerCreatorTest, ContainerSize)
{
	CPoint size (100, 100);
	testAttribute<CScrollView> (
	    kCScrollView, kAttrContainerSize, size, nullptr,
	    [&] (CScrollView* v) { return v->getContainerSize ().getSize () == size; });
}

TEST_CASE (CScrollViewContainerCreatorTest, HorizontalScrollbar)
{
	testAttribute<CScrollView> (
	    kCScrollView, kAttrHorizontalScrollbar, true, nullptr,
	    [&] (CScrollView* v) { return v->getStyle () & CScrollView::kHorizontalScrollbar; });
}

TEST_CASE (CScrollViewContainerCreatorTest, VerticalScrollbar)
{
	testAttribute<CScrollView> (
	    kCScrollView, kAttrVerticalScrollbar, true, nullptr,
	    [&] (CScrollView* v) { return v->getStyle () & CScrollView::kVerticalScrollbar; });
}

TEST_CASE (CScrollViewContainerCreatorTest, AutoDragScrolling)
{
	testAttribute<CScrollView> (
	    kCScrollView, kAttrAutoDragScrolling, true, nullptr,
	    [&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoDragScrolling; });
}

TEST_CASE (CScrollViewContainerCreatorTest, DontDrawFrame)
{
	DummyUIDescription uiDesc;
	testAttribute<CScrollView> (kCScrollView, kAttrBordered, true, &uiDesc, [&] (CScrollView* v) {
		return v->getStyle () & ~CScrollView::kDontDrawFrame;
	});
	testAttribute<CScrollView> (kCScrollView, kAttrBordered, false, &uiDesc, [&] (CScrollView* v) {
		return v->getStyle () & CScrollView::kDontDrawFrame;
	});
}

TEST_CASE (CScrollViewContainerCreatorTest, OverlayScrollbars)
{
	testAttribute<CScrollView> (
	    kCScrollView, kAttrOverlayScrollbars, true, nullptr,
	    [&] (CScrollView* v) { return v->getStyle () & CScrollView::kOverlayScrollbars; });
}

TEST_CASE (CScrollViewContainerCreatorTest, FollowFocusView)
{
	testAttribute<CScrollView> (
	    kCScrollView, kAttrFollowFocusView, true, nullptr,
	    [&] (CScrollView* v) { return v->getStyle () & CScrollView::kFollowFocusView; });
}

TEST_CASE (CScrollViewContainerCreatorTest, AutoHideScrollbars)
{
	testAttribute<CScrollView> (
	    kCScrollView, kAttrAutoHideScrollbars, true, nullptr,
	    [&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoHideScrollbars; });
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarWidth)
{
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarWidth, 5., nullptr,
	                            [&] (CScrollView* v) { return v->getScrollbarWidth () == 5.; });
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarBackgroundColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarBackgroundColor, kColorName, &uiDesc,
	                            [&] (CScrollView* v) {
		                            auto sb = v->getVerticalScrollbar ();
		                            if (!sb)
			                            sb = v->getHorizontalScrollbar ();
		                            return sb->getBackgroundColor () == uiDesc.color;
	                            });
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarFrameColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarFrameColor, kColorName, &uiDesc,
	                            [&] (CScrollView* v) {
		                            auto sb = v->getVerticalScrollbar ();
		                            if (!sb)
			                            sb = v->getHorizontalScrollbar ();
		                            return sb->getFrameColor () == uiDesc.color;
	                            });
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarScrollerColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarScrollerColor, kColorName, &uiDesc,
	                            [&] (CScrollView* v) {
		                            auto sb = v->getVerticalScrollbar ();
		                            if (!sb)
			                            sb = v->getHorizontalScrollbar ();
		                            return sb->getScrollerColor () == uiDesc.color;
	                            });
}

} // VSTGUI
