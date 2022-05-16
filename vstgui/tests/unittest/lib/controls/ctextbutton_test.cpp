// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

TEST_CASE (CTextButtonTest, MouseEventsKickStyle)
{
	auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
	b->setStyle (CTextButton::kKickStyle);
	b->setValue (b->getMin ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());
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
	EXPECT_EQ (dispatchMouseCancelEvent (b), EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());

	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
}

TEST_CASE (CTextButtonTest, MouseEventsOnOffStyle)
{
	auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
	b->setStyle (CTextButton::kOnOffStyle);
	b->setValue (b->getMin ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Right),
			   EventConsumeState::NotHandled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMax ());
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
	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_EQ (b->getValue (), b->getMin ());
	EXPECT_EQ (dispatchMouseEvent<MouseUpEvent> (b, {0., 0.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());
	EXPECT_EQ (b->getValue (), b->getMin ());

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::Handled);
	EXPECT_TRUE (b->isEditing ());
	EXPECT_EQ (dispatchMouseCancelEvent (b), EventConsumeState::Handled);
	EXPECT_FALSE (b->isEditing ());

	EXPECT_EQ (dispatchMouseEvent<MouseMoveEvent> (b, {10., 10.}, MouseButton::Left),
			   EventConsumeState::NotHandled);
}

TEST_CASE (CTextButtonTest, KeyEvents)
{
	auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
	b->setStyle (CTextButton::kOnOffStyle);
	b->setValue (b->getMin ());
	KeyboardEvent retEvent;
	retEvent.virt = VirtualKey::Return;
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT_EQ (b->getValue (), b->getMax ());
	retEvent.consumed.reset ();
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT_EQ (b->getValue (), b->getMin ());

	KeyboardEvent charEvent;
	charEvent.character = 't';
	b->onKeyboardEvent (charEvent);
	EXPECT_FALSE (charEvent.consumed);

	b->setStyle (CTextButton::kKickStyle);
	retEvent.consumed.reset ();
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT_EQ (b->getValue (), b->getMin ());
	retEvent.consumed.reset ();
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT_EQ (b->getValue (), b->getMin ());

	KeyboardEvent upEvent;
	upEvent.type = EventType::KeyUp;
	upEvent.virt = VirtualKey::Return;
	b->onKeyboardEvent (upEvent);
	EXPECT_FALSE (upEvent.consumed);
}

TEST_CASE (CTextButtonTest, FocusPathSetting)
{
	auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
	EXPECT_FALSE (b->drawFocusOnTop ());
}

} // VSTGUI
