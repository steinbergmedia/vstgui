// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#include "waylandplatform.h"
#include "linuxfactory.h"
#include "../../cfileselector.h"
#include "../../cframe.h"
#include "../../cstring.h"
#include "../../events.h"
#include "waylandframe.h"
// #include "x11dragging.h"
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
#include <xkbcommon/xkbcommon.h>
//#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "waylandclientcontext.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
namespace Wayland {

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
	SharedPointer<IWaylandHost> waylandHost;
	std::atomic<uint32_t> useCount {0};
	cairo_device_t* device {nullptr};
	wl_display* display {nullptr};
	WaylandClientContext clientContext;

	bool inDispatch {false};

	Impl ()
	{
	}

	void init (const SharedPointer<IWaylandHost>& inWaylandHost)
	{
		if (++useCount != 1)
			return;

		waylandHost = inWaylandHost;

		if (waylandHost == nullptr)
			return;

		display = waylandHost->openWaylandConnection ();
		if (display == nullptr)
			return;

		clientContext.initWayland (display);

		RunLoop::get ()->registerEventHandler (wl_display_get_fd (display), this);
		flush ();
	}

	void exit ()
	{
		if (--useCount != 0)
			return;

		cairo_device_finish (device);
		cairo_device_destroy (device);
		device = nullptr;

		RunLoop::get ()->unregisterEventHandler (this);

		flush ();

		if (waylandHost && display)
			waylandHost->closeWaylandConnection (display);
		waylandHost = nullptr;

		display = nullptr;
	}

	void flush ()
	{
		if (display && !inDispatch)
			wl_display_flush (display);
	}

	// IEventHandler
	void onEvent () final
	{
		inDispatch = true;
		if (wl_display_prepare_read (display) == 0)
			wl_display_read_events (display);
		wl_display_dispatch_pending (display);
		inDispatch = false;
		flush ();
	}
};

//------------------------------------------------------------------------
RunLoop& RunLoop::instance ()
{
	static RunLoop gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
void RunLoop::init (const SharedPointer<IWaylandHost>& waylandHost)
{
	instance ().impl->init (waylandHost);
}

//------------------------------------------------------------------------
void RunLoop::exit () { instance ().impl->exit (); }

//------------------------------------------------------------------------
void RunLoop::flush () { instance ().impl->flush (); }

//------------------------------------------------------------------------
const SharedPointer<IRunLoop> RunLoop::get ()
{
	return getPlatformFactory ().asLinuxFactory ()->getRunLoop ();
}

//------------------------------------------------------------------------
wl_display* RunLoop::getDisplay () { return instance ().impl->display; }

//------------------------------------------------------------------------
using namespace WaylandServerDelegate;

const IWaylandClientContext& RunLoop::getClientContext ()
{
	return instance ().impl->clientContext;
}

//------------------------------------------------------------------------
bool RunLoop::hasPointerInput ()
{
	return (instance ().impl->clientContext.getSeatCapabilities () & WL_SEAT_CAPABILITY_POINTER) != 0;
}

//------------------------------------------------------------------------
bool RunLoop::hasKeyboardInput ()
{
	return (instance ().impl->clientContext.getSeatCapabilities () & WL_SEAT_CAPABILITY_KEYBOARD) != 0;
}

//------------------------------------------------------------------------
RunLoop::RunLoop () { impl = std::unique_ptr<Impl> (new Impl); }

//------------------------------------------------------------------------
RunLoop::~RunLoop () noexcept = default;

//------------------------------------------------------------------------
void RunLoop::setDevice (cairo_device_t* device)
{
	if (impl->device != device)
	{
		cairo_device_destroy (impl->device);
		impl->device = cairo_device_reference (device);
	}
}

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
