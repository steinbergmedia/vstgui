// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformopenglview__
#define __iplatformopenglview__

#include "../vstguifwd.h"

/// @cond ignore

#if VSTGUI_OPENGL_SUPPORT

namespace VSTGUI {
class IPlatformFrame;

//-----------------------------------------------------------------------------
struct PixelFormat
{
	// TODO: do we need more ?

	enum {
		kDoubleBuffered = 1 << 0,
		kAccelerated	= 1 << 1,
		kMultiSample	= 1 << 2,
		kModernOpenGL	= 1 << 3		// Mac only currently. Indicates to use the NSOpenGLProfileVersion3_2Core. Not tested !
	};

	uint32_t depthSize;
	uint32_t samples;		///< only used when kMultiSample is set
	uint32_t flags;

	PixelFormat () : depthSize (32), samples (0), flags (kAccelerated) {}
	PixelFormat (const PixelFormat& pf) : depthSize (pf.depthSize), samples (pf.samples), flags (pf.flags) {}
	
	PixelFormat& operator() (const PixelFormat& pf) { depthSize = pf.depthSize; samples = pf.samples; flags = pf.flags; return *this; }
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
	virtual void viewSizeChanged (const CRect& visibleSize) = 0; ///< visibleSize is cframe relative

	virtual bool makeContextCurrent () = 0;	///< make OpenGL context active
	virtual bool lockContext () = 0;		///< lock changes to context
	virtual bool unlockContext () = 0;		///< unlock changes to context

	virtual void swapBuffers () = 0;		///< swap buffers and clear active OpenGL context
};

} // namespace

#endif // VSTGUI_OPENGL_SUPPORT

/// @endcond

#endif // __iplatformopenglview__
