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
//-----------------------------------------------------------------------------
@interface VSTGUI_CALayer : CALayer
//-----------------------------------------------------------------------------
- (void)setDrawDelegate:(VSTGUI::IPlatformViewLayerDelegate*)viewLayerDelegate;
@end

static Class viewLayerClass = nullptr;

//-----------------------------------------------------------------------------
static id VSTGUI_CALayer_Init (id self, SEL _cmd)
{
	__OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(init));
	if (self)
	{
		[self setNeedsDisplayOnBoundsChange:YES];
	}
	return self;
}

//-----------------------------------------------------------------------------
static id<CAAction> VSTGUI_CALayer_ActionForKey (id self, SEL _cmd, NSString* event)
{
	return nil;
}

//-----------------------------------------------------------------------------
static void VSTGUI_CALayer_SetDrawDelegate (id self, SEL _cmd, VSTGUI::IPlatformViewLayerDelegate* delegate)
{
	OBJC_SET_VALUE (self, _viewLayerDelegate, delegate);
	
}

#define VISUALIZE_LAYER 0

//-----------------------------------------------------------------------------
static void VSTGUI_CALayer_DrawInContext (id self, SEL _cmd, CGContextRef ctx)
{
	VSTGUI::IPlatformViewLayerDelegate* _viewLayerDelegate = (VSTGUI::IPlatformViewLayerDelegate*)OBJC_GET_VALUE(self, _viewLayerDelegate);
	if (_viewLayerDelegate)
	{
	#if VISUALIZE_LAYER
		CGContextClearRect (ctx, [self bounds]);
	#endif
		CGRect dirtyRect = CGContextGetClipBoundingBox (ctx);
		if ([self contentsAreFlipped] == [self isGeometryFlipped])
		{
			CGContextScaleCTM (ctx, 1, -1);
			CGContextTranslateCTM (ctx, 0, -[self bounds].size.height);
			dirtyRect.origin.y = (-dirtyRect.origin.y - dirtyRect.size.height) + [self bounds].size.height;
		}
		CGContextSaveGState (ctx);
		VSTGUI::CGDrawContext drawContext (ctx, VSTGUI::CRectFromCGRect ([(CALayer*)self bounds]));
		_viewLayerDelegate->drawViewLayer (&drawContext, VSTGUI::CRectFromCGRect (dirtyRect));
		CGContextRestoreGState (ctx);

	#if VISUALIZE_LAYER
		CGContextSetRGBFillColor (ctx, 1., 0., 0., 0.3);
		CGContextFillRect (ctx, [self bounds]);
		CGContextSetRGBFillColor (ctx, 0., 1., 0., 0.3);
		CGContextFillRect (ctx, dirtyRect);
	#endif
	}
}

#endif

namespace VSTGUI {

#if !TARGET_OS_IPHONE
//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_CALayer ()
{
	if (viewLayerClass)
		objc_disposeClassPair (viewLayerClass);
}

//-----------------------------------------------------------------------------
static void initCALayerClass ()
{
	if (viewLayerClass)
		return;
	
	AutoreleasePool ap;
	NSMutableString* caLayerClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_CALayer"] autorelease];
	viewLayerClass = generateUniqueClass (caLayerClassName, [CALayer class]);
	VSTGUI_CHECK_YES(class_addMethod (viewLayerClass, @selector(init), IMP (VSTGUI_CALayer_Init), "@@:@:"))
	VSTGUI_CHECK_YES(class_addMethod (viewLayerClass, @selector(actionForKey:), IMP (VSTGUI_CALayer_ActionForKey), "@@:@:@@"))
	VSTGUI_CHECK_YES(class_addMethod (viewLayerClass, @selector(setDrawDelegate:), IMP (VSTGUI_CALayer_SetDrawDelegate), "v@:@:^:"))
	char funcSig[100];
	sprintf (funcSig, "v@:@:%s:", @encode (CGContextRef));
	VSTGUI_CHECK_YES(class_addMethod (viewLayerClass, @selector(drawInContext:), IMP (VSTGUI_CALayer_DrawInContext), "v@:@:^:"))
	VSTGUI_CHECK_YES(class_addIvar (viewLayerClass, "_viewLayerDelegate", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))
	objc_registerClassPair (viewLayerClass);
}
#endif

//-----------------------------------------------------------------------------
CAViewLayer::CAViewLayer (CALayer* parent)
: layer (nullptr)
{
#if !TARGET_OS_IPHONE
	initCALayerClass ();
	layer = [[viewLayerClass alloc] init];
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
	[(VSTGUI_CALayer*)layer setDrawDelegate:drawDelegate];
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

} // namespace

#endif // MAC_COCOA
