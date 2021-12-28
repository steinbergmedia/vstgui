// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cbuttonstate.h"
#include "cpoint.h"

#if VSTGUI_ENABLE_DEPRECATED_METHODS
#include "vstkeycode.h"
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** EventType
 *	@ingroup new_in_4_11
 */
enum class EventType : uint32_t
{
	Unknown,
	MouseDown,
	MouseMove,
	MouseUp,
	MouseCancel,
	MouseEnter,
	MouseExit,
	MouseWheel,
	ZoomGesture,
	KeyUp,
	KeyDown,
};

//------------------------------------------------------------------------
/** EventConsumeState
 *	@ingroup new_in_4_11
 */
struct EventConsumeState
{
	static constexpr uint32_t NotHandled = 0;
	static constexpr uint32_t Handled = 1;
	static constexpr uint32_t Last = 2;

	void operator= (bool state)
	{
		if (state)
			data |= Handled;
		else
			data &= ~Handled;
	}
	operator bool () { return data & Handled; }

	void reset () { data = NotHandled; }

	uint32_t data {NotHandled};
};

//------------------------------------------------------------------------
/** Event
 *	@ingroup new_in_4_11
 */
struct Event
{
	Event () noexcept;
	Event (const Event&) = delete;
	Event& operator= (const Event&) = delete;
	Event (Event&&) = default;
	Event& operator= (Event&&) = default;

	/** Type */
	EventType type {EventType::Unknown};
	/** Unique ID*/
	uint64_t id;
	/** Timestamp */
	uint64_t timestamp;
	/** Consumed? If this is true, event dispatching is stopped. */
	EventConsumeState consumed;
};

//------------------------------------------------------------------------
inline const Event& noEvent ()
{
	static Event e;
	return e;
}

//------------------------------------------------------------------------
/** Modifiers
 *	@ingroup new_in_4_11
 */
struct Modifiers
{
	Modifiers () = default;
	Modifiers (const Modifiers&) = default;
	explicit Modifiers (ModifierKey modifier) : data (cast (modifier)) {}

	/** test if no modifier key is set */
	bool empty () const { return data == 0; }
	/** test if modifier key is set */
	bool has (ModifierKey modifier) const { return data & cast (modifier); }
	/** test if modifier key is set exclusively */
	bool is (ModifierKey modifier) const { return data == cast (modifier); }
	/** test if the modifier keys are set exclusively */
	bool is (const std::initializer_list<ModifierKey>& modifiers) const
	{
		uint32_t d = 0;
		for (auto& mod : modifiers)
			d |= cast (mod);
		return data == d;
	}
	/** test if modifier key is set */
	bool operator| (ModifierKey modifier) const { return has (modifier); }
	/** test if modifier key is set exclusively */
	bool operator== (ModifierKey modifier) const { return is (modifier); }

	/** add a modifier key */
	void add (ModifierKey modifier) { data |= cast (modifier); }
	/** remove a modifier key */
	void remove (ModifierKey modifier) { data &= ~cast (modifier); }
	/** clear all modifiers */
	void clear () { data = 0; }
	/** set to one modifier key */
	Modifiers& operator= (ModifierKey modifier)
	{
		data = cast (modifier);
		return *this;
	}

private:
	static uint32_t cast (ModifierKey mod) { return static_cast<uint32_t> (mod); }
	uint32_t data {0};
};

//------------------------------------------------------------------------
/** ModifierEvent
 *	@ingroup new_in_4_11
 */
struct ModifierEvent : Event
{
	/** pressed modifiers */
	Modifiers modifiers {};
};

//------------------------------------------------------------------------
/** MousePositionEvent
 *	@ingroup new_in_4_11
 */
struct MousePositionEvent : ModifierEvent
{
	CPoint mousePosition;
};

//------------------------------------------------------------------------
/** MouseButton
 *	@ingroup new_in_4_11
 */
enum class MouseButton : uint32_t
{
	None = 0,
	Left = 1 << 1,
	Middle = 1 << 2,
	Right = 1 << 3,
	Fourth = 1 << 4,
	Fifth = 1 << 5,
};

