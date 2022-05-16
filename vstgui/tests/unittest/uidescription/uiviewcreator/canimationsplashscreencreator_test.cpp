// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/csplashscreen.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CAnimationSplashScreenCreatorTest, SplashBitmap)
{
	DummyUIDescription uidesc;
	testAttribute<CAnimationSplashScreen> (
	    kCAnimationSplashScreen, kAttrSplashBitmap, kBitmapName, &uidesc,
	    [&] (CAnimationSplashScreen* v) { return v->getSplashBitmap () == uidesc.bitmap; });
}

TEST_CASE (CAnimationSplashScreenCreatorTest, SplashOrigin)
{
	DummyUIDescription uidesc;
	CPoint p (20, 20);
	testAttribute<CAnimationSplashScreen> (
	    kCAnimationSplashScreen, kAttrSplashOrigin, p, &uidesc,
	    [&] (CAnimationSplashScreen* v) { return v->getSplashRect ().getTopLeft () == p; });
}

TEST_CASE (CAnimationSplashScreenCreatorTest, SplashSize)
{
	DummyUIDescription uidesc;
	CPoint p (20, 20);
	testAttribute<CAnimationSplashScreen> (
	    kCAnimationSplashScreen, kAttrSplashSize, p, &uidesc,
	    [&] (CAnimationSplashScreen* v) { return v->getSplashRect ().getSize () == p; });
}

TEST_CASE (CAnimationSplashScreenCreatorTest, AnimationIndex)
{
	DummyUIDescription uidesc;
	testAttribute<CAnimationSplashScreen> (
	    kCAnimationSplashScreen, kAttrAnimationIndex, 1, &uidesc,
	    [&] (CAnimationSplashScreen* v) { return v->getAnimationIndex () == 1; });
}

TEST_CASE (CAnimationSplashScreenCreatorTest, AnimationTime)
{
	DummyUIDescription uidesc;
	testAttribute<CAnimationSplashScreen> (
	    kCAnimationSplashScreen, kAttrAnimationTime, 222, &uidesc,
	    [&] (CAnimationSplashScreen* v) { return v->getAnimationTime () == 222; });
}

} // VSTGUI
