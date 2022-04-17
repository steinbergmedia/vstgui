// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "caviewlayer.h"

#if MAC_COCOA

#import "cgdrawcontext.h"
#import "macglobals.h"
#import <QuartzCore/QuartzCore.h>

#if __clang__ 
	#if __clang_major__ >= 3 && __has_feature(objc_arc)
		#define ARC_ENABLED 1
	#endif // __has_feature(objc_arc)
#endif // __clang__

#if TARGET_OS_IPHONE
//-----------------------------------------------------------------------------
@interface VSTGUI_CALayer : CALayer
//-----------------------------------------------------------------------------
{
	VSTGUI::IPlatformViewLayerDelegate* _viewLayerDelegate;
}
@end

//-----------------------------------------------------------------------------
@implementation VSTGUI_CALayer
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
- (id)init
{
	self = [super init];
	if (self)
	{
		self.needsDisplayOnBoundsChange = YES;
	}
	return self;
}

//-----------------------------------------------------------------------------
- (id<CAAction>)actionForKey:(NSString *)event
{
	// no implicit animations
	return nil;
}

//-----------------------------------------------------------------------------
- (void)setDrawDelegate:(VSTGUI::IPlatformViewLayerDelegate*)viewLayerDelegate
{
	_viewLayerDelegate = viewLayerDelegate;
}

//-----------------------------------------------------------------------------
- (void)drawInContext:(CGContextRef)ctx
{
	if (_viewLayerDelegate)
	{
		CGRect dirtyRect = CGContextGetClipBoundingBox (ctx);
		VSTGUI::CGDrawContext drawContext (ctx, VSTGUI::CRectFromCGRect (self.bounds));
		_viewLayerDelegate->drawViewLayer (&drawContext, VSTGUI::CRectFromCGRect (dirtyRect));
	}
}

@end
#else
#import "cocoa/cocoahelpers.h"
#import "cocoa/autoreleasepool.h"
#import "cocoa/objcclassbuilder.h"
//-----------------------------------------------------------------------------
@interface VSTGUI_CALayer : CALayer
//-----------------------------------------------------------------------------
- (void)setDrawDelegate:(VSTGUI::IPlatformViewLayerDelegate*)viewLayerDelegate;
@end

//-----------------------------------------------------------------------------
struct VSTGUI_macOS_CALayer
{
	static id newInstance ()
	{
		return [[instance ().viewLayerClass alloc] init];
	}

private:
	static constexpr const auto viewLayerDelegateVarName = "_viewLayerDelegate";

	Class viewLayerClass {nullptr};

	//-----------------------------------------------------------------------------
	VSTGUI_macOS_CALayer ()
	{
		viewLayerClass = VSTGUI::ObjCClassBuilder ()
							 .init ("VSTGUI_CALayer", [CALayer class])
							 .addMethod (@selector (init), Init)
							 .addMethod (@selector (actionForKey:), ActionForKey)
							 .addMethod (@selector (setDrawDelegate:), SetDrawDelegate)
							 .addMethod (@selector (drawInContext:), DrawInContext)
							 .addIvar<void*> (viewLayerDelegateVarName)
							 .finalize ();
	}

	//-----------------------------------------------------------------------------
	~VSTGUI_macOS_CALayer () noexcept
	{
		objc_disposeClassPair (viewLayerClass);
	}

	//-----------------------------------------------------------------------------
	static VSTGUI_macOS_CALayer& instance ()
	{
		static VSTGUI_macOS_CALayer gInstance;
		return gInstance;
	}

	//-----------------------------------------------------------------------------
	static id Init (id self, SEL _cmd)
	{
		self = VSTGUI::ObjCInstance (self).callSuper<id (id, SEL), id> (_cmd);
		if (self)
		{
			[self setNeedsDisplayOnBoundsChange:YES];
		}
		return self;
	}

	//-----------------------------------------------------------------------------
	static id<CAAction> ActionForKey (id self, SEL _cmd, NSString* event) { return nil; }

