// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CCheckBoxCreatorTest, Title)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrTitle, "title", &uidesc,
	                          [] (CCheckBox* b) { return b->getTitle () == "title"; });
}

TEST_CASE (CCheckBoxCreatorTest, Font)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrFont, kFontName, &uidesc,
	                          [&] (CCheckBox* b) { return uidesc.font == b->getFont (); }, true);
}

TEST_CASE (CCheckBoxCreatorTest, FontColor)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrFontColor, kColorName, &uidesc,
	                          [&] (CCheckBox* b) { return b->getFontColor () == uidesc.color; });
}

TEST_CASE (CCheckBoxCreatorTest, BoxFrameColor)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (
	    kCCheckBox, kAttrBoxframeColor, kColorName, &uidesc,
	    [&] (CCheckBox* b) { return b->getBoxFrameColor () == uidesc.color; });
}

TEST_CASE (CCheckBoxCreatorTest, BoxFillColor)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrBoxfillColor, kColorName, &uidesc,
	                          [&] (CCheckBox* b) { return b->getBoxFillColor () == uidesc.color; });
}

TEST_CASE (CCheckBoxCreatorTest, CheckmarkColor)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (
	    kCCheckBox, kAttrCheckmarkColor, kColorName, &uidesc,
	    [&] (CCheckBox* b) { return b->getCheckMarkColor () == uidesc.color; });
}

TEST_CASE (CCheckBoxCreatorTest, DrawCrossbox)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrDrawCrossbox, true, &uidesc, [&] (CCheckBox* b) {
		return b->getStyle () & CCheckBox::kDrawCrossBox;
	});
}

TEST_CASE (CCheckBoxCreatorTest, AutoSizeToFit)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrAutosizeToFit, true, &uidesc, [&] (CCheckBox* b) {
		return b->getStyle () & CCheckBox::kAutoSizeToFit;
	});
}

TEST_CASE (CCheckBoxCreatorTest, FrameWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrFrameWidth, 15., &uidesc,
	                          [&] (CCheckBox* b) { return b->getFrameWidth () == 15.; });
}

TEST_CASE (CCheckBoxCreatorTest, RoundRectRadius)
{
	DummyUIDescription uidesc;
	testAttribute<CCheckBox> (kCCheckBox, kAttrRoundRectRadius, 12., &uidesc,
	                          [&] (CCheckBox* b) { return b->getRoundRectRadius () == 12.; });
}

} // VSTGUI
