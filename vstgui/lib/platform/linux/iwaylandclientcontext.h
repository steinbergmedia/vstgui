//************************************************************************************************
//
// Wayland Server Delegate
//
// Copyright (c) 2023 CCL Software Licensing GmbH. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// - Neither the name of the wayland-server-delegate project nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Filename    : iwaylandclientcontext.h
// Description : Wayland Client Context Interface
//
//************************************************************************************************

#ifndef _iwaylandclientcontext_h
#define _iwaylandclientcontext_h

#include <wayland-client.h>

struct xdg_wm_base;
struct zwp_linux_dmabuf_v1;

namespace WaylandServerDelegate {

//************************************************************************************************
// WaylandOutput
//************************************************************************************************

struct WaylandOutput
{
	wl_output* handle = nullptr;
	int scaleFactor = 1;
	int32_t x = 0;
	int32_t y = 0;
	int32_t width = 0;
	int32_t height = 0;
	int32_t physicalWidth = 0;
	int32_t physicalHeight = 0;
	
	int32_t subPixelOrientation = 0;
	int32_t transformType = 0;
	int32_t refreshRate = 0;
	char manufacturer[128] = "";
	char model[128] = "";
	
	bool operator == (const WaylandOutput& other) const
	{
		return handle == other.handle;
	}
};

//************************************************************************************************
// IContextListener
//************************************************************************************************

struct IContextListener
{
	enum ChangeType
	{
		kSeatCapabilitiesChanged,
		kOutputsChanged
	};

	virtual ~IContextListener () {}

	virtual void contextChanged (ChangeType changeType) = 0;
};

//************************************************************************************************
// IWaylandClientContext
//************************************************************************************************

struct IWaylandClientContext
{
	virtual ~IWaylandClientContext () {}

	virtual bool addListener (IContextListener* listener) = 0;
	virtual bool removeListener (IContextListener* listener) = 0;

	virtual wl_compositor* getCompositor () const = 0;
	virtual wl_subcompositor* getSubCompositor () const = 0;
	virtual wl_shm* getSharedMemory () const = 0;
	virtual wl_seat* getSeat () const = 0;
	virtual xdg_wm_base* getWindowManager () const = 0;
	virtual zwp_linux_dmabuf_v1* getDmaBuffer () const = 0;

	virtual uint32_t getSeatCapabilities () const = 0;
	virtual const char* getSeatName () const = 0;

	virtual int countOutputs () const = 0;
	virtual const WaylandOutput& getOutput (int index) const = 0;
};

} // namespace WaylandServerDelegate

#endif // _iwaylandclientcontext_h
