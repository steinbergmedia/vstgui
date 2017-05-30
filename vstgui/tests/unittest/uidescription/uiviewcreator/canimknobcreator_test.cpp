// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/cknob.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CAnimKnobCreatorTest,

	TEST(heightOfOneImage,
		DummyUIDescription uidesc;
		testAttribute<CAnimKnob>(kCAnimKnob, kAttrHeightOfOneImage, 10, &uidesc, [] (CAnimKnob* v) {
			return v->getHeightOfOneImage () == 10;
		});
	);

	TEST(subPixmaps,
		DummyUIDescription uidesc;
		testAttribute<CAnimKnob>(kCAnimKnob, kAttrSubPixmaps, 11, &uidesc, [] (CAnimKnob* v) {
			return v->getNumSubPixmaps () == 11;
		});
	);
);

} // VSTGUI