	//-----------------------------------------------------------------------------
	static void SetDrawDelegate (id self, SEL _cmd, VSTGUI::IPlatformViewLayerDelegate* delegate)
	{
		using namespace VSTGUI;
		if (auto var = ObjCInstance (self).getVariable<IPlatformViewLayerDelegate*> (
				viewLayerDelegateVarName))
			var->set (delegate);
	}

//-----------------------------------------------------------------------------
	static void DrawInContext (id self, SEL _cmd, CGContextRef ctx)
	{
		using namespace VSTGUI;

		if (auto var = ObjCInstance (self).getVariable<IPlatformViewLayerDelegate*> (
				viewLayerDelegateVarName);
			var.has_value ())
		{
			static bool visualizeLayer = false;
			if (visualizeLayer)
				CGContextClearRect (ctx, [self bounds]);

			CGRect dirtyRect = CGContextGetClipBoundingBox (ctx);
			if ([self contentsAreFlipped] == [self isGeometryFlipped])
			{
				CGContextScaleCTM (ctx, 1, -1);
				CGContextTranslateCTM (ctx, 0, -[self bounds].size.height);
				dirtyRect.origin.y =
					(-dirtyRect.origin.y - dirtyRect.size.height) + [self bounds].size.height;
			}
			CGContextSaveGState (ctx);
			CGDrawContext drawContext (ctx, CRectFromCGRect ([(CALayer*)self bounds]));
			var->get ()->drawViewLayer (&drawContext, CRectFromCGRect (dirtyRect));
			CGContextRestoreGState (ctx);

			if (visualizeLayer)
			{
				CGContextSetRGBFillColor (ctx, 1., 0., 0., 0.3);
				CGContextFillRect (ctx, [self bounds]);
				CGContextSetRGBFillColor (ctx, 0., 1., 0., 0.3);
				CGContextFillRect (ctx, dirtyRect);
			}
		}
	}
};

#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
CAViewLayer::CAViewLayer (CALayer* parent)
: layer (nullptr)
{
#if !TARGET_OS_IPHONE
	layer = VSTGUI_macOS_CALayer::newInstance ();
#else
	layer = [VSTGUI_CALayer new];
#endif
	[layer setContentsScale:parent.contentsScale];
	[parent addSublayer:layer];
}

//-----------------------------------------------------------------------------
CAViewLayer::~CAViewLayer () noexcept
{
	if (layer)
	{
		[layer removeFromSuperlayer];
	#if !ARC_ENABLED
		[layer release];
	#endif
	}
}

//-----------------------------------------------------------------------------
bool CAViewLayer::init (IPlatformViewLayerDelegate* drawDelegate)
{
	[static_cast<VSTGUI_CALayer*> (layer) setDrawDelegate:drawDelegate];
	return true;
}

//-----------------------------------------------------------------------------
void CAViewLayer::invalidRect (const CRect& size)
{
	if (layer)
	{
		CGRect r = CGRectFromCRect (size);
		if (layer.contentsAreFlipped == layer.isGeometryFlipped)
		{
			r.origin.y = (-r.origin.y - r.size.height) + layer.frame.size.height;
		}
		[layer setNeedsDisplayInRect:r];
	}
}

//-----------------------------------------------------------------------------
void CAViewLayer::setSize (const CRect& size)
{
	CRect r (size);
	r.makeIntegral ();
    CGRect cgRect = CGRectFromCRect (r);
    if (layer.contentsAreFlipped == layer.isGeometryFlipped)
    {
        CGRect parentSize = layer.superlayer.frame;
		cgRect.origin.y = (-cgRect.origin.y - cgRect.size.height) + parentSize.size.height;
    }
	if (CGRectEqualToRect (layer.frame, cgRect) == false)
		layer.frame = cgRect;
}

//-----------------------------------------------------------------------------
void CAViewLayer::setZIndex (uint32_t zIndex)
{
	if (layer)
		layer.zPosition = static_cast<CGFloat>(zIndex);
}

//-----------------------------------------------------------------------------
void CAViewLayer::setAlpha (float alpha)
{
	if (layer)
		layer.opacity = alpha;
}

//-----------------------------------------------------------------------------
void CAViewLayer::draw (CDrawContext* context, const CRect& updateRect)
{
}

//-----------------------------------------------------------------------------
void CAViewLayer::onScaleFactorChanged (double newScaleFactor)
{
	if (layer)
		layer.contentsScale = newScaleFactor;
}

} // VSTGUI

#endif // MAC_COCOA
