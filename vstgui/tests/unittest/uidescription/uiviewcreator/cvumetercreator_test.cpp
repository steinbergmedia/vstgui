// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/cvumeter.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CVuMeterCreatorTest,

	TEST(offBitmap,
		DummyUIDescription uidesc;
		testAttribute<CVuMeter>(kCVuMeter, kAttrOffBitmap, kBitmapName, &uidesc, [&] (CVuMeter* v) {
			return v->getOffBitmap() == uidesc.bitmap;
		});
	);

	TEST(orientation,
		DummyUIDescription uidesc;
		testAttribute<CVuMeter>(kCVuMeter, kAttrOrientation, "horizontal", &uidesc, [&] (CVuMeter* v) {
			return v->getStyle() == CVuMeter::kHorizontal;
		});
		testAttribute<CVuMeter>(kCVuMeter, kAttrOrientation, "vertical", &uidesc, [&] (CVuMeter* v) {
			return v->getStyle() == CVuMeter::kVertical;
		});
	);

	TEST(numLed,
		DummyUIDescription uidesc;
		testAttribute<CVuMeter>(kCVuMeter, kAttrNumLed, 5, &uidesc, [&] (CVuMeter* v) {
			return v->getNbLed () == 5;
		});
	);

	TEST(decreaseStepValue,
		DummyUIDescription uidesc;
		testAttribute<CVuMeter>(kCVuMeter, kAttrDecreaseStepValue, 15., &uidesc, [&] (CVuMeter* v) {
			return v->getDecreaseStepValue () == 15.;
		});
	);

	TEST(orientationValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCVuMeter, kAttrOrientation, &uidesc, {"horizontal", "vertical"});
	);

);

} // VSTGUI
