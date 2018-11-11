// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "uiopenglview.h"

#if TARGET_OS_IPHONE
#if VSTGUI_OPENGL_SUPPORT

#import <GLKit/GLKit.h>

#if __has_feature(objc_arc) && __clang_major__ >= 3
#define ARC_ENABLED 1
#endif // __has_feature(objc_arc)

@interface VSTGUI_GLKView : GLKView
{
	VSTGUI::GLKitOpenGLView* view;
}
@end

//-----------------------------------------------------------------------------
@implementation VSTGUI_GLKView
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
- (id)initWithFrame:(CGRect)frame context:(EAGLContext *)context GLKitOpenGLView:(VSTGUI::GLKitOpenGLView*)_view
{
	self = [super initWithFrame:frame context:context];
	if (self)
	{
		view = _view;
	}
	return self;
}

//-----------------------------------------------------------------------------
- (void)drawRect:(CGRect)rect
{
	VSTGUI::CRect r (rect.origin.x, rect.origin.y, 0, 0);
	r.setWidth (rect.size.width);
	r.setHeight (rect.size.height);
	view->doDraw (r);
}

@end

namespace VSTGUI {

//-----------------------------------------------------------------------------
GLKitOpenGLView::GLKitOpenGLView (UIView* parent)
: parent (parent)
, platformView (nil)
, view (nullptr)
, lock (nil)
{
	
}

//-----------------------------------------------------------------------------
GLKitOpenGLView::~GLKitOpenGLView ()
{
	
}

//-----------------------------------------------------------------------------
bool GLKitOpenGLView::init (IOpenGLView* _view, PixelFormat* _pixelFormat)
{
	if (platformView || parent == nil)
		return false;

	EAGLContext* glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	if (glContext == nil)
		return false;

#if !ARC_ENABLED
	[glContext autorelease];
#endif
	lock = [NSRecursiveLock new];
	
	CGRect r = CGRectMake (0, 0, 100, 100);
	platformView = [[VSTGUI_GLKView alloc] initWithFrame:r context:glContext GLKitOpenGLView:this];
	if (platformView == nil)
		return false;
	if (_pixelFormat)
	{
		pixelFormat = *_pixelFormat;
	}
	if (pixelFormat.kMultiSample)
		platformView.drawableMultisample = GLKViewDrawableMultisample4X;
	if (pixelFormat.depthSize == 16)
		platformView.drawableDepthFormat = GLKViewDrawableDepthFormat16;
	else if (pixelFormat.depthSize == 24)
		platformView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
	
	return true;
}

//-----------------------------------------------------------------------------
void GLKitOpenGLView::remove ()
{
	if (platformView)
	{
		[platformView removeFromSuperview];
	#if !ARC_ENABLED
		[platformView release];
		[lock release];
	#endif
		platformView = nil;
		view = nullptr;
		lock = nil;
	}
}

//-----------------------------------------------------------------------------
void GLKitOpenGLView::invalidRect (const CRect& rect)
{
	if (platformView)
	{
		CGRect r = CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ());
		[platformView setNeedsDisplayInRect:r];
	}
}

//-----------------------------------------------------------------------------
void GLKitOpenGLView::viewSizeChanged (const CRect& visibleSize)
{
	if (platformView)
	{
		[lock lock];
		CGRect r = CGRectMake (visibleSize.left, visibleSize.top, visibleSize.getWidth (), visibleSize.getHeight ());
		platformView.frame = r;
		if ([platformView superview] == nil)
		{
			[parent addSubview:platformView];
		}
		[lock unlock];
	}
}

//-----------------------------------------------------------------------------
bool GLKitOpenGLView::makeContextCurrent ()
{
	if (platformView)
	{
		return [EAGLContext setCurrentContext:platformView.context] ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool GLKitOpenGLView::lockContext ()
{
	[lock lock];
	return true;
}

//-----------------------------------------------------------------------------
bool GLKitOpenGLView::unlockContext ()
{
	[lock unlock];
	return true;
}

//-----------------------------------------------------------------------------
void GLKitOpenGLView::swapBuffers ()
{
}

//-----------------------------------------------------------------------------
void GLKitOpenGLView::doDraw (const CRect& r)
{
	if (view)
	{
		lockContext ();
		view->drawOpenGL (r);
		unlockContext ();
	}
	
}

} // VSTGUI

#endif // VSTGUI_OPENGL_SUPPORT
#endif // TARGET_OS_IPHONE
