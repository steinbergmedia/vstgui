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

TEST_CASE (CRockerSwitchCreatorTest, HeightOfOneImage)
{
	DummyUIDescription uidesc;
	testAttribute<CRockerSwitch> (
	    kCRockerSwitch, kAttrHeightOfOneImage, 10, &uidesc,
	    [] (CRockerSwitch* v) { return v->getHeightOfOneImage () == 10; });
}

TEST_CASE (CRockerSwitchCreatorTest, SubPixmaps)
{
	DummyUIDescription uidesc;
	testAttribute<CRockerSwitch> (kCRockerSwitch, kAttrSubPixmaps, 11, &uidesc,
	                              [] (CRockerSwitch* v) { return v->getNumSubPixmaps () == 11; });
}

#endif

} // VSTGUI
