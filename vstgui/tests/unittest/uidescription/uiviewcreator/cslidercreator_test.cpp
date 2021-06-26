// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cslider.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CSliderCreatorTest, Mode)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrMode, "touch", &uidesc,
	                        [&] (CSlider* v) { return v->getSliderMode () == CSliderMode::Touch; });
	testAttribute<CSlider> (kCSlider, kAttrMode, "relative touch", &uidesc, [&] (CSlider* v) {
		return v->getSliderMode () == CSliderMode::RelativeTouch;
	});
	testAttribute<CSlider> (kCSlider, kAttrMode, "free click", &uidesc, [&] (CSlider* v) {
		return v->getSliderMode () == CSliderMode::FreeClick;
	});
	testAttribute<CSlider> (kCSlider, kAttrMode, "ramp", &uidesc,
	                        [&] (CSlider* v) { return v->getSliderMode () == CSliderMode::Ramp; });
	testAttribute<CSlider> (kCSlider, kAttrMode, "use global", &uidesc, [&] (CSlider* v) {
		return v->getSliderMode () == CSliderMode::UseGlobal;
	});
}

TEST_CASE (CSliderCreatorTest, HandleBitmap)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrHandleBitmap, kBitmapName, &uidesc,
	                        [&] (CSlider* v) { return v->getHandle () == uidesc.bitmap; });
}

TEST_CASE (CSliderCreatorTest, HandleOffset)
{
	DummyUIDescription uidesc;
	CPoint p (20, 20);
	testAttribute<CSlider> (kCSlider, kAttrHandleOffset, p, &uidesc,
	                        [&] (CSlider* v) { return v->getOffsetHandle () == p; });
}

TEST_CASE (CSliderCreatorTest, BitmapOffset)
{
	DummyUIDescription uidesc;
	CPoint p (20, 20);
	testAttribute<CSlider> (kCSlider, kAttrBitmapOffset, p, &uidesc,
	                        [&] (CSlider* v) { return v->getBackgroundOffset () == p; });
}

TEST_CASE (CSliderCreatorTest, ZoomFactor)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrZoomFactor, 15., &uidesc,
	                        [&] (CSlider* v) { return v->getZoomFactor () == 15.; });
}

TEST_CASE (CSliderCreatorTest, Orientation)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrOrientation, "horizontal", &uidesc,
	                        [&] (CSlider* v) { return v->getStyle () & CSlider::kHorizontal; });
	testAttribute<CSlider> (kCSlider, kAttrOrientation, "vertical", &uidesc,
	                        [&] (CSlider* v) { return v->getStyle () & CSlider::kVertical; });
}

TEST_CASE (CSliderCreatorTest, ReverseOrientation)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrReverseOrientation, true, &uidesc, [&] (CSlider* v) {
		auto style = v->getStyle ();
		if (style & CSlider::kHorizontal)
			return style & CSlider::kRight;
		return style & CSlider::kTop;
	});
	testAttribute<CSlider> (kCSlider, kAttrReverseOrientation, false, &uidesc, [&] (CSlider* v) {
		auto style = v->getStyle ();
		if (style & CSlider::kHorizontal)
			return style & CSlider::kLeft;
		return style & CSlider::kBottom;
	});
}

TEST_CASE (CSliderCreatorTest, DrawFrame)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawFrame, true, &uidesc,
	                        [&] (CSlider* v) { return v->getDrawStyle () & CSlider::kDrawFrame; });
	testAttribute<CSlider> (kCSlider, kAttrDrawFrame, false, &uidesc, [&] (CSlider* v) {
		return !(v->getDrawStyle () & CSlider::kDrawFrame);
	});
}

TEST_CASE (CSliderCreatorTest, DrawBack)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawBack, true, &uidesc,
	                        [&] (CSlider* v) { return v->getDrawStyle () & CSlider::kDrawBack; });
	testAttribute<CSlider> (kCSlider, kAttrDrawBack, false, &uidesc, [&] (CSlider* v) {
		return !(v->getDrawStyle () & CSlider::kDrawBack);
	});
}

TEST_CASE (CSliderCreatorTest, DrawValue)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawValue, true, &uidesc,
	                        [&] (CSlider* v) { return v->getDrawStyle () & CSlider::kDrawValue; });
	testAttribute<CSlider> (kCSlider, kAttrDrawValue, false, &uidesc, [&] (CSlider* v) {
		return !(v->getDrawStyle () & CSlider::kDrawValue);
	});
}

TEST_CASE (CSliderCreatorTest, DrawValueFromCenter)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawValueFromCenter, true, &uidesc, [&] (CSlider* v) {
		return v->getDrawStyle () & CSlider::kDrawValueFromCenter;
	});
	testAttribute<CSlider> (kCSlider, kAttrDrawValueFromCenter, false, &uidesc, [&] (CSlider* v) {
		return !(v->getDrawStyle () & CSlider::kDrawValueFromCenter);
	});
}

TEST_CASE (CSliderCreatorTest, DrawValueInverted)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawValueInverted, true, &uidesc, [&] (CSlider* v) {
		return v->getDrawStyle () & CSlider::kDrawInverted;
	});
	testAttribute<CSlider> (kCSlider, kAttrDrawValueInverted, false, &uidesc, [&] (CSlider* v) {
		return !(v->getDrawStyle () & CSlider::kDrawInverted);
	});
}

TEST_CASE (CSliderCreatorTest, FrameColor)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawFrameColor, kColorName, &uidesc,
	                        [&] (CSlider* v) { return v->getFrameColor () == uidesc.color; });
}

TEST_CASE (CSliderCreatorTest, BackColor)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawBackColor, kColorName, &uidesc,
	                        [&] (CSlider* v) { return v->getBackColor () == uidesc.color; });
}

TEST_CASE (CSliderCreatorTest, ValueColor)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrDrawValueColor, kColorName, &uidesc,
	                        [&] (CSlider* v) { return v->getValueColor () == uidesc.color; });
}

TEST_CASE (CSliderCreatorTest, OrientationValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCSlider, kAttrOrientation, &uidesc, {"horizontal", "vertical"});
}

TEST_CASE (CSliderCreatorTest, ModeValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCSlider, kAttrMode, &uidesc,
	                    {"touch", "relative touch", "free click", "ramp", "use global"});
}

TEST_CASE (CSliderCreatorTest, FrameWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CSlider> (kCSlider, kAttrFrameWidth, 10, &uidesc,
	                        [&] (CSlider* v) { return v->getFrameWidth () == 10; });
}

} // VSTGUI
