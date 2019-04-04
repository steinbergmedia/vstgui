// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/ctextedit.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CTextEditCreatorTest,

	TEST(immediateTextChange,
		UIDescriptionAdapter uidesc;
		testAttribute<CTextEdit>(kCTextEdit, kAttrImmediateTextChange, true, &uidesc, [] (CTextEdit* v) {
			return v->getImmediateTextChange ();
		});
	);

	TEST(doubleClick,
		UIDescriptionAdapter uidesc;
		testAttribute<CTextEdit>(kCTextEdit, kAttrStyleDoubleClick, true, &uidesc, [] (CTextEdit* v) {
			return v->getStyle () & CTextEdit::kDoubleClickStyle;
		});
	);

	TEST(autoSizeToFit,
		DummyUIDescription uidesc;
		testAttribute<CTextEdit> (kCTextEdit, kAttrSecureStyle, true, &uidesc, [&] (CTextEdit* v) {
			return v->getSecureStyle () == true;
		});
	);

	TEST(autoSizeToFit,
		DummyUIDescription uidesc;
		auto testValue = "This is a placeholder";
		testAttribute<CTextEdit> (kCTextEdit, kAttrPlaceholderTitle, testValue, &uidesc, [&] (CTextEdit* b) {
			return b->getPlaceholderString () == testValue;
		});
	);

);

} // VSTGUI
