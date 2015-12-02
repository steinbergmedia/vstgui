//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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

	uint32_t flags;
	uint32_t depthSize;
	uint32_t samples;		///< only used when kMultiSample is set

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
class IPlatformOpenGLView : public CBaseObject
{
public:

	virtual bool init (IOpenGLView* view, PixelFormat* pixelFormat = 0) = 0;
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
