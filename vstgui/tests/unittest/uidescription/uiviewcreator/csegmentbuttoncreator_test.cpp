// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/csegmentbutton.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CSegmentButtonCreatorTest, Font)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (kCSegmentButton, kAttrFont, kFontName, &uidesc,
	                               [&] (CSegmentButton* v) { return v->getFont () == uidesc.font; },
	                               true);
}

TEST_CASE (CSegmentButtonCreatorTest, Style)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrStyle, "horizontal", &uidesc,
	    [&] (CSegmentButton* v) { return v->getStyle () == CSegmentButton::Style::kHorizontal; });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrStyle, "vertical", &uidesc,
	    [&] (CSegmentButton* v) { return v->getStyle () == CSegmentButton::Style::kVertical; });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrStyle, "horizontal-inverse", &uidesc, [&] (CSegmentButton* v) {
		    return v->getStyle () == CSegmentButton::Style::kHorizontalInverse;
	    });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrStyle, "vertical-inverse", &uidesc, [&] (CSegmentButton* v) {
		    return v->getStyle () == CSegmentButton::Style::kVerticalInverse;
	    });
}

TEST_CASE (CSegmentButtonCreatorTest, SelectionMode)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrSelectionMode, "Single", &uidesc, [&] (CSegmentButton* v) {
		    return v->getSelectionMode () == CSegmentButton::SelectionMode::kSingle;
	    });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrSelectionMode, "Single-Toggle", &uidesc, [&] (CSegmentButton* v) {
		    return v->getSelectionMode () == CSegmentButton::SelectionMode::kSingleToggle;
	    });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrSelectionMode, "Multiple", &uidesc, [&] (CSegmentButton* v) {
		    return v->getSelectionMode () == CSegmentButton::SelectionMode::kMultiple;
	    });
	testPossibleValues (kCSegmentButton, kAttrSelectionMode, &uidesc,
	                    {"Single", "Single-Toggle", "Multiple"});
}

TEST_CASE (CSegmentButtonCreatorTest, TextColor)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTextColor, kColorName, &uidesc,
	    [&] (CSegmentButton* v) { return v->getTextColor () == uidesc.color; });
}

TEST_CASE (CSegmentButtonCreatorTest, TextColorHighlighted)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTextColorHighlighted, kColorName, &uidesc,
	    [&] (CSegmentButton* v) { return v->getTextColorHighlighted () == uidesc.color; });
}

TEST_CASE (CSegmentButtonCreatorTest, FrameColor)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrFrameColor, kColorName, &uidesc,
	    [&] (CSegmentButton* v) { return v->getFrameColor () == uidesc.color; });
}

TEST_CASE (CSegmentButtonCreatorTest, FrameWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (kCSegmentButton, kAttrFrameWidth, 5., &uidesc,
	                               [&] (CSegmentButton* v) { return v->getFrameWidth () == 5.; });
}

TEST_CASE (CSegmentButtonCreatorTest, RoundRadius)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (kCSegmentButton, kAttrRoundRadius, 15., &uidesc,
	                               [&] (CSegmentButton* v) { return v->getRoundRadius () == 15.; });
}

TEST_CASE (CSegmentButtonCreatorTest, TextIconMargin)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (kCSegmentButton, kAttrIconTextMargin, 15., &uidesc,
	                               [&] (CSegmentButton* v) { return v->getTextMargin () == 15; });
}

TEST_CASE (CSegmentButtonCreatorTest, TextAlignment)
{
	DummyUIDescription uiDesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTextAlignment, "left", &uiDesc,
	    [&] (CSegmentButton* v) { return v->getTextAlignment () == kLeftText; });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTextAlignment, "center", &uiDesc,
	    [&] (CSegmentButton* v) { return v->getTextAlignment () == kCenterText; });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTextAlignment, "right", &uiDesc,
	    [&] (CSegmentButton* v) { return v->getTextAlignment () == kRightText; });
}

TEST_CASE (CSegmentButtonCreatorTest, Gradient)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrGradient, kGradientName, &uidesc,
	    [&] (CSegmentButton* v) { return v->getGradient () == uidesc.gradient; }, true);
}

TEST_CASE (CSegmentButtonCreatorTest, GradientHighlighted)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrGradientHighlighted, kGradientName, &uidesc,
	    [&] (CSegmentButton* v) { return v->getGradientHighlighted () == uidesc.gradient; }, true);
}

TEST_CASE (CSegmentButtonCreatorTest, SegmentNames)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (kCSegmentButton, kAttrSegmentNames, "s1,s2,s3", &uidesc,
	                               [&] (CSegmentButton* v) {
		                               EXPECT (v->getSegments ()[0].name == "s1");
		                               EXPECT (v->getSegments ()[1].name == "s2");
		                               EXPECT (v->getSegments ()[2].name == "s3");
		                               return v->getSegments ().size () == 3;
	                               });
}

TEST_CASE (CSegmentButtonCreatorTest, TruncateMode)
{
	DummyUIDescription uidesc;
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTruncateMode, "head", &uidesc, [] (CSegmentButton* v) {
		    return v->getTextTruncateMode () == CDrawMethods::kTextTruncateHead;
	    });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTruncateMode, "tail", &uidesc, [] (CSegmentButton* v) {
		    return v->getTextTruncateMode () == CDrawMethods::kTextTruncateTail;
	    });
	testAttribute<CSegmentButton> (
	    kCSegmentButton, kAttrTruncateMode, "", &uidesc, [] (CSegmentButton* v) {
		    return v->getTextTruncateMode () == CDrawMethods::kTextTruncateNone;
	    });
}

TEST_CASE (CSegmentButtonCreatorTest, TruncateModeValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCSegmentButton, kAttrTruncateMode, &uidesc, {"head", "tail", "none"});
}

TEST_CASE (CSegmentButtonCreatorTest, OrientationValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCSegmentButton, kAttrStyle, &uidesc,
	                    {"horizontal", "vertical", "horizontal-inverse", "vertical-inverse"});
}

} // VSTGUI
