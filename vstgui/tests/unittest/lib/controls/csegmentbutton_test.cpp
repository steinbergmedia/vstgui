// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

	TEST(insertSegment,
		CSegmentButton b (CRect (0, 0, 10, 10));
		CSegmentButton::Segment s;
		s.name = "0";
		b.addSegment (std::move (s));
		s.name = "1";
		b.addSegment (std::move (s), 0);
		EXPECT(b.getSegments ()[0].name == "1");
		EXPECT(b.getSegments ()[1].name == "0");
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

		b.setStyle (CSegmentButton::Style::kHorizontal);
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
		b.setStyle (CSegmentButton::Style::kVertical);
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

		b.setStyle (CSegmentButton::Style::kHorizontal);
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
		b.setStyle (CSegmentButton::Style::kVertical);
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

		b.setStyle (CSegmentButton::Style::kVertical);
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
		b.setStyle (CSegmentButton::Style::kHorizontal);
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

		b.setStyle (CSegmentButton::Style::kVertical);
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
		b.setStyle (CSegmentButton::Style::kHorizontal);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
		EXPECT (b.onKeyDown (keycode) == 1);
		EXPECT (b.getSelectedSegment () == 1);
	);

	TEST(horizontalSegmentSizeCalculation,
		const auto numSegments = 5;
		CRect r (0, 0, 100, 100);
		auto b = new CSegmentButton (r);
		b->setStyle (CSegmentButton::Style::kHorizontal);
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
		b->setStyle (CSegmentButton::Style::kVertical);
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
		b->setStyle (CSegmentButton::Style::kHorizontal);
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
		b->setStyle (CSegmentButton::Style::kHorizontal);
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

	TEST(multiSelection,
		CSegmentButton b (CRect (0, 0, 10, 10));
		b.setSelectionMode (CSegmentButton::SelectionMode::kMultiple);
		b.addSegment ({});
		b.addSegment ({});
		b.addSegment ({});
		b.selectSegment (0, true);
		b.selectSegment (1, false);
		b.selectSegment (2, true);
		EXPECT(b.isSegmentSelected (0) == true);
		EXPECT(b.isSegmentSelected (1) == false);
		EXPECT(b.isSegmentSelected (2) == true);
	);

	TEST(multiSelectionMaxEntries,
		CSegmentButton b (CRect (0, 0, 10, 10));
		b.setSelectionMode (CSegmentButton::SelectionMode::kMultiple);
		CSegmentButton::Segment s;
		for (auto i = 0; i < 32; ++i)
		{
			EXPECT(b.addSegment (s) == true);
		}
		EXPECT(b.addSegment (s) == false);
		EXPECT(b.addSegment (std::move (s)) == false);
	);
);

} // VSTGUI
