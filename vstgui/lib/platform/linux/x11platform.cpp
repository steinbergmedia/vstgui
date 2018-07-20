// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11platform.h"
#include "../../cfileselector.h"
#include "../../cframe.h"
#include "../../cstring.h"
#include "x11frame.h"
#include "cairobitmap.h"
#include <cassert>
#include <chrono>
#include <array>
#include <dlfcn.h>
#include <iostream>
#include <link.h>
#include <unordered_map>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

//------------------------------------------------------------------------
namespace VSTGUI {

struct X11FileSelector : CNewFileSelector
{
	X11FileSelector (CFrame* parent, Style style) : CNewFileSelector (parent), style (style) {}

	bool runInternal (CBaseObject* delegate) override
	{
		this->delegate = delegate;
		return false;
	}

	void cancelInternal () override {}

	bool runModalInternal () override { return false; }

	Style style;
	SharedPointer<CBaseObject> delegate;
};

//------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* parent, Style style)
{
	return new X11FileSelector (parent, style);
}

//------------------------------------------------------------------------
namespace X11 {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
struct Platform::Impl
{
	std::string path;
};

//------------------------------------------------------------------------
Platform& Platform::getInstance ()
{
	static Platform gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
Platform::Platform ()
{
	impl = std::unique_ptr<Impl> (new Impl);

	Cairo::Bitmap::setGetResourcePathFunc ([this]() {
		auto path = getPath ();
		path += "/Contents/Resources/";
		return path;
	});
}

//------------------------------------------------------------------------
Platform::~Platform ()
{
	Cairo::Bitmap::setGetResourcePathFunc ([]() { return std::string (); });
}

//------------------------------------------------------------------------
uint64_t Platform::getCurrentTimeMs ()
{
	using namespace std::chrono;
	return duration_cast<milliseconds> (steady_clock::now ().time_since_epoch ()).count ();
}

//------------------------------------------------------------------------
std::string Platform::getPath ()
{
	if (impl->path.empty () && soHandle)
	{
		struct link_map* map;
		if (dlinfo (soHandle, RTLD_DI_LINKMAP, &map) == 0)
		{
			auto path = std::string (map->l_name);
			for (int i = 0; i < 3; i++)
			{
				int delPos = path.find_last_of ('/');
				if (delPos == -1)
				{
					fprintf (stderr, "Could not determine bundle location.\n");
					return {}; // unexpected
				}
				path.erase (delPos, path.length () - delPos);
			}
			auto rp = realpath (path.data (), nullptr);
			path = rp;
			free (rp);
			std::swap (impl->path, path);
		}
	}
	return impl->path;
}

//------------------------------------------------------------------------
struct RunLoop::Impl : IEventHandler
{
	using WindowEventHandlerMap = std::unordered_map<uint32_t, IFrameEventHandler*>;

	SharedPointer<IRunLoop> runLoop;
	std::atomic<uint32_t> useCount{0};
	xcb_connection_t* xcbConnection{nullptr};
	WindowEventHandlerMap windowEventHandlerMap;
	uint32_t cursorFont{0};
	std::array<xcb_cursor_t, CCursorType::kCursorHand> cursors{XCB_CURSOR_NONE};

	void init (const SharedPointer<IRunLoop>& inRunLoop)
	{
		if (++useCount != 1)
			return;
		runLoop = inRunLoop;
		xcbConnection = xcb_connect (nullptr, nullptr);
		runLoop->registerEventHandler (xcb_get_file_descriptor (xcbConnection), this);

		std::string cursor ("cursor");
		cursorFont = xcb_generate_id (xcbConnection);
		xcb_open_font (xcbConnection, cursorFont, cursor.size (), cursor.data ());
	}

	void exit ()
	{
		if (--useCount != 0)
			return;
		if (xcbConnection)
		{
			if (cursorFont)
			{
				for (auto c : cursors)
				{
					if (c != XCB_CURSOR_NONE)
						xcb_free_cursor (xcbConnection, c);
				}
				xcb_close_font (xcbConnection, cursorFont);
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
		if (it == windowEventHandlerMap.end ())
			return;
		it->second->onEvent (event);
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
					dispatchEvent (*ev, ev->child);
					break;
				}
				case XCB_KEY_RELEASE:
				{
					auto ev = reinterpret_cast<xcb_key_release_event_t*> (event);
					dispatchEvent (*ev, ev->child);
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
				case XCB_CLIENT_MESSAGE:
				{
					auto ev = reinterpret_cast<xcb_client_message_event_t*> (event);
					dispatchEvent (*ev, ev->window);
					break;
				}
			}
			std::free (event);
		}
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
uint32_t RunLoop::getCursorID (CCursorType cursor)
{
	if (impl->cursors[cursor] == XCB_CURSOR_NONE)
	{
		impl->cursors[cursor] = xcb_generate_id (impl->xcbConnection);
		uint16_t which = XC_left_ptr;
		switch (cursor)
		{
			case kCursorDefault:
			{
				which = XC_left_ptr;
				break;
			}
			case kCursorHSize:
			{
				which = XC_sb_h_double_arrow;
				break;
			}
			case kCursorVSize:
			{
				which = XC_sb_v_double_arrow;
				break;
			}
			case kCursorNESWSize:
			case kCursorNWSESize:
			case kCursorSizeAll:
			{
				which = XC_crosshair;
				break;
			}
			case kCursorWait:
			{
				which = XC_watch;
				break;
			}
			case kCursorHand:
			{
				which = XC_hand2;
				break;
			}
		}
		xcb_create_glyph_cursor (impl->xcbConnection, impl->cursors[cursor], impl->cursorFont,
								 impl->cursorFont, which, which + 1, 0, 0, 0, 0xffff, 0xffff,
								 0xffff);
	}
	return impl->cursors[cursor];
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
