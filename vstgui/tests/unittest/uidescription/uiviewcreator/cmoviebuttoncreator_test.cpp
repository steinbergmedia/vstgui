// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/cmoviebutton.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CMovieButtonCreatorTest,

	TEST(heightOfOneImage,
		DummyUIDescription uidesc;
		testAttribute<CMovieButton>(kCMovieButton, kAttrHeightOfOneImage, 10, &uidesc, [] (CMovieButton* v) {
			return v->getHeightOfOneImage () == 10;
		});
	);

	TEST(subPixmaps,
		DummyUIDescription uidesc;
		testAttribute<CMovieButton>(kCMovieButton, kAttrSubPixmaps, 11, &uidesc, [] (CMovieButton* v) {
			return v->getNumSubPixmaps () == 11;
		});
	);
);

} // VSTGUI
