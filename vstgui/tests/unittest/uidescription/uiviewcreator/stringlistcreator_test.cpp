// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/cstringlist.h"
#include "../../../../lib/cstring.h"
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
TESTCASE(StringListControlCreatorTest,

	TEST(styleHover,
		DummyUIDescription uidesc;
		testAttribute<CListControl>(kCStringListControl, kAttrStyleHover, true, &uidesc, [&] (CListControl* v) {
			return (getConfigurator (v)->getFlags () & CListControlRowDesc::Hoverable) != 0;
		});
		testAttribute<CListControl>(kCStringListControl, kAttrStyleHover, false, &uidesc, [&] (CListControl* v) {
			return (getConfigurator (v)->getFlags () & CListControlRowDesc::Hoverable) == 0;
		});
	);

	TEST(rowHeight,
		DummyUIDescription uidesc;
		testAttribute<CListControl>(kCStringListControl, kAttrRowHeight, 14., &uidesc, [&] (CListControl* v) {
			return getConfigurator (v)->getRowHeight () == 14.;
		});
	);

	TEST(font,
		DummyUIDescription uidesc;
		testAttribute<CListControl>(kCStringListControl, kAttrFont, kFontName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getFont () == uidesc.font;
		}, true);
	);

	TEST(textAlignment,
		DummyUIDescription uidesc;
		testAttribute<CListControl>(kCStringListControl, kAttrTextAlignment, strRight, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getTextAlign () == kRightText;
		});
		testAttribute<CListControl>(kCStringListControl, kAttrTextAlignment, strLeft, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getTextAlign () == kLeftText;
		});
		testAttribute<CListControl>(kCStringListControl, kAttrTextAlignment, strCenter, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getTextAlign () == kCenterText;
		});
	);

	TEST(fontColor,
		DummyUIDescription uidesc;
		testAttribute<CListControl> (kCStringListControl, kAttrFontColor, kColorName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getFontColor () == uidesc.color;
		});
	);

	TEST(selectedFontColor,
		DummyUIDescription uidesc;
		testAttribute<CListControl> (kCStringListControl, kAttrSelectedFontColor, kColorName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getSelectedFontColor () == uidesc.color;
		});
	);

	TEST(backColor,
		DummyUIDescription uidesc;
		testAttribute<CListControl> (kCStringListControl, kAttrBackColor, kColorName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getBackColor () == uidesc.color;
		});
	);

	TEST(selectedBackColor,
		DummyUIDescription uidesc;
		testAttribute<CListControl> (kCStringListControl, kAttrSelectedBackColor, kColorName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getSelectedBackColor () == uidesc.color;
		});
	);

	TEST(hoverColor,
		DummyUIDescription uidesc;
		testAttribute<CListControl> (kCStringListControl, kAttrHoverColor, kColorName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getHoverColor () == uidesc.color;
		});
	);

	TEST(lineColor,
		DummyUIDescription uidesc;
		testAttribute<CListControl> (kCStringListControl, kAttrLineColor, kColorName, &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getLineColor () == uidesc.color;
		});
	);

	TEST(lineWidth,
		DummyUIDescription uidesc;
		testAttribute<CListControl>(kCStringListControl, kAttrLineWidth, 14., &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getLineWidth () == 14.;
		});
	);

	TEST(lineWidth,
		DummyUIDescription uidesc;
		testAttribute<CListControl>(kCStringListControl, kAttrTextInset, 14., &uidesc, [&] (CListControl* v) {
			return getDrawer (v)->getTextInset () == 14.;
		});
	);

);

} // VSTGUI
