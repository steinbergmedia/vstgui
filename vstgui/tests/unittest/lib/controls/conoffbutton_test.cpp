// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

TEST_CASE (COnOffButtonTest, MouseEvents)
{
	auto b = owned (new COnOffButton (CRect (10, 10, 50, 20)));
	b->setValue (b->getMin ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
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
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (b, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseCancelEvent (b), EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
}

TEST_CASE (COnOffButtonTest, KeyEvents)
{
	auto b = owned (new COnOffButton (CRect (10, 10, 50, 20)));
	b->setValue (b->getMin ());
	KeyboardEvent event;
	event.virt = VirtualKey::Return;
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT_EQ (b->getValue (), b->getMax ());
	event.consumed.reset ();
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT_EQ (b->getValue (), b->getMin ());

	event.consumed.reset ();
	event.virt = VirtualKey::None;
	event.character = 't';
	b->onKeyboardEvent (event);
	EXPECT_FALSE (event.consumed);
}

} // VSTGUI
