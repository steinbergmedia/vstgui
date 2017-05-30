// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(COnOffButtonCreatorTest,

	TEST(title,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrTitle, "title", &uidesc, [] (CCheckBox* b) {
			return b->getTitle () == "title";
		});
	);

	TEST(font,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrFont, kFontName, &uidesc, [&] (CCheckBox* b) {
			return uidesc.font == b->getFont ();
		}, true);
	);

	TEST(fontColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrFontColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getFontColor () == uidesc.color;
		});
	);

	TEST(boxFrameColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrBoxframeColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getBoxFrameColor () == uidesc.color;
		});
	);

	TEST(boxFillColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrBoxfillColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getBoxFillColor () == uidesc.color;
		});
	);

	TEST(checkmarkColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrCheckmarkColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getCheckMarkColor () == uidesc.color;
		});
	);

	TEST(drawCrossbox,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrDrawCrossbox, true, &uidesc, [&] (CCheckBox* b) {
			return b->getStyle () & CCheckBox::kDrawCrossBox;
		});
	);

	TEST(autoSizeToFit,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrAutosizeToFit, true, &uidesc, [&] (CCheckBox* b) {
			return b->getStyle () & CCheckBox::kAutoSizeToFit;
		});
	);
);

} // VSTGUI
