// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
			return v->getStyle () == CSegmentButton::Style::kHorizontal;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrStyle, "vertical", &uidesc, [&] (CSegmentButton* v) {
			return v->getStyle () == CSegmentButton::Style::kVertical;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrStyle, "horizontal-inverse", &uidesc, [&] (CSegmentButton* v) {
			return v->getStyle () == CSegmentButton::Style::kHorizontalInverse;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrStyle, "vertical-inverse", &uidesc, [&] (CSegmentButton* v) {
			return v->getStyle () == CSegmentButton::Style::kVerticalInverse;
		});
	);

	TEST(selectionMode,
		DummyUIDescription uidesc;
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrSelectionMode, "Single", &uidesc, [&] (CSegmentButton* v) {
			return v->getSelectionMode () == CSegmentButton::SelectionMode::kSingle;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrSelectionMode, "Single-Toggle", &uidesc, [&] (CSegmentButton* v) {
			return v->getSelectionMode () == CSegmentButton::SelectionMode::kSingleToggle;
		});
		testAttribute<CSegmentButton>(kCSegmentButton, kAttrSelectionMode, "Multiple", &uidesc, [&] (CSegmentButton* v) {
			return v->getSelectionMode () == CSegmentButton::SelectionMode::kMultiple;
		});
		testPossibleValues (kCSegmentButton, kAttrSelectionMode, &uidesc, {"Single", "Single-Toggle", "Multiple"});
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
		testPossibleValues (kCSegmentButton, kAttrStyle, &uidesc, {"horizontal", "vertical", "horizontal-inverse", "vertical-inverse"});
	);

);

} // VSTGUI
