// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/ccontrol.h"
#include "../../../../lib/controls/icontrollistener.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

namespace {

struct DummyListener : public IControlListener
{
	void valueChanged (CControl* pControl) override {}
};

} // anonymous

TESTCASE(CControlCreatorTest,

	TEST(defaultValue,
		testAttribute<CControl>(kCControl, kAttrDefaultValue, 1., nullptr, [] (CControl* v) {
			return v->getDefaultValue () == 1.;
		});
	);

	TEST(minValue,
		testAttribute<CControl>(kCControl, kAttrMinValue, 0.5, nullptr, [] (CControl* v) {
			return v->getMin () == 0.5;
		});
	);

	TEST(maxValue,
		testAttribute<CControl>(kCControl, kAttrMaxValue, 0.5, nullptr, [] (CControl* v) {
			return v->getMax () == 0.5;
		});
	);

	TEST(wheelIncValue,
		testAttribute<CControl>(kCControl, kAttrWheelIncValue, 0.5, nullptr, [] (CControl* v) {
			return v->getWheelInc () == 0.5;
		});
	);

	TEST(tagUnknown,
		DummyUIDescription uidesc;
		testAttribute<CControl>(kCControl, kAttrControlTag, kTagName, &uidesc, [&] (CControl* v) {
			return v->getTag() == -1 && v->getListener() == nullptr;
		});
	);

	TEST(tagStrEmpty,
		DummyUIDescription uidesc;
		testAttribute<CControl>(kCControl, kAttrControlTag, "", &uidesc, [&] (CControl* v) {
			return v->getTag() == -1 && v->getListener() == nullptr;
		});
	);

	TEST(tagWithNumber,
		DummyUIDescription uidesc;
		testAttribute<CControl>(kCControl, kAttrControlTag, "5", &uidesc, [&] (CControl* v) {
			return v->getTag() == 5 && v->getListener() == nullptr;
		});
	);

	TEST(tagNoListener,
		DummyUIDescription uidesc;
		uidesc.tag = 5;
		testAttribute<CControl>(kCControl, kAttrControlTag, kTagName, &uidesc, [&] (CControl* v) {
			return v->getTag() == 5 && v->getListener() == nullptr;
		}, true);
	);

	TEST(tagWithListener,
		DummyUIDescription uidesc;
		DummyListener listener;
		uidesc.tag = 5;
		uidesc.listener = &listener;
		testAttribute<CControl>(kCControl, kAttrControlTag, kTagName, &uidesc, [&] (CControl* v) {
			return v->getTag() == 5 && v->getListener() == &listener;
		}, true);
	);
);

} // VSTGUI