//------------------------------------------------------------------------
/** MouseEventButtonState
 *	@ingroup new_in_4_11
 */
struct MouseEventButtonState
{
	bool isLeft () const { return data == MouseButton::Left; }
	bool isMiddle () const { return data == MouseButton::Middle; }
	bool isRight () const { return data == MouseButton::Right; }
	bool is (MouseButton pos) const { return data == pos; }
	bool isOther (uint32_t index) const { return data == static_cast<MouseButton> (1 << index); }
	bool has (MouseButton pos) const
	{
		return static_cast<bool> (static_cast<uint32_t> (data) & static_cast<uint32_t> (pos));
	}
	bool empty () const { return data == MouseButton::None; }

	void add (MouseButton pos)
	{
		data =
			static_cast<MouseButton> (static_cast<uint32_t> (data) | static_cast<uint32_t> (pos));
	}
	void set (MouseButton pos) { data = pos; }
	void clear () { data = MouseButton::None; }

	MouseEventButtonState () = default;
	MouseEventButtonState (const MouseEventButtonState&) = default;
	MouseEventButtonState (MouseButton pos) { set (pos); }

	bool operator== (const MouseEventButtonState& other) const { return data == other.data; }
	bool operator!= (const MouseEventButtonState& other) const { return data != other.data; }

private:
	MouseButton data {MouseButton::None};
};

//------------------------------------------------------------------------
/** MouseEvent
 *	@ingroup new_in_4_11
 */
struct MouseEvent : MousePositionEvent
{
	MouseEventButtonState buttonState;
};

//------------------------------------------------------------------------
/** MouseEnterEvent
 *	@ingroup new_in_4_11
 */
struct MouseEnterEvent : MouseEvent
{
	MouseEnterEvent () { type = EventType::MouseEnter; }
	MouseEnterEvent (CPoint pos, MouseEventButtonState buttons, Modifiers mods)
	: MouseEnterEvent ()
	{
		mousePosition = pos;
		buttonState = buttons;
		modifiers = mods;
	}
	MouseEnterEvent (const MouseEvent& e)
	: MouseEnterEvent (e.mousePosition, e.buttonState, e.modifiers)
	{
	}
};

//------------------------------------------------------------------------
/** MouseExitEvent
 *	@ingroup new_in_4_11
 */
struct MouseExitEvent : MouseEvent
{
	MouseExitEvent () { type = EventType::MouseExit; }
	MouseExitEvent (CPoint pos, MouseEventButtonState buttons, Modifiers mods)
	: MouseExitEvent ()
	{
		mousePosition = pos;
		buttonState = buttons;
		modifiers = mods;
	}
	MouseExitEvent (const MouseEvent& e)
	: MouseExitEvent (e.mousePosition, e.buttonState, e.modifiers)
	{
	}
};

//------------------------------------------------------------------------
/** MouseDownUpMoveEvent
 *	@ingroup new_in_4_11
 */
struct MouseDownUpMoveEvent : MouseEvent
{
	uint32_t clickCount {0};

	void ignoreFollowUpMoveAndUpEvents (bool state)
	{
		if (state)
			consumed.data |= IgnoreFollowUpEventsMask;
		else
			consumed.data &= ~IgnoreFollowUpEventsMask;
	}

	bool ignoreFollowUpMoveAndUpEvents ()
	{
		return consumed.data & IgnoreFollowUpEventsMask;
	}

	static constexpr uint32_t IgnoreFollowUpEvents = EventConsumeState::Last;
	static constexpr uint32_t IgnoreFollowUpEventsMask = 1 << IgnoreFollowUpEvents;

protected:
	MouseDownUpMoveEvent () = default;
	MouseDownUpMoveEvent (const CPoint& pos, MouseEventButtonState buttons, Modifiers mods = {})
	{
		mousePosition = pos;
		buttonState = buttons;
		modifiers = mods;
	}
};

//------------------------------------------------------------------------
/** MouseDownEvent
 *	@ingroup new_in_4_11
 */
