// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/csplashscreen.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CAnimationSplashScreenCreatorTest,

	TEST(splashBitmap,
		DummyUIDescription uidesc;
		testAttribute<CAnimationSplashScreen>(kCAnimationSplashScreen, kAttrSplashBitmap, kBitmapName, &uidesc, [&] (CAnimationSplashScreen* v) {
			return v->getSplashBitmap() == uidesc.bitmap;
		});
	);

	TEST(splashOrigin,
		DummyUIDescription uidesc;
		CPoint p (20, 20);
		testAttribute<CAnimationSplashScreen>(kCAnimationSplashScreen, kAttrSplashOrigin, p, &uidesc, [&] (CAnimationSplashScreen* v) {
			return v->getSplashRect().getTopLeft() == p;
		});
	);

	TEST(splashSize,
		DummyUIDescription uidesc;
		CPoint p (20, 20);
		testAttribute<CAnimationSplashScreen>(kCAnimationSplashScreen, kAttrSplashSize, p, &uidesc, [&] (CAnimationSplashScreen* v) {
			return v->getSplashRect().getSize() == p;
		});
	);

	TEST(animationIndex,
		DummyUIDescription uidesc;
		testAttribute<CAnimationSplashScreen>(kCAnimationSplashScreen, kAttrAnimationIndex, 1, &uidesc, [&] (CAnimationSplashScreen* v) {
			return v->getAnimationIndex () == 1;
		});
	);

	TEST(animationTime,
		DummyUIDescription uidesc;
		testAttribute<CAnimationSplashScreen>(kCAnimationSplashScreen, kAttrAnimationTime, 222, &uidesc, [&] (CAnimationSplashScreen* v) {
			return v->getAnimationTime () == 222;
		});
	);

);

} // VSTGUI
