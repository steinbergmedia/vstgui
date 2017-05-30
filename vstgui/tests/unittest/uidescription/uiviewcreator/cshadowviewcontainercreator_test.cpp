// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/cshadowviewcontainer.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CShadowViewContainerCreatorTest,

	TEST(shadowIntensity,
		DummyUIDescription uidesc;
		testAttribute<CShadowViewContainer>(kCShadowViewContainer, kAttrShadowIntensity, 0.5, &uidesc, [] (CShadowViewContainer* v) {
			return v->getShadowIntensity () == 0.5f;
		});
	);

	TEST(shadowBlurSize,
		DummyUIDescription uidesc;
		testAttribute<CShadowViewContainer>(kCShadowViewContainer, kAttrShadowBlurSize, 0.5, &uidesc, [] (CShadowViewContainer* v) {
			return v->getShadowBlurSize () == 0.5f;
		});
	);

	TEST(shadowOffset,
		DummyUIDescription uidesc;
		CPoint p (20, 20);
		testAttribute<CShadowViewContainer>(kCShadowViewContainer, kAttrShadowOffset, p, &uidesc, [&] (CShadowViewContainer* v) {
			return v->getShadowOffset () == p;
		});
	);
	
	TEST(shadowBlurSizeMinMax,
		DummyUIDescription uidesc;
		testMinMaxValues(kCShadowViewContainer, kAttrShadowBlurSize, &uidesc, 0.8, 20);
	);

	TEST(shadowIntensityMinMax,
		DummyUIDescription uidesc;
		testMinMaxValues(kCShadowViewContainer, kAttrShadowIntensity, &uidesc, 0., 1.);
	);
);

} // VSTGUI
