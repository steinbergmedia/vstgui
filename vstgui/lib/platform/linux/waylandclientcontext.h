//-----------------------------------------------------------------------------
// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iwaylandclientcontext.h"
#include <memory>

namespace VSTGUI::Wayland {

//------------------------------------------------------------------------
// WaylandClientContext
//------------------------------------------------------------------------
class WaylandClientContext final : public WaylandServerDelegate::IWaylandClientContext
{
public:
//------------------------------------------------------------------------
	// WaylandServerDelegate
	using WaylandOutput = WaylandServerDelegate::WaylandOutput;
	using IContextListener = WaylandServerDelegate::IContextListener;

	WaylandClientContext ();
	~WaylandClientContext ();

	bool initWayland (wl_display* display);

	// IWaylandClientContext
	bool addListener (IContextListener* listener) override;
	bool removeListener (IContextListener* listener) override;
	wl_compositor* getCompositor () const override;
	wl_subcompositor* getSubCompositor () const override;
	wl_shm* getSharedMemory () const override;
	wl_seat* getSeat () const override;
	xdg_wm_base* getWindowManager () const override;
	uint32_t getSeatCapabilities () const override;
	const char* getSeatName () const override;
	int countOutputs () const override;
	zwp_linux_dmabuf_v1* getDmaBuffer () const override;
	const WaylandOutput& getOutput (int index) const override;

//------------------------------------------------------------------------
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // namespace VSTGUI::Wayland
