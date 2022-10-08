// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cmoviebitmap.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

#if VSTGUI_ENABLE_DEPRECATED_METHODS

TEST_CASE (CMovieBitmapCreatorTest, HeightOfOneImage)
{
	DummyUIDescription uidesc;
	testAttribute<CMovieBitmap> (kCMovieBitmap, kAttrHeightOfOneImage, 10, &uidesc,
	                             [] (CMovieBitmap* v) { return v->getHeightOfOneImage () == 10; });
}

TEST_CASE (CMovieBitmapCreatorTest, SubPixmaps)
{
	DummyUIDescription uidesc;
	testAttribute<CMovieBitmap> (kCMovieBitmap, kAttrSubPixmaps, 11, &uidesc,
	                             [] (CMovieBitmap* v) { return v->getNumSubPixmaps () == 11; });
}

#endif

} // VSTGUI
