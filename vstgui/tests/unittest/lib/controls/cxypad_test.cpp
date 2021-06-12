// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cxypad.h"
#include "../../../../lib/events.h"
#include "../../unittests.h"

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

	MouseDownEvent downEvent ({0., 0.}, {MouseEventButtonState::Left});
	pad.dispatchEvent (downEvent);
	MouseMoveEvent moveEvent ({10., 10.}, {MouseEventButtonState::Left});
	pad.dispatchEvent (moveEvent);
	float x = 1.f;
	float y = 1.f;
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (x == 0.1f);
	EXPECT (y == 0.1f);
	moveEvent.mousePosition = {110., 110.};
	pad.dispatchEvent (moveEvent);
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (x == 1.f);
	EXPECT (y == 1.f);
	moveEvent.mousePosition = {-10., -10.};
	pad.dispatchEvent (moveEvent);
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (x == 0.f);
	EXPECT (y == 0.f);
	MouseUpEvent upEvent ({-10., -10.}, {MouseEventButtonState::Left});
	pad.dispatchEvent (upEvent);
}

TEST_CASE (CXYPadTest, CancelMouseInteraction)
{
	CXYPad pad (CRect (0, 0, 100, 100));

	float startX {-1.f};
	float startY {-1.f};
	pad.calculateXY (pad.getValue (), startX, startY);
	MouseDownEvent downEvent ({0., 0.}, {MouseEventButtonState::Left});
	pad.dispatchEvent (downEvent);
	MouseMoveEvent moveEvent ({10., 10.}, {MouseEventButtonState::Left});
	pad.dispatchEvent (moveEvent);
	float x {-1.f};
	float y {-1.f};
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (startX != x);
	EXPECT (startY != y);
	MouseCancelEvent cancelEvent;
	pad.dispatchEvent (cancelEvent);
	pad.calculateXY (pad.getValue (), x, y);
	EXPECT (startX == x);
	EXPECT (startY == y);
}

TEST_CASE (CXYPadTest, OtherMouseInteraction)
{
	CXYPad pad (CRect (0, 0, 100, 100));
	
	MouseDownEvent downEvent ({0., 0.}, {MouseEventButtonState::Right});
	MouseMoveEvent moveEvent (downEvent.mousePosition, downEvent.buttonState);
	MouseUpEvent upEvent (downEvent.mousePosition, downEvent.buttonState);

	pad.dispatchEvent (downEvent);
	EXPECT_FALSE (downEvent.consumed);
	pad.dispatchEvent (moveEvent);
	EXPECT_FALSE (moveEvent.consumed);
	pad.dispatchEvent (upEvent);
	EXPECT_FALSE (upEvent.consumed);
}

TEST_CASE (CXYPadTest, StopTrackingOnMouseExit)
{
	CXYPad pad (CRect (0, 0, 100, 100));
	pad.setStopTrackingOnMouseExit (true);
	pad.setRoundRectRadius (0.f);

	MouseDownEvent downEvent ({0., 0.}, {MouseEventButtonState::Left});
	MouseMoveEvent moveEvent (downEvent.mousePosition, downEvent.buttonState);
	pad.dispatchEvent (downEvent);
	EXPECT_TRUE (downEvent.consumed);
	EXPECT_TRUE (pad.isEditing ());
	moveEvent.mousePosition = {50., 50.};
	pad.dispatchEvent (moveEvent);
	EXPECT_TRUE (moveEvent.consumed);
	EXPECT_TRUE (pad.isEditing ());
	moveEvent.consumed.reset ();
	moveEvent.mousePosition = {150., 150.};
	pad.dispatchEvent (moveEvent);
	EXPECT_TRUE (moveEvent.consumed);
	EXPECT_TRUE (moveEvent.ignoreFollowUpMoveAndUpEvents ());
	EXPECT_FALSE (pad.isEditing ());
}

TEST_CASE (CXYPadTest, MouseWheel)
{
	CXYPad pad (CRect (0, 0, 100, 100));
	float x = 1.f;
	float y = 1.f;
	CXYPad::calculateXY (pad.getValue (), x, y);
	EXPECT (x == 0.f && y == 0.f);
	MouseWheelEvent event;
	event.mousePosition = CPoint (1, 1);
	event.deltaX = 1.;
	pad.onMouseWheelEvent (event);
	CXYPad::calculateXY (pad.getValue (), x, y);
	EXPECT (x == pad.getWheelInc () && y == 0.f);
	event.deltaX = 0.;
	event.deltaY = 1.;
	pad.onMouseWheelEvent (event);
	CXYPad::calculateXY (pad.getValue (), x, y);
	EXPECT (x == pad.getWheelInc () && y == pad.getWheelInc ());
}

} // VSTGUI
