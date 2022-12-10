// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "caviewlayer.h"

#if MAC_COCOA

#import "coregraphicsdevicecontext.h"
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
	VSTGUI::ICAViewLayerPrivate* _viewLayer;
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
- (void)setCAViewLayer:(VSTGUI::ICAViewLayerPrivate*)viewLayer
{
	_viewLayer = viewLayer;
}

//-----------------------------------------------------------------------------
- (void)drawInContext:(CGContextRef)ctx
{
	if (_viewLayer)
		_viewLayer->drawLayer (ctx);
}

@end
#else
#import "cocoa/cocoahelpers.h"
#import "cocoa/autoreleasepool.h"
#import "cocoa/objcclassbuilder.h"
//-----------------------------------------------------------------------------
@interface VSTGUI_CALayer : CALayer
//-----------------------------------------------------------------------------
- (void)setCAViewLayer:(VSTGUI::ICAViewLayerPrivate*)viewLayer;
@end

//-----------------------------------------------------------------------------
struct VSTGUI_macOS_CALayer : VSTGUI::RuntimeObjCClass<VSTGUI_macOS_CALayer>
{
	static constexpr const auto viewLayerVarName = "_viewLayer";

	//-----------------------------------------------------------------------------
	static Class CreateClass ()
	{
		return VSTGUI::ObjCClassBuilder ()
			.init ("VSTGUI_CALayer", [CALayer class])
			.addMethod (@selector (init), Init)
			.addMethod (@selector (actionForKey:), ActionForKey)
			.addMethod (@selector (setCAViewLayer:), SetCAViewLayer)
			.addMethod (@selector (drawInContext:), DrawInContext)
			.addIvar<VSTGUI::ICAViewLayerPrivate*> (viewLayerVarName)
			.finalize ();
	}

	//-----------------------------------------------------------------------------
	static id Init (id self, SEL _cmd)
	{
		self = makeInstance (self).callSuper<id (id, SEL), id> (_cmd);
		if (self)
		{
			[self setNeedsDisplayOnBoundsChange:YES];
		}
		return self;
	}

	//-----------------------------------------------------------------------------
	static id<CAAction> ActionForKey (id self, SEL _cmd, NSString* event) { return nil; }

	//-----------------------------------------------------------------------------
	static void SetCAViewLayer (id self, SEL _cmd, VSTGUI::CAViewLayer* viewLayer)
	{
		using namespace VSTGUI;
		if (auto var = makeInstance (self).getVariable<VSTGUI::CAViewLayer*> (viewLayerVarName))
			var->set (viewLayer);
	}

//-----------------------------------------------------------------------------
	static void DrawInContext (id self, SEL _cmd, CGContextRef ctx)
	{
		using namespace VSTGUI;

		if (auto var =
				makeInstance (self).getVariable<VSTGUI::ICAViewLayerPrivate*> (viewLayerVarName);
			var.has_value ())
		{
			var->get ()->drawLayer (ctx);
		}
	}
};

#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
CAViewLayer::CAViewLayer (CALayer* parent)
{
#if !TARGET_OS_IPHONE
	layer = [VSTGUI_macOS_CALayer::alloc () init];
#else
	layer = [VSTGUI_CALayer new];
#endif
	[layer setContentsScale:parent.contentsScale];
	[parent addSublayer:layer];
	[(id)layer setCAViewLayer:this];
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
bool CAViewLayer::init (IPlatformViewLayerDelegate* delegate)
{
	drawDelegate = delegate;
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
void CAViewLayer::onScaleFactorChanged (double newScaleFactor)
{
	if (layer)
		layer.contentsScale = newScaleFactor;
}

//-----------------------------------------------------------------------------
void CAViewLayer::drawLayer (void* cgContext)
{
	CGContextRef ctx = reinterpret_cast<CGContextRef> (cgContext);

#if DEBUG
	static bool visualizeLayer = false;
	if (visualizeLayer)
		CGContextClearRect (ctx, [layer bounds]);
#endif

	CGRect dirtyRect = CGContextGetClipBoundingBox (ctx);
	if ([layer contentsAreFlipped] == [layer isGeometryFlipped])
	{
		CGContextScaleCTM (ctx, 1, -1);
		CGContextTranslateCTM (ctx, 0, -[layer bounds].size.height);
		dirtyRect.origin.y =
			(-dirtyRect.origin.y - dirtyRect.size.height) + [layer bounds].size.height;
	}
	CGContextSaveGState (ctx);

	auto device = getPlatformFactory ().getGraphicsDeviceFactory ().getDeviceForScreen (
		DefaultScreenIdentifier);
	if (!device)
		return;
	auto cgDevice = std::static_pointer_cast<CoreGraphicsDevice> (device);
	if (auto deviceContext =
			std::make_shared<CoreGraphicsDeviceContext> (*cgDevice.get (), cgContext))
	{
		deviceContext->beginDraw ();
		drawDelegate->drawViewLayerRects (deviceContext, layer.contentsScale,
										  {1, CRectFromCGRect (dirtyRect)});
		deviceContext->endDraw ();
	}

	CGContextRestoreGState (ctx);

#if DEBUG
	if (visualizeLayer)
	{
		CGContextSetRGBFillColor (ctx, 1., 0., 0., 0.3);
		CGContextFillRect (ctx, [layer bounds]);
		CGContextSetRGBFillColor (ctx, 0., 1., 0., 0.3);
		CGContextFillRect (ctx, dirtyRect);
	}
#endif
}

} // VSTGUI

#endif // MAC_COCOA
