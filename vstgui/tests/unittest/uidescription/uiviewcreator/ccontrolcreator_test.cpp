// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/ccontrol.h"
#include "../../../../lib/controls/icontrollistener.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

namespace {

struct DummyListener : public IControlListener
{
	void valueChanged (CControl* pControl) override {}
};

} // anonymous

TEST_CASE (CControlCreatorTest, DefaultValue)
{
	testAttribute<CControl> (kCControl, kAttrDefaultValue, 1., nullptr,
	                         [] (CControl* v) { return v->getDefaultValue () == 1.; });
}

TEST_CASE (CControlCreatorTest, MinValue)
{
	testAttribute<CControl> (kCControl, kAttrMinValue, 0.5, nullptr,
	                         [] (CControl* v) { return v->getMin () == 0.5; });
}

TEST_CASE (CControlCreatorTest, MaxValue)
{
	testAttribute<CControl> (kCControl, kAttrMaxValue, 0.5, nullptr,
	                         [] (CControl* v) { return v->getMax () == 0.5; });
}

TEST_CASE (CControlCreatorTest, WheelIncValue)
{
	testAttribute<CControl> (kCControl, kAttrWheelIncValue, 0.5, nullptr,
	                         [] (CControl* v) { return v->getWheelInc () == 0.5; });
}

TEST_CASE (CControlCreatorTest, TagUnknown)
{
	DummyUIDescription uidesc;
	testAttribute<CControl> (kCControl, kAttrControlTag, kTagName, &uidesc, [&] (CControl* v) {
		return v->getTag () == -1 && v->getListener () == nullptr;
	});
}

TEST_CASE (CControlCreatorTest, TagStrEmpty)
{
	DummyUIDescription uidesc;
	testAttribute<CControl> (kCControl, kAttrControlTag, "", &uidesc, [&] (CControl* v) {
		return v->getTag () == -1 && v->getListener () == nullptr;
	});
}

TEST_CASE (CControlCreatorTest, TagWithNumber)
{
	DummyUIDescription uidesc;
	testAttribute<CControl> (kCControl, kAttrControlTag, "5", &uidesc, [&] (CControl* v) {
		return v->getTag () == 5 && v->getListener () == nullptr;
	});
}

TEST_CASE (CControlCreatorTest, TagNoListener)
{
	DummyUIDescription uidesc;
	uidesc.tag = 5;
	testAttribute<CControl> (
	    kCControl, kAttrControlTag, kTagName, &uidesc,
	    [&] (CControl* v) { return v->getTag () == 5 && v->getListener () == nullptr; }, true);
}

TEST_CASE (CControlCreatorTest, TagWithListener)
{
	DummyUIDescription uidesc;
	DummyListener listener;
	uidesc.tag = 5;
	uidesc.listener = &listener;
	testAttribute<CControl> (
	    kCControl, kAttrControlTag, kTagName, &uidesc,
	    [&] (CControl* v) { return v->getTag () == 5 && v->getListener () == &listener; }, true);
}

} // VSTGUI
