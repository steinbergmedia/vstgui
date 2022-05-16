// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../iplatformopenglview.h"

#if VSTGUI_OPENGL_SUPPORT
#if MAC_COCOA

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
struct NSOpenGLView;
struct NSView;
#endif

namespace VSTGUI {
class NSViewFrame;

//-----------------------------------------------------------------------------
class CocoaOpenGLView : public IPlatformOpenGLView
{
public:
	CocoaOpenGLView (NSView* parent);
	~CocoaOpenGLView () noexcept override = default;

	bool init (IOpenGLView* view, PixelFormat* pixelFormat = nullptr) override;
	void remove () override;

	void invalidRect (const CRect& rect) override;
	void viewSizeChanged (const CRect& visibleSize) override;

	bool makeContextCurrent () override;
	bool lockContext () override;
	bool unlockContext () override;

	void swapBuffers () override;
	
	void doDraw (const CRect& r);
	void reshape ();
protected:

	NSView* parent;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	NSOpenGLView* platformView;
#pragma clang diagnostic pop
	IOpenGLView* view;
	PixelFormat pixelFormat;
};

} // VSTGUI

#endif // MAC_COCOA
#endif // VSTGUI_OPENGL_SUPPORT
