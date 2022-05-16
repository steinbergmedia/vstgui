// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/csplitview.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CSplitViewCreatorTest, SeparatorWidth)
{
	testAttribute<CSplitView> (kCSplitView, kAttrSeparatorWidth, 123, nullptr,
	                           [] (CSplitView* v) { return v->getSeparatorWidth () == 123; });
}

TEST_CASE (CSplitViewCreatorTest, Orientation)
{
	DummyUIDescription uidesc;
	testAttribute<CSplitView> (
	    kCSplitView, kAttrOrientation, "horizontal", &uidesc,
	    [&] (CSplitView* v) { return v->getStyle () == CSplitView::kHorizontal; });
	testAttribute<CSplitView> (
	    kCSplitView, kAttrOrientation, "vertical", &uidesc,
	    [&] (CSplitView* v) { return v->getStyle () == CSplitView::kVertical; });
}

TEST_CASE (CSplitViewCreatorTest, ResizeMethod)
{
	DummyUIDescription uidesc;
	testAttribute<CSplitView> (
	    kCSplitView, kAttrResizeMethod, "first", &uidesc,
	    [&] (CSplitView* v) { return v->getResizeMethod () == CSplitView::kResizeFirstView; });
	testAttribute<CSplitView> (
	    kCSplitView, kAttrResizeMethod, "second", &uidesc,
	    [&] (CSplitView* v) { return v->getResizeMethod () == CSplitView::kResizeSecondView; });
	testAttribute<CSplitView> (
	    kCSplitView, kAttrResizeMethod, "last", &uidesc,
	    [&] (CSplitView* v) { return v->getResizeMethod () == CSplitView::kResizeLastView; });
	testAttribute<CSplitView> (kCSplitView, kAttrResizeMethod, "all", &uidesc, [&] (CSplitView* v) {
		return v->getResizeMethod () == CSplitView::kResizeAllViews;
	});
}

TEST_CASE (CSplitViewCreatorTest, OrientationValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCSplitView, kAttrOrientation, &uidesc, {"horizontal", "vertical"});
}

TEST_CASE (CSplitViewCreatorTest, ResizeMethodValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCSplitView, kAttrResizeMethod, &uidesc,
	                    {"first", "second", "last", "all"});
}

} // VSTGUI
