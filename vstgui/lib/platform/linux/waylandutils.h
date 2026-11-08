// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#pragma once

#include "waylandplatform.h"
#include "wayland-client-protocol.h"
#include "iwaylandclientcontext.h"
#include <sys/mman.h>

struct wl_subsurface;
struct wl_buffer;
struct wl_shm_pool;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Wayland {

//------------------------------------------------------------------------
// ChildWindow
//------------------------------------------------------------------------
class ChildWindow : public wl_surface_listener
{
public:
	//--------------------------------------------------------------------
	struct ShmBuffer
	{
		wl_buffer* buffer {nullptr};
		wl_shm_pool* pool {nullptr};
		void* data {MAP_FAILED};
		int allocatedSize {0};
		int byteSize {0};
		int fd {-1};
		int bufferWidth {0};
		int bufferHeight {0};
	};

	ChildWindow (IWaylandFrame* waylandFrame, CPoint size);
	~ChildWindow () noexcept;

	void setFrame (IPlatformFrameCallback* frame);
	IPlatformFrameCallback* getFrame () const;
	void setSize (const CRect& rect);
	const CPoint& getSize () const;
	void* getBuffer () const;
	int getBufferStride () const;
	int getScalerFactor () const;
	const ShmBuffer& getShmBuffer () const;
	wl_surface* getSurface () const;
	void commit (const CRect& rect);

	//--------------------------------------------------------------------
private:
	// wl_surface_listener
	static void onEnter (void* data, wl_surface* wl_surface, wl_output* output);
	static void onLeave (void* data, wl_surface* wl_surface, wl_output* output);
	static void onPreferredBufferScale (void* data, wl_surface* wl_surface, int32_t factor);
	static void onPreferredBufferTransform (void* data, wl_surface* wl_surface, uint32_t transform);

	static void updateScaleFactor (ChildWindow* self, int32_t factor);

	void initialize ();
	void terminate ();
	void createShmBuffer (ShmBuffer& shmBuffer);
	void destroyShmBuffer ();

	SharedPointer<IWaylandFrame> waylandFrame;
	IPlatformFrameCallback* frameCallback {nullptr};
	wl_surface* surface {nullptr};
	wl_subsurface* subSurface {nullptr};
	CPoint size;
	ShmBuffer shmBuffer;
	bool initialized {false};
	int scaleFactor {1};
};

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
