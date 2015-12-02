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
);

} // VSTGUI
