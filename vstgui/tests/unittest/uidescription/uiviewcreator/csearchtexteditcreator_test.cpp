// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/csearchtextedit.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CSearchTextEditCreatorTest, ClearMarkInset)
{
	UIDescriptionAdapter uidesc;
	CPoint p (10, 11);
	testAttribute<CSearchTextEdit> (
	    kCSearchTextEdit, kAttrClearMarkInset, p, &uidesc,
	    [&] (CSearchTextEdit* v) { return v->getClearMarkInset () == p; });
}

} // VSTGUI
