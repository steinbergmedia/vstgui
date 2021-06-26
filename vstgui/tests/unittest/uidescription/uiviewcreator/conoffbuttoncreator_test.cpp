// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (COnOffButtonCreatorTest, Create)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (kAttrClass, kCOnOffButton);

	auto view = owned (factory.createView (a, nullptr));
	auto control = view.cast<COnOffButton> ();
	EXPECT (control);
	UIAttributes a2;
	EXPECT (factory.getAttributesForView (view, nullptr, a2));
}

} // VSTGUI
