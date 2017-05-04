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

TESTCASE(CTextLabelCreatorTest,

	TEST(title,
		UIDescriptionAdapter uidesc;
		testAttribute<CTextLabel>(kCTextLabel, kAttrTitle, "Title", &uidesc, [] (CTextLabel* v) {
			return v->getText() == "Title";
		});
	);

	TEST(truncateMode,
		UIDescriptionAdapter uidesc;
		testAttribute<CTextLabel>(kCTextLabel, kAttrTruncateMode, "head", &uidesc, [] (CTextLabel* v) {
			return v->getTextTruncateMode() == CTextLabel::kTruncateHead;
		});
		testAttribute<CTextLabel>(kCTextLabel, kAttrTruncateMode, "tail", &uidesc, [] (CTextLabel* v) {
			return v->getTextTruncateMode() == CTextLabel::kTruncateTail;
		});
		testAttribute<CTextLabel>(kCTextLabel, kAttrTruncateMode, "", &uidesc, [] (CTextLabel* v) {
			return v->getTextTruncateMode() == CTextLabel::kTruncateNone;
		});
		testPossibleValues (kCTextLabel, kAttrTruncateMode, &uidesc, {"head", "tail", "none"});
	);
);

} // VSTGUI
