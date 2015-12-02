//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/ccontrol.h"
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

	TEST(backgroundOffset,
		CPoint offset (20, 20);
		testAttribute<CControl>(kCControl, kAttrBackgroundOffset, offset, nullptr, [&] (CControl* v) {
			return v->getBackOffset() == offset;
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
