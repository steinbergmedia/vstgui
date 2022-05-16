// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11platform.h"
#include "../../cfileselector.h"
#include "../../cframe.h"
#include "../../cstring.h"
#include "../../events.h"
#include "x11frame.h"
#include "x11dragging.h"
#include "cairobitmap.h"
#include <cassert>
#include <chrono>
#include <array>
#include <dlfcn.h>
#include <iostream>
#include <locale>
#include <link.h>
#include <unordered_map>
#include <codecvt>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_util.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_aux.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <X11/Xlib.h>

// c++11 compile error workaround
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit
#undef None

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
namespace X11 {

//------------------------------------------------------------------------
namespace {

using VirtMap = std::unordered_map<xkb_keysym_t, VirtualKey>;
const VirtMap keyMap = {{XKB_KEY_BackSpace, VirtualKey::Back},
						{XKB_KEY_Tab, VirtualKey::Tab},
						{XKB_KEY_Clear, VirtualKey::Clear},
						{XKB_KEY_Return, VirtualKey::Return},
						{XKB_KEY_Pause, VirtualKey::Pause},
						{XKB_KEY_Escape, VirtualKey::Escape},
						{XKB_KEY_space, VirtualKey::Space},
						{XKB_KEY_End, VirtualKey::End},
						{XKB_KEY_Home, VirtualKey::Home},

						{XKB_KEY_Left, VirtualKey::Left},
						{XKB_KEY_Up, VirtualKey::Up},
						{XKB_KEY_Right, VirtualKey::Right},
						{XKB_KEY_Down, VirtualKey::Down},
						{XKB_KEY_Page_Up, VirtualKey::PageUp},
						{XKB_KEY_Page_Down, VirtualKey::PageDown},

						{XKB_KEY_Select, VirtualKey::Select},
						{XKB_KEY_Print, VirtualKey::Print},
						{XKB_KEY_KP_Enter, VirtualKey::Enter},
						{XKB_KEY_Insert, VirtualKey::Insert},
						{XKB_KEY_Delete, VirtualKey::Delete},
						{XKB_KEY_Help, VirtualKey::Help},
						// Numpads ???
						{XKB_KEY_KP_Multiply, VirtualKey::Multiply},
						{XKB_KEY_KP_Add, VirtualKey::Add},
						{XKB_KEY_KP_Separator, VirtualKey::Separator},
						{XKB_KEY_KP_Subtract, VirtualKey::Subtract},
						{XKB_KEY_KP_Decimal, VirtualKey::Decimal},
						{XKB_KEY_KP_Divide, VirtualKey::Divide},
						{XKB_KEY_F1, VirtualKey::F1},
						{XKB_KEY_F2, VirtualKey::F2},
						{XKB_KEY_F3, VirtualKey::F3},
						{XKB_KEY_F4, VirtualKey::F4},
						{XKB_KEY_F5, VirtualKey::F5},
						{XKB_KEY_F6, VirtualKey::F6},
						{XKB_KEY_F7, VirtualKey::F7},
						{XKB_KEY_F8, VirtualKey::F8},
						{XKB_KEY_F9, VirtualKey::F9},
						{XKB_KEY_F10, VirtualKey::F10},
						{XKB_KEY_F11, VirtualKey::F11},
						{XKB_KEY_F12, VirtualKey::F12},
						{XKB_KEY_Num_Lock, VirtualKey::NumLock},
						{XKB_KEY_Scroll_Lock, VirtualKey::Scroll}, // correct ?
#if 0
						{XKB_KEY_Shift_L, VirtualKey::SHIFT},
						{XKB_KEY_Shift_R, VirtualKey::SHIFT},
						{XKB_KEY_Control_L, VirtualKey::CONTROL},
						{XKB_KEY_Control_R, VirtualKey::CONTROL},
						{XKB_KEY_Alt_L, VirtualKey::ALT},
						{XKB_KEY_Alt_R, VirtualKey::ALT},
#endif
						{XKB_KEY_VoidSymbol, VirtualKey::None}};
const VirtMap shiftKeyMap = {{XKB_KEY_KP_Page_Up, VirtualKey::PageUp},
							 {XKB_KEY_KP_Page_Down, VirtualKey::PageDown},
							 {XKB_KEY_KP_Home, VirtualKey::Home},
							 {XKB_KEY_KP_End, VirtualKey::End}};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
struct RunLoop::Impl : IEventHandler
{
	using WindowEventHandlerMap = std::unordered_map<uint32_t, IFrameEventHandler*>;

