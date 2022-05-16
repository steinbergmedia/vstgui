// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cstringlist.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

//------------------------------------------------------------------------
static StringListControlDrawer* getDrawer (CListControl* c)
{
	return dynamic_cast<StringListControlDrawer*> (c->getDrawer ());
}

//------------------------------------------------------------------------
static StaticListControlConfigurator* getConfigurator (CListControl* c)
{
	return dynamic_cast<StaticListControlConfigurator*> (c->getConfigurator ());
}

//------------------------------------------------------------------------

TEST_CASE (StringListControlCreatorTest, StyleHover)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrStyleHover, true, &uidesc, [&] (CListControl* v) {
		    return (getConfigurator (v)->getFlags () & CListControlRowDesc::Hoverable) != 0;
	    });
	testAttribute<CListControl> (
	    kCStringListControl, kAttrStyleHover, false, &uidesc, [&] (CListControl* v) {
		    return (getConfigurator (v)->getFlags () & CListControlRowDesc::Hoverable) == 0;
	    });
}

TEST_CASE (StringListControlCreatorTest, RowHeight)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrRowHeight, 14., &uidesc,
	    [&] (CListControl* v) { return getConfigurator (v)->getRowHeight () == 14.; });
}

TEST_CASE (StringListControlCreatorTest, Font)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrFont, kFontName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getFont () == uidesc.font; }, true);
}

TEST_CASE (StringListControlCreatorTest, TextAlignment)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrTextAlignment, strRight, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getTextAlign () == kRightText; });
	testAttribute<CListControl> (
	    kCStringListControl, kAttrTextAlignment, strLeft, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getTextAlign () == kLeftText; });
	testAttribute<CListControl> (
	    kCStringListControl, kAttrTextAlignment, strCenter, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getTextAlign () == kCenterText; });
}

TEST_CASE (StringListControlCreatorTest, FontColor)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrFontColor, kColorName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getFontColor () == uidesc.color; });
}

TEST_CASE (StringListControlCreatorTest, SelectedFontColor)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrSelectedFontColor, kColorName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getSelectedFontColor () == uidesc.color; });
}

TEST_CASE (StringListControlCreatorTest, BackColor)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrBackColor, kColorName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getBackColor () == uidesc.color; });
}

TEST_CASE (StringListControlCreatorTest, SelectedBackColor)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrSelectedBackColor, kColorName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getSelectedBackColor () == uidesc.color; });
}

TEST_CASE (StringListControlCreatorTest, HoverColor)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrHoverColor, kColorName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getHoverColor () == uidesc.color; });
}

TEST_CASE (StringListControlCreatorTest, LineColor)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrLineColor, kColorName, &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getLineColor () == uidesc.color; });
}

TEST_CASE (StringListControlCreatorTest, LineWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrLineWidth, 14., &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getLineWidth () == 14.; });
}

TEST_CASE (StringListControlCreatorTest, TextInset)
{
	DummyUIDescription uidesc;
	testAttribute<CListControl> (
	    kCStringListControl, kAttrTextInset, 14., &uidesc,
	    [&] (CListControl* v) { return getDrawer (v)->getTextInset () == 14.; });
}

} // VSTGUI
