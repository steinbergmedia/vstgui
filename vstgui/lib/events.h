// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cbuttonstate.h"
#include "cpoint.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** EventType
 *	@ingroup new_in_4_11
 */
enum class EventType : uint32_t
{
	Unknown,
	KeyUp,
	KeyRepeat,
	KeyDown,
	MouseWheel,
};

//------------------------------------------------------------------------
/** Event
 *	@ingroup new_in_4_11
 */
struct Event
{
	Event () noexcept;
	Event (const Event&) = delete;

	/** Type */
	EventType type {EventType::Unknown};
	/** Unique ID*/
	uint64_t id;
	/** Timestamp */
	uint64_t timestamp;
	/** Consumed? If this is true, event dispatching is stopped. */
	bool consumed {false};
};

//------------------------------------------------------------------------
/** Modifiers
 *	@ingroup new_in_4_11
 */
struct Modifiers
{
	explicit Modifiers (uint32_t data = 0) : data (data) {}
	Modifiers (const Modifiers&) = default;
	
	bool empty () const { return data == 0;}
	bool has (ModifierKey modifier) const { return data & cast (modifier); }
	bool is (ModifierKey modifier) const { return data == cast (modifier); }

	bool operator| (ModifierKey modifier) const { return has (modifier); }
	bool operator== (ModifierKey modifier) const { return is (modifier); }

	void add (ModifierKey modifier) { data |= cast (modifier); }
	void remove (ModifierKey modifier) { data &= ~cast (modifier); }
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
/** MouseWheelEvent
 *	@ingroup new_in_4_11
 */
struct MouseWheelEvent : MousePositionEvent
{
	enum Flags
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
// Keyboard Events
//------------------------------------------------------------------------
/** VirtualKey
 *	@ingroup new_in_4_11
 */
enum class VirtualKey : uint32_t
{
	Unknown = 0,

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

	Equals

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
	/** the super key (Control key on macOS, Windows key on Windows and Super key on other platforms)*/
	Super = 1 << 3,
};

//------------------------------------------------------------------------
/** KeyboardEvent
 *	@ingroup new_in_4_11
 */
struct KeyboardEvent : ModifierEvent
{
	/** UTF-16 character */
	uint32_t character {0};
	/** virtual key */
	VirtualKey virt {VirtualKey::Unknown};
};

//------------------------------------------------------------------------
/** event as mouse position event or nullpointer if not a mouse position event
 *	@ingroup new_in_4_11
 */
inline MousePositionEvent* asMousePositionEvent (Event& event)
{
	switch (event.type)
	{
		case EventType::MouseWheel:
			return static_cast<MousePositionEvent*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** event as modifier event or nullpointer if not a modifier event
 *	@ingroup new_in_4_11
 */
inline ModifierEvent* asModifierEvent (Event& event)
{
	switch (event.type)
	{
		case EventType::KeyDown:
		case EventType::KeyRepeat:
		case EventType::KeyUp:
		case EventType::MouseWheel:
			return static_cast<ModifierEvent*> (&event);
		default: break;
	}
	return nullptr;
}

//------------------------------------------------------------------------
/** cast to a mouse wheel event
 *	@ingroup new_in_4_11
 */
inline MouseWheelEvent& castMouseWheelEvent (Event& event)
{
	vstgui_assert (event.type == EventType::MouseWheel);
	return static_cast<MouseWheelEvent&> (event);
}

//------------------------------------------------------------------------
/**
 *	@ingroup new_in_4_11
 */
inline CButtonState buttonStateFromEventModifiers (const Modifiers& mods)
{
	CButtonState state;
	if (mods.has (ModifierKey::Control))
		state |= kControl;
	if (mods.has (ModifierKey::Shift))
		state |= kShift;
	if (mods.has (ModifierKey::Alt))
		state |= kAlt;
	return state;
}


//------------------------------------------------------------------------
} // VSTGUI
