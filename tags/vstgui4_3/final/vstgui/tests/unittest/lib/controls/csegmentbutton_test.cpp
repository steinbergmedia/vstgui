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

#include "../../../../lib/controls/csegmentbutton.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../unittests.h"

namespace VSTGUI {

TESTCASE(CSegmentButtonTest,

	TEST(addSegment,
		CSegmentButton b (CRect (0, 0, 10, 10));
		EXPECT (b.getSegments().size() == 0);
		CSegmentButton::Segment s;
		s.name = "1";
		b.addSegment (s);
		EXPECT (b.getSegments().size() == 1);
		s.name = "2";
		b.addSegment (s);
		EXPECT (b.getSegments().size() == 2);
		EXPECT (b.getSegments()[1].name == "2");
		s.name = "3";
		b.addSegment(s, 0);
		EXPECT (b.getSegments().size() == 3);
		EXPECT (b.getSegments()[0].name == "3");
		EXPECT (b.getSegments()[1].name == "1");
		EXPECT (b.getSegments()[2].name == "2");
	);

	TEST(removeSegment,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment(s);
		s.name = "1";
		b.addSegment(s);
		s.name = "2";
		b.addSegment(s);
		s.name = "3";
		b.addSegment(s);
		EXPECT (b.getSegments().size() == 4);
		b.removeSegment (0);
		EXPECT (b.getSegments().size() == 3);
		EXPECT (b.getSegments()[0].name == "1");
		EXPECT (b.getSegments()[1].name == "2");
		EXPECT (b.getSegments()[2].name == "3");
	);
	
	TEST(selectedSegment,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment(s);
		s.name = "1";
		b.addSegment(s);
		s.name = "2";
		b.addSegment(s);
		s.name = "3";
		b.addSegment(s);
		b.setSelectedSegment (1);
		EXPECT (b.getSelectedSegment () == 1);
		b.setSelectedSegment (2);
		EXPECT (b.getSelectedSegment () == 2);
		b.setSelectedSegment (3);
		EXPECT (b.getSelectedSegment () == 3);
		b.setSelectedSegment (4);
		EXPECT (b.getSelectedSegment () == 3);
		b.setSelectedSegment (0);
		EXPECT (b.getSelectedSegment () == 0);
	);
	
	TEST(rightKeyEvent,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment(s);
		s.name = "1";
		b.addSegment(s);
		s.name = "2";
		b.addSegment(s);

		b.setStyle (CSegmentButton::kHorizontal);
		b.setSelectedSegment (0);
		VstKeyCode keycode {};
		keycode.virt = VKEY_RIGHT;
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 2);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 2);

