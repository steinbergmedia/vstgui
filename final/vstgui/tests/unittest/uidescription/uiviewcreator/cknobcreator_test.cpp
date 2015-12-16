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
#include "../../../../lib/controls/cknob.h"
#include "../../../../lib/cstring.h"
#include "../../../../lib/cbitmap.h"
#include "../../../../lib/cgradient.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CKnobCreatorTest,

	TEST(angleStart,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrAngleStart, 20., &uidesc, [&] (CKnob* v) {
			return static_cast<int32_t> (v->getStartAngle () / kPI * 180.) == 20;
		});
	);

	TEST(angleRange,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrAngleRange, 100., &uidesc, [&] (CKnob* v) {
			return static_cast<int32_t> (v->getRangeAngle () / kPI * 180.) == 100;
		});
	);

	TEST(valueInset,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrValueInset, 10., &uidesc, [&] (CKnob* v) {
			return v->getInsetValue () == 10.;
		});
	);

	TEST(coronaInset,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaInset, 10., &uidesc, [&] (CKnob* v) {
			return v->getCoronaInset () == 10.;
		});
	);

	TEST(zoomFactor,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrZoomFactor, 10., &uidesc, [&] (CKnob* v) {
			return v->getZoomFactor () == 10.;
		});
	);

	TEST(handleLineWidth,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrHandleLineWidth, 10., &uidesc, [&] (CKnob* v) {
			return v->getHandleLineWidth () == 10.;
		});
	);

	TEST(coronaColor,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaColor, kColorName, &uidesc, [&] (CKnob* v) {
			return v->getCoronaColor () == uidesc.color;
		});
	);

	TEST(handleShadowColor,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrHandleShadowColor, kColorName, &uidesc, [&] (CKnob* v) {
			return v->getColorShadowHandle () == uidesc.color;
		});
	);

	TEST(handleColor,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrHandleColor, kColorName, &uidesc, [&] (CKnob* v) {
			return v->getColorHandle () == uidesc.color;
		});
	);

	TEST(handleBitmap,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrHandleBitmap, kBitmapName, &uidesc, [&] (CKnob* v) {
			return v->getHandleBitmap () == uidesc.bitmap;
		});
	);

	TEST(circleDrawing,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCircleDrawing, true, &uidesc, [&] (CKnob* v) {
			return v->getDrawStyle () & CKnob::kHandleCircleDrawing;
		});
		testAttribute<CKnob>(kCKnob, kAttrCircleDrawing, false, &uidesc, [&] (CKnob* v) {
			return !(v->getDrawStyle () & CKnob::kHandleCircleDrawing);
		});
	);

	TEST(coronaDrawing,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaDrawing, true, &uidesc, [&] (CKnob* v) {
			return v->getDrawStyle() & CKnob::kCoronaDrawing;
		});
		testAttribute<CKnob>(kCKnob, kAttrCoronaDrawing, false, &uidesc, [&] (CKnob* v) {
			return !(v->getDrawStyle() & CKnob::kCoronaDrawing);
		});
	);

	TEST(coronaFromCenter,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaFromCenter, true, &uidesc, [&] (CKnob* v) {
			return v->getDrawStyle() & CKnob::kCoronaFromCenter;
		});
		testAttribute<CKnob>(kCKnob, kAttrCoronaFromCenter, false, &uidesc, [&] (CKnob* v) {
			return !(v->getDrawStyle() & CKnob::kCoronaFromCenter);
		});
	);

	TEST(coronaInverted,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaInverted, true, &uidesc, [&] (CKnob* v) {
			return v->getDrawStyle() & CKnob::kCoronaInverted;
		});
		testAttribute<CKnob>(kCKnob, kAttrCoronaInverted, false, &uidesc, [&] (CKnob* v) {
			return !(v->getDrawStyle() & CKnob::kCoronaInverted);
		});
	);

	TEST(coronaDashDot,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaDashDot, true, &uidesc, [&] (CKnob* v) {
			return v->getDrawStyle() & CKnob::kCoronaLineDashDot;
		});
		testAttribute<CKnob>(kCKnob, kAttrCoronaDashDot, false, &uidesc, [&] (CKnob* v) {
			return !(v->getDrawStyle() & CKnob::kCoronaLineDashDot);
		});
	);

	TEST(coronaOutline,
		DummyUIDescription uidesc;
		testAttribute<CKnob>(kCKnob, kAttrCoronaOutline, true, &uidesc, [&] (CKnob* v) {
			return v->getDrawStyle() & CKnob::kCoronaOutline;
		});
		testAttribute<CKnob>(kCKnob, kAttrCoronaOutline, false, &uidesc, [&] (CKnob* v) {
			return !(v->getDrawStyle() & CKnob::kCoronaOutline);
		});
	);
);

} // VSTGUI
