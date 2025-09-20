//-----------------------------------------------------------------------------
// Flags       : clang-format auto
// Project     : VST SDK
//
// Category    : EditorHost
// Filename    : public.sdk/samples/vst-hosting/editorhost/source/platform/linux/clientcontext.h
// Created by  : Steinberg 05.2025
// Description : Implemenation of WaylandServerDelegate::IWaylandClientContext
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2023, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
