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
#include "../../../../lib/crowcolumnview.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CRowColumnViewCreatorTest,

	TEST(rowStyle,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrRowStyle, true, nullptr, [] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kRowStyle; });
	);

	TEST(columnStyle,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrRowStyle, false, nullptr, [] (CRowColumnView* v) { return v->getStyle () == CRowColumnView::kColumnStyle; });
	);

	TEST(spacing,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrSpacing, 5., nullptr, [] (CRowColumnView* v) { return v->getSpacing () == 5.; });
	);

	TEST(margin,
		CRect margin (5,6,7,8);
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrMargin, margin, nullptr, [&] (CRowColumnView* v) { return v->getMargin () == margin; });
	);

	TEST(animateViewResizing,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrAnimateViewResizing, true, nullptr, [] (CRowColumnView* v) { return v->isAnimateViewResizing (); });
	);

	TEST(equalSizeLayoutStretch,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "stretch", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kStretchEqualy; });
	);

	TEST(equalSizeLayoutCenter,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "center", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kCenterEqualy; });
	);

	TEST(equalSizeLayoutRightBottom,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "right-bottom", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kRightBottomEqualy; });
	);

	TEST(equalSizeLayoutLeftTop,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrEqualSizeLayout, "left-top", nullptr, [] (CRowColumnView* v) { return v->getLayoutStyle() == CRowColumnView::kLeftTopEqualy; });
	);

	TEST(animationTime,
		testAttribute<CRowColumnView> (kCRowColumnView, kAttrViewResizeAnimationTime, 100, nullptr, [] (CRowColumnView* v) { return v->getViewResizeAnimationTime() == 100; });
	);

	TEST(equalSizeLayoutValues,
		testPossibleValues(kCRowColumnView, kAttrEqualSizeLayout, nullptr, {"left-top", "stretch", "center", "right-bottom"});
	);
);

} // VSTGUI
