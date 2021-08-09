// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cvumeter.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CVuMeterCreatorTest, OffBitmap)
{
	DummyUIDescription uidesc;
	testAttribute<CVuMeter> (kCVuMeter, kAttrOffBitmap, kBitmapName, &uidesc,
	                         [&] (CVuMeter* v) { return v->getOffBitmap () == uidesc.bitmap; });
}

TEST_CASE (CVuMeterCreatorTest, Orientation)
{
	DummyUIDescription uidesc;
	testAttribute<CVuMeter> (kCVuMeter, kAttrOrientation, "horizontal", &uidesc,
	                         [&] (CVuMeter* v) { return v->getStyle () == CVuMeter::kHorizontal; });
	testAttribute<CVuMeter> (kCVuMeter, kAttrOrientation, "vertical", &uidesc,
	                         [&] (CVuMeter* v) { return v->getStyle () == CVuMeter::kVertical; });
}

TEST_CASE (CVuMeterCreatorTest, NumLed)
{
	DummyUIDescription uidesc;
	testAttribute<CVuMeter> (kCVuMeter, kAttrNumLed, 5, &uidesc,
	                         [&] (CVuMeter* v) { return v->getNbLed () == 5; });
}

TEST_CASE (CVuMeterCreatorTest, DecreaseStepValue)
{
	DummyUIDescription uidesc;
	testAttribute<CVuMeter> (kCVuMeter, kAttrDecreaseStepValue, 15., &uidesc,
	                         [&] (CVuMeter* v) { return v->getDecreaseStepValue () == 15.; });
}

TEST_CASE (CVuMeterCreatorTest, OrientationValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCVuMeter, kAttrOrientation, &uidesc, {"horizontal", "vertical"});
}

} // VSTGUI