struct MouseDownEvent : MouseDownUpMoveEvent
{
	MouseDownEvent () { type = EventType::MouseDown; }
	MouseDownEvent (const CPoint& pos, MouseEventButtonState buttons)
	: MouseDownUpMoveEvent (pos, buttons)
	{
		type = EventType::MouseDown;
	}
};

//------------------------------------------------------------------------
/** MouseUpEvent
 *	@ingroup new_in_4_11
 */
struct MouseUpEvent : MouseDownUpMoveEvent
{
	MouseUpEvent () { type = EventType::MouseUp; }
	MouseUpEvent (const CPoint& pos, MouseEventButtonState buttons)
	: MouseDownUpMoveEvent (pos, buttons)
	{
		type = EventType::MouseUp;
	}
};

//------------------------------------------------------------------------
/** MouseMoveEvent
 *	@ingroup new_in_4_11
 */
struct MouseMoveEvent : MouseDownUpMoveEvent
{
	MouseMoveEvent () { type = EventType::MouseMove; }
	MouseMoveEvent (const CPoint& pos, MouseEventButtonState buttons = {})
	: MouseDownUpMoveEvent (pos, buttons)
	{
		type = EventType::MouseMove;
	}
	MouseMoveEvent (const MouseDownUpMoveEvent& other)
	: MouseDownUpMoveEvent (other.mousePosition, other.buttonState, other.modifiers)
	{
		type = EventType::MouseMove;
	}
};

//------------------------------------------------------------------------
/** MouseCancelEvent
 *	@ingroup new_in_4_11
 */
struct MouseCancelEvent : Event
{
	MouseCancelEvent () { type = EventType::MouseCancel; }
};

//------------------------------------------------------------------------
/** MouseWheelEvent
 *	@ingroup new_in_4_11
 */
struct MouseWheelEvent : MousePositionEvent
{
	enum Flags : uint32_t
	{
		/** deltaX and deltaY are inverted */
		DirectionInvertedFromDevice = 1 << 0,
		/** indicates a precise scroll event where deltaX and deltaY are multiplied by 0.1. If you
		 *  divide the deltas by 0.1 you will get exact pixel movement.
		 */
		PreciseDeltas = 1 << 1,
	};
	CCoord deltaX {0.};
	CCoord deltaY {0.};

	uint32_t flags {0};

	MouseWheelEvent () { type = EventType::MouseWheel; }
};

//------------------------------------------------------------------------
/** GestureEvent
 *	@ingroup new_in_4_11
 */
struct GestureEvent : MousePositionEvent
{
	enum class Phase : uint32_t
	{
		Unknown,
		Begin,
		Changed,
		End,
	};

	Phase phase {Phase::Unknown};
};

//------------------------------------------------------------------------
/** ZoomGestureEvent
 *	@ingroup new_in_4_11
 */
struct ZoomGestureEvent : GestureEvent
{
	double zoom;

	ZoomGestureEvent () { type = EventType::ZoomGesture; }
};

//------------------------------------------------------------------------
// Keyboard Events
//------------------------------------------------------------------------
/** VirtualKey
 *	@ingroup new_in_4_11
 */
enum class VirtualKey : uint32_t
{
	None = 0,

	Back,
	Tab,
	Clear,
	Return,
	Pause,
	Escape,
	Space,
	Next,
	End,
	Home,

	Left,
	Up,
	Right,
	Down,
	PageUp,
	PageDown,

	Select,
	Print,
	Enter,
	Snapshot,
	Insert,
	Delete,
	Help,

	NumPad0,
	NumPad1,
	NumPad2,
	NumPad3,
	NumPad4,
	NumPad5,
	NumPad6,
	NumPad7,
	NumPad8,
	NumPad9,

	Multiply,
	Add,
	Separator,
	Subtract,
	Decimal,
	Divide,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	NumLock,
	Scroll,

	ShiftModifier,
	ControlModifier,
	AltModifier,

	Equals,
	// DO NOT CHANGE THE ORDER ABOVE

};