	SharedPointer<IRunLoop> runLoop;
	std::atomic<uint32_t> useCount {0};
	xcb_connection_t* xcbConnection {nullptr};
	xcb_cursor_context_t* cursorContext {nullptr};
	xkb_context* xkbContext {nullptr};
	xkb_state* xkbState {nullptr};
	xkb_state* xkbUnprocessedState {nullptr};
	xkb_keymap* xkbKeymap {nullptr};
	WindowEventHandlerMap windowEventHandlerMap;
	std::array<xcb_cursor_t, CCursorType::kCursorIBeam + 1> cursors {{XCB_CURSOR_NONE}};
	KeyboardEvent lastUnprocessedKeyEvent;
	uint32_t lastUtf32KeyEventChar {0};

	void init (const SharedPointer<IRunLoop>& inRunLoop)
	{
		if (++useCount != 1)
			return;
		runLoop = inRunLoop;
		int screenNo;
		xcbConnection = xcb_connect (nullptr, &screenNo);
		runLoop->registerEventHandler (xcb_get_file_descriptor (xcbConnection), this);
		auto screen = xcb_aux_get_screen (xcbConnection, screenNo);
		xcb_cursor_context_new (xcbConnection, screen, &cursorContext);

		xcb_xkb_use_extension (xcbConnection, XKB_X11_MIN_MAJOR_XKB_VERSION,
							   XKB_X11_MIN_MINOR_XKB_VERSION);
		xkbContext = xkb_context_new (XKB_CONTEXT_NO_FLAGS);

		int32_t deviceId = xkb_x11_get_core_keyboard_device_id (xcbConnection);
		if (deviceId > -1)
		{
			xkbKeymap = xkb_x11_keymap_new_from_device (xkbContext, xcbConnection, deviceId,
														XKB_KEYMAP_COMPILE_NO_FLAGS);
			xkbState = xkb_state_new (xkbKeymap);
			xkbUnprocessedState = xkb_state_new (xkbKeymap);

			auto xkbStateCookie = xcb_xkb_get_state (xcbConnection, deviceId);
			auto* xkbStateReply = xcb_xkb_get_state_reply (xcbConnection, xkbStateCookie, nullptr);
			if (xkbStateReply)
			{
				xkb_state_update_mask (xkbState,
									   xkbStateReply->baseMods,
									   xkbStateReply->latchedMods,
									   xkbStateReply->lockedMods,
									   xkbStateReply->baseGroup,
									   xkbStateReply->latchedGroup,
									   xkbStateReply->lockedGroup);
				free (xkbStateReply);
			}
		}
	}

	void exit ()
	{
		if (--useCount != 0)
			return;
		if (xcbConnection)
		{
			if (xkbUnprocessedState)
				xkb_state_unref (xkbUnprocessedState);
			if (xkbState)
				xkb_state_unref (xkbState);
			if (xkbKeymap)
				xkb_keymap_unref (xkbKeymap);
			if (xkbContext)
				xkb_context_unref (xkbContext);
			if (cursorContext)
			{
				for (auto c : cursors)
				{
					if (c != XCB_CURSOR_NONE)
						xcb_free_cursor (xcbConnection, c);
				}
				xcb_cursor_context_free (cursorContext);
			}

			xcb_disconnect (xcbConnection);
		}
		runLoop->unregisterEventHandler (this);
		runLoop = nullptr;
	}

	template<typename T>
	void dispatchEvent (T& event, xcb_window_t windowId)
	{
		auto it = windowEventHandlerMap.find (windowId);

		if (it != windowEventHandlerMap.end ())
		{
			it->second->onEvent (event);
			return;
		}

		// we may receive a proxied drag-and-drop event; if that is the case,
		// the window has the attribute XdndProxy, and acts as a proxy for the
		// other window designated by the value of this attribute.
		if (std::is_same<T, xcb_client_message_event_t>::value)
		{
			xcb_client_message_event_t& cmsg =
				reinterpret_cast<xcb_client_message_event_t&> (event);

			if (isXdndClientMessage (cmsg))
			{
				xcb_window_t targetId = getXdndProxy (windowId);
				if (targetId != 0)
					it = windowEventHandlerMap.find (targetId);
				if (it != windowEventHandlerMap.end ())
				{
					it->second->onEvent (cmsg, windowId);
					return;
				}
			}
		}
	}

