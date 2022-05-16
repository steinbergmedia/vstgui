// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

TEST_CASE (CCheckboxTest, MouseEvents)
{
	auto b = owned (new CCheckBox (CRect (10, 10, 50, 20)));
	b->setValue (b->getMin ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Right),
			   EventConsumeState::Handled | MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMin ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (b, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMin ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	MouseCancelEvent cancelEvent;
	b->dispatchEvent (cancelEvent);
	EXPECT_TRUE (cancelEvent.consumed);
	EXPECT (b->isEditing () == false);
	EXPECT_EQ (b->getValue (), b->getMin ());
}

TEST_CASE (CCheckboxTest, KeyEvents)
{
	auto b = owned (new CCheckBox (CRect (10, 10, 50, 20)));
	b->setValue (b->getMin ());
	KeyboardEvent event;
	event.virt = VirtualKey::Return;
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b->getValue () == b->getMax ());
	event.consumed.reset ();
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b->getValue () == b->getMin ());

	event.virt = VirtualKey::None;
	event.character = 't';
	event.consumed.reset ();
	b->onKeyboardEvent (event);
	EXPECT_FALSE (event.consumed);
}

} // VSTGUI
