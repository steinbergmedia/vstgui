// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/clayeredviewcontainer.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CLayeredViewContainerCreatorTest,

	TEST(zIndex,
		testAttribute<CLayeredViewContainer>(kCLayeredViewContainer, kAttrZIndex, 1, nullptr, [] (CLayeredViewContainer* v) {
			return v->getZIndex () == 1;
		});
	);
);

} // VSTGUI