	//------------------------------------------------------------------------
	void onKeyEvent (const xcb_key_press_event_t& event, bool isKeyDown)
	{
		if (!xkbUnprocessedState)
			return;

		KeyboardEvent keyEvent;
		keyEvent.type = isKeyDown ? EventType::KeyDown : EventType::KeyUp;

		if (event.state & XCB_MOD_MASK_SHIFT)
			keyEvent.modifiers.add (ModifierKey::Shift);
		if (event.state & XCB_MOD_MASK_CONTROL)
			keyEvent.modifiers.add (ModifierKey::Control);
		if (event.state & (XCB_MOD_MASK_1 | XCB_MOD_MASK_5))
			keyEvent.modifiers.add (ModifierKey::Alt);

		auto ksym = xkb_state_key_get_one_sym (xkbUnprocessedState, event.detail);
		xkb_state_update_key (xkbState, event.detail, isKeyDown ? XKB_KEY_DOWN : XKB_KEY_UP);

		VirtMap::const_iterator it;
		bool ksymMapped = false;
		if (!ksymMapped && keyEvent.modifiers.has(ModifierKey::Shift))
		{
			it = shiftKeyMap.find (ksym);
			ksymMapped = it != shiftKeyMap.end ();
		}
		if (!ksymMapped)
		{
			it = keyMap.find (ksym);
			ksymMapped = it != keyMap.end ();
		}

		if (ksymMapped)
		{
			keyEvent.virt = it->second;
			lastUtf32KeyEventChar = 0;
		}
		else
		{
			keyEvent.character = xkb_state_key_get_utf32 (xkbState, event.detail);
			lastUtf32KeyEventChar = keyEvent.character;
		}

		lastUnprocessedKeyEvent = std::move (keyEvent);
	}

	void onEvent () override
	{
		while (auto event = xcb_poll_for_event (xcbConnection))
		{
			auto type = event->response_type & ~0x80;
			switch (type)
			{
				case XCB_KEY_PRESS:
				{
					auto ev = reinterpret_cast<xcb_key_press_event_t*> (event);
					onKeyEvent (*ev, true);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_KEY_RELEASE:
				{
					auto ev = reinterpret_cast<xcb_key_release_event_t*> (event);
					onKeyEvent (*ev, false);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_BUTTON_PRESS:
				{
					auto ev = reinterpret_cast<xcb_button_press_event_t*> (event);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_BUTTON_RELEASE:
				{
					auto ev = reinterpret_cast<xcb_button_release_event_t*> (event);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_MOTION_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_motion_notify_event_t*> (event);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_ENTER_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_enter_notify_event_t*> (event);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_LEAVE_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_leave_notify_event_t*> (event);
					dispatchEvent (*ev, ev->event);
					break;
				}
				case XCB_EXPOSE:
				{
					auto ev = reinterpret_cast<xcb_expose_event_t*> (event);
					dispatchEvent (*ev, ev->window);
					break;
				}
				case XCB_UNMAP_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_unmap_notify_event_t*> (event);
					break;
				}
				case XCB_MAP_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_map_notify_event_t*> (event);
					dispatchEvent (*ev, ev->window);
					break;
				}
				case XCB_CONFIGURE_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_configure_notify_event_t*> (event);
					break;
				}
				case XCB_PROPERTY_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_property_notify_event_t*> (event);
					dispatchEvent (*ev, ev->window);
					break;
				}
				case XCB_SELECTION_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_selection_notify_event_t*> (event);
					dispatchEvent (*ev, ev->requestor);
					break;
				}
				case XCB_CLIENT_MESSAGE:
				{
					auto ev = reinterpret_cast<xcb_client_message_event_t*> (event);
					dispatchEvent (*ev, ev->window);
					break;
				}
				case XCB_FOCUS_IN:
				case XCB_FOCUS_OUT:
				{
					auto ev = reinterpret_cast<xcb_focus_in_event_t*> (event);
					dispatchEvent (*ev, ev->event);
					break;
				}
			}
			std::free (event);
		}
		xcb_aux_sync (xcbConnection);
		xcb_flush (xcbConnection);
	}
};

