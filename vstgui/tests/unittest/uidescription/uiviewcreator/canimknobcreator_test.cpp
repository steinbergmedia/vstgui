// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cknob.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CAnimKnobCreatorTest, HeightOfOneImage)
{
	DummyUIDescription uidesc;
	testAttribute<CAnimKnob> (kCAnimKnob, kAttrHeightOfOneImage, 10, &uidesc,
	                          [] (CAnimKnob* v) { return v->getHeightOfOneImage () == 10; });
}

TEST_CASE (CAnimKnobCreatorTest, SubPixmaps)
{
	DummyUIDescription uidesc;
	testAttribute<CAnimKnob> (kCAnimKnob, kAttrSubPixmaps, 11, &uidesc,
	                          [] (CAnimKnob* v) { return v->getNumSubPixmaps () == 11; });
}

TEST_CASE (CAnimKnobCreatorTest, InverseBitmap)
{
	DummyUIDescription uidesc;
	testAttribute<CAnimKnob> (kCAnimKnob, kAttrInverseBitmap, true, &uidesc,
	                          [] (CAnimKnob* v) { return v->getInverseBitmap () == true; });
	testAttribute<CAnimKnob> (kCAnimKnob, kAttrInverseBitmap, false, &uidesc,
	                          [] (CAnimKnob* v) { return v->getInverseBitmap () == false; });
}

} // VSTGUI
