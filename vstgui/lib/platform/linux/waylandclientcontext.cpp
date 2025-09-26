// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "waylandclientcontext.h"

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

// Wayland globals
#include "wayland-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "linux-dmabuf-v1-server-protocol.h"
#include "linux-dmabuf-v1-client-protocol.h"

using namespace WaylandServerDelegate;

namespace VSTGUI::Wayland  {

//------------------------------------------------------------------------
using ContextListeners = std::vector<IContextListener*>;
using NamedWaylandOutput = std::pair<uint32_t, WaylandOutput>;
using WaylandOutputs = std::vector<NamedWaylandOutput>;
using StringType = std::string;

//------------------------------------------------------------------------
// WaylandGlobals
// https://wayland.freedesktop.org/docs/html/
// https://wayland-book.com/introduction.html
//------------------------------------------------------------------------
struct WaylandGlobals
{
	using WlInterfaceName = std::string;
	using Globals = std::unordered_map<uint32_t, WlInterfaceName>;

	wl_compositor* compositor {nullptr};
	wl_seat* seat {nullptr};
	wl_shm* shm {nullptr};
	wl_subcompositor* subcompositor {nullptr};
	xdg_wm_base* wm_base {nullptr};
	zwp_linux_dmabuf_v1* dmabuf {nullptr};

	Globals objects; // Hold a map of all globals' interface names for better tracking!
};

//------------------------------------------------------------------------
static void handlePing (void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial)
{
	auto globals = reinterpret_cast<WaylandGlobals*> (data);
	xdg_wm_base_pong (globals->wm_base, serial);
}

//------------------------------------------------------------------------
static void addXdgWmBaseListener (WaylandGlobals& globals)
{
	static const struct xdg_wm_base_listener xdgWmBaseListener = {
	    .ping = handlePing,
	};

	xdg_wm_base_add_listener (globals.wm_base, &xdgWmBaseListener, &globals);
}

//------------------------------------------------------------------------
static void handleGlobal (void* data, struct wl_registry* registry, uint32_t name, const char* interface,
	            uint32_t version)
{
	printf ("interface: '%s', version: %d, name: %d\n", interface, version, name);

	auto& globals = *reinterpret_cast<WaylandGlobals*> (data);
	globals.objects.insert ({name, {interface}});
	if (std::string_view (interface) == wl_compositor_interface.name)
	{
		static constexpr uint32_t kVersion = 6;
		void* object =
			wl_registry_bind (registry, name, &wl_compositor_interface, std::min (kVersion, version));
		auto compositor = reinterpret_cast<wl_compositor*> (object);
		globals.compositor = compositor;
		return;
	}
	if (std::string_view (interface) == wl_subcompositor_interface.name)
	{
		static constexpr uint32_t kVersion = 1;
		void* object =
			wl_registry_bind (registry, name, &wl_subcompositor_interface, std::min (kVersion, version));
		auto subcompositor = reinterpret_cast<wl_subcompositor*> (object);
		globals.subcompositor = subcompositor;
		return;
	}
	if (std::string_view (interface) == wl_shm_interface.name)
	{
		static constexpr uint32_t kVersion = 1;
		void* object = wl_registry_bind (registry, name, &wl_shm_interface, std::min (kVersion, version));
		auto shm = reinterpret_cast<wl_shm*> (object);
		globals.shm = shm;
		return;
	}
	if (std::string_view (interface) == xdg_wm_base_interface.name)
	{
		static constexpr uint32_t kVersion = 6;
		void* object =
			wl_registry_bind (registry, name, &xdg_wm_base_interface, std::min (kVersion, version));
		auto wm_base = reinterpret_cast<xdg_wm_base*> (object);
		globals.wm_base = wm_base;
		return;
	}
	if (std::string_view (interface) == wl_seat_interface.name)
	{
		static constexpr uint32_t kVersion = 8;
		void* object = wl_registry_bind (registry, name, &wl_seat_interface, std::min (kVersion, version));
		auto seat = reinterpret_cast<wl_seat*> (object);
		globals.seat = seat;
		return;
	}
	if (std::string_view (interface) == wl_output_interface.name)
	{
		// We bind outputs in the WaylandClientContext
		/*
		static constexpr uint32_t kVersion = 3;
		void* object =
			wl_registry_bind (registry, name, &wl_output_interface, std::min (kVersion, version));
		auto output = reinterpret_cast<wl_output*> (object);
		return;
		*/
	}
	if (std::string_view (interface) == zwp_linux_dmabuf_v1_interface.name)
	{
		static constexpr uint32_t kVersion = 4;
		void* object = wl_registry_bind (
			registry, name, &zwp_linux_dmabuf_v1_interface, std::min (kVersion, version));
		auto dmabuf = reinterpret_cast<zwp_linux_dmabuf_v1*> (object);
		globals.dmabuf = dmabuf;
		return;
	}
}

//------------------------------------------------------------------------
static void handleGlobalRemove (void* data, struct wl_registry* registry, uint32_t name)
{
	auto globals = reinterpret_cast<WaylandGlobals*> (data);
	const auto iter = globals->objects.find (name);
	if (iter == globals->objects.end ())
		return;

	globals->objects.erase (iter);
}

//------------------------------------------------------------------------
static bool bindGlobals (wl_display* display, WaylandGlobals& globals)
{
	if (!display)
		return false;

	static const struct wl_registry_listener registry_listener = {
	    .global = handleGlobal,
	    .global_remove = handleGlobalRemove,
	};

	auto registry = wl_display_get_registry (display);
	wl_registry_add_listener (registry, &registry_listener, &globals);

	// Roundtrip to call the listener's callbacks.
	wl_display_roundtrip (display);

	addXdgWmBaseListener (globals);

	return true;
}

//------------------------------------------------------------------------
// WaylandClientContext::Impl
//------------------------------------------------------------------------
struct WaylandClientContext::Impl
{
	wl_display* display = nullptr;
	WaylandGlobals globals;
	WaylandOutputs outputs;
	ContextListeners contextListeners;
	uint32_t seatCapabilities = 0;
	StringType seatName;

