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

#import "cocoaopenglview.h"

#if MAC_COCOA
#if VSTGUI_OPENGL_SUPPORT

#import "nsviewframe.h"
#import "cocoahelpers.h"
#import "autoreleasepool.h"

#import <OpenGL/OpenGL.h>

static Class openGLViewClass = 0;

//-----------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSOpenGLView)
- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format callback:(VSTGUI::CocoaOpenGLView*)callback;
@end

namespace VSTGUI {

//-----------------------------------------------------------------------------
CocoaOpenGLView::CocoaOpenGLView (NSView* parent)
: parent (parent)
, platformView (0)
, view (0)
{
	initClass ();
}

//-----------------------------------------------------------------------------
CocoaOpenGLView::~CocoaOpenGLView ()
{
}

//-----------------------------------------------------------------------------
bool CocoaOpenGLView::init (IOpenGLView* view, PixelFormat* _pixelFormat)
{
	if (platformView || openGLViewClass == 0)
		return false;
	if (parent)
	{
		NSRect r = NSMakeRect (0, 0, 100, 100);

		std::vector<NSOpenGLPixelFormatAttribute> formatAttributes;
		if (_pixelFormat)
		{
			pixelFormat = *_pixelFormat;
			formatAttributes.push_back (NSOpenGLPFADepthSize);
			formatAttributes.push_back (pixelFormat.depthSize);
			if (pixelFormat.flags & PixelFormat::kAccelerated)
			{
				formatAttributes.push_back (NSOpenGLPFANoRecovery);
				formatAttributes.push_back (NSOpenGLPFAAccelerated);
			}
			if (pixelFormat.flags & PixelFormat::kDoubleBuffered)
			{
				formatAttributes.push_back (NSOpenGLPFADoubleBuffer);
				formatAttributes.push_back (NSOpenGLPFABackingStore);
			}
			if (pixelFormat.flags & PixelFormat::kMultiSample)
			{
				formatAttributes.push_back (NSOpenGLPFAMultisample);
				formatAttributes.push_back (true);
				formatAttributes.push_back (NSOpenGLPFASampleBuffers);
				formatAttributes.push_back (2);
				formatAttributes.push_back (NSOpenGLPFASamples);
				formatAttributes.push_back (pixelFormat.samples);
			}
		#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
			if (pixelFormat.flags & PixelFormat::kModernOpenGL)
			{
				formatAttributes.push_back (NSOpenGLPFAOpenGLProfile);
				formatAttributes.push_back (NSOpenGLProfileVersion3_2Core);
			}
		#endif
		}
		else
		{
			formatAttributes.push_back (NSOpenGLPFANoRecovery);
			formatAttributes.push_back (NSOpenGLPFAAccelerated);
			formatAttributes.push_back (NSOpenGLPFADoubleBuffer);
			formatAttributes.push_back (NSOpenGLPFABackingStore);
			formatAttributes.push_back (NSOpenGLPFADepthSize);
			formatAttributes.push_back (32);
		#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
			formatAttributes.push_back (NSOpenGLPFAOpenGLProfile);
			formatAttributes.push_back (NSOpenGLProfileVersionLegacy);
		#endif
		}
		formatAttributes.push_back (0);
		NSOpenGLPixelFormat* nsPixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:&formatAttributes.front ()] autorelease];
		platformView = [[openGLViewClass alloc] initWithFrame:r pixelFormat:nsPixelFormat callback:this];
		if (platformView)
		{
			NSOpenGLContext* context = [platformView openGLContext];
			GLint swapInterval = 1;
			[context setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

		#if DEBUG && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
			if (pixelFormat.flags & PixelFormat::kModernOpenGL)
				CGLEnable (static_cast<CGLContextObj> ([context CGLContextObj]), kCGLCECrashOnRemovedFunctions);
		#endif
			this->view = view;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void CocoaOpenGLView::remove ()
{
	if (platformView)
	{
		AutoreleasePool ap;
		[platformView removeFromSuperview];
		[platformView release];
		platformView = nil;
		view = 0;
	}
}

//-----------------------------------------------------------------------------
void CocoaOpenGLView::invalidRect (const CRect& rect)
{
	if (platformView)
	{
		[platformView setNeedsDisplayInRect:nsRectFromCRect (rect)];
	}
}

//-----------------------------------------------------------------------------
bool CocoaOpenGLView::makeContextCurrent ()
{
	if (platformView)
	{
		NSOpenGLContext* context = [platformView openGLContext];
		if (context)
		{
			[context makeCurrentContext];
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CocoaOpenGLView::lockContext ()
{
	if (platformView)
	{
		NSOpenGLContext* context = [platformView openGLContext];
		if (context)
		{
			CGLLockContext ((CGLContextObj)[context CGLContextObj]);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CocoaOpenGLView::unlockContext ()
{
	if (platformView)
	{
		NSOpenGLContext* context = [platformView openGLContext];
		if (context)
		{
			CGLUnlockContext ((CGLContextObj)[context CGLContextObj]);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void CocoaOpenGLView::swapBuffers ()
{
	if (platformView && pixelFormat.flags & PixelFormat::kDoubleBuffered)
	{
		NSOpenGLContext* context = [platformView openGLContext];
		if (context)
		{
			[context flushBuffer];
			[NSOpenGLContext clearCurrentContext];
		}
	}
}

//-----------------------------------------------------------------------------
void CocoaOpenGLView::viewSizeChanged (const CRect& visibleSize)
{
	if (platformView)
	{
		lockContext ();
		NSRect r = nsRectFromCRect (visibleSize);
		[platformView setFrame:r];
		if ([platformView superview] == nil)
		{
			[parent addSubview:platformView];
		}
		unlockContext ();
	}
}

//-----------------------------------------------------------------------------
void CocoaOpenGLView::doDraw (const CRect& rect)
{
	if (view)
	{
		lockContext ();
		view->drawOpenGL (rect);
		unlockContext ();
	}
}

//-----------------------------------------------------------------------------
void CocoaOpenGLView::reshape ()
{
	lockContext ();
	NSOpenGLContext* context = [platformView openGLContext];
	if (context)
		[context update];
	unlockContext ();
	view->reshape ();
	[platformView setNeedsDisplay:YES];
}

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_NSOpenGLView ()
{
	if (openGLViewClass)
		objc_disposeClassPair (openGLViewClass);
}

//-----------------------------------------------------------------------------
static id VSTGUI_NSOpenGLView_Init (id self, SEL _cmd, NSRect frameRect, NSOpenGLPixelFormat* format, CocoaOpenGLView* callback)
{
	__OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(initWithFrame:pixelFormat:), frameRect, format);
	if (self)
	{
		OBJC_SET_VALUE(self, cocoaOpenGLView, callback);
		callback->remember ();
	}
	return self;
}

//-----------------------------------------------------------------------------
static void VSTGUI_NSOpenGLView_Dealloc (id self, SEL _cmd)
{
	CocoaOpenGLView* callback = (CocoaOpenGLView*)OBJC_GET_VALUE(self, cocoaOpenGLView);
	if (callback)
		callback->forget ();
	__OBJC_SUPER(self)
	objc_msgSendSuper (SUPER, @selector(dealloc));
}

//-----------------------------------------------------------------------------
static void VSTGUI_NSOpenGLView_Update_Reshape (id self, SEL _cmd)
{
	CocoaOpenGLView* callback = (CocoaOpenGLView*)OBJC_GET_VALUE(self, cocoaOpenGLView);
	if (callback)
		callback->reshape ();
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSOpenGLView_isFlipped (id self, SEL _cmd)
{
	return YES;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSOpenGLView_drawRect (id self, SEL _cmd, NSRect rect)
{
	CocoaOpenGLView* callback = (CocoaOpenGLView*)OBJC_GET_VALUE(self, cocoaOpenGLView);
	if (callback)
		callback->doDraw (rectFromNSRect(rect));
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSOpenGLView_mouseXXX (id self, SEL _cmd, NSEvent* theEvent)
{
	if ([self nextResponder])
		[[self nextResponder] performSelector:_cmd withObject:theEvent];
}


//-----------------------------------------------------------------------------
void CocoaOpenGLView::initClass ()
{
	if (openGLViewClass)
		return;

	AutoreleasePool ap;

	const char* nsRectEncoded = @encode(NSRect);
	char funcSig[100];

	NSMutableString* viewClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSOpenGLView"] autorelease];
	openGLViewClass = generateUniqueClass (viewClassName, [NSOpenGLView class]);
	if (openGLViewClass)
	{
		sprintf (funcSig, "@@:@:%s:@:^:", nsRectEncoded);
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(initWithFrame:pixelFormat:callback:), IMP (VSTGUI_NSOpenGLView_Init), funcSig))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(dealloc), IMP (VSTGUI_NSOpenGLView_Dealloc), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(update), IMP (VSTGUI_NSOpenGLView_Update_Reshape), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(reshape), IMP (VSTGUI_NSOpenGLView_Update_Reshape), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(isFlipped), IMP (VSTGUI_NSOpenGLView_isFlipped), "B@:@:"))
		sprintf (funcSig, "v@:@:%s:", nsRectEncoded);
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(drawRect:), IMP (VSTGUI_NSOpenGLView_drawRect), funcSig))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(mouseMoved:), IMP (VSTGUI_NSOpenGLView_mouseXXX), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(rightMouseDown:), IMP (VSTGUI_NSOpenGLView_mouseXXX), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (openGLViewClass, @selector(rightMouseUp:), IMP (VSTGUI_NSOpenGLView_mouseXXX), "v@:@:^:"))

		VSTGUI_CHECK_YES(class_addIvar (openGLViewClass, "cocoaOpenGLView", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))

		objc_registerClassPair (openGLViewClass);
	}
}

} // namespace

#endif // VSTGUI_OPENGL_SUPPORT
#endif // MAC_COCOA
