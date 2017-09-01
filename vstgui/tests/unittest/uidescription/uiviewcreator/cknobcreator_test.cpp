// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
