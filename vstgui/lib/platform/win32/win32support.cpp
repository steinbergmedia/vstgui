// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32support.h"

#if WINDOWS

#include "../../events.h"
#include "../common/fileresourceinputstream.h"
#include "../platform_win32.h"
#include "win32factory.h"

#include <shlwapi.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
HINSTANCE GetInstance ()
{
	if (auto wf = getPlatformFactory ().asWin32Factory ())
		return wf->getInstance ();
	return nullptr;
}

//-----------------------------------------------------------------------------
ID2D1Factory* getD2DFactory ()
{
	return getPlatformFactory ().asWin32Factory ()->getD2DFactory ();
}

//-----------------------------------------------------------------------------
IWICImagingFactory* getWICImageingFactory ()
{
	return getPlatformFactory ().asWin32Factory ()->getWICImagingFactory ();
}

//-----------------------------------------------------------------------------
IDWriteFactory* getDWriteFactory ()
{
	return getPlatformFactory ().asWin32Factory ()->getDirectWriteFactory ();
}

//-----------------------------------------------------------------------------
void useD2D () {}

//-----------------------------------------------------------------------------
void unuseD2D () {}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VirtualKey translateWinVirtualKey (WPARAM winVKey)
{
	switch (winVKey)
	{
		case VK_BACK: return VirtualKey::Back;
		case VK_TAB: return VirtualKey::Tab;
		case VK_CLEAR: return VirtualKey::Clear;
		case VK_RETURN: return VirtualKey::Return;
		case VK_PAUSE: return VirtualKey::Pause;
		case VK_ESCAPE: return VirtualKey::Escape;
		case VK_SPACE: return VirtualKey::Space;
// TODO:		case VK_NEXT: return VirtualKey::Next;
		case VK_END: return VirtualKey::End;
		case VK_HOME: return VirtualKey::Home;
		case VK_LEFT: return VirtualKey::Left;
		case VK_RIGHT: return VirtualKey::Right;
		case VK_UP: return VirtualKey::Up;
		case VK_DOWN: return VirtualKey::Down;
		case VK_PRIOR: return VirtualKey::PageUp;
		case VK_NEXT: return VirtualKey::PageDown;
		case VK_SELECT: return VirtualKey::Select;
		case VK_PRINT: return VirtualKey::Print;
		case VK_SNAPSHOT: return VirtualKey::Snapshot;
		case VK_INSERT: return VirtualKey::Insert;
		case VK_DELETE: return VirtualKey::Delete;
		case VK_HELP: return VirtualKey::Help;
		case VK_NUMPAD0: return VirtualKey::NumPad0;
		case VK_NUMPAD1: return VirtualKey::NumPad1;
		case VK_NUMPAD2: return VirtualKey::NumPad2;
		case VK_NUMPAD3: return VirtualKey::NumPad3;
		case VK_NUMPAD4: return VirtualKey::NumPad4;
		case VK_NUMPAD5: return VirtualKey::NumPad5;
		case VK_NUMPAD6: return VirtualKey::NumPad6;
		case VK_NUMPAD7: return VirtualKey::NumPad7;
		case VK_NUMPAD8: return VirtualKey::NumPad8;
		case VK_NUMPAD9: return VirtualKey::NumPad9;
		case VK_MULTIPLY: return VirtualKey::Multiply;
		case VK_ADD: return VirtualKey::Add;
		case VK_SEPARATOR: return VirtualKey::Separator;
		case VK_SUBTRACT: return VirtualKey::Subtract;
		case VK_DECIMAL: return VirtualKey::Decimal;
		case VK_DIVIDE: return VirtualKey::Divide;
		case VK_F1: return VirtualKey::F1;
		case VK_F2: return VirtualKey::F2;
		case VK_F3: return VirtualKey::F3;
		case VK_F4: return VirtualKey::F4;
		case VK_F5: return VirtualKey::F5;
		case VK_F6: return VirtualKey::F6;
		case VK_F7: return VirtualKey::F7;
		case VK_F8: return VirtualKey::F8;
		case VK_F9: return VirtualKey::F9;
		case VK_F10: return VirtualKey::F10;
		case VK_F11: return VirtualKey::F11;
		case VK_F12: return VirtualKey::F12;
		case VK_NUMLOCK: return VirtualKey::NumLock;
		case VK_SCROLL: return VirtualKey::Scroll;
		case VK_SHIFT: return VirtualKey::ShiftModifier;
		case VK_CONTROL: return VirtualKey::ControlModifier;
		case VK_MENU: return VirtualKey::AltModifier;
		case VK_OEM_PLUS: return VirtualKey::Equals;
	}
	return VirtualKey::None;
}

//-----------------------------------------------------------------------------
void updateModifiers (Modifiers& modifiers)
{
	if (GetAsyncKeyState (VK_SHIFT) < 0)
		modifiers.add (ModifierKey::Shift);
	if (GetAsyncKeyState (VK_CONTROL) < 0)
		modifiers.add (ModifierKey::Control);
	if (GetAsyncKeyState (VK_MENU) < 0)
		modifiers.add (ModifierKey::Alt);
	if (GetAsyncKeyState (VK_LWIN) < 0 || GetAsyncKeyState (VK_RWIN) < 0)
		modifiers.add (ModifierKey::Super);
}

//-----------------------------------------------------------------------------
Optional<KeyboardEvent> keyMessageToKeyboardEvent (WPARAM wParam, LPARAM lParam)
{
	KeyboardEvent event;
	updateModifiers (event.modifiers);
	event.virt = translateWinVirtualKey (wParam);
	if (event.virt != VirtualKey::None)
		return Optional<KeyboardEvent> (std::move (event));
	return {};
}


/// @endcond ignore

} // VSTGUI

#endif // WINDOWS
