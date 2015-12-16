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
#include "../../../../lib/csplitview.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CSplitViewCreatorTest,

	TEST(separatorWidth,
		testAttribute<CSplitView>(kCSplitView, kAttrSeparatorWidth, 123, nullptr, [] (CSplitView* v) {
			return v->getSeparatorWidth () == 123;
		});
	);

	TEST(orientation,
		DummyUIDescription uidesc;
		testAttribute<CSplitView>(kCSplitView, kAttrOrientation, "horizontal", &uidesc, [&] (CSplitView* v) {
			return v->getStyle() == CSplitView::kHorizontal;
		});
		testAttribute<CSplitView>(kCSplitView, kAttrOrientation, "vertical", &uidesc, [&] (CSplitView* v) {
			return v->getStyle() == CSplitView::kVertical;
		});
	);

	TEST(resizeMethod,
		DummyUIDescription uidesc;
		testAttribute<CSplitView>(kCSplitView, kAttrResizeMethod, "first", &uidesc, [&] (CSplitView* v) {
			return v->getResizeMethod() == CSplitView::kResizeFirstView;
		});
		testAttribute<CSplitView>(kCSplitView, kAttrResizeMethod, "second", &uidesc, [&] (CSplitView* v) {
			return v->getResizeMethod() == CSplitView::kResizeSecondView;
		});
		testAttribute<CSplitView>(kCSplitView, kAttrResizeMethod, "last", &uidesc, [&] (CSplitView* v) {
			return v->getResizeMethod() == CSplitView::kResizeLastView;
		});
		testAttribute<CSplitView>(kCSplitView, kAttrResizeMethod, "all", &uidesc, [&] (CSplitView* v) {
			return v->getResizeMethod() == CSplitView::kResizeAllViews;
		});
	);

	TEST(orientationValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCSplitView, kAttrOrientation, &uidesc, {"horizontal", "vertical"});
	);

	TEST(resizeMethodValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCSplitView, kAttrResizeMethod, &uidesc, {"first", "second", "last", "all"});
	);

);

} // VSTGUI