		b.setSelectedSegment (1);
		b.setStyle (CSegmentButton::kVertical);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
	);

	TEST(leftKeyEvent,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment(s);
		s.name = "1";
		b.addSegment(s);
		s.name = "2";
		b.addSegment(s);

		b.setStyle (CSegmentButton::kHorizontal);
		b.setSelectedSegment (2);
		VstKeyCode keycode {};
		keycode.virt = VKEY_LEFT;
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 0);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 0);

		b.setSelectedSegment (1);
		b.setStyle (CSegmentButton::kVertical);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
	);

	TEST(downKeyEvent,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment(s);
		s.name = "1";
		b.addSegment(s);
		s.name = "2";
		b.addSegment(s);

		b.setStyle (CSegmentButton::kVertical);
		b.setSelectedSegment (0);
		VstKeyCode keycode {};
		keycode.virt = VKEY_DOWN;
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 2);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 2);

		b.setSelectedSegment (1);
		b.setStyle (CSegmentButton::kHorizontal);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
	);

	TEST(upKeyEvent,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment(s);
		s.name = "1";
		b.addSegment(s);
		s.name = "2";
		b.addSegment(s);

		b.setStyle (CSegmentButton::kVertical);
		b.setSelectedSegment (2);
		VstKeyCode keycode {};
		keycode.virt = VKEY_UP;
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 0);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 0);

		b.setSelectedSegment (1);
		b.setStyle (CSegmentButton::kHorizontal);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
	);

	TEST(horizontalSegmentSizeCalculation,
		const auto numSegments = 5;
		CRect r (0, 0, 100, 100);
		auto b = new CSegmentButton (r);
		b->setStyle (CSegmentButton::kHorizontal);
		for (auto i = 0; i < numSegments; ++i)
			b->addSegment ({});
		for (const auto& s : b->getSegments())
			EXPECT (s.rect == CRect (0, 0, 0, 0));
		auto root = owned (new CViewContainer (r));
		auto parent = new CViewContainer (r);
		root->addView (parent);
		parent->addView (b);
		parent->attached (root);
		EXPECT (b->getSegments ()[0].rect == CRect (0, 0, 20, 100));
		EXPECT (b->getSegments ()[1].rect == CRect (20, 0, 40, 100));
		EXPECT (b->getSegments ()[2].rect == CRect (40, 0, 60, 100));
		EXPECT (b->getSegments ()[3].rect == CRect (60, 0, 80, 100));
		EXPECT (b->getSegments ()[4].rect == CRect (80, 0, 100, 100));
		parent->removed (root);
	);
	
	TEST(verticalSegmentSizeCalculation,
		const auto numSegments = 5;
		CRect r (0, 0, 100, 100);
		auto b = new CSegmentButton (r);
		b->setStyle (CSegmentButton::kVertical);
		for (auto i = 0; i < numSegments; ++i)
			b->addSegment ({});
		for (const auto& s : b->getSegments())
			EXPECT (s.rect == CRect (0, 0, 0, 0));
		auto root = owned (new CViewContainer (r));
		auto parent = new CViewContainer (r);
		root->addView (parent);
		parent->addView (b);
		parent->attached (root);
		EXPECT (b->getSegments ()[0].rect == CRect (0, 0, 100, 20));
		EXPECT (b->getSegments ()[1].rect == CRect (0, 20, 100, 40));
		EXPECT (b->getSegments ()[2].rect == CRect (0, 40, 100, 60));
		EXPECT (b->getSegments ()[3].rect == CRect (0, 60, 100, 80));
		EXPECT (b->getSegments ()[4].rect == CRect (0, 80, 100, 100));
		parent->removed (root);
	);

	TEST(updateViewSize,
		const auto numSegments = 5;
		auto b = new CSegmentButton (CRect (0, 0, 50, 100));
		b->setStyle (CSegmentButton::kHorizontal);
		for (auto i = 0; i < numSegments; ++i)
			b->addSegment ({});
		for (const auto& s : b->getSegments())
			EXPECT (s.rect == CRect (0, 0, 0, 0));
		CRect r (0, 0, 100, 100);
		auto root = owned (new CViewContainer (r));
		auto parent = new CViewContainer (r);
		root->addView (parent);
		parent->addView (b);
		parent->attached (root);
		EXPECT (b->getSegments ()[0].rect == CRect (0, 0, 10, 100));
		EXPECT (b->getSegments ()[1].rect == CRect (10, 0, 20, 100));
		EXPECT (b->getSegments ()[2].rect == CRect (20, 0, 30, 100));
		EXPECT (b->getSegments ()[3].rect == CRect (30, 0, 40, 100));
		EXPECT (b->getSegments ()[4].rect == CRect (40, 0, 50, 100));
		b->setViewSize (r);
		EXPECT (b->getSegments ()[0].rect == CRect (0, 0, 20, 100));
		EXPECT (b->getSegments ()[1].rect == CRect (20, 0, 40, 100));
		EXPECT (b->getSegments ()[2].rect == CRect (40, 0, 60, 100));
		EXPECT (b->getSegments ()[3].rect == CRect (60, 0, 80, 100));
		EXPECT (b->getSegments ()[4].rect == CRect (80, 0, 100, 100));
		parent->removed (root);
	);

	TEST(mouseDownEvent,
		const auto numSegments = 5;
		CRect r (0, 0, 100, 100);
		auto b = new CSegmentButton (r);
		b->setStyle (CSegmentButton::kHorizontal);
		for (auto i = 0; i < numSegments; ++i)
			b->addSegment ({});
		for (const auto& s : b->getSegments())
			EXPECT (s.rect == CRect (0, 0, 0, 0));
		auto root = owned (new CViewContainer (r));
		auto parent = new CViewContainer (r);
		root->addView (parent);
		parent->addView (b);
		parent->attached (root);
		CPoint p (0, 0);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseDownEventHandledButDontNeedMovedOrUpEvents);
		EXPECT (b->getSelectedSegment () == 0);
		p (25, 0);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseDownEventHandledButDontNeedMovedOrUpEvents);
		EXPECT (b->getSelectedSegment () == 1);
		p (45, 0);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseDownEventHandledButDontNeedMovedOrUpEvents);
		EXPECT (b->getSelectedSegment () == 2);
		p (65, 0);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseDownEventHandledButDontNeedMovedOrUpEvents);
		EXPECT (b->getSelectedSegment () == 3);
		p (85, 0);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseDownEventHandledButDontNeedMovedOrUpEvents);
		EXPECT (b->getSelectedSegment () == 4);

		parent->removed (root);
	);

	TEST(focusPathSetting,
		CSegmentButton b (CRect (0, 0, 10, 10));
		EXPECT (b.drawFocusOnTop () == false);
	);
);

} // VSTGUI
