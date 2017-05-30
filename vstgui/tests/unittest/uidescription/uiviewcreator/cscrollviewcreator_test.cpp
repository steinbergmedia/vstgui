// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/cscrollview.h"
#include "../../../../lib/controls/cscrollbar.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CScrollViewContainerCreatorTest,

	TEST(containerSize,
		CPoint size (100, 100);
		testAttribute<CScrollView> (kCScrollView, kAttrContainerSize, size, nullptr, [&] (CScrollView* v) { return v->getContainerSize ().getSize () == size; });
	);

	TEST(horizontalScrollbar,
		testAttribute<CScrollView> (kCScrollView, kAttrHorizontalScrollbar, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kHorizontalScrollbar; });
	);

	TEST(verticalScrollbar,
		testAttribute<CScrollView> (kCScrollView, kAttrVerticalScrollbar, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kVerticalScrollbar; });
	);

	TEST(autoDragScrolling,
		testAttribute<CScrollView> (kCScrollView, kAttrAutoDragScrolling, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoDragScrolling; });
	);

	TEST(dontDrawFrame,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrBordered, true, &uiDesc, [&] (CScrollView* v) { return v->getStyle () & ~CScrollView::kDontDrawFrame; });
		testAttribute<CScrollView> (kCScrollView, kAttrBordered, false, &uiDesc, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kDontDrawFrame; });
	);

	TEST(overlayScrollbars,
		testAttribute<CScrollView> (kCScrollView, kAttrOverlayScrollbars, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kOverlayScrollbars; });
	);

	TEST(followFocusView,
		testAttribute<CScrollView> (kCScrollView, kAttrFollowFocusView, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kFollowFocusView; });
	);

	TEST(autoHideScrollbars,
		testAttribute<CScrollView> (kCScrollView, kAttrAutoHideScrollbars, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoHideScrollbars; });
	);

	TEST(scrollbarWidth,
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarWidth, 5., nullptr, [&] (CScrollView* v) { return v->getScrollbarWidth () == 5.; });
	);

	TEST(scrollbarBackgroundColor,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarBackgroundColor, kColorName, &uiDesc, [&] (CScrollView* v) {
			auto sb = v->getVerticalScrollbar ();
			if (!sb)
				sb = v->getHorizontalScrollbar ();
			return sb->getBackgroundColor () == uiDesc.color;
		});
	);

	TEST(scrollbarFrameColor,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarFrameColor, kColorName, &uiDesc, [&] (CScrollView* v) {
			auto sb = v->getVerticalScrollbar ();
			if (!sb)
				sb = v->getHorizontalScrollbar ();
			return sb->getFrameColor () == uiDesc.color;
		});
	);

	TEST(scrollbarScrollerColor,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarScrollerColor, kColorName, &uiDesc, [&] (CScrollView* v) {
			auto sb = v->getVerticalScrollbar ();
			if (!sb)
				sb = v->getHorizontalScrollbar ();
			return sb->getScrollerColor () == uiDesc.color;
		});
	);
);

} // VSTGUI