//------------------------------------------------------------------------
/** ModifierKey
 *	@ingroup new_in_4_11
 */
enum class ModifierKey : uint32_t
{
	/** the left or right shift key */
	Shift = 1 << 0,
	/** the alternate key */
	Alt = 1 << 1,
	/** the control key (Command key on macOS and control key on other platforms) */
	Control = 1 << 2,
	/** the super key (Control key on macOS, Windows key on Windows and Super key on other
	   platforms)*/
	Super = 1 << 3,
	
	None = 0
};

//------------------------------------------------------------------------
/** KeyboardEvent
 *	@ingroup new_in_4_11
 */
struct KeyboardEvent : ModifierEvent
{
	/** UTF-32 character */
	char32_t character {0};
	/** virtual key */
	VirtualKey virt {VirtualKey::None};
	/** indicates for a key down event if this is a repeated key down */
	bool isRepeat {false};

	KeyboardEvent (EventType t = EventType::KeyDown) { type = t; }
};

//------------------------------------------------------------------------
/** event as mouse position event or nullpointer if not a mouse position event
 *	@ingroup new_in_4_11
 */
template <typename EventT, typename OutputT = MousePositionEvent,
          typename MousePositionEventT = typename std::conditional<
              std::is_const_v<EventT>, typename std::add_const_t<OutputT>, OutputT>::type>
