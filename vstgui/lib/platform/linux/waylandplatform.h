// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#pragma once

#include "../../vstguifwd.h"
#include "waylandframe.h"
#include <atomic>
#include <memory>
#include <cairo/cairo.h>

struct xdg_wm_base;
struct wl_compositor;
struct wl_subcompositor;
struct wl_shm;
struct wl_seat;

namespace WaylandServerDelegate {
	class IWaylandClientContext;
}

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
namespace Wayland {

class Frame;
class Timer;

//------------------------------------------------------------------------
struct RunLoop
{
	using IWaylandClientContext = WaylandServerDelegate::IWaylandClientContext;
	static void init (const SharedPointer<IWaylandHost>& waylandHost);
	static void exit ();
	static const SharedPointer<IRunLoop> get ();

	static void flush ();

	static wl_display* getDisplay ();
	static const IWaylandClientContext& getClientContext ();
	static bool hasPointerInput ();
	static bool hasKeyboardInput ();

	void setDevice (cairo_device_t* device);
	static RunLoop& instance ();

private:
	RunLoop ();
	~RunLoop () noexcept;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