//------------------------------------------------------------------------
RunLoop& RunLoop::instance ()
{
	static RunLoop gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
void RunLoop::init (const SharedPointer<IRunLoop>& runLoop)
{
	instance ().impl->init (runLoop);
}

//------------------------------------------------------------------------
void RunLoop::exit ()
{
	instance ().impl->exit ();
}

//------------------------------------------------------------------------
const SharedPointer<IRunLoop> RunLoop::get ()
{
	return instance ().impl->runLoop;
}

//------------------------------------------------------------------------
RunLoop::RunLoop ()
{
	impl = std::unique_ptr<Impl> (new Impl);
}

//------------------------------------------------------------------------
RunLoop::~RunLoop () noexcept = default;

//------------------------------------------------------------------------
void RunLoop::registerWindowEventHandler (uint32_t windowId, IFrameEventHandler* handler)
{
	impl->windowEventHandlerMap.emplace (windowId, handler);
}

//------------------------------------------------------------------------
void RunLoop::unregisterWindowEventHandler (uint32_t windowId)
{
	auto it = impl->windowEventHandlerMap.find (windowId);
	if (it == impl->windowEventHandlerMap.end ())
		return;
	impl->windowEventHandlerMap.erase (it);
}

//------------------------------------------------------------------------
xcb_connection_t* RunLoop::getXcbConnection () const
{
	return impl->xcbConnection;
}

//------------------------------------------------------------------------
namespace {

template<typename T>
uint32_t makeCursor (xcb_cursor_context_t* context, const T& names)
{
	for (auto& name : names)
	{
		auto result = xcb_cursor_load_cursor (context, name);
		if (result != XCB_CURSOR_NONE)
			return result;
	}
	return XCB_CURSOR_NONE;
}

template<size_t count>
using CharPtrArray = std::array<const char*, count>;

constexpr auto CursorDefaultNames = //
	CharPtrArray<4> {"left_ptr", "arrow", "dnd-none", "op_left_arrow"};
constexpr auto CursorWaitNames = //
	CharPtrArray<3> {"wait", "watch", "progress"};
constexpr auto CursorHSizeNames = //
	CharPtrArray<8> {"size_hor", "sb_h_double_arrow", "h_double_arrow", "e-resize",
					 "w-resize", "row-resize",		  "right_side",		"left_side"};
constexpr auto CursorVSizeNames = //
	CharPtrArray<12> {"size_ver",	   "sb_v_double_arrow", "v_double_arrow",	"n-resize",
					  "s-resize",	   "col-resize",		"top_side",			"bottom_side",
					  "base_arrow_up", "base_arrow_down",	"based_arrow_down", "based_arrow_up"};
constexpr auto CursorNESWSizeNames = //
	CharPtrArray<5> {"size_bdiag", "fd_double_arrow", "bottom_left_corner", "top_right_corner"};
constexpr auto CursorNWSESizeNames = //
	CharPtrArray<5> {"size_fdiag", "bd_double_arrow", "bottom_right_corner", "top_left_corner"};
constexpr auto CursorSizeAllNames = //
	CharPtrArray<4> {"cross", "diamond-cross", "cross-reverse", "crosshair"};
constexpr auto CursorCopyNames = //
	CharPtrArray<2> {"dnd-copy", "copy"};
constexpr auto CursorNotAllowedNames = //
	CharPtrArray<4> {"forbidden", "circle", "dnd-no-drop", "not-allowed"};
constexpr auto CursorHandNames = //
	CharPtrArray<4> {"openhand", "hand1", "all_scroll", "all-scroll"};
constexpr auto CursorIBeamNames = //
	CharPtrArray<3> {"ibeam", "xterm", "text"};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
uint32_t RunLoop::getCursorID (CCursorType cursor)
{
	if (impl->cursors[cursor] == XCB_CURSOR_NONE && impl->cursorContext)
	{
		uint32_t cursorID = XCB_CURSOR_NONE;
		switch (cursor)
		{
			case kCursorDefault:
				cursorID = makeCursor (impl->cursorContext, CursorDefaultNames);
				break;
			case kCursorWait:
				cursorID = makeCursor (impl->cursorContext, CursorWaitNames);
				break;
			case kCursorHSize:
				cursorID = makeCursor (impl->cursorContext, CursorHSizeNames);
				break;
			case kCursorVSize:
				cursorID = makeCursor (impl->cursorContext, CursorVSizeNames);
				break;
			case kCursorNESWSize:
				cursorID = makeCursor (impl->cursorContext, CursorNESWSizeNames);
				break;
			case kCursorNWSESize:
				cursorID = makeCursor (impl->cursorContext, CursorNWSESizeNames);
				break;
			case kCursorSizeAll:
				cursorID = makeCursor (impl->cursorContext, CursorSizeAllNames);
				break;
			case kCursorCopy:
				cursorID = makeCursor (impl->cursorContext, CursorCopyNames);
				break;
			case kCursorNotAllowed:
				cursorID = makeCursor (impl->cursorContext, CursorNotAllowedNames);
				break;
			case kCursorHand:
				cursorID = makeCursor (impl->cursorContext, CursorHandNames);
				break;
			case kCursorIBeam:
				cursorID = makeCursor (impl->cursorContext, CursorIBeamNames);
				break;
		}
		impl->cursors[cursor] = cursorID;
	}
	return impl->cursors[cursor];
}

//------------------------------------------------------------------------
KeyboardEvent&& RunLoop::getCurrentKeyEvent () const
{
	return std::move (impl->lastUnprocessedKeyEvent);
}

//------------------------------------------------------------------------
Optional<UTF8String> RunLoop::convertCurrentKeyEventToText () const
{
	if (impl->lastUtf32KeyEventChar == 0)
		return {};

	try
	{

		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return Optional<UTF8String> (UTF8String (conv.to_bytes (impl->lastUtf32KeyEventChar)));
	}
	catch (...)
	{
	}
	return {};
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
