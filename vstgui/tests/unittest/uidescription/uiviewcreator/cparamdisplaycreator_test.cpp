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
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFont, kFontName, &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getFont () == uiDesc.font; },
	                              true);
}

TEST_CASE (CParamDisplayCreatorTest, FontColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrFontColor, kColorName, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getFontColor () == uiDesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, BackColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrBackColor, kColorName, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getBackColor () == uiDesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, FrameColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrFrameColor, kColorName, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getFrameColor () == uiDesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, ShadowColor)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrShadowColor, kColorName, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getShadowColor () == uiDesc.color; });
}

TEST_CASE (CParamDisplayCreatorTest, TextInset)
{
	DummyUIDescription uiDesc;
	CPoint inset (5, 6);
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrTextInset, inset, &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getTextInset () == inset; });
}

TEST_CASE (CParamDisplayCreatorTest, FontAntialias)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFontAntialias, true, &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getAntialias (); });
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFontAntialias, false, &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getAntialias () == false; });
}

TEST_CASE (CParamDisplayCreatorTest, TextAlignment)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrTextAlignment, "left", &uiDesc,
	    [&] (CParamDisplay* v) { return v->getHoriAlign () == kLeftText; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrTextAlignment, "center", &uiDesc,
	    [&] (CParamDisplay* v) { return v->getHoriAlign () == kCenterText; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrTextAlignment, "right", &uiDesc,
	    [&] (CParamDisplay* v) { return v->getHoriAlign () == kRightText; });
}

TEST_CASE (CParamDisplayCreatorTest, RoundRectRadius)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrRoundRectRadius, 15., &uiDesc,
	    [&] (CParamDisplay* v) { return v->getRoundRectRadius () == 15.; });
}

TEST_CASE (CParamDisplayCreatorTest, FrameWidth)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrFrameWidth, 12., &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getFrameWidth () == 12.; });
}

TEST_CASE (CParamDisplayCreatorTest, TextRotation)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrTextRotation, 89., &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getTextRotation () == 89.; });
}

TEST_CASE (CParamDisplayCreatorTest, Styles)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyle3DIn, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::k3DIn; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyle3DOut, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::k3DOut; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyleNoFrame, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kNoFrame; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyleNoDraw, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kNoDrawStyle; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyleNoText, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kNoTextStyle; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyleShadowText, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kShadowText; });
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrStyleRoundRect, true, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getStyle () & CParamDisplay::kRoundRectStyle; });
}

TEST_CASE (CParamDisplayCreatorTest, ValuePrecision)
{
	DummyUIDescription uiDesc;
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrValuePrecision, 3, &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getPrecision () == 3; });
}

TEST_CASE (CParamDisplayCreatorTest, TextRotationMinMax)
{
	DummyUIDescription uidesc;
	testMinMaxValues (kCParamDisplay, kAttrTextRotation, &uidesc, 0., 360.);
}

TEST_CASE (CParamDisplayCreatorTest, BackgroundOffset)
{
	DummyUIDescription uiDesc;
	CPoint offset (20, 20);
	testAttribute<CParamDisplay> (kCParamDisplay, kAttrBackgroundOffset, offset, &uiDesc,
	                              [&] (CParamDisplay* v) { return v->getBackOffset () == offset; });
}

TEST_CASE (CParamDisplayCreatorTest, ShadowOffset)
{
	DummyUIDescription uiDesc;
	CPoint offset (15, 9);
	testAttribute<CParamDisplay> (
	    kCParamDisplay, kAttrTextShadowOffset, offset, &uiDesc,
	    [&] (CParamDisplay* v) { return v->getShadowTextOffset () == offset; });
}

} // VSTGUI
