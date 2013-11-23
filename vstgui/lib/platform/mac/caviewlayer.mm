//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#import "caviewlayer.h"

#if MAC_COCOA

#import "cgdrawcontext.h"
#import "macglobals.h"
#import <QuartzCore/QuartzCore.h>

#if __has_feature(objc_arc) && __clang_major__ >= 3
#define ARC_ENABLED 1
#endif // __has_feature(objc_arc)

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

static Class viewLayerClass = 0;

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

//-----------------------------------------------------------------------------
static void VSTGUI_CALayer_DrawInContext (id self, SEL _cmd, CGContextRef ctx)
{
	VSTGUI::IPlatformViewLayerDelegate* _viewLayerDelegate = (VSTGUI::IPlatformViewLayerDelegate*)OBJC_GET_VALUE(self, _viewLayerDelegate);
	if (_viewLayerDelegate)
	{
		CGRect dirtyRect = CGContextGetClipBoundingBox (ctx);
		VSTGUI::CGDrawContext drawContext (ctx, VSTGUI::CRectFromCGRect (NSRectToCGRect ([self bounds])));
		_viewLayerDelegate->drawViewLayer (&drawContext, VSTGUI::CRectFromCGRect (dirtyRect));
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
: layer (0)
{
#if !TARGET_OS_IPHONE
	initCALayerClass ();
	layer = [[viewLayerClass alloc] init];
#else
	layer = [VSTGUI_CALayer new];
#endif
	[parent addSublayer:layer];
}

//-----------------------------------------------------------------------------
CAViewLayer::~CAViewLayer ()
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
		[layer setNeedsDisplayInRect:CGRectFromCRect (size)];
	}
}

//-----------------------------------------------------------------------------
void CAViewLayer::setSize (const CRect& size)
{
	layer.frame = CGRectFromCRect (size);
}

} // namespace

#endif // MAC_COCOA
