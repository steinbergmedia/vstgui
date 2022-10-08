// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cswitch.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

#if VSTGUI_ENABLE_DEPRECATED_METHODS

TEST_CASE (CVerticalSwitchCreatorTest, HeightOfOneImage)
{
	DummyUIDescription uidesc;
	testAttribute<CVerticalSwitch> (
	    kCVerticalSwitch, kAttrHeightOfOneImage, 10, &uidesc,
	    [] (CVerticalSwitch* v) { return v->getHeightOfOneImage () == 10; });
}

TEST_CASE (CVerticalSwitchCreatorTest, SubPixmaps)
{
	DummyUIDescription uidesc;
	testAttribute<CVerticalSwitch> (
	    kCVerticalSwitch, kAttrSubPixmaps, 11, &uidesc,
	    [] (CVerticalSwitch* v) { return v->getNumSubPixmaps () == 11; });
}

#endif

TEST_CASE (CVerticalSwitchCreatorTest, InverseBitmap)
{
	DummyUIDescription uidesc;
	testAttribute<CVerticalSwitch> (
	    kCVerticalSwitch, kAttrInverseBitmap, true, &uidesc,
	    [] (CVerticalSwitch* v) { return v->getInverseBitmap () == true; });
	testAttribute<CVerticalSwitch> (
	    kCVerticalSwitch, kAttrInverseBitmap, false, &uidesc,
	    [] (CVerticalSwitch* v) { return v->getInverseBitmap () == false; });
}

} // VSTGUI
