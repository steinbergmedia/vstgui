// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

// TODO: Get rid of all X11 references resp. find a generic way for both platforms

#pragma once

#include "iplatformframe.h"
#include "platform_x11.h"

struct wl_surface;
struct wl_display;
struct xdg_surface;
struct xdg_toplevel;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Wayland {

using X11::IEventHandler;
using X11::ITimerHandler;
using X11::IRunLoop;

//------------------------------------------------------------------------
class IWaylandHost : public virtual IReference
{
public:
	virtual wl_display* openWaylandConnection () = 0;
	virtual bool closeWaylandConnection (wl_display* display) = 0;
};

//------------------------------------------------------------------------
class IWaylandFrame : public virtual IReference
{
public:
	virtual wl_surface* getWaylandSurface (wl_display* display) = 0;
	virtual xdg_surface* getParentSurface (CRect& parentSize, wl_display* display) = 0;
	virtual xdg_toplevel* getParentToplevel (wl_display* display) = 0;
};

//------------------------------------------------------------------------
class FrameConfig : public IPlatformFrameConfig
{
public:
	SharedPointer<IRunLoop> runLoop;
	SharedPointer<IWaylandHost> waylandHost;
	SharedPointer<IWaylandFrame> waylandFrame;
};

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
