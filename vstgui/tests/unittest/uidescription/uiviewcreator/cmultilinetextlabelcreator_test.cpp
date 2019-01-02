// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/ctextlabel.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CMultiLineTextLabelCreatorTest,

	TEST(autoHeight,
		UIDescriptionAdapter uidesc;
		testAttribute<CMultiLineTextLabel>(kCMultiLineTextLabel, kAttrAutoHeight, true, &uidesc, [] (CMultiLineTextLabel* v) {
			return v->getAutoHeight () == true;
		});
	);

	TEST(lineLayout,
		UIDescriptionAdapter uidesc;
		testAttribute<CMultiLineTextLabel>(kCMultiLineTextLabel, kAttrLineLayout, "truncate", &uidesc, [] (CMultiLineTextLabel* v) {
			return v->getLineLayout() == CMultiLineTextLabel::LineLayout::truncate;
		});
		testAttribute<CMultiLineTextLabel>(kCMultiLineTextLabel, kAttrLineLayout, "wrap", &uidesc, [] (CMultiLineTextLabel* v) {
			return v->getLineLayout() == CMultiLineTextLabel::LineLayout::wrap;
		});
		testAttribute<CMultiLineTextLabel>(kCMultiLineTextLabel, kAttrLineLayout, "clip", &uidesc, [] (CMultiLineTextLabel* v) {
			return v->getLineLayout() == CMultiLineTextLabel::LineLayout::clip;
		});
	);

	TEST(lineLayoutPossibleValues,
		UIDescriptionAdapter uidesc;
		testPossibleValues (kCMultiLineTextLabel, kAttrLineLayout, &uidesc, {"clip", "truncate", "wrap"});
	);
);

} // VSTGUI
