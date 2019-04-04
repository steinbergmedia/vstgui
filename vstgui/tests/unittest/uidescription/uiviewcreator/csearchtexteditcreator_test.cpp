// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/csearchtextedit.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CSearchTextEditCreatorTest,

	TEST(clearMarkInset,
		UIDescriptionAdapter uidesc;
		CPoint p (10, 11);
		testAttribute<CSearchTextEdit>(kCSearchTextEdit, kAttrClearMarkInset, p, &uidesc, [&] (CSearchTextEdit* v) {
			return v->getClearMarkInset () == p;
		});
	);

);

} // VSTGUI
