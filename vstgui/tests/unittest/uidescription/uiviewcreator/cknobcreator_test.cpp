// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/cbitmap.h"
#include "../../../../lib/cgradient.h"
#include "../../../../lib/controls/cknob.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CKnobCreatorTest, AngleStart)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrAngleStart, 20., &uidesc, [&] (CKnob* v) {
		return static_cast<int32_t> (v->getStartAngle () / Constants::pi * 180.) == 20;
	});
}

TEST_CASE (CKnobCreatorTest, AngleRange)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrAngleRange, 100., &uidesc, [&] (CKnob* v) {
		return static_cast<int32_t> (v->getRangeAngle () / Constants::pi * 180.) == 100;
	});
}

TEST_CASE (CKnobCreatorTest, ValueInset)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrValueInset, 10., &uidesc,
	                      [&] (CKnob* v) { return v->getInsetValue () == 10.; });
}

TEST_CASE (CKnobCreatorTest, CoronaInset)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaInset, 10., &uidesc,
	                      [&] (CKnob* v) { return v->getCoronaInset () == 10.; });
}

TEST_CASE (CKnobCreatorTest, ZoomFactor)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrZoomFactor, 10., &uidesc,
	                      [&] (CKnob* v) { return v->getZoomFactor () == 10.; });
}

TEST_CASE (CKnobCreatorTest, HandleLineWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrHandleLineWidth, 10., &uidesc,
	                      [&] (CKnob* v) { return v->getHandleLineWidth () == 10.; });
}

TEST_CASE (CKnobCreatorTest, CoronaColor)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaColor, kColorName, &uidesc,
	                      [&] (CKnob* v) { return v->getCoronaColor () == uidesc.color; });
}

TEST_CASE (CKnobCreatorTest, HandleShadowColor)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrHandleShadowColor, kColorName, &uidesc,
	                      [&] (CKnob* v) { return v->getColorShadowHandle () == uidesc.color; });
}

TEST_CASE (CKnobCreatorTest, HandleColor)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrHandleColor, kColorName, &uidesc,
	                      [&] (CKnob* v) { return v->getColorHandle () == uidesc.color; });
}

TEST_CASE (CKnobCreatorTest, HandleBitmap)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrHandleBitmap, kBitmapName, &uidesc,
	                      [&] (CKnob* v) { return v->getHandleBitmap () == uidesc.bitmap; });
}

TEST_CASE (CKnobCreatorTest, CircleDrawing)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCircleDrawing, true, &uidesc, [&] (CKnob* v) {
		return v->getDrawStyle () & CKnob::kHandleCircleDrawing;
	});
	testAttribute<CKnob> (kCKnob, kAttrCircleDrawing, false, &uidesc, [&] (CKnob* v) {
		return !(v->getDrawStyle () & CKnob::kHandleCircleDrawing);
	});
}

TEST_CASE (CKnobCreatorTest, CoronaDrawing)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaDrawing, true, &uidesc,
	                      [&] (CKnob* v) { return v->getDrawStyle () & CKnob::kCoronaDrawing; });
	testAttribute<CKnob> (kCKnob, kAttrCoronaDrawing, false, &uidesc,
	                      [&] (CKnob* v) { return !(v->getDrawStyle () & CKnob::kCoronaDrawing); });
}

TEST_CASE (CKnobCreatorTest, CoronaFromCenter)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaFromCenter, true, &uidesc,
	                      [&] (CKnob* v) { return v->getDrawStyle () & CKnob::kCoronaFromCenter; });
	testAttribute<CKnob> (kCKnob, kAttrCoronaFromCenter, false, &uidesc, [&] (CKnob* v) {
		return !(v->getDrawStyle () & CKnob::kCoronaFromCenter);
	});
}

TEST_CASE (CKnobCreatorTest, CoronaInverted)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaInverted, true, &uidesc,
	                      [&] (CKnob* v) { return v->getDrawStyle () & CKnob::kCoronaInverted; });
	testAttribute<CKnob> (kCKnob, kAttrCoronaInverted, false, &uidesc, [&] (CKnob* v) {
		return !(v->getDrawStyle () & CKnob::kCoronaInverted);
	});
}

TEST_CASE (CKnobCreatorTest, CoronaDashDot)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaDashDot, true, &uidesc, [&] (CKnob* v) {
		return v->getDrawStyle () & CKnob::kCoronaLineDashDot;
	});
	testAttribute<CKnob> (kCKnob, kAttrCoronaDashDot, false, &uidesc, [&] (CKnob* v) {
		return !(v->getDrawStyle () & CKnob::kCoronaLineDashDot);
	});
}

TEST_CASE (CKnobCreatorTest, CoronaOutline)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaOutline, true, &uidesc,
	                      [&] (CKnob* v) { return v->getDrawStyle () & CKnob::kCoronaOutline; });
	testAttribute<CKnob> (kCKnob, kAttrCoronaOutline, false, &uidesc,
	                      [&] (CKnob* v) { return !(v->getDrawStyle () & CKnob::kCoronaOutline); });
}

TEST_CASE (CKnobCreatorTest, CoronaLineCapButt)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaLineCapButt, true, &uidesc, [&] (CKnob* v) {
		return v->getDrawStyle () & CKnob::kCoronaLineCapButt;
	});
	testAttribute<CKnob> (kCKnob, kAttrCoronaLineCapButt, false, &uidesc, [&] (CKnob* v) {
		return !(v->getDrawStyle () & CKnob::kCoronaLineCapButt);
	});
}

TEST_CASE (CKnobCreatorTest, SkipHandleDrawing)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrSkipHandleDrawing, true, &uidesc, [&] (CKnob* v) {
		return v->getDrawStyle () & CKnob::kSkipHandleDrawing;
	});
	testAttribute<CKnob> (kCKnob, kAttrSkipHandleDrawing, false, &uidesc, [&] (CKnob* v) {
		return !(v->getDrawStyle () & CKnob::kSkipHandleDrawing);
	});
}

TEST_CASE (CKnobCreatorTest, CoronaOutlineWithAdd)
{
	DummyUIDescription uidesc;
	testAttribute<CKnob> (kCKnob, kAttrCoronaOutlineWidthAdd, 10., &uidesc,
	                      [&] (CKnob* v) { return v->getCoronaOutlineWidthAdd () == 10.; });
}

} // VSTGUI
