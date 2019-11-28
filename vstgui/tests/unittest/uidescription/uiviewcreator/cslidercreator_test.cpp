// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/controls/cslider.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CSliderCreatorTest,

	TEST(mode,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrMode, "touch", &uidesc, [&] (CSlider* v) {
			return v->getSliderMode () == CSliderMode::Touch;
		});
		testAttribute<CSlider>(kCSlider, kAttrMode, "relative touch", &uidesc, [&] (CSlider* v) {
			return v->getSliderMode () == CSliderMode::RelativeTouch;
		});
		testAttribute<CSlider>(kCSlider, kAttrMode, "free click", &uidesc, [&] (CSlider* v) {
			return v->getSliderMode () == CSliderMode::FreeClick;
		});
		testAttribute<CSlider>(kCSlider, kAttrMode, "ramp", &uidesc, [&] (CSlider* v) {
			return v->getSliderMode () == CSliderMode::Ramp;
		});
		testAttribute<CSlider>(kCSlider, kAttrMode, "use global", &uidesc, [&] (CSlider* v) {
			return v->getSliderMode () == CSliderMode::UseGlobal;
		});
	);

	TEST(handleBitmap,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrHandleBitmap, kBitmapName, &uidesc, [&] (CSlider* v) {
			return v->getHandle () == uidesc.bitmap;
		});
	);

	TEST(handleOffset,
		DummyUIDescription uidesc;
		CPoint p (20, 20);
		testAttribute<CSlider>(kCSlider, kAttrHandleOffset, p, &uidesc, [&] (CSlider* v) {
			return v->getOffsetHandle () == p;
		});
	);

	TEST(bitmapOffset,
		DummyUIDescription uidesc;
		CPoint p (20, 20);
		testAttribute<CSlider>(kCSlider, kAttrBitmapOffset, p, &uidesc, [&] (CSlider* v) {
			return v->getBackgroundOffset () == p;
		});
	);

	TEST(zoomFactor,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrZoomFactor, 15., &uidesc, [&] (CSlider* v) {
			return v->getZoomFactor () == 15.;
		});
	);

	TEST(orientation,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrOrientation, "horizontal", &uidesc, [&] (CSlider* v) {
			return v->getStyle() & CSlider::kHorizontal;
		});
		testAttribute<CSlider>(kCSlider, kAttrOrientation, "vertical", &uidesc, [&] (CSlider* v) {
			return v->getStyle() & CSlider::kVertical;
		});
	);

	TEST(reverseOrientation,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrReverseOrientation, true, &uidesc, [&] (CSlider* v) {
			auto style = v->getStyle ();
			if (style & CSlider::kHorizontal)
				return style & CSlider::kRight;
			return style & CSlider::kTop;
		});
		testAttribute<CSlider>(kCSlider, kAttrReverseOrientation, false, &uidesc, [&] (CSlider* v) {
			auto style = v->getStyle ();
			if (style & CSlider::kHorizontal)
				return style & CSlider::kLeft;
			return style & CSlider::kBottom;
		});
	);

	TEST(drawFrame,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawFrame, true, &uidesc, [&] (CSlider* v) {
			return v->getDrawStyle() & CSlider::kDrawFrame;
		});
		testAttribute<CSlider>(kCSlider, kAttrDrawFrame, false, &uidesc, [&] (CSlider* v) {
			return !(v->getDrawStyle() & CSlider::kDrawFrame);
		});
	);

	TEST(drawBack,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawBack, true, &uidesc, [&] (CSlider* v) {
			return v->getDrawStyle() & CSlider::kDrawBack;
		});
		testAttribute<CSlider>(kCSlider, kAttrDrawBack, false, &uidesc, [&] (CSlider* v) {
			return !(v->getDrawStyle() & CSlider::kDrawBack);
		});
	);

	TEST(drawValue,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawValue, true, &uidesc, [&] (CSlider* v) {
			return v->getDrawStyle() & CSlider::kDrawValue;
		});
		testAttribute<CSlider>(kCSlider, kAttrDrawValue, false, &uidesc, [&] (CSlider* v) {
			return !(v->getDrawStyle() & CSlider::kDrawValue);
		});
	);

	TEST(drawValueFromCenter,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawValueFromCenter, true, &uidesc, [&] (CSlider* v) {
			return v->getDrawStyle() & CSlider::kDrawValueFromCenter;
		});
		testAttribute<CSlider>(kCSlider, kAttrDrawValueFromCenter, false, &uidesc, [&] (CSlider* v) {
			return !(v->getDrawStyle() & CSlider::kDrawValueFromCenter);
		});
	);

	TEST(drawValueInverted,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawValueInverted, true, &uidesc, [&] (CSlider* v) {
			return v->getDrawStyle() & CSlider::kDrawInverted;
		});
		testAttribute<CSlider>(kCSlider, kAttrDrawValueInverted, false, &uidesc, [&] (CSlider* v) {
			return !(v->getDrawStyle() & CSlider::kDrawInverted);
		});
	);

	TEST(frameColor,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawFrameColor, kColorName, &uidesc, [&] (CSlider* v) {
			return v->getFrameColor() == uidesc.color;
		});
	);

	TEST(backColor,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawBackColor, kColorName, &uidesc, [&] (CSlider* v) {
			return v->getBackColor() == uidesc.color;
		});
	);

	TEST(valueColor,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrDrawValueColor, kColorName, &uidesc, [&] (CSlider* v) {
			return v->getValueColor() == uidesc.color;
		});
	);

	TEST(orientationValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCSlider, kAttrOrientation, &uidesc, {"horizontal", "vertical"});
	);

	TEST(modeValues,
		DummyUIDescription uidesc;
		testPossibleValues (kCSlider, kAttrMode, &uidesc, {"touch", "relative touch", "free click", "ramp", "use global"});
	);

	TEST(frameWidth,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrFrameWidth, 10, &uidesc, [&] (CSlider* v) {
			return v->getFrameWidth() == 10;
		});
	);

);

} // VSTGUI
