// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

/// @cond ignore

#if VSTGUI_OPENGL_SUPPORT

namespace VSTGUI {
class IPlatformFrame;

//-----------------------------------------------------------------------------
struct PixelFormat
{
	// TODO: add more if we need more...

	enum {
		kDoubleBuffered = 1 << 0,
		kMultiSample	= 1 << 2,
		kModernOpenGL	= 1 << 3		// Mac only. Indicates to use the NSOpenGLProfileVersion3_2Core.
	};

	uint32_t depthBufferSize {32};
	uint32_t stencilBufferSize {0};
	/** only used when kMultiSample is set */
	uint32_t samples {0};
	uint32_t flags {kDoubleBuffered};

	PixelFormat () = default;
	PixelFormat (const PixelFormat&) = default;
	PixelFormat& operator= (const PixelFormat&) = default;
};

//-----------------------------------------------------------------------------
class IOpenGLView
{
public:
	virtual void drawOpenGL (const CRect& updateRect) = 0;
	virtual void reshape () = 0;
};

//-----------------------------------------------------------------------------
class IPlatformOpenGLView : public AtomicReferenceCounted
{
public:

	virtual bool init (IOpenGLView* view, PixelFormat* pixelFormat = nullptr) = 0;
	virtual void remove () = 0;

	virtual void invalidRect (const CRect& rect) = 0;
	/** visibleSize is cframe relative */
	virtual void viewSizeChanged (const CRect& visibleSize) = 0;
	/** make OpenGL context active */
	virtual bool makeContextCurrent () = 0;
	/** lock changes to context */
	virtual bool lockContext () = 0;
	/** unlock changes to context */
	virtual bool unlockContext () = 0;
	/** swap buffers and clear active OpenGL context */
	virtual void swapBuffers () = 0;
};

} // VSTGUI

#endif // VSTGUI_OPENGL_SUPPORT

/// @endcond
