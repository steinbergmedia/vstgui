// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "events.h"
#include "platform/platformfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace EventPrivate {
static uint64_t counter = 0;
} // EventPrivate

//------------------------------------------------------------------------
Event::Event () noexcept
: id (++EventPrivate::counter), timestamp (getPlatformFactory ().getTicks ())
{
}

//------------------------------------------------------------------------
CButtonState buttonStateFromEventModifiers (const Modifiers& mods)
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
CButtonState buttonStateFromMouseEvent (const MouseEvent& event)
{
	CButtonState state = buttonStateFromEventModifiers (event.modifiers);
	if (event.buttonState.has (MouseButton::Left))
		state |= kLButton;
	if (event.buttonState.has (MouseButton::Right))
		state |= kRButton;
	if (event.buttonState.has (MouseButton::Middle))
		state |= kMButton;
	if (event.buttonState.has (MouseButton::Fourth))
		state |= kButton4;
	if (event.buttonState.has (MouseButton::Fifth))
		state |= kButton5;
	if (auto downEvent = asMouseDownEvent (event))
	{
		if (downEvent->clickCount > 1)
			state |= kDoubleClick;
	}
	return state;
}

//------------------------------------------------------------------------
} // VSTGUI
