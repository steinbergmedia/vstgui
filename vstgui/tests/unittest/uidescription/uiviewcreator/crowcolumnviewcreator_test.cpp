// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/crowcolumnview.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CRowColumnViewCreatorTest,

	TEST(rowStyle,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrRowStyle, true, nullptr, [] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kRowStyle; });
	);

	TEST(columnStyle,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrRowStyle, false, nullptr, [] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kColumnStyle; });
	);

	TEST(spacing,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrSpacing, 5., nullptr, [] (CRowColumnView* v) { return v->getSpacing () == 5.; });
	);

	TEST(margin,
		CRect margin (5,6,7,8);
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrMargin, margin, nullptr, [&] (CRowColumnView* v) { return v->getMargin () == margin; });
	);

	TEST(animateViewResizing,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrAnimateViewResizing, true, nullptr, [] (CRowColumnView* v) { return v->isAnimateViewResizing (); });
	);

	TEST(equalSizeLayoutStretch,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "stretch", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kStretchEqualy; });
	);

	TEST(equalSizeLayoutCenter,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "center", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kCenterEqualy; });
	);

	TEST(equalSizeLayoutRightBottom,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "right-bottom", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kRightBottomEqualy; });
	);

	TEST(equalSizeLayoutLeftTop,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "left-top", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kLeftTopEqualy; });
	);

	TEST(animationTime,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrViewResizeAnimationTime, 100, nullptr, [] (CRowColumnView* v) { return v->getViewResizeAnimationTime() == 100; });
	);

	TEST(equalSizeLayoutValues,
		testPossibleValues(kCRowColumnView, kAttrEqualSizeLayout, nullptr, {"left-top", "stretch", "center", "right-bottom"});
	);
);

} // VSTGUI
