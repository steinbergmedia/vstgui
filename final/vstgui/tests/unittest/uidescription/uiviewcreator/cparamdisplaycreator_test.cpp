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
#include "../../../../lib/controls/cparamdisplay.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CParamDisplayCreatorTest,

	TEST(font,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrFont, kFontName, &uiDesc, [&] (CParamDisplay* v) {
			return v->getFont () == uiDesc.font;
		}, true);
	);

	TEST(fontColor,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrFontColor, kColorName, &uiDesc, [&] (CParamDisplay* v) {
			return v->getFontColor () == uiDesc.color;
		});
	);

	TEST(backColor,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrBackColor, kColorName, &uiDesc, [&] (CParamDisplay* v) {
			return v->getBackColor () == uiDesc.color;
		});
	);

	TEST(frameColor,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrFrameColor, kColorName, &uiDesc, [&] (CParamDisplay* v) {
			return v->getFrameColor () == uiDesc.color;
		});
	);

	TEST(shadowColor,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrShadowColor, kColorName, &uiDesc, [&] (CParamDisplay* v) {
			return v->getShadowColor () == uiDesc.color;
		});
	);

	TEST(textInset,
		DummyUIDescription uiDesc;
		CPoint inset (5, 6);
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrTextInset, inset, &uiDesc, [&] (CParamDisplay* v) {
			return v->getTextInset () == inset;
		});
	);

	TEST(fontAntialias,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrFontAntialias, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getAntialias ();
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrFontAntialias, false, &uiDesc, [&] (CParamDisplay* v) {
			return v->getAntialias () == false;
		});
	);

	TEST(textAlignment,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrTextAlignment, "left", &uiDesc, [&] (CParamDisplay* v) {
			return v->getHoriAlign () == kLeftText;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrTextAlignment, "center", &uiDesc, [&] (CParamDisplay* v) {
			return v->getHoriAlign () == kCenterText;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrTextAlignment, "right", &uiDesc, [&] (CParamDisplay* v) {
			return v->getHoriAlign () == kRightText;
		});
	);

	TEST(roundRectRadius,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrRoundRectRadius, 15., &uiDesc, [&] (CParamDisplay* v) {
			return v->getRoundRectRadius () == 15.;
		});
	);

	TEST(frameWidth,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrFrameWidth, 12., &uiDesc, [&] (CParamDisplay* v) {
			return v->getFrameWidth () == 12.;
		});
	);

	TEST(textRotation,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrTextRotation, 89., &uiDesc, [&] (CParamDisplay* v) {
			return v->getTextRotation () == 89.;
		});
	);

	TEST(styles,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyle3DIn, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & k3DIn;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyle3DOut, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & k3DOut;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyleNoFrame, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & kNoFrame;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyleNoDraw, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & kNoDrawStyle;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyleNoText, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & kNoTextStyle;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyleShadowText, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & kShadowText;
		});
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrStyleRoundRect, true, &uiDesc, [&] (CParamDisplay* v) {
			return v->getStyle() & kRoundRectStyle;
		});
	);

	TEST(valuePrecision,
		DummyUIDescription uiDesc;
		testAttribute<CParamDisplay>(kCParamDisplay, kAttrValuePrecision, 3, &uiDesc, [&] (CParamDisplay* v) {
			return v->getPrecision () == 3;
		});
	);

	TEST(textRotationMinMax,
		DummyUIDescription uidesc;
		testMinMaxValues(kCParamDisplay, kAttrTextRotation, &uidesc, 0., 360.);
	);

);

} // VSTGUI
