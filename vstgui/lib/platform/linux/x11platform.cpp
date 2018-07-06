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
#include <dlfcn.h>
#include <iostream>
#include <link.h>
#include <unordered_map>
#include <xcb/xcb.h>
#include <X11/Xlib.h>

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
struct Platform::Impl : IEventHandler
{
	using WindowEventHandlerMap = std::unordered_map<uint32_t, IFrameEventHandler*>;

	xcb_connection_t* xcbConnection{nullptr};
	std::string path;
	WindowEventHandlerMap windowEventHandlerMap;

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
			switch (event->response_type & ~0x80)
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
					dispatchEvent (*ev, ev->child);
					break;
				}
				case XCB_BUTTON_RELEASE:
				{
					auto ev = reinterpret_cast<xcb_button_release_event_t*> (event);
					dispatchEvent (*ev, ev->child);
					break;
				}
				case XCB_MOTION_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_motion_notify_event_t*> (event);
					dispatchEvent (*ev, ev->child);
					break;
				}
				case XCB_ENTER_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_enter_notify_event_t*> (event);
					dispatchEvent (*ev, ev->child);
					break;
				}
				case XCB_LEAVE_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_leave_notify_event_t*> (event);
					dispatchEvent (*ev, ev->child);
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
					break;
				}
				case XCB_CONFIGURE_NOTIFY:
				{
					auto ev = reinterpret_cast<xcb_configure_notify_event_t*> (event);
					break;
				}
			}
			std::free (event);
		}
	}
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

	impl->xcbConnection = xcb_connect (nullptr, nullptr);

	Cairo::Bitmap::setGetResourcePathFunc ([this]() {
		auto path = getPath ();
		path += "/Contents/Resources/";
		return path;
	});

	if (auto runLoop = RunLoop::get ())
		runLoop->registerEventHandler (xcb_get_file_descriptor (impl->xcbConnection), impl.get ());
}

//------------------------------------------------------------------------
Platform::~Platform ()
{
	Cairo::Bitmap::setGetResourcePathFunc ([]() { return std::string (); });

	if (impl->xcbConnection)
		xcb_disconnect (impl->xcbConnection);

	if (auto runLoop = RunLoop::get ())
		runLoop->unregisterEventHandler (impl.get ());
}

//------------------------------------------------------------------------
uint64_t Platform::getCurrentTimeMs ()
{
	using namespace std::chrono;
	return duration_cast<milliseconds> (steady_clock::now ().time_since_epoch ()).count ();
}

//------------------------------------------------------------------------
void Platform::registerWindowEventHandler (uint32_t windowId, IFrameEventHandler* handler)
{
	impl->windowEventHandlerMap.emplace (windowId, handler);
}

//------------------------------------------------------------------------
void Platform::unregisterWindowEventHandler (uint32_t windowId)
{
	auto it = impl->windowEventHandlerMap.find (windowId);
	if (it == impl->windowEventHandlerMap.end ())
		return;
	impl->windowEventHandlerMap.erase (it);
}

//------------------------------------------------------------------------
xcb_connection_t* Platform::getXcbConnection () const
{
	return impl->xcbConnection;
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
} // X11
} // VSTGUI
