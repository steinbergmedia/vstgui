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
	DummyUIDescription uidesc;
	CPoint size (100, 100);
	testAttribute<CScrollView> (
		kCScrollView, kAttrContainerSize, size, uidesc,
		[&] (CScrollView* v) { return v->getContainerSize ().getSize () == size; });
}

TEST_CASE (CScrollViewContainerCreatorTest, HorizontalScrollbar)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (
		kCScrollView, kAttrHorizontalScrollbar, true, uidesc,
		[&] (CScrollView* v) { return v->getStyle () & CScrollView::kHorizontalScrollbar; });
}

TEST_CASE (CScrollViewContainerCreatorTest, VerticalScrollbar)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (
		kCScrollView, kAttrVerticalScrollbar, true, uidesc,
		[&] (CScrollView* v) { return v->getStyle () & CScrollView::kVerticalScrollbar; });
}

TEST_CASE (CScrollViewContainerCreatorTest, AutoDragScrolling)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (
		kCScrollView, kAttrAutoDragScrolling, true, uidesc,
		[&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoDragScrolling; });
}

TEST_CASE (CScrollViewContainerCreatorTest, DontDrawFrame)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (kCScrollView, kAttrBordered, true, uidesc, [&] (CScrollView* v) {
		return v->getStyle () & ~CScrollView::kDontDrawFrame;
	});
	testAttribute<CScrollView> (kCScrollView, kAttrBordered, false, uidesc, [&] (CScrollView* v) {
		return v->getStyle () & CScrollView::kDontDrawFrame;
	});
}

TEST_CASE (CScrollViewContainerCreatorTest, OverlayScrollbars)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (
		kCScrollView, kAttrOverlayScrollbars, true, uidesc,
		[&] (CScrollView* v) { return v->getStyle () & CScrollView::kOverlayScrollbars; });
}

TEST_CASE (CScrollViewContainerCreatorTest, FollowFocusView)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (
		kCScrollView, kAttrFollowFocusView, true, uidesc,
		[&] (CScrollView* v) { return v->getStyle () & CScrollView::kFollowFocusView; });
}

TEST_CASE (CScrollViewContainerCreatorTest, AutoHideScrollbars)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (
		kCScrollView, kAttrAutoHideScrollbars, true, uidesc,
		[&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoHideScrollbars; });
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarWidth, 5., uidesc,
								[&] (CScrollView* v) { return v->getScrollbarWidth () == 5.; });
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarBackgroundColor)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarBackgroundColor, kColorName, uidesc,
								[&] (CScrollView* v) {
									auto sb = v->getVerticalScrollbar ();
									if (!sb)
			                            sb = v->getHorizontalScrollbar ();
									return sb->getBackgroundColor () == uidesc.color;
								});
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarFrameColor)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarFrameColor, kColorName, uidesc,
								[&] (CScrollView* v) {
									auto sb = v->getVerticalScrollbar ();
									if (!sb)
			                            sb = v->getHorizontalScrollbar ();
									return sb->getFrameColor () == uidesc.color;
								});
}

TEST_CASE (CScrollViewContainerCreatorTest, ScrollbarScrollerColor)
{
	DummyUIDescription uidesc;
	testAttribute<CScrollView> (kCScrollView, kAttrScrollbarScrollerColor, kColorName, uidesc,
								[&] (CScrollView* v) {
									auto sb = v->getVerticalScrollbar ();
									if (!sb)
			                            sb = v->getHorizontalScrollbar ();
									return sb->getScrollerColor () == uidesc.color;
								});
}

} // VSTGUI
