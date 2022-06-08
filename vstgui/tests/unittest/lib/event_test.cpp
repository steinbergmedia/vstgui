// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/events.h"
#include "../unittests.h"

//------------------------------------------------------------------------
namespace VSTGUI {

TEST_CASE (EventTest, UniqueIDTest)
{
	Event e1;
	Event e2;
	EXPECT_NE (e1.id, e2.id);
}

TEST_CASE (EventTest, DefaultConstructorTypeTest)
{
	MouseEnterEvent enterEvent;
	EXPECT_EQ (enterEvent.type, EventType::MouseEnter);
	MouseExitEvent exitEvent;
	EXPECT_EQ (exitEvent.type, EventType::MouseExit);
	MouseDownEvent downEvent;
	EXPECT_EQ (downEvent.type, EventType::MouseDown);
	MouseMoveEvent moveEvent;
	EXPECT_EQ (moveEvent.type, EventType::MouseMove);
	MouseUpEvent upEvent;
	EXPECT_EQ (upEvent.type, EventType::MouseUp);
	MouseCancelEvent cancelEvent;
	EXPECT_EQ (cancelEvent.type, EventType::MouseCancel);
	MouseWheelEvent wheelEvent;
	EXPECT_EQ (wheelEvent.type, EventType::MouseWheel);
	ZoomGestureEvent zoomGestureEvent;
	EXPECT_EQ (zoomGestureEvent.type, EventType::ZoomGesture);
	KeyboardEvent keyEvent;
	EXPECT_EQ (keyEvent.type, EventType::KeyDown);
}

TEST_CASE (EventTest, NoEventTest)
{
	auto& event = noEvent ();
	EXPECT_EQ (event.type, EventType::Unknown);
}

TEST_CASE (EventTest, ConsumedTest)
{
	EventConsumeState consumed;
	EXPECT_FALSE (consumed);
	consumed = true;
	EXPECT_TRUE (consumed);
	consumed = false;
	EXPECT_FALSE (consumed);
	consumed = true;
	EXPECT_TRUE (consumed);
	consumed.reset ();
	EXPECT_FALSE (consumed);
}

TEST_CASE (EventTest, CastMouseEnterEventTest)
{
	MouseEnterEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), &e);
	EXPECT_EQ (asMouseDownEvent (e), nullptr);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastMouseExitEventTest)
{
	MouseExitEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), &e);
	EXPECT_EQ (asMouseDownEvent (e), nullptr);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastMouseDownEventTest)
{
	MouseDownEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), &e);
	EXPECT_EQ (asMouseDownEvent (e), &e);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastMouseMoveEventTest)
{
	MouseMoveEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), &e);
	EXPECT_EQ (asMouseDownEvent (e), &e);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastMouseUpEventTest)
{
	MouseUpEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), &e);
	EXPECT_EQ (asMouseDownEvent (e), &e);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastMouseCancelEventTest)
{
	MouseCancelEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), nullptr);
	EXPECT_EQ (asMouseEvent (e), nullptr);
	EXPECT_EQ (asMouseDownEvent (e), nullptr);
	EXPECT_EQ (asModifierEvent (e), nullptr);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastMouseWheelEventTest)
{
	MouseWheelEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), nullptr);
	EXPECT_EQ (asMouseDownEvent (e), nullptr);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastZoomGestureEventTest)
{
	ZoomGestureEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), &e);
	EXPECT_EQ (asMouseEvent (e), nullptr);
	EXPECT_EQ (asMouseDownEvent (e), nullptr);
	EXPECT_EQ (asModifierEvent (e), nullptr);
	EXPECT_EQ (asKeyboardEvent (e), nullptr);
}

TEST_CASE (EventTest, CastKeyboardEventTest)
{
	KeyboardEvent event;
	Event& e = event;
	EXPECT_EQ (asMousePositionEvent (e), nullptr);
	EXPECT_EQ (asMouseEvent (e), nullptr);
	EXPECT_EQ (asMouseDownEvent (e), nullptr);
	EXPECT_EQ (asModifierEvent (e), &e);
	EXPECT_EQ (asKeyboardEvent (e), &e);
}

TEST_CASE (EventTest, ButtonStateFromEventModifierTest)
{
	Modifiers mods;
	EXPECT_EQ (buttonStateFromEventModifiers (mods), 0);
	mods.add (ModifierKey::Control);
	EXPECT_EQ (buttonStateFromEventModifiers (mods), kControl);
	mods.add (ModifierKey::Shift);
	EXPECT_EQ (buttonStateFromEventModifiers (mods), kControl | kShift);
	mods.add (ModifierKey::Alt);
	EXPECT_EQ (buttonStateFromEventModifiers (mods), kControl | kShift | kAlt);
}

TEST_CASE (EventTest, ButtonStateFromMouseEventTest)
{
	MouseDownEvent event;
	EXPECT_EQ (buttonStateFromMouseEvent (event), 0);
	event.buttonState.set (MouseButton::Left);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kLButton);
	event.buttonState.set (MouseButton::Right);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kRButton);
	event.buttonState.set (MouseButton::Middle);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kMButton);
	event.buttonState.set (MouseButton::Fourth);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kButton4);
	event.buttonState.set (MouseButton::Fifth);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kButton5);
	event.clickCount = 2;
	event.buttonState.set (MouseButton::Left);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kLButton | kDoubleClick);
	event.modifiers.add (ModifierKey::Shift);
	EXPECT_EQ (buttonStateFromMouseEvent (event), kLButton | kDoubleClick | kShift);
}

TEST_CASE (EventTest, IgnoreFollowUpEvent)
{
	MouseDownEvent e;
	EXPECT_FALSE (e.ignoreFollowUpMoveAndUpEvents ());
	e.ignoreFollowUpMoveAndUpEvents (true);
	EXPECT_TRUE (e.ignoreFollowUpMoveAndUpEvents ());
	EXPECT_FALSE (e.consumed);
	e.consumed = true;
	EXPECT_TRUE (e.ignoreFollowUpMoveAndUpEvents ());
	EXPECT_TRUE (e.consumed);
}

TEST_CASE (EventTest, ModifiersEquality)
{
	Modifiers altMod (ModifierKey::Alt);
	Modifiers ctrlMod (ModifierKey::Control);
	EXPECT_FALSE ((altMod == ctrlMod));
	EXPECT_TRUE ((altMod != ctrlMod));
}

//------------------------------------------------------------------------
} // VSTGUI
