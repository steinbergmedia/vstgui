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
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrRowStyle, true, uidesc,
		[] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kRowStyle; });
}

TEST_CASE (CRowColumnViewCreatorTest, ColumnStyle)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrRowStyle, false, uidesc,
		[] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kColumnStyle; });
}

TEST_CASE (CRowColumnViewCreatorTest, Spacing)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrSpacing, 5., uidesc,
								   [] (CRowColumnView* v) { return v->getSpacing () == 5.; });
}

TEST_CASE (CRowColumnViewCreatorTest, Margin)
{
	DummyUIDescription uidesc;
	CRect margin (5, 6, 7, 8);
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrMargin, margin, uidesc,
								   [&] (CRowColumnView* v) { return v->getMargin () == margin; });
}

TEST_CASE (CRowColumnViewCreatorTest, AnimateViewResizing)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrAnimateViewResizing, true, uidesc,
								   [] (CRowColumnView* v) { return v->isAnimateViewResizing (); });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutStretch)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrEqualSizeLayout, "stretch", uidesc,
		[] (CRowColumnView* v) { return v->getLayoutStyle () == CRowColumnView::kStretchEqualy; });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutCenter)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrEqualSizeLayout, "center", uidesc,
		[] (CRowColumnView* v) { return v->getLayoutStyle () == CRowColumnView::kCenterEqualy; });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutRightBottom)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrEqualSizeLayout, "right-bottom", uidesc, [] (CRowColumnView* v) {
			return v->getLayoutStyle () == CRowColumnView::kRightBottomEqualy;
		});
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutLeftTop)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrEqualSizeLayout, "left-top", uidesc,
		[] (CRowColumnView* v) { return v->getLayoutStyle () == CRowColumnView::kLeftTopEqualy; });
}

TEST_CASE (CRowColumnViewCreatorTest, AnimationTime)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (
		kCRowColumnView, kAttrViewResizeAnimationTime, 100, uidesc,
		[] (CRowColumnView* v) { return v->getViewResizeAnimationTime () == 100; });
}

TEST_CASE (CRowColumnViewCreatorTest, EqualSizeLayoutValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCRowColumnView, kAttrEqualSizeLayout, uidesc,
						{"left-top", "stretch", "center", "right-bottom", "top-left", "top-center",
						 "top-right", "middle-left", "middle-center", "middle-right", "bottom-left",
						 "bottom-center", "bottom-right"});
}

TEST_CASE (CRowColumnViewCreatorTest, HideClippedSubviews)
{
	DummyUIDescription uidesc;
	testAttribute<CRowColumnView> (kCRowColumnView, kAttrHideClippedSubviews, true, uidesc,
								   [] (CRowColumnView* v) { return v->hideClippedSubviews (); });
}

} // VSTGUI
