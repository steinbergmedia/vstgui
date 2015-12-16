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
#include "../../../../lib/cscrollview.h"
#include "../../../../lib/controls/cscrollbar.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CScrollViewContainerCreatorTest,

	TEST(containerSize,
		CPoint size (100, 100);
		testAttribute<CScrollView> (kCScrollView, kAttrContainerSize, size, nullptr, [&] (CScrollView* v) { return v->getContainerSize ().getSize () == size; });
	);

	TEST(horizontalScrollbar,
		testAttribute<CScrollView> (kCScrollView, kAttrHorizontalScrollbar, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kHorizontalScrollbar; });
	);

	TEST(verticalScrollbar,
		testAttribute<CScrollView> (kCScrollView, kAttrVerticalScrollbar, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kVerticalScrollbar; });
	);

	TEST(autoDragScrolling,
		testAttribute<CScrollView> (kCScrollView, kAttrAutoDragScrolling, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoDragScrolling; });
	);

	TEST(dontDrawFrame,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrBordered, true, &uiDesc, [&] (CScrollView* v) { return v->getStyle () & ~CScrollView::kDontDrawFrame; });
		testAttribute<CScrollView> (kCScrollView, kAttrBordered, false, &uiDesc, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kDontDrawFrame; });
	);

	TEST(overlayScrollbars,
		testAttribute<CScrollView> (kCScrollView, kAttrOverlayScrollbars, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kOverlayScrollbars; });
	);

	TEST(followFocusView,
		testAttribute<CScrollView> (kCScrollView, kAttrFollowFocusView, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kFollowFocusView; });
	);

	TEST(autoHideScrollbars,
		testAttribute<CScrollView> (kCScrollView, kAttrAutoHideScrollbars, true, nullptr, [&] (CScrollView* v) { return v->getStyle () & CScrollView::kAutoHideScrollbars; });
	);

	TEST(scrollbarWidth,
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarWidth, 5., nullptr, [&] (CScrollView* v) { return v->getScrollbarWidth () == 5.; });
	);

	TEST(scrollbarBackgroundColor,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarBackgroundColor, kColorName, &uiDesc, [&] (CScrollView* v) {
			auto sb = v->getVerticalScrollbar ();
			if (!sb)
				sb = v->getHorizontalScrollbar ();
			return sb->getBackgroundColor () == uiDesc.color;
		});
	);

	TEST(scrollbarFrameColor,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarFrameColor, kColorName, &uiDesc, [&] (CScrollView* v) {
			auto sb = v->getVerticalScrollbar ();
			if (!sb)
				sb = v->getHorizontalScrollbar ();
			return sb->getFrameColor () == uiDesc.color;
		});
	);

	TEST(scrollbarScrollerColor,
		DummyUIDescription uiDesc;
		testAttribute<CScrollView> (kCScrollView, kAttrScrollbarScrollerColor, kColorName, &uiDesc, [&] (CScrollView* v) {
			auto sb = v->getVerticalScrollbar ();
			if (!sb)
				sb = v->getHorizontalScrollbar ();
			return sb->getScrollerColor () == uiDesc.color;
		});
	);
);

} // VSTGUI
