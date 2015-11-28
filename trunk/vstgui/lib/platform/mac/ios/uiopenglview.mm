//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

} // namespace

#endif // VSTGUI_OPENGL_SUPPORT
#endif // TARGET_OS_IPHONE
