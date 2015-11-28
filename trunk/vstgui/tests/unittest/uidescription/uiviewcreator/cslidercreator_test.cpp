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
#include "../../../../lib/controls/cslider.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CSliderCreatorTest,

	TEST(mode,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrMode, "touch", &uidesc, [&] (CSlider* v) {
			return v->getMode() == CSlider::kTouchMode;
		});
		testAttribute<CSlider>(kCSlider, kAttrMode, "relative touch", &uidesc, [&] (CSlider* v) {
			return v->getMode() == CSlider::kRelativeTouchMode;
		});
		testAttribute<CSlider>(kCSlider, kAttrMode, "free click", &uidesc, [&] (CSlider* v) {
			return v->getMode() == CSlider::kFreeClickMode;
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
			return v->getOffset () == p;
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
			return v->getStyle() & kHorizontal;
		});
		testAttribute<CSlider>(kCSlider, kAttrOrientation, "vertical", &uidesc, [&] (CSlider* v) {
			return v->getStyle() & kVertical;
		});
	);

	TEST(transparentHandle,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrTransparentHandle, true, &uidesc, [&] (CSlider* v) {
			return v->getDrawTransparentHandle ();
		});
		testAttribute<CSlider>(kCSlider, kAttrTransparentHandle, false, &uidesc, [&] (CSlider* v) {
			return !v->getDrawTransparentHandle ();
		});
	);

	TEST(reverseOrientation,
		DummyUIDescription uidesc;
		testAttribute<CSlider>(kCSlider, kAttrReverseOrientation, true, &uidesc, [&] (CSlider* v) {
			auto style = v->getStyle ();
			if (style & kHorizontal)
				return style & kRight;
			return style & kTop;
		});
		testAttribute<CSlider>(kCSlider, kAttrReverseOrientation, false, &uidesc, [&] (CSlider* v) {
			auto style = v->getStyle ();
			if (style & kHorizontal)
				return style & kLeft;
			return style & kBottom;
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
		testPossibleValues (kCSlider, kAttrMode, &uidesc, {"touch", "relative touch", "free click"});
	);

);

} // VSTGUI