inline MousePositionEventT* asMousePositionEvent (EventT& event)
{
	switch (event.type)
	{
		case EventType::ZoomGesture: [[fallthrough]];
		case EventType::MouseWheel: [[fallthrough]];
		case EventType::MouseDown: [[fallthrough]];
		case EventType::MouseMove: [[fallthrough]];
		case EventType::MouseUp: [[fallthrough]];
		case EventType::MouseEnter: [[fallthrough]];
		case EventType::MouseExit: return static_cast<MousePositionEventT*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** event as mouse position event or nullpointer if not a mouse position event
 *	@ingroup new_in_4_11
 */
template <typename EventT, typename OutputT = MouseEvent,
          typename MouseEventT = typename std::conditional<
              std::is_const_v<EventT>, typename std::add_const_t<OutputT>, OutputT>::type>
inline MouseEventT* asMouseEvent (EventT& event)
{
	switch (event.type)
	{
		case EventType::MouseDown: [[fallthrough]];
		case EventType::MouseMove: [[fallthrough]];
		case EventType::MouseUp: [[fallthrough]];
		case EventType::MouseEnter: [[fallthrough]];
		case EventType::MouseExit: return static_cast<MouseEventT*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** event as mouse down event or nullpointer if not a mouse down event
 *	@ingroup new_in_4_11
 */
template <typename EventT, typename OutputT = MouseDownEvent,
          typename MouseDownEventT = typename std::conditional<
              std::is_const_v<EventT>, typename std::add_const_t<OutputT>, OutputT>::type>
inline MouseDownEventT* asMouseDownEvent (EventT& event)
{
	switch (event.type)
	{
		case EventType::MouseDown: [[fallthrough]];
		case EventType::MouseMove: [[fallthrough]];
		case EventType::MouseUp: return static_cast<MouseDownEventT*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** event as modifier event or nullpointer if not a modifier event
 *	@ingroup new_in_4_11
 */
template <typename EventT, typename OutputT = ModifierEvent,
          typename ModifierEventT = typename std::conditional<
              std::is_const_v<EventT>, typename std::add_const_t<OutputT>, OutputT>::type>
inline ModifierEventT* asModifierEvent (EventT& event)
{
	switch (event.type)
	{
		case EventType::KeyDown: [[fallthrough]];
		case EventType::KeyUp: [[fallthrough]];
		case EventType::MouseEnter: [[fallthrough]];
		case EventType::MouseExit: [[fallthrough]];
		case EventType::MouseWheel: [[fallthrough]];
		case EventType::MouseDown: [[fallthrough]];
		case EventType::MouseMove: [[fallthrough]];
		case EventType::MouseUp: return static_cast<ModifierEventT*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** event as keyboard event or nullpointer if not a keyboard event
 *	@ingroup new_in_4_11
 */
template <typename EventT, typename OutputT = KeyboardEvent,
          typename KeyboardEventT = typename std::conditional<
              std::is_const_v<EventT>, typename std::add_const_t<OutputT>, OutputT>::type>
inline KeyboardEventT* asKeyboardEvent (EventT& event)
{
	switch (event.type)
	{
		case EventType::KeyDown: [[fallthrough]];
		case EventType::KeyUp: return static_cast<KeyboardEventT*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** cast event to a mouse position event
 *	@ingroup new_in_4_11
 */
inline MousePositionEvent& castMousePositionEvent (Event& event)
{
	vstgui_assert (event.type >= EventType::MouseDown && event.type <= EventType::ZoomGesture);
	return static_cast<MousePositionEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse event
 *	@ingroup new_in_4_11
 */
inline MouseEvent& castMouseEvent (Event& event)
{
	vstgui_assert (event.type >= EventType::MouseDown && event.type <= EventType::MouseExit);
	return static_cast<MouseEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse down event
 *	@ingroup new_in_4_11
 */
inline MouseDownEvent& castMouseDownEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseDown);
	return static_cast<MouseDownEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse move event
 *	@ingroup new_in_4_11
 */
inline MouseMoveEvent& castMouseMoveEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseMove);
	return static_cast<MouseMoveEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse up event
 *	@ingroup new_in_4_11
 */
inline MouseUpEvent& castMouseUpEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseUp);
	return static_cast<MouseUpEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse enter event
 *	@ingroup new_in_4_11
 */
inline MouseEnterEvent& castMouseEnterEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseEnter);
	return static_cast<MouseEnterEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse exit event
 *	@ingroup new_in_4_11
 */
inline MouseExitEvent& castMouseExitEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseExit);
	return static_cast<MouseExitEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse cancel event
 *	@ingroup new_in_4_11
 */
inline MouseCancelEvent& castMouseCancelEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseCancel);
	return static_cast<MouseCancelEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse wheel event
 *	@ingroup new_in_4_11
 */
inline MouseWheelEvent& castMouseWheelEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseWheel);
	return static_cast<MouseWheelEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a zoom gesture event
 *	@ingroup new_in_4_11
 */
inline ZoomGestureEvent& castZoomGestureEvent (Event& event)
{
	vstgui_assert (event.type == EventType::ZoomGesture);
	return static_cast<ZoomGestureEvent&> (event);
}

//------------------------------------------------------------------------
/** cast event to a mouse wheel event
 *	@ingroup new_in_4_11
 */
inline KeyboardEvent& castKeyboardEvent (Event& event)
{
	vstgui_assert (event.type == EventType::KeyDown || event.type == EventType::KeyUp);
	return static_cast<KeyboardEvent&> (event);
}

//------------------------------------------------------------------------
/** convert from new Modifiers to old CButtonState
 *	@ingroup new_in_4_11
 */
CButtonState buttonStateFromEventModifiers (const Modifiers& mods);

//------------------------------------------------------------------------
/** convert from new MouseEvent to old CButtonState
 *	@ingroup new_in_4_11
 */
CButtonState buttonStateFromMouseEvent (const MouseEvent& event);

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
/** helper function to convert from new VirtualKey to old VstVirtualKey
 *
 *	returns 0 if key cannot be mapped
 *	@ingroup new_in_4_11
 */
inline unsigned char toVstVirtualKey (VirtualKey key)
{
	auto k = static_cast<uint32_t> (key);
	if (k <= static_cast<uint32_t> (VirtualKey::Equals))
		return static_cast<unsigned char> (k);
	return 0;
}

inline VirtualKey fromVstVirtualKey (uint32_t key)
{
	auto k = static_cast<VirtualKey> (key);
	if (k <= VirtualKey::Equals)
		return k;
	return VirtualKey::None;
}

VstKeyCode toVstKeyCode (const KeyboardEvent& event);
#endif

//------------------------------------------------------------------------
} // VSTGUI
