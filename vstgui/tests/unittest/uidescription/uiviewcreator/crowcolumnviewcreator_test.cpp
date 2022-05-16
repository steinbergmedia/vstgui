// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/crowcolumnview.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CRowColumnViewCreatorTest, RowStyle)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrRowStyle, true, nullptr,
	    [] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kRowStyle; });
}

TEST_CASE (CRowColumnViewCreatorTest, ColumnStyle)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrRowStyle, false, nullptr,
	    [] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kColumnStyle; });
}

TEST_CASE (CRowColumnViewCreatorTest, Spacing)
{
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrSpacing, 5., nullptr,
	                               [] (CRowColumnView* v) { return v->getSpacing () == 5.; });
}

TEST_CASE (CRowColumnViewCreatorTest, Margin)
{
	CRect margin (5, 6, 7, 8);
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrMargin, margin, nullptr,
	                               [&] (CRowColumnView* v) { return v->getMargin () == margin; });
}

TEST_CASE (CRowColumnViewCreatorTest, AnimateViewResizing)
{
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrAnimateViewResizing, true, nullptr,
	                               [] (CRowColumnView* v) { return v->isAnimateViewResizing (); });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutStretch)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrEqualSizeLayout, "stretch", nullptr,
	    [] (CRowColumnView* v) { return v->getLayoutStyle () == CRowColumnView::kStretchEqualy; });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutCenter)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrEqualSizeLayout, "center", nullptr,
	    [] (CRowColumnView* v) { return v->getLayoutStyle () == CRowColumnView::kCenterEqualy; });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutRightBottom)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrEqualSizeLayout, "right-bottom", nullptr, [] (CRowColumnView* v) {
		    return v->getLayoutStyle () == CRowColumnView::kRightBottomEqualy;
	    });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutLeftTop)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrEqualSizeLayout, "left-top", nullptr,
	    [] (CRowColumnView* v) { return v->getLayoutStyle () == CRowColumnView::kLeftTopEqualy; });
}

TEST_CASE (CRowColumnViewCreatorTest, AnimationTime)
{
	testAttribute<CRowColumnView> (
	    kCRowColumnView, kAttrViewResizeAnimationTime, 100, nullptr,
	    [] (CRowColumnView* v) { return v->getViewResizeAnimationTime () == 100; });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutValues)
{
	testPossibleValues (kCRowColumnView, kAttrEqualSizeLayout, nullptr,
	                    {"left-top", "stretch", "center", "right-bottom"});
}

TEST_CASE (CRowColumnViewCreatorTest, HideClippedSubviews)
{
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrHideClippedSubviews, true, nullptr,
	                               [] (CRowColumnView* v) { return v->hideClippedSubviews (); });
}

} // VSTGUI
