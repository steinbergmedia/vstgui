// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/cshadowviewcontainer.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CShadowViewContainerCreatorTest, ShadowIntensity)
{
	DummyUIDescription uidesc;
	testAttribute<CShadowViewContainer> (
	    kCShadowViewContainer, kAttrShadowIntensity, 0.5, &uidesc,
	    [] (CShadowViewContainer* v) { return v->getShadowIntensity () == 0.5f; });
}

TEST_CASE (CShadowViewContainerCreatorTest, ShadowBlurSize)
{
	DummyUIDescription uidesc;
	testAttribute<CShadowViewContainer> (
	    kCShadowViewContainer, kAttrShadowBlurSize, 0.5, &uidesc,
	    [] (CShadowViewContainer* v) { return v->getShadowBlurSize () == 0.5f; });
}

TEST_CASE (CShadowViewContainerCreatorTest, ShadowOffset)
{
	DummyUIDescription uidesc;
	CPoint p (20, 20);
	testAttribute<CShadowViewContainer> (
	    kCShadowViewContainer, kAttrShadowOffset, p, &uidesc,
	    [&] (CShadowViewContainer* v) { return v->getShadowOffset () == p; });
}

TEST_CASE (CShadowViewContainerCreatorTest, ShadowBlurSizeMinMax)
{
	DummyUIDescription uidesc;
	testMinMaxValues (kCShadowViewContainer, kAttrShadowBlurSize, &uidesc, 0.8, 20);
}

TEST_CASE (CShadowViewContainerCreatorTest, ShadowIntensityMinMax)
{
	DummyUIDescription uidesc;
	testMinMaxValues (kCShadowViewContainer, kAttrShadowIntensity, &uidesc, 0., 1.);
}

} // VSTGUI
