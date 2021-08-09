// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

TEST_CASE (CKickButtonTest, MouseEvents)
{
	auto b = owned (new CKickButton (CRect (10, 10, 50, 20), nullptr, 0, nullptr));
	b->setValue (b->getMin ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMin ());
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMin ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_EQ (b->getValue (), b->getMax ());
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseCancelEvent (b), EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMin ());
}

TEST_CASE (CKickButtonTest, KeyEvents)
{
	auto b = owned (new CKickButton (CRect (10, 10, 50, 20), nullptr, 0, nullptr));
	b->setValue (b->getMin ());
	KeyboardEvent event;
	event.virt = VirtualKey::Return;
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT_EQ (b->getValue (), b->getMax ());
	event.consumed.reset ();
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT_EQ (b->getValue (), b->getMax ());
	event.consumed.reset ();
	event.type = EventType::KeyUp;
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT_EQ (b->getValue (), b->getMin ());

	KeyboardEvent event2;
	event2.character = 't';
	b->onKeyboardEvent (event2);
	EXPECT_FALSE (event2.consumed);
	event2.type = EventType::KeyUp;
	b->onKeyboardEvent (event2);
	EXPECT_FALSE (event2.consumed);
}

} // VSTGUI
