// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uiopenglview__
#define __uiopenglview__

#include "../../iplatformopenglview.h"

#if TARGET_OS_IPHONE
#if VSTGUI_OPENGL_SUPPORT

#ifdef __OBJC__
	@class UIView, GLKView, NSRecursiveLock;
#else
	struct GLKView;
	struct UIView;
	struct NSRecursiveLock;
#endif

namespace VSTGUI {

//------------------------------------------------------------------------------------
class GLKitOpenGLView : public IPlatformOpenGLView
{
public:
	GLKitOpenGLView (UIView* parent);
	~GLKitOpenGLView ();

	virtual bool init (IOpenGLView* view, PixelFormat* pixelFormat = nullptr) override;
	virtual void remove () override;
	
	virtual void invalidRect (const CRect& rect) override;
	virtual void viewSizeChanged (const CRect& visibleSize) override;
	
	virtual bool makeContextCurrent () override;
	virtual bool lockContext () override;
	virtual bool unlockContext () override;
	
	virtual void swapBuffers () override;

	void doDraw (const CRect& r);

//------------------------------------------------------------------------------------
protected:
	UIView* parent;
	GLKView* platformView;
	IOpenGLView* view;
	NSRecursiveLock* lock;
	PixelFormat pixelFormat;
};

} // namespace

#endif // VSTGUI_OPENGL_SUPPORT
#endif // TARGET_OS_IPHONE

#endif // __uiopenglview__
