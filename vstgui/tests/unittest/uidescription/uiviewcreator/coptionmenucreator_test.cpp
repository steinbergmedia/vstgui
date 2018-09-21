// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/coptionmenu.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(COptionMenuCreatorTest,

	TEST(popupStyle,
		UIDescriptionAdapter uidesc;
		testAttribute<COptionMenu>(kCOptionMenu, kAttrMenuPopupStyle, true, &uidesc, [] (COptionMenu* v) {
			return v->getStyle () & COptionMenu::kPopupStyle;
		});
	);

	TEST(checkStyle,
		UIDescriptionAdapter uidesc;
		testAttribute<COptionMenu>(kCOptionMenu, kAttrMenuCheckStyle, true, &uidesc, [] (COptionMenu* v) {
			return v->getStyle () & COptionMenu::kCheckStyle;
		});
	);
);

} // VSTGUI
