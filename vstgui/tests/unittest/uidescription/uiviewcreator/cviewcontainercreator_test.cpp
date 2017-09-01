// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CViewContainerCreatorTest,

	TEST(backgroundColor,
		DummyUIDescription uidesc;
		testAttribute<CViewContainer>(kCViewContainer, kAttrBackgroundColor, kColorName, &uidesc, [&] (CViewContainer* v) {
			return v->getBackgroundColor () == uidesc.color;
		});
		testAttribute<CViewContainer>(kCViewContainer, kAttrBackgroundColor, kColorName, &uidesc, [&] (CViewContainer* v) {
			return v->getBackgroundColor () == uidesc.color;
		}, true);
	);
	
	TEST(backgroundColorDrawStyle,
		DummyUIDescription uidesc;
		testAttribute<CViewContainer>(kCViewContainer, kAttrBackgroundColorDrawStyle, "stroked", &uidesc, [] (CViewContainer* v) {
			return v->getBackgroundColorDrawStyle () == kDrawStroked;
		});
		testAttribute<CViewContainer>(kCViewContainer, kAttrBackgroundColorDrawStyle, "filled", &uidesc, [] (CViewContainer* v) {
			return v->getBackgroundColorDrawStyle () == kDrawFilled;
		});
		testAttribute<CViewContainer>(kCViewContainer, kAttrBackgroundColorDrawStyle, "filled and stroked", &uidesc, [] (CViewContainer* v) {
			return v->getBackgroundColorDrawStyle () == kDrawFilledAndStroked;
		});
	);

	TEST(backgroundColorDrawStyleValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCViewContainer, kAttrBackgroundColorDrawStyle, &uidesc, {"stroked", "filled", "filled and stroked"});
	);
);

} // VSTGUI