// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/csegmentbutton.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

TEST_CASE (CSegmentButtonTest, AddSegment)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	EXPECT (b.getSegments ().size () == 0);
	CSegmentButton::Segment s;
	s.name = "1";
	b.addSegment (s);
	EXPECT (b.getSegments ().size () == 1);
	s.name = "2";
	b.addSegment (s);
	EXPECT (b.getSegments ().size () == 2);
	EXPECT (b.getSegments ()[1].name == "2");
	s.name = "3";
	b.addSegment (s, 0);
	EXPECT (b.getSegments ().size () == 3);
	EXPECT (b.getSegments ()[0].name == "3");
	EXPECT (b.getSegments ()[1].name == "1");
	EXPECT (b.getSegments ()[2].name == "2");
}

TEST_CASE (CSegmentButtonTest, InsertSegment)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (std::move (s));
	s.name = "1";
	b.addSegment (std::move (s), 0);
	EXPECT (b.getSegments ()[0].name == "1");
	EXPECT (b.getSegments ()[1].name == "0");
}

TEST_CASE (CSegmentButtonTest, RemoveSegment)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (s);
	s.name = "1";
	b.addSegment (s);
	s.name = "2";
	b.addSegment (s);
	s.name = "3";
	b.addSegment (s);
	EXPECT (b.getSegments ().size () == 4);
	b.removeSegment (0);
	EXPECT (b.getSegments ().size () == 3);
	EXPECT (b.getSegments ()[0].name == "1");
	EXPECT (b.getSegments ()[1].name == "2");
	EXPECT (b.getSegments ()[2].name == "3");
}

TEST_CASE (CSegmentButtonTest, SelectedSegment)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (s);
	s.name = "1";
	b.addSegment (s);
	s.name = "2";
	b.addSegment (s);
	s.name = "3";
	b.addSegment (s);
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
}

TEST_CASE (CSegmentButtonTest, RightKeyEvent)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (s);
	s.name = "1";
	b.addSegment (s);
	s.name = "2";
	b.addSegment (s);

	b.setStyle (CSegmentButton::Style::kHorizontal);
	b.setSelectedSegment (0);

	KeyboardEvent event;
	event.virt = VirtualKey::Right;
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 2);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 2);

	b.setSelectedSegment (1);
	b.setStyle (CSegmentButton::Style::kVertical);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
}

TEST_CASE (CSegmentButtonTest, LeftKeyEvent)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (s);
	s.name = "1";
	b.addSegment (s);
	s.name = "2";
	b.addSegment (s);

	b.setStyle (CSegmentButton::Style::kHorizontal);
	b.setSelectedSegment (2);

	KeyboardEvent event;
	event.virt = VirtualKey::Left;
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 0);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 0);

	b.setSelectedSegment (1);
	b.setStyle (CSegmentButton::Style::kVertical);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
}

TEST_CASE (CSegmentButtonTest, DownKeyEvent)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (s);
	s.name = "1";
	b.addSegment (s);
	s.name = "2";
	b.addSegment (s);

	b.setStyle (CSegmentButton::Style::kVertical);
	b.setSelectedSegment (0);

	KeyboardEvent event;
	event.virt = VirtualKey::Down;
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 2);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 2);

	b.setSelectedSegment (1);
	b.setStyle (CSegmentButton::Style::kHorizontal);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
}

TEST_CASE (CSegmentButtonTest, UpKeyEvent)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	CSegmentButton::Segment s;
	s.name = "0";
	b.addSegment (s);
	s.name = "1";
	b.addSegment (s);
	s.name = "2";
	b.addSegment (s);

	b.setStyle (CSegmentButton::Style::kVertical);
	b.setSelectedSegment (2);
	KeyboardEvent event;
	event.virt = VirtualKey::Up;
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 0);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 0);

	b.setSelectedSegment (1);
	b.setStyle (CSegmentButton::Style::kHorizontal);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
	event.consumed.reset ();
	b.onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b.getSelectedSegment () == 1);
}

TEST_CASE (CSegmentButtonTest, HorizontalSegmentSizeCalculation)
{
	const auto numSegments = 5;
	CRect r (0, 0, 100, 100);
	auto b = new CSegmentButton (r);
	b->setStyle (CSegmentButton::Style::kHorizontal);
	for (auto i = 0; i < numSegments; ++i)
		b->addSegment ({});
	for (const auto& s : b->getSegments ())
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
}

TEST_CASE (CSegmentButtonTest, VerticalSegmentSizeCalculation)
{
	const auto numSegments = 5;
	CRect r (0, 0, 100, 100);
	auto b = new CSegmentButton (r);
	b->setStyle (CSegmentButton::Style::kVertical);
	for (auto i = 0; i < numSegments; ++i)
		b->addSegment ({});
	for (const auto& s : b->getSegments ())
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
}

TEST_CASE (CSegmentButtonTest, UpdateViewSize)
{
	const auto numSegments = 5;
	auto b = new CSegmentButton (CRect (0, 0, 50, 100));
	b->setStyle (CSegmentButton::Style::kHorizontal);
	for (auto i = 0; i < numSegments; ++i)
		b->addSegment ({});
	for (const auto& s : b->getSegments ())
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
}

TEST_CASE (CSegmentButtonTest, MouseDownEvent)
{
	const auto numSegments = 5;
	CRect r (0, 0, 100, 100);
	auto b = new CSegmentButton (r);
	b->setStyle (CSegmentButton::Style::kHorizontal);
	for (auto i = 0; i < numSegments; ++i)
		b->addSegment ({});
	for (const auto& s : b->getSegments ())
		EXPECT (s.rect == CRect (0, 0, 0, 0));
	auto root = owned (new CViewContainer (r));
	auto parent = new CViewContainer (r);
	root->addView (parent);
	parent->addView (b);
	parent->attached (root);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), 0);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {25., 0.}, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), 1);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {45., 0.}, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), 2);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {65., 0.}, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), 3);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {85., 0.}, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), 4);

	parent->removed (root);
}

TEST_CASE (CSegmentButtonTest, MouseDownEventWithManySegments)
{
	// Create segment button with 32 segments and attach it
	const auto numSegments = 32;
	CRect r (0, 0, 20 * numSegments, 100);
	auto b = new CSegmentButton (r);
	b->setStyle (CSegmentButton::Style::kHorizontal);
	for (auto i = 0; i < numSegments; ++i)
		b->addSegment ({});
	for (const auto& s : b->getSegments ())
		EXPECT_EQ (s.rect, CRect (0, 0, 0, 0));
	auto root = owned (new CViewContainer (r));
	auto parent = new CViewContainer (r);
	root->addView (parent);
	parent->addView (b);
	parent->attached (root);

	// Select the e.g. 20th segment
	constexpr auto kSelectedSegment = 20;
	CPoint p (20 * kSelectedSegment + 5, 0);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, p, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), kSelectedSegment);

	parent->removed (root);
}

TEST_CASE (CSegmentButtonTest, MouseDownEventOnLastSegment)
{
	// Create segment button with 31 segments and attach it.
	// 31 segments causing rounding errors inside segment button.
	const auto numSegments = 31;
	CRect r (0, 0, 20 * numSegments, 100);
	auto b = new CSegmentButton (r);
	b->setStyle (CSegmentButton::Style::kHorizontal);
	for (auto i = 0; i < numSegments; ++i)
		b->addSegment ({});
	for (const auto& s : b->getSegments ())
		EXPECT_EQ (s.rect, CRect (0, 0, 0, 0));
	auto root = owned (new CViewContainer (r));
	auto parent = new CViewContainer (r);
	root->addView (parent);
	parent->addView (b);
	parent->attached (root);

	// Select the last segment
	constexpr auto kSelectedSegment = numSegments - 1;
	CPoint p (0, 0);
	p (20 * kSelectedSegment + 5, 0);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, p, MouseButton::Left),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_EQ (b->getSelectedSegment (), kSelectedSegment);

	parent->removed (root);
}

TEST_CASE (CSegmentButtonTest, FocusPathSetting)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	EXPECT (b.drawFocusOnTop () == false);
}

TEST_CASE (CSegmentButtonTest, MultiSelection)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	b.setSelectionMode (CSegmentButton::SelectionMode::kMultiple);
	b.addSegment ({});
	b.addSegment ({});
	b.addSegment ({});
	b.selectSegment (0, true);
	b.selectSegment (1, false);
	b.selectSegment (2, true);
	EXPECT (b.isSegmentSelected (0) == true);
	EXPECT (b.isSegmentSelected (1) == false);
	EXPECT (b.isSegmentSelected (2) == true);
}

TEST_CASE (CSegmentButtonTest, MultiSelectionMaxEntries)
{
	CSegmentButton b (CRect (0, 0, 10, 10));
	b.setSelectionMode (CSegmentButton::SelectionMode::kMultiple);
	CSegmentButton::Segment s;
	for (auto i = 0; i < 32; ++i)
	{
		EXPECT (b.addSegment (s) == true);
	}
	EXPECT (b.addSegment (s) == false);
	EXPECT (b.addSegment (std::move (s)) == false);
}

} // VSTGUI
