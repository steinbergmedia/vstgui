// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#define GL_SILENCE_DEPRECATION

#import "cocoaopenglview.h"

#if MAC_COCOA
#if VSTGUI_OPENGL_SUPPORT

#import "nsviewframe.h"
#import "cocoahelpers.h"
#import "objcclassbuilder.h"
#import "autoreleasepool.h"

#import <OpenGL/OpenGL.h>
#import <vector>

//-----------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSOpenGLView)
- (id)initWithFrame:(NSRect)frameRect
		pixelFormat:(NSOpenGLPixelFormat*)format
		   callback:(VSTGUI::CocoaOpenGLView*)callback;
@end

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct VSTGUI_NSOpenGLView : RuntimeObjCClass<VSTGUI_NSOpenGLView>
{
	static constexpr const auto cocoaOpenGLViewVarName = "cocoaOpenGLView";

	//-----------------------------------------------------------------------------
	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("VSTGUI_NSOpenGLView", [NSOpenGLView class])
			.addMethod (@selector (initWithFrame:pixelFormat:callback:), Init)
			.addMethod (@selector (dealloc), Dealloc)
			.addMethod (@selector (update), Update_Reshape)
			.addMethod (@selector (reshape), Update_Reshape)
			.addMethod (@selector (isFlipped), IsFlipped)
			.addMethod (@selector (drawRect:), DrawRect)
			.addMethod (@selector (mouseMoved:), MouseXXX)
			.addMethod (@selector (rightMouseDown:), MouseXXX)
			.addMethod (@selector (rightMouseUp:), MouseXXX)
			.addIvar<CocoaOpenGLView*> (cocoaOpenGLViewVarName)
			.finalize ();
	}

	//-----------------------------------------------------------------------------
	static id Init (id self, SEL _cmd, NSRect frameRect, NSOpenGLPixelFormat* format,
					CocoaOpenGLView* callback)
	{
		auto obj = makeInstance (self);
		self = obj.callSuper<id (id, SEL, NSRect, NSOpenGLPixelFormat*), id> (
			@selector (initWithFrame:pixelFormat:), frameRect, format);
		if (self)
		{
			if (auto var = obj.getVariable<CocoaOpenGLView*> (cocoaOpenGLViewVarName))
			{
				var->set (callback);
				callback->remember ();
			}
		}
		return self;
	}

	//-----------------------------------------------------------------------------
	static void Dealloc (id self, SEL _cmd)
	{
		auto obj = makeInstance (self);
		if (auto var = obj.getVariable<CocoaOpenGLView*> (cocoaOpenGLViewVarName))
		{
			if (auto callback = var->get ())
				callback->forget ();
		}
		obj.callSuper<void (id, SEL)> (_cmd);
	}

	//-----------------------------------------------------------------------------
	static void Update_Reshape (id self, SEL _cmd)
	{
		auto obj = makeInstance (self);
		if (auto var = obj.getVariable<CocoaOpenGLView*> (cocoaOpenGLViewVarName))
		{
			if (auto callback = var->get ())
				callback->reshape ();
		}
	}

	//------------------------------------------------------------------------------------
	static BOOL IsFlipped (id self, SEL _cmd) { return YES; }

	//------------------------------------------------------------------------------------
	static void DrawRect (id self, SEL _cmd, NSRect rect)
	{
		auto obj = makeInstance (self);
		if (auto var = obj.getVariable<CocoaOpenGLView*> (cocoaOpenGLViewVarName))
		{
			if (auto callback = var->get ())
				callback->doDraw (rectFromNSRect (rect));
		}
	}

	//------------------------------------------------------------------------------------
	static void MouseXXX (id self, SEL _cmd, NSEvent* theEvent)
	{
		if ([self nextResponder])
			[[self nextResponder] performSelector:_cmd withObject:theEvent];
	}
};

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
}

//-----------------------------------------------------------------------------
bool CocoaOpenGLView::init (IOpenGLView* inView, PixelFormat* _pixelFormat)
{
	if (platformView)
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
		NSOpenGLPixelFormat* nsPixelFormat = [[[NSOpenGLPixelFormat alloc]
			initWithAttributes:&formatAttributes.front ()] autorelease];
		platformView = [VSTGUI_NSOpenGLView::alloc () initWithFrame:r
														pixelFormat:nsPixelFormat
														   callback:this];
		if (platformView)
		{
			NSOpenGLContext* context = [platformView openGLContext];
			GLint value = 1;
			[context setValues:&value forParameter:CocoaOpenGLContextParameterSwapInterval];
			value = 0;
			[context setValues:&value forParameter:CocoaOpenGLContextParameterSurfaceOpacity];

#if DEBUG
			if (pixelFormat.flags & PixelFormat::kModernOpenGL)
			{
				CGLEnable (static_cast<CGLContextObj> ([context CGLContextObj]),
						   kCGLCECrashOnRemovedFunctions);
			}
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

} // VSTGUI

#endif // VSTGUI_OPENGL_SUPPORT
#endif // MAC_COCOA
