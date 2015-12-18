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
#include "../../../../lib/controls/csegmentbutton.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CSegmentButtonCreatorTest,

	TEST(font,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrFont, kFontName, &uidesc, [&] (CSegmentButton* v) {
			return v->getFont () == uidesc.font;
		}, true);
	);

	TEST(style,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrStyle, "horizontal", &uidesc, [&] (CSegmentButton* v) {
			return v->getStyle () == CSegmentButton::kHorizontal;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrStyle, "vertical", &uidesc, [&] (CSegmentButton* v) {
			return v->getStyle () == CSegmentButton::kVertical;
		});
	);

	TEST(textColor,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTextColor, kColorName, &uidesc, [&] (CSegmentButton* v) {
			return v->getTextColor () == uidesc.color;
		});
	);

	TEST(textColorHighlighted,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTextColorHighlighted, kColorName, &uidesc, [&] (CSegmentButton* v) {
			return v->getTextColorHighlighted () == uidesc.color;
		});
	);

	TEST(frameColor,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrFrameColor, kColorName, &uidesc, [&] (CSegmentButton* v) {
			return v->getFrameColor () == uidesc.color;
		});
	);

	TEST(frameWidth,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrFrameWidth, 5., &uidesc, [&] (CSegmentButton* v) {
			return v->getFrameWidth() == 5.;
		});
	);

	TEST(roundRadius,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrRoundRadius, 15., &uidesc, [&] (CSegmentButton* v) {
			return v->getRoundRadius () == 15.;
		});
	);

	TEST(textIconMargin,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrIconTextMargin, 15., &uidesc, [&] (CSegmentButton* v) {
			return v->getTextMargin () == 15;
		});
	);

	TEST(textAlignment,
		DummyUIDescription uiDesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTextAlignment, "left", &uiDesc, [&] (CSegmentButton* v) {
			return v->getTextAlignment () == kLeftText;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTextAlignment, "center", &uiDesc, [&] (CSegmentButton* v) {
			return v->getTextAlignment () == kCenterText;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTextAlignment, "right", &uiDesc, [&] (CSegmentButton* v) {
			return v->getTextAlignment () == kRightText;
		});
	);

	TEST(gradient,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrGradient, kGradientName, &uidesc, [&] (CSegmentButton* v) {
			return v->getGradient () == uidesc.gradient;
		}, true);
	);

	TEST(gradientHighlighted,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrGradientHighlighted, kGradientName, &uidesc, [&] (CSegmentButton* v) {
			return v->getGradientHighlighted () == uidesc.gradient;
		}, true);
	);

	TEST(segmentNames,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrSegmentNames, "s1,s2,s3", &uidesc, [&] (CSegmentButton* v) {
			EXPECT(v->getSegments()[0].name == "s1");
			EXPECT(v->getSegments()[1].name == "s2");
			EXPECT(v->getSegments()[2].name == "s3");
			return v->getSegments ().size () == 3;
		});
	);

	TEST(truncateMode,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTruncateMode, "head", &uidesc, [] (CSegmentButton* v) {
			return v->getTextTruncateMode () == CDrawMethods::kTextTruncateHead;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTruncateMode, "tail", &uidesc, [] (CSegmentButton* v) {
			return v->getTextTruncateMode () == CDrawMethods::kTextTruncateTail;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrTruncateMode, "", &uidesc, [] (CSegmentButton* v) {
			return v->getTextTruncateMode () == CDrawMethods::kTextTruncateNone;
		});
	);
	
	TEST(truncateModeValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCSegmentButton, kAttrTruncateMode, &uidesc, {"head", "tail", "none"});
	);

	TEST(orientationValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCSegmentButton, kAttrStyle, &uidesc, {"horizontal", "vertical"});
	);

);

} // VSTGUI
