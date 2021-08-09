// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cxypad.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

TEST_CASE (CXYPadTest, valueCalculation)
{
	auto value = CXYPad::calculateValue (0.f, 0.f);
	float x = 1.f;
	float y = 1.f;
	CXYPad::calculateXY (value, x, y);
	EXPECT (x == 0.f);
	EXPECT (y == 0.f);
	value = CXYPad::calculateValue (1.f, 0.f);
	CXYPad::calculateXY (value, x, y);
	EXPECT (x == 1.f);
	EXPECT (y == 0.f);
	value = CXYPad::calculateValue (0.f, 1.f);
	CXYPad::calculateXY (value, x, y);
	EXPECT (x == 0.f);
	EXPECT (y == 1.f);
	value = CXYPad::calculateValue (0.5f, 0.5f);
	CXYPad::calculateXY (value, x, y);
	EXPECT (x == 0.5f);
	EXPECT (y == 0.5f);
	value = CXYPad::calculateValue (0.25f, 0.25f);
	CXYPad::calculateXY (value, x, y);
	EXPECT (x == 0.25f);
	EXPECT (y == 0.25f);
}

TEST_CASE (CXYPadTest, MouseLeftDownInteraction)
{
	CXYPad pad (CRect (0, 0, 100, 100));
	pad.setRoundRectRadius (0.f);

	dispatchMouseEvent<MouseDownEvent> (&pad, {0., 0.}, MouseButton::Left);
	dispatchMouseEvent<MouseMoveEvent> (&pad, {10., 10.}, MouseButton::Left);
	float x = 1.f;
	float y = 1.f;
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (x == 0.1f);
	EXPECT (y == 0.1f);
	dispatchMouseEvent<MouseMoveEvent> (&pad, {110., 110.}, MouseButton::Left);
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (x == 1.f);
	EXPECT (y == 1.f);
	dispatchMouseEvent<MouseMoveEvent> (&pad, {-10., -10.}, MouseButton::Left);
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (x == 0.f);
	EXPECT (y == 0.f);
	dispatchMouseEvent<MouseUpEvent> (&pad, {-10., -10.}, MouseButton::Left);
	EXPECT_FALSE(pad.isEditing ());
}

TEST_CASE (CXYPadTest, CancelMouseInteraction)
{
	CXYPad pad (CRect (0, 0, 100, 100));

	float startX {-1.f};
	float startY {-1.f};
	pad.calculateXY (pad.getValue (), startX, startY);
	dispatchMouseEvent<MouseDownEvent> (&pad, {0., 0.}, MouseButton::Left);
	dispatchMouseEvent<MouseMoveEvent> (&pad, {10., 10.}, MouseButton::Left);
	float x {-1.f};
	float y {-1.f};
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (startX != x);
	EXPECT (startY != y);
	dispatchMouseCancelEvent (&pad);
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (startX == x);
	EXPECT (startY == y);
}

TEST_CASE (CXYPadTest, OtherMouseInteraction)
{
	CXYPad pad (CRect (0, 0, 100, 100));

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&pad, {0., 0.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (&pad, {0., 0.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (&pad, {0., 0.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
}

TEST_CASE (CXYPadTest, StopTrackingOnMouseExit)
{
	CXYPad pad (CRect (0, 0, 100, 100));
	pad.setStopTrackingOnMouseExit (true);
	pad.setRoundRectRadius (0.f);

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&pad, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (pad.isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&pad, {50., 50.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (pad.isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&pad, {150., 150.}, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_FALSE (pad.isEditing ());
}

TEST_CASE (CXYPadTest, MouseWheel)
{
	CXYPad pad (CRect (0, 0, 100, 100));
	float x = 1.f;
	float y = 1.f;
	CXYPad::calculateXY (pad.getValue (), x, y);
	EXPECT (x == 0.f && y == 0.f);
	dispatchMouseWheelEvent (&pad, {1., 1.}, 1., 0.);
	CXYPad::calculateXY (pad.getValue (), x, y);
	EXPECT (x == pad.getWheelInc () && y == 0.f);
	dispatchMouseWheelEvent (&pad, {1., 1.}, 0., 1.);
	CXYPad::calculateXY (pad.getValue (), x, y);
	EXPECT (x == pad.getWheelInc () && y == pad.getWheelInc ());
}

} // VSTGUI
