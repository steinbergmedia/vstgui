// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cparamdisplay.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CParamDisplayCreatorTest, Font)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrFont, kFontName, uidesc,
		[&] (CParamDisplay* v) { return v->getFont () == uidesc.font; }, true);
}

TEST_CASE (CParamDisplayCreatorTest, FontColor)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrFontColor, kColorName, uidesc,
		[&] (CParamDisplay* v) { return v->getFontColor () == uidesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, BackColor)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrBackColor, kColorName, uidesc,
		[&] (CParamDisplay* v) { return v->getBackColor () == uidesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, FrameColor)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrFrameColor, kColorName, uidesc,
		[&] (CParamDisplay* v) { return v->getFrameColor () == uidesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, ShadowColor)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrShadowColor, kColorName, uidesc,
		[&] (CParamDisplay* v) { return v->getShadowColor () == uidesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, TextInset)
{
	DummyUIDescription uidesc;
	CPoint inset (5, 6);
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrTextInset, inset, uidesc,
								  [&] (CParamDisplay* v) { return v->getTextInset () == inset; });
}

TEST_CASE (CParamDisplayCreatorTest, FontAntialias)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFontAntialias, true, uidesc,
								  [&] (CParamDisplay* v) { return v->getAntialias (); });
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFontAntialias, false, uidesc,
								  [&] (CParamDisplay* v) { return v->getAntialias () == false; });
}

TEST_CASE (CParamDisplayCreatorTest, TextAlignment)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrTextAlignment, "left", uidesc,
		[&] (CParamDisplay* v) { return v->getHoriAlign () == kLeftText; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrTextAlignment, "center", uidesc,
		[&] (CParamDisplay* v) { return v->getHoriAlign () == kCenterText; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrTextAlignment, "right", uidesc,
		[&] (CParamDisplay* v) { return v->getHoriAlign () == kRightText; });
}

TEST_CASE (CParamDisplayCreatorTest, RoundRectRadius)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrRoundRectRadius, 15., uidesc,
		[&] (CParamDisplay* v) { return v->getRoundRectRadius () == 15.; });
}

TEST_CASE (CParamDisplayCreatorTest, FrameWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFrameWidth, 12., uidesc,
								  [&] (CParamDisplay* v) { return v->getFrameWidth () == 12.; });
}

TEST_CASE (CParamDisplayCreatorTest, TextRotation)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrTextRotation, 89., uidesc,
								  [&] (CParamDisplay* v) { return v->getTextRotation () == 89.; });
}

TEST_CASE (CParamDisplayCreatorTest, Styles)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyle3DIn, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::k3DIn; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyle3DOut, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::k3DOut; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyleNoFrame, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kNoFrame; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyleNoDraw, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kNoDrawStyle; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyleNoText, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kNoTextStyle; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyleShadowText, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kShadowText; });
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrStyleRoundRect, true, uidesc,
		[&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kRoundRectStyle; });
}

TEST_CASE (CParamDisplayCreatorTest, ValuePrecision)
{
	DummyUIDescription uidesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrValuePrecision, 3, uidesc,
								  [&] (CParamDisplay* v) { return v->getPrecision () == 3; });
}

TEST_CASE (CParamDisplayCreatorTest, TextRotationMinMax)
{
	DummyUIDescription uidesc;
	testMinMaxValues (kCParamDisplay, kAttrTextRotation, uidesc, 0., 360.);
}

TEST_CASE (CParamDisplayCreatorTest, BackgroundOffset)
{
	DummyUIDescription uidesc;
	CPoint offset (20, 20);
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrBackgroundOffset, offset, uidesc,
								  [&] (CParamDisplay* v) { return v->getBackOffset () == offset; });
}

TEST_CASE (CParamDisplayCreatorTest, ShadowOffset)
{
	DummyUIDescription uidesc;
	CPoint offset (15, 9);
	testAttribute<CParamDisplay> (
		kCParamDisplay, kAttrTextShadowOffset, offset, uidesc,
		[&] (CParamDisplay* v) { return v->getShadowTextOffset () == offset; });
}

} // VSTGUI
