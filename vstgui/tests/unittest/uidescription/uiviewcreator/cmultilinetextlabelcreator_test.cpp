// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/ctextlabel.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CMultiLineTextLabelCreatorTest, AutoHeight)
{
	UIDescriptionAdapter uidesc;
	testAttribute<CMultiLineTextLabel> (
	    kCMultiLineTextLabel, kAttrAutoHeight, true, &uidesc,
	    [] (CMultiLineTextLabel* v) { return v->getAutoHeight () == true; });
}

TEST_CASE (CMultiLineTextLabelCreatorTest, LineLayout)
{
	UIDescriptionAdapter uidesc;
	testAttribute<CMultiLineTextLabel> (
	    kCMultiLineTextLabel, kAttrLineLayout, "truncate", &uidesc, [] (CMultiLineTextLabel* v) {
		    return v->getLineLayout () == CMultiLineTextLabel::LineLayout::truncate;
	    });
	testAttribute<CMultiLineTextLabel> (
	    kCMultiLineTextLabel, kAttrLineLayout, "wrap", &uidesc, [] (CMultiLineTextLabel* v) {
		    return v->getLineLayout () == CMultiLineTextLabel::LineLayout::wrap;
	    });
	testAttribute<CMultiLineTextLabel> (
	    kCMultiLineTextLabel, kAttrLineLayout, "clip", &uidesc, [] (CMultiLineTextLabel* v) {
		    return v->getLineLayout () == CMultiLineTextLabel::LineLayout::clip;
	    });
}

TEST_CASE (CMultiLineTextLabelCreatorTest, LineLayoutPossibleValues)
{
	UIDescriptionAdapter uidesc;
	testPossibleValues (kCMultiLineTextLabel, kAttrLineLayout, &uidesc,
	                    {"clip", "truncate", "wrap"});
}

} // VSTGUI
