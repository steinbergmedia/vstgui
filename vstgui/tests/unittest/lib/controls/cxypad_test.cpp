// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cxypad.h"
#include "../../unittests.h"

namespace VSTGUI {

TESTCASE(CXYPadTest,

	TEST(valueCalculation,
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
	);

	TEST(mouseLeftDownInteraction,
		CXYPad pad (CRect (0, 0, 100, 100));
		pad.setRoundRectRadius (0.f);
		CPoint p (0, 0);
		pad.onMouseDown (p, kLButton);
		p (10, 10);
		pad.onMouseMoved (p, kLButton);
		float x = 1.f;
		float y = 1.f;
		pad.calculateXY (pad.getValue (), x, y);
		EXPECT(x == 0.1f);
		EXPECT(y == 0.1f);
		p (110, 110);
		pad.onMouseMoved (p, kLButton);
		pad.calculateXY (pad.getValue (), x, y);
		EXPECT(x == 1.f);
		EXPECT(y == 1.f);
		p (-10, -10);
		pad.onMouseMoved (p, kLButton);
		pad.calculateXY (pad.getValue (), x, y);
		EXPECT(x == 0.f);
		EXPECT(y == 0.f);
		pad.onMouseUp (p, kLButton);
	);

	TEST(cancelMouseInteraction,
		CXYPad pad (CRect (0, 0, 100, 100));
		float startX;
		float startY;
		pad.calculateXY (pad.getValue (), startX, startY);
		CPoint p (0, 0);
		pad.onMouseDown (p, kLButton);
		p (10, 10);
		pad.onMouseMoved (p, kLButton);
		float x;
		float y;
		pad.calculateXY (pad.getValue (), x, y);
		EXPECT(startX != x);
		EXPECT(startY != y);
		pad.onMouseCancel ();
		pad.calculateXY (pad.getValue (), x, y);
		EXPECT(startX == x);
		EXPECT(startY == y);
	);

	TEST(otherMouseInteraction,
		CXYPad pad (CRect (0, 0, 100, 100));
		CPoint p (0, 0);
		EXPECT (pad.onMouseDown (p, kRButton) == kMouseEventNotHandled);
		EXPECT (pad.onMouseMoved (p, kRButton) == kMouseEventNotHandled);
		EXPECT (pad.onMouseUp (p, kRButton) == kMouseEventNotHandled);
	);

	TEST(stopTrackingOnMouseExit,
		CXYPad pad (CRect (0, 0, 100, 100));
		pad.setStopTrackingOnMouseExit (true);
		pad.setRoundRectRadius (0.f);
		CPoint p (0, 0);
		EXPECT (pad.onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (pad.isEditing ());
		p (50, 50);
		EXPECT (pad.onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (pad.isEditing ());
		p (150, 150);
		EXPECT (pad.onMouseMoved (p, kLButton) == kMouseMoveEventHandledButDontNeedMoreEvents);
		EXPECT (pad.isEditing () == false);
	);
	
	TEST(mouseWheel,
		CXYPad pad (CRect (0, 0, 100, 100));
		float x = 1.f;
		float y = 1.f;
		CXYPad::calculateXY (pad.getValue (), x, y);
		EXPECT(x == 0.f && y == 0.f);
		pad.onWheel (CPoint (1, 1), CMouseWheelAxis::kMouseWheelAxisX, 1.f, 0);
		CXYPad::calculateXY (pad.getValue (), x, y);
		EXPECT(x == pad.getWheelInc () && y == 0.f);
		pad.onWheel (CPoint (1, 1), CMouseWheelAxis::kMouseWheelAxisY, 1.f, 0);
		CXYPad::calculateXY (pad.getValue (), x, y);
		EXPECT(x == pad.getWheelInc () && y == pad.getWheelInc ());

	);
);

} // VSTGUI
