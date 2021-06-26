// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/cstring.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CViewContainerCreatorTest, BackgroundColor)
{
	DummyUIDescription uidesc;
	testAttribute<CViewContainer> (
	    kCViewContainer, kAttrBackgroundColor, kColorName, &uidesc,
	    [&] (CViewContainer* v) { return v->getBackgroundColor () == uidesc.color; });
	testAttribute<CViewContainer> (
	    kCViewContainer, kAttrBackgroundColor, kColorName, &uidesc,
	    [&] (CViewContainer* v) { return v->getBackgroundColor () == uidesc.color; }, true);
}

TEST_CASE (CViewContainerCreatorTest, BackgroundColorDrawStyle)
{
	DummyUIDescription uidesc;
	testAttribute<CViewContainer> (
	    kCViewContainer, kAttrBackgroundColorDrawStyle, "stroked", &uidesc,
	    [] (CViewContainer* v) { return v->getBackgroundColorDrawStyle () == kDrawStroked; });
	testAttribute<CViewContainer> (
	    kCViewContainer, kAttrBackgroundColorDrawStyle, "filled", &uidesc,
	    [] (CViewContainer* v) { return v->getBackgroundColorDrawStyle () == kDrawFilled; });
	testAttribute<CViewContainer> (kCViewContainer, kAttrBackgroundColorDrawStyle,
	                               "filled and stroked", &uidesc, [] (CViewContainer* v) {
		                               return v->getBackgroundColorDrawStyle () ==
		                                      kDrawFilledAndStroked;
	                               });
}

TEST_CASE (CViewContainerCreatorTest, BackgroundColorDrawStyleValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCViewContainer, kAttrBackgroundColorDrawStyle, &uidesc,
	                    {"stroked", "filled", "filled and stroked"});
}

} // VSTGUI