	void setSeatCapabilities (int capabilities);
	void notifyListeners (IContextListener::ChangeType changeType);

	void addWaylandOutput (uint32_t name, wl_output* out);
	void removeWaylandOutput (uint32_t name);
	void addRegistryListener ();
	void addSeatListener ();
	void addOutputsListener ();
};

//------------------------------------------------------------------------
void WaylandClientContext::Impl::setSeatCapabilities (int capabilities)
{
	seatCapabilities = capabilities;
	// seatCapabilities &= ~(WL_SEAT_CAPABILITY_KEYBOARD);
	notifyListeners (IContextListener::kSeatCapabilitiesChanged);
}

//------------------------------------------------------------------------
void WaylandClientContext::Impl::notifyListeners (IContextListener::ChangeType changeType)
{
	for (auto& el : contextListeners)
		el->contextChanged (changeType);
}

//------------------------------------------------------------------------
void WaylandClientContext::Impl::addWaylandOutput (uint32_t name, wl_output* output)
{
	outputs.push_back ({name, {output}});
	notifyListeners (IContextListener::kOutputsChanged);
}

//------------------------------------------------------------------------
void WaylandClientContext::Impl::removeWaylandOutput (uint32_t name)
{
	auto iter = std::find_if (outputs.begin (), outputs.end (),
	                          [name] (const auto& el) { return el.first == name; });

	if (iter == outputs.end ())
		return;

	outputs.erase (iter);
	notifyListeners (IContextListener::kOutputsChanged);
}

//------------------------------------------------------------------------
void WaylandClientContext::Impl::addRegistryListener ()
{
	static const struct wl_registry_listener registry_listener = {
	    .global =
	        [] (void* data, struct wl_registry* registry, uint32_t name, const char* interface,
	            uint32_t version) {
		        // We only track outputs here for now
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        if (std::string_view (interface) == wl_output_interface.name)
		        {
			        static constexpr uint32_t kVersion = 3;
			        void* object =
			            wl_registry_bind (registry, name, &wl_output_interface, std::min (kVersion, version));
			        auto output = reinterpret_cast<wl_output*> (object);
			        self->addWaylandOutput (name, output);
			        return;
		        }

	        },
	    .global_remove =
	        [] (void* data, struct wl_registry* registry, uint32_t name) {
		        // We only track outputs here for now
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        self->removeWaylandOutput (name);
	        },
	};

	auto registry = wl_display_get_registry (display);
	wl_registry_add_listener (registry, &registry_listener, this);
}

//------------------------------------------------------------------------
void WaylandClientContext::Impl::addSeatListener ()
{
	static const struct wl_seat_listener listener = {
	    .capabilities =
	        [] (void* data, struct wl_seat* wl_seat, uint32_t capabilities) {
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        self->setSeatCapabilities (capabilities);
	        },
	    .name =
	        [] (void* data, struct wl_seat* wl_seat, const char* name) {
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        if (name)
			        self->seatName = name;
	        }};

	wl_seat_add_listener (globals.seat, &listener, this);
}

//------------------------------------------------------------------------
void WaylandClientContext::Impl::addOutputsListener ()
{
	static const wl_output_listener listener = {
	    .geometry =
	        [] (void* data, wl_output* wl_output, int32_t x, int32_t y, int32_t physical_width,
	            int32_t physical_height, int32_t subpixel, const char* make, const char* model,
	            int32_t transform) {
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        auto iter = std::find_if (
		            self->outputs.begin (), self->outputs.end (),
		            [wl_output] (const auto& el) { return el.second.handle == wl_output; });

		        if (iter != self->outputs.end ())
		        {
			        WaylandOutput& output = iter->second;
			        output.handle = wl_output;
			        output.x = x;
			        output.y = y;
			        output.physicalWidth = physical_width;
			        output.physicalHeight = physical_height;
			        output.subPixelOrientation = subpixel;
			        output.transformType = transform;
			        const std::string_view makeStr {make, sizeof (output.manufacturer)};
			        std::copy (makeStr.begin (), makeStr.end (), output.manufacturer);
			        const std::string_view modelStr {model, sizeof (output.model)};
			        std::copy (modelStr.begin (), modelStr.end (), output.model);
		        }
	        },

	    .mode =
	        [] (void* data, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height,
	            int32_t refresh) {
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        auto iter = std::find_if (
		            self->outputs.begin (), self->outputs.end (),
		            [wl_output] (const auto& el) { return el.second.handle == wl_output; });

		        if (iter != self->outputs.end ())
		        {
			        auto& output = iter->second;
			        output.width = width;
			        output.height = height;
			        output.refreshRate = refresh;
		        }
	        },

	    .done =
	        [] (void* data, wl_output* wl_output) {
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        self->notifyListeners (
		            IContextListener::kOutputsChanged);
	        },

	    .scale =
	        [] (void* data, wl_output* wl_output, int32_t factor) {
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
		        auto iter = std::find_if (
		            self->outputs.begin (), self->outputs.end (),
		            [wl_output] (const auto& el) { return el.second.handle == wl_output; });

		        if (iter != self->outputs.end ())
		        {
			        auto& output = iter->second;
			        output.scaleFactor = factor;
		        }
	        },

	    .name =
	        [] (void* data, wl_output* wl_output, const char* name) {
		        // Hmm, was never called.
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
	        },

	    .description =
	        [] (void* data, wl_output* wl_output, const char* description) {
		        // Hmm, was never called.
		        auto self = reinterpret_cast<WaylandClientContext::Impl*> (data);
	        },
	};

	for (auto& output : outputs)
		wl_output_add_listener (output.second.handle, &listener, this);
}

//------------------------------------------------------------------------
// WaylandClientContext
//------------------------------------------------------------------------
WaylandClientContext::WaylandClientContext ()
{
	impl = std::make_unique<Impl> ();
}

//------------------------------------------------------------------------
WaylandClientContext::~WaylandClientContext () {}
//------------------------------------------------------------------------
bool WaylandClientContext::initWayland (wl_display* display)
{
	impl->display = display;
	bool done = bindGlobals (display, impl->globals);

	impl->addRegistryListener ();
	impl->addSeatListener ();

	// Initializes seat capabilities, outputs etc.
	wl_display_roundtrip (impl->display);

	impl->addOutputsListener ();
	return done;
}

//------------------------------------------------------------------------
bool WaylandClientContext::addListener (IContextListener* listener)
{
	if (!listener)
		return false;

	impl->contextListeners.push_back (listener);
	return true;
}

//------------------------------------------------------------------------
bool WaylandClientContext::removeListener (IContextListener* listener)
{
	const auto found = [listener] (const auto el) { return el == listener; };

	auto iter = std::find_if (impl->contextListeners.begin (), impl->contextListeners.end (), found);
	if (iter == impl->contextListeners.end ())
		return false;

	impl->contextListeners.erase (iter);
	return true;
}

//------------------------------------------------------------------------
wl_compositor* WaylandClientContext::getCompositor () const
{
	return impl->globals.compositor;
}

//------------------------------------------------------------------------
wl_subcompositor* WaylandClientContext::getSubCompositor () const
{
	return impl->globals.subcompositor;
}

//------------------------------------------------------------------------
wl_shm* WaylandClientContext::getSharedMemory () const
{
	return impl->globals.shm;
}

//------------------------------------------------------------------------
wl_seat* WaylandClientContext::getSeat () const
{
	return impl->globals.seat;
}

//------------------------------------------------------------------------
xdg_wm_base* WaylandClientContext::getWindowManager () const
{
	return impl->globals.wm_base;
}

//------------------------------------------------------------------------
uint32_t WaylandClientContext::getSeatCapabilities () const
{
	return impl->seatCapabilities;
}

//------------------------------------------------------------------------
const char* WaylandClientContext::getSeatName () const
{
	return impl->seatName.data ();
}

//------------------------------------------------------------------------
int WaylandClientContext::countOutputs () const
{
	return impl->outputs.size ();
}

//------------------------------------------------------------------------
zwp_linux_dmabuf_v1* WaylandClientContext::getDmaBuffer () const
{
	return impl->globals.dmabuf;
}

//------------------------------------------------------------------------
const WaylandOutput& WaylandClientContext::getOutput (int index) const
{
	if (index < impl->outputs.size ())
		return impl->outputs[index].second;

	static WaylandOutput output {};
	return output;
}

//------------------------------------------------------------------------
} // namespace VSTGUI::Wayland
