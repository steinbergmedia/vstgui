// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CKickButtonCreatorTest, HeightOfOneImage)
{
	DummyUIDescription uidesc;
	testAttribute<CKickButton> (kCKickButton, kAttrHeightOfOneImage, 10, &uidesc,
	                            [] (CKickButton* v) { return v->getHeightOfOneImage () == 10; });
}

TEST_CASE (CKickButtonCreatorTest, SubPixmaps)
{
	DummyUIDescription uidesc;
	testAttribute<CKickButton> (kCKickButton, kAttrSubPixmaps, 11, &uidesc,
	                            [] (CKickButton* v) { return v->getNumSubPixmaps () == 11; });
}

} // VSTGUI
