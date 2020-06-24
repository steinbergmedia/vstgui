// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#define GL_SILENCE_DEPRECATION

#import "cocoaopenglview.h"

#if MAC_COCOA
#if VSTGUI_OPENGL_SUPPORT

#import "nsviewframe.h"
#import "cocoahelpers.h"
#import "autoreleasepool.h"

#import <OpenGL/OpenGL.h>
#import <vector>

static Class openGLViewClass = nullptr;

//-----------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSOpenGLView)
- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format callback:(VSTGUI::CocoaOpenGLView*)callback;
@end

namespace VSTGUI {

#ifdef MAC_OS_X_VERSION_10_12
static constexpr auto CocoaOpenGLContextParameterSwapInterval = NSOpenGLContextParameterSwapInterval;
static constexpr auto CocoaOpenGLContextParameterSurfaceOpacity = NSOpenGLContextParameterSurfaceOpacity;
#else
static constexpr auto CocoaOpenGLContextParameterSwapInterval = NSOpenGLCPSwapInterval;
static constexpr auto CocoaOpenGLContextParameterSurfaceOpacity = NSOpenGLCPSurfaceOpacity;
#endif

//-----------------------------------------------------------------------------
CocoaOpenGLView::CocoaOpenGLView (NSView* parent)
: parent (parent)
, platformView (nullptr)
, view (nullptr)
{
	initClass ();
}

//-----------------------------------------------------------------------------
bool CocoaOpenGLView::init (IOpenGLView* inView, PixelFormat* _pixelFormat)
{
	if (platformView || openGLViewClass == nullptr)
		return false;
	if (parent)
	{
		NSRect r = NSMakeRect (0, 0, 100, 100);

		std::vector<NSOpenGLPixelFormatAttribute> formatAttributes;
		if (_pixelFormat)
		{
			pixelFormat = *_pixelFormat;
			formatAttributes.emplace_back (NSOpenGLPFADepthSize);
			formatAttributes.emplace_back (pixelFormat.depthBufferSize);
			formatAttributes.emplace_back (NSOpenGLPFAStencilSize);
			formatAttributes.emplace_back (pixelFormat.stencilBufferSize);
			formatAttributes.emplace_back (NSOpenGLPFANoRecovery);
			formatAttributes.emplace_back (NSOpenGLPFAAccelerated);
			if (pixelFormat.flags & PixelFormat::kDoubleBuffered)
			{
				formatAttributes.emplace_back (NSOpenGLPFADoubleBuffer);
				formatAttributes.emplace_back (NSOpenGLPFABackingStore);
			}
			if (pixelFormat.flags & PixelFormat::kMultiSample)
			{
				formatAttributes.emplace_back (NSOpenGLPFAMultisample);
				formatAttributes.emplace_back (true);
				formatAttributes.emplace_back (NSOpenGLPFASampleBuffers);
				formatAttributes.emplace_back (2);
				formatAttributes.emplace_back (NSOpenGLPFASamples);
				formatAttributes.emplace_back (pixelFormat.samples);
			}
			if (pixelFormat.flags & PixelFormat::kModernOpenGL)
			{
				formatAttributes.emplace_back (NSOpenGLPFAOpenGLProfile);
				formatAttributes.emplace_back (NSOpenGLProfileVersion3_2Core);
			}
			else
			{
				formatAttributes.emplace_back (NSOpenGLPFAOpenGLProfile);
				formatAttributes.emplace_back (NSOpenGLProfileVersionLegacy);
			}
		}
		else
		{
			formatAttributes.emplace_back (NSOpenGLPFANoRecovery);
			formatAttributes.emplace_back (NSOpenGLPFAAccelerated);
			formatAttributes.emplace_back (NSOpenGLPFADoubleBuffer);
			formatAttributes.emplace_back (NSOpenGLPFABackingStore);
			formatAttributes.emplace_back (NSOpenGLPFADepthSize);
			formatAttributes.emplace_back (32);
			formatAttributes.emplace_back (NSOpenGLPFAOpenGLProfile);
			formatAttributes.emplace_back (NSOpenGLProfileVersionLegacy);
		}
		formatAttributes.emplace_back (0);
		NSOpenGLPixelFormat* nsPixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:&formatAttributes.front ()] autorelease];
		platformView = [[openGLViewClass alloc] initWithFrame:r pixelFormat:nsPixelFormat callback:this];
		if (platformView)
		{
			NSOpenGLContext* context = [platformView openGLContext];
			GLint value = 1;
			[context setValues:&value forParameter:CocoaOpenGLContextParameterSwapInterval];
			value = 0;
			[context setValues:&value forParameter:CocoaOpenGLContextParameterSurfaceOpacity];

		#if DEBUG
			if (pixelFormat.flags & PixelFormat::kModernOpenGL)
				CGLEnable (static_cast<CGLContextObj> ([context CGLContextObj]), kCGLCECrashOnRemovedFunctions);
		#endif
			view = inView;
			platformView.wantsBestResolutionOpenGLSurface = YES;
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
		view = nullptr;
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

static id (*SuperInitWithFramePixelFormat) (id, SEL, NSRect, NSOpenGLPixelFormat*) = (id (*) (id, SEL, NSRect, NSOpenGLPixelFormat*))objc_msgSendSuper;
//-----------------------------------------------------------------------------
static id VSTGUI_NSOpenGLView_Init (id self, SEL _cmd, NSRect frameRect, NSOpenGLPixelFormat* format, CocoaOpenGLView* callback)
{
	__OBJC_SUPER(self)
	self = SuperInitWithFramePixelFormat (SUPER, @selector(initWithFrame:pixelFormat:), frameRect, format);
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
	SuperDealloc (SUPER, @selector(dealloc));
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

} // VSTGUI

#endif // VSTGUI_OPENGL_SUPPORT
#endif // MAC_COCOA
