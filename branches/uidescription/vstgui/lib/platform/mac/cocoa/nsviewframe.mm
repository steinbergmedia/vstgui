
#import "nsviewframe.h"

#if MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION

#import "cocoahelpers.h"
#import "cocoadragcontainer.h"
#import "cocoatextedit.h"
#import "nsviewoptionmenu.h"
#import "autoreleasepool.h"
#import "../cgdrawcontext.h"
#import "../cgbitmap.h"
#import "../../../cvstguitimer.h"

#if MAC_CARBON
	#import "../carbon/hiviewframe.h"
#endif

USING_NAMESPACE_VSTGUI

static Class viewClass = 0;
static CocoaDragContainer* gCocoaDragContainer = 0;

//------------------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSView)
- (id) initWithFrame: (IPlatformFrameCallback*) frame parent: (NSView*) parent andSize: (const CRect*) size;
- (BOOL) onMouseDown: (NSEvent*) event;
- (BOOL) onMouseUp: (NSEvent*) event;
- (BOOL) onMouseMoved: (NSEvent*) event;
@end

//------------------------------------------------------------------------------------
HIDDEN bool nsViewGetCurrentMouseLocation (void* nsView, CPoint& where)
{
	NSView* view = (NSView*)nsView;
	NSPoint p = [[view window] mouseLocationOutsideOfEventStream];
	p = [view convertPoint:p fromView:nil];
	where = pointFromNSPoint (p);
	return true;
}

//------------------------------------------------------------------------------------
static id VSTGUI_NSView_Init (id self, SEL _cmd, void* _frame, NSView* parentView, const void* _size)
{
	const CRect* size = (const CRect*)_size;
	IPlatformFrameCallback* frame = (IPlatformFrameCallback*)_frame;
	NSRect nsSize = nsRectFromCRect (*size);

	__OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(initWithFrame:), nsSize); // self = [super initWithFrame: nsSize];
	if (self)
	{
		OBJC_SET_VALUE(self, _vstguiframe, frame); //		_vstguiframe = frame;

		[parentView addSubview: self];

		[self registerForDraggedTypes:[NSArray arrayWithObjects:NSStringPboardType, NSFilenamesPboardType, nil]];
		
		NSTrackingArea* trackingArea = [[[NSTrackingArea alloc] initWithRect:[self frame] options:NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingActiveInActiveApp|NSTrackingInVisibleRect owner:self userInfo:nil] autorelease];
		[self addTrackingArea: trackingArea];
		
		[self setFocusRingType:NSFocusRingTypeNone];
	}
	return self;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_isFlipped (id self, SEL _cmd) { return YES; }
static BOOL VSTGUI_NSView_acceptsFirstResponder (id self, SEL _cmd) { return YES; }
static BOOL VSTGUI_NSView_canBecomeKeyView (id self, SEL _cmd) { return YES; }

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_becomeFirstResponder (id self, SEL _cmd)
{
#if 0
	IPlatformFrameCallback* frame = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (frame && [[self window] isKeyWindow])
		frame->platformOnActivate (true);
#endif
	return YES;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_resignFirstResponder (id self, SEL _cmd)
{
#if 0
	IPlatformFrameCallback* frame = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (frame)
		frame->platformOnActivate (false);
#endif
	return YES;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_viewDidMoveToWindow (id self, SEL _cmd)
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	NSWindow* window = [self window];
	if (window)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowKeyStateChanged:) name:NSWindowDidBecomeKeyNotification object:window];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowKeyStateChanged:) name:NSWindowDidResignKeyNotification object:window];
	}
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_windowKeyStateChanged (id self, SEL _cmd, NSNotification* notification)
{
	IPlatformFrameCallback* frame = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	NSView* firstResponder = (NSView*)[[self window] firstResponder];
	if (![firstResponder isKindOfClass:[NSView class]])
		firstResponder = nil;
	if (frame && firstResponder && [firstResponder isDescendantOf:self])
	{
		frame->platformOnActivate ([[self window] isKeyWindow] ? true : false);
	}
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_isOpaque (id self, SEL _cmd)
{
	#if 0 // TODO: check this
	IPlatformFrameCallback* frame = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (frame)
	{
		BOOL transparent = (frame->getTransparency () 
				   || (frame->getBackground () && !frame->getBackground ()->getNoAlpha ())
				   || (!frame->getBackground () && frame->getBackgroundColor().alpha != 255)
					  );
		return !transparent;
	}
	return YES;
	#else
	return NO;
	#endif
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_drawRect (id self, SEL _cmd, NSRect rect)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (_vstguiframe)
	{
		NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];
		
		CGDrawContext drawContext ((CGContextRef)[nsContext graphicsPort], rectFromNSRect ([self bounds]));
		const NSRect* dirtyRects;
		NSInteger numDirtyRects;
		[self getRectsBeingDrawn:&dirtyRects count:&numDirtyRects];
		for (NSInteger i = 0; i < numDirtyRects; i++)
		{
			_vstguiframe->platformDrawRect (&drawContext, rectFromNSRect (dirtyRects[i]));
		}
	}
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_onMouseDown (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return NO;

	long buttons = eventButton (theEvent);
	[[self window] makeFirstResponder:self];
	unsigned int modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	if (modifiers & NSShiftKeyMask)
		buttons |= kShift;
	if (modifiers & NSCommandKeyMask)
		buttons |= kControl;
	if (modifiers & NSAlternateKeyMask)
		buttons |= kAlt;
	if (modifiers & NSControlKeyMask)
		buttons |= kApple;
	if ([theEvent clickCount] > 1)
		buttons |= kDoubleClick;
	CPoint p = pointFromNSPoint (nsPoint);
	CMouseEventResult result = _vstguiframe->platformOnMouseDown (p, buttons);
	return (result != kMouseEventNotHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_onMouseUp (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return NO;

	long buttons = eventButton (theEvent);
	unsigned int modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	if (modifiers & NSShiftKeyMask)
		buttons |= kShift;
	if (modifiers & NSCommandKeyMask)
		buttons |= kControl;
	if (modifiers & NSAlternateKeyMask)
		buttons |= kAlt;
	if (modifiers & NSControlKeyMask)
		buttons |= kApple;
	CPoint p = pointFromNSPoint (nsPoint);
	CMouseEventResult result = _vstguiframe->platformOnMouseUp (p, buttons);
	return (result != kMouseEventNotHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_onMouseMoved (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return NO;

	long buttons = eventButton (theEvent);
	unsigned int modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	if (modifiers & NSShiftKeyMask)
		buttons |= kShift;
	if (modifiers & NSCommandKeyMask)
		buttons |= kControl;
	if (modifiers & NSAlternateKeyMask)
		buttons |= kAlt;
	if (modifiers & NSControlKeyMask)
		buttons |= kApple;
	CPoint p = pointFromNSPoint (nsPoint);
	CMouseEventResult result = _vstguiframe->platformOnMouseMoved (p, buttons);
	return (result != kMouseEventNotHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseDown (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseDown: theEvent])
		objc_msgSendSuper (SUPER, @selector(mouseDown:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_rightMouseDown (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseDown: theEvent])
		objc_msgSendSuper (SUPER, @selector(rightMouseDown:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_otherMouseDown (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseDown: theEvent])
		objc_msgSendSuper (SUPER, @selector(otherMouseDown:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseUp (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseUp: theEvent])
		objc_msgSendSuper (SUPER, @selector(mouseUp:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_rightMouseUp (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseUp: theEvent])
		objc_msgSendSuper (SUPER, @selector(rightMouseUp:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_otherMouseUp (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseUp: theEvent])
		objc_msgSendSuper (SUPER, @selector(otherMouseUp:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseMoved (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseMoved: theEvent])
		objc_msgSendSuper (SUPER, @selector(mouseMoved:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseDragged (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseMoved: theEvent])
		objc_msgSendSuper (SUPER, @selector(mouseDragged:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_rightMouseDragged (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseMoved: theEvent])
		objc_msgSendSuper (SUPER, @selector(rightMouseDragged:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_otherMouseDragged (id self, SEL _cmd, NSEvent* theEvent)
{
	__OBJC_SUPER(self)
	if (![self onMouseMoved: theEvent])
		objc_msgSendSuper (SUPER, @selector(otherMouseDragged:), theEvent);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_scrollWheel (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return;

	long buttons = 0;
	unsigned int modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	if (modifiers & NSShiftKeyMask)
		buttons |= kShift;
	if (modifiers & NSCommandKeyMask)
		buttons |= kControl;
	if (modifiers & NSAlternateKeyMask)
		buttons |= kAlt;
	if (modifiers & NSControlKeyMask)
		buttons |= kApple;
	CPoint p = pointFromNSPoint (nsPoint);
	if ([theEvent deltaX])
		_vstguiframe->platformOnMouseWheel (p, kMouseWheelAxisX, [theEvent deltaX], buttons);
	if ([theEvent deltaY])
		_vstguiframe->platformOnMouseWheel (p, kMouseWheelAxisY, [theEvent deltaY], buttons);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseEntered (id self, SEL _cmd, NSEvent* theEvent)
{
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseExited (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return;
	long buttons = eventButton (theEvent);
	unsigned int modifiers = [theEvent modifierFlags];
	NSPoint nsPoint;
	nsPoint = [NSEvent mouseLocation];
	nsPoint = [[self window] convertScreenToBase:nsPoint];

	nsPoint = [self convertPoint:nsPoint fromView:nil];
	if (modifiers & NSShiftKeyMask)
		buttons |= kShift;
	if (modifiers & NSCommandKeyMask)
		buttons |= kControl;
	if (modifiers & NSAlternateKeyMask)
		buttons |= kAlt;
	if (modifiers & NSControlKeyMask)
		buttons |= kApple;
	CPoint p = pointFromNSPoint (nsPoint);
	_vstguiframe->platformOnMouseExited (p, buttons);
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_acceptsFirstMouse (id self, SEL _cmd, NSEvent* event)
{
	return YES; // click through
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_performKeyEquivalent (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (_vstguiframe)
	{
		VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);
		if (_vstguiframe->platformOnKeyDown (keyCode))
			return YES;
	}
	return NO;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_keyDown (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return;

	VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);
	
	bool res = _vstguiframe->platformOnKeyDown (keyCode);
	if (!res&& keyCode.virt == VKEY_TAB)
	{
		if (keyCode.modifier & kShift)
			[[self window] selectKeyViewPrecedingView:self];
		else
			[[self window] selectKeyViewFollowingView:self];
	}
	else if (!res)
		[[self nextResponder] keyDown:theEvent];
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_keyUp (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return;

	VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);

	bool res = _vstguiframe->platformOnKeyUp (keyCode);
	if (!res)
		[[self nextResponder] keyUp:theEvent];
}

//------------------------------------------------------------------------------------
static NSDragOperation VSTGUI_NSView_draggingEntered (id self, SEL _cmd, id sender)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return NSDragOperationNone;

    NSPasteboard *pboard = [sender draggingPasteboard];

	gCocoaDragContainer = new CocoaDragContainer (pboard);

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);

	[[NSCursor operationNotAllowedCursor] set];
	_vstguiframe->platformOnDragEnter (gCocoaDragContainer, where);

	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
static NSDragOperation VSTGUI_NSView_draggingUpdated (id self, SEL _cmd, id sender)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return NSDragOperationNone;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	_vstguiframe->platformOnDragMove (gCocoaDragContainer, where);

	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_draggingExited (id self, SEL _cmd, id sender)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe || !gCocoaDragContainer)
		return;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	_vstguiframe->platformOnDragLeave (gCocoaDragContainer, where);
	[[NSCursor arrowCursor] set];

	gCocoaDragContainer->forget ();
	gCocoaDragContainer = 0;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_performDragOperation (id self, SEL _cmd, id sender)
{
	IPlatformFrameCallback* _vstguiframe = (IPlatformFrameCallback*)OBJC_GET_VALUE(self, _vstguiframe);
	if (!_vstguiframe)
		return NO;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	bool result = _vstguiframe->platformOnDrop (gCocoaDragContainer, where);
	[[NSCursor arrowCursor] set];
	gCocoaDragContainer->forget ();
	gCocoaDragContainer = 0;
	return result;
}

BEGIN_NAMESPACE_VSTGUI

//------------------------------------------------------------------------------------
class CocoaTooltipWindow : public CBaseObject
{
public:
	CocoaTooltipWindow ();
	~CocoaTooltipWindow ();

	void set (NSViewFrame* nsViewFrame, const CRect& rect, const char* tooltip);
	void hide ();

	CMessageResult notify (CBaseObject* sender, const char* message);
protected:
	CVSTGUITimer* timer;
	CFrame* frame;
	NSWindow* window;
	NSTextField* textfield;
};

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) void cleanup_VSTGUI_NSView ()
{
	if (viewClass)
		objc_disposeClassPair (viewClass);
}

//-----------------------------------------------------------------------------
void NSViewFrame::initClass ()
{
	if (viewClass == 0)
	{
		BOOL res;
		AutoreleasePool ap ();
		NSMutableString* viewClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSView"] autorelease];
		viewClass = generateUniqueClass (viewClassName, [NSView class]);
		res = class_addMethod (viewClass, @selector(initWithFrame:parent:andSize:), IMP (VSTGUI_NSView_Init), "@@:@:^:^:^:");
	//	res = class_addMethod (viewClass, @selector(dealloc), IMP (VSTGUI_NSView_Dealloc), "v@:@:");
		res = class_addMethod (viewClass, @selector(viewDidMoveToWindow), IMP (VSTGUI_NSView_viewDidMoveToWindow), "v@:@:");
		res = class_addMethod (viewClass, @selector(windowKeyStateChanged:), IMP (VSTGUI_NSView_windowKeyStateChanged), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(isFlipped), IMP (VSTGUI_NSView_isFlipped), "B@:@:");
		res = class_addMethod (viewClass, @selector(acceptsFirstResponder), IMP (VSTGUI_NSView_acceptsFirstResponder), "B@:@:");
		res = class_addMethod (viewClass, @selector(becomeFirstResponder), IMP (VSTGUI_NSView_becomeFirstResponder), "B@:@:");
		res = class_addMethod (viewClass, @selector(resignFirstResponder), IMP (VSTGUI_NSView_resignFirstResponder), "B@:@:");
		res = class_addMethod (viewClass, @selector(canBecomeKeyView), IMP (VSTGUI_NSView_canBecomeKeyView), "B@:@:");
		res = class_addMethod (viewClass, @selector(isOpaque), IMP (VSTGUI_NSView_isOpaque), "B@:@:");
		const char* nsRectEncoded = @encode(NSRect);
		char funcSig[100];
		sprintf (funcSig, "v@:@:%s:", nsRectEncoded);
		res = class_addMethod (viewClass, @selector(drawRect:), IMP (VSTGUI_NSView_drawRect), funcSig);
		res = class_addMethod (viewClass, @selector(onMouseDown:), IMP (VSTGUI_NSView_onMouseDown), "B@:@:^:");
		res = class_addMethod (viewClass, @selector(onMouseUp:), IMP (VSTGUI_NSView_onMouseUp), "B@:@:^:");
		res = class_addMethod (viewClass, @selector(onMouseMoved:), IMP (VSTGUI_NSView_onMouseMoved), "B@:@:^:");
		res = class_addMethod (viewClass, @selector(mouseDown:), IMP (VSTGUI_NSView_mouseDown), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(rightMouseDown:), IMP (VSTGUI_NSView_rightMouseDown), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(otherMouseDown:), IMP (VSTGUI_NSView_otherMouseDown), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(mouseUp:), IMP (VSTGUI_NSView_mouseUp), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(rightMouseUp:), IMP (VSTGUI_NSView_rightMouseUp), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(otherMouseUp:), IMP (VSTGUI_NSView_otherMouseUp), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(mouseMoved:), IMP (VSTGUI_NSView_mouseMoved), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(mouseDragged:), IMP (VSTGUI_NSView_mouseDragged), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(rightMouseDragged:), IMP (VSTGUI_NSView_rightMouseDragged), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(otherMouseDragged:), IMP (VSTGUI_NSView_otherMouseDragged), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(scrollWheel:), IMP (VSTGUI_NSView_scrollWheel), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(mouseEntered:), IMP (VSTGUI_NSView_mouseEntered), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(mouseExited:), IMP (VSTGUI_NSView_mouseExited), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(acceptsFirstMouse:), IMP (VSTGUI_NSView_acceptsFirstMouse), "B@:@:^:");
		res = class_addMethod (viewClass, @selector(performKeyEquivalent:), IMP (VSTGUI_NSView_performKeyEquivalent), "B@:@:^:");
		res = class_addMethod (viewClass, @selector(keyDown:), IMP (VSTGUI_NSView_keyDown), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(keyUp:), IMP (VSTGUI_NSView_keyUp), "v@:@:^:");
		#if __LP64__
		res = class_addMethod (viewClass, @selector(draggingEntered:), IMP (VSTGUI_NSView_draggingEntered), "L@:@:^:");
		res = class_addMethod (viewClass, @selector(draggingUpdated:), IMP (VSTGUI_NSView_draggingUpdated), "L@:@:^:");
		#else
		res = class_addMethod (viewClass, @selector(draggingEntered:), IMP (VSTGUI_NSView_draggingEntered), "I@:@:^:");
		res = class_addMethod (viewClass, @selector(draggingUpdated:), IMP (VSTGUI_NSView_draggingUpdated), "I@:@:^:");
		#endif
		res = class_addMethod (viewClass, @selector(draggingExited:), IMP (VSTGUI_NSView_draggingExited), "v@:@:^:");
		res = class_addMethod (viewClass, @selector(performDragOperation:), IMP (VSTGUI_NSView_performDragOperation), "B@:@:^:");

		res = class_addIvar (viewClass, "_vstguiframe", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*));
		objc_registerClassPair (viewClass);
	}
}

//-----------------------------------------------------------------------------
NSViewFrame::NSViewFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent)
: nsView (0)
, tooltipWindow (0)
{
	initClass ();
	nsView = [[viewClass alloc] initWithFrame: frame parent: (NSView*)parent andSize: &size];
}

//-----------------------------------------------------------------------------
NSViewFrame::~NSViewFrame ()
{
	if (tooltipWindow)
		tooltipWindow->forget ();
	[nsView removeFromSuperview];
	[nsView release];
}

// IPlatformFrame
//-----------------------------------------------------------------------------
bool NSViewFrame::getGlobalPosition (CPoint& pos) const
{
	return false;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::setSize (const CRect& newSize)
{
	unsigned int oldResizeMask = [nsView autoresizingMask];
	NSRect oldFrame = [nsView frame];
	[nsView setAutoresizingMask: 0];
	NSRect r = nsRectFromCRect (newSize);
	[nsView setFrame: r];
	[nsView setAutoresizingMask: oldResizeMask];
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::getSize (CRect& size) const
{
	size = rectFromNSRect ([nsView frame]);
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::getCurrentMousePosition (CPoint& mousePosition) const
{
	NSPoint p = [[nsView window] mouseLocationOutsideOfEventStream];
	p = [nsView convertPoint:p fromView:nil];
	mousePosition = pointFromNSPoint (p);
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::getCurrentMouseButtons (long& buttons) const
{
	UInt32 state = GetCurrentButtonState ();
	if (state == kEventMouseButtonPrimary)
		buttons |= kLButton;
	if (state == kEventMouseButtonSecondary)
		buttons |= kRButton;
	if (state == kEventMouseButtonTertiary)
		buttons |= kMButton;
	if (state == 4)
		buttons |= kButton4;
	if (state == 5)
		buttons |= kButton5;

	state = GetCurrentKeyModifiers ();
	if (state & cmdKey)
		buttons |= kControl;
	if (state & shiftKey)
		buttons |= kShift;
	if (state & optionKey)
		buttons |= kAlt;
	if (state & controlKey)
		buttons |= kApple;
	// for the one buttons
	if (buttons & kApple && buttons & kLButton)
	{
		buttons &= ~(kApple | kLButton);
		buttons |= kRButton;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::setMouseCursor (CCursorType type)
{
	@try {
	NSCursor* cur = 0;
	switch (type)
	{
		case kCursorWait: cur = [NSCursor arrowCursor]; break;
		case kCursorHSize: cur = [NSCursor resizeLeftRightCursor]; break;
		case kCursorVSize: cur = [NSCursor resizeUpDownCursor]; break;
		case kCursorSizeAll: cur = [NSCursor crosshairCursor]; break;
		case kCursorNESWSize: cur = [NSCursor crosshairCursor]; break;
		case kCursorNWSESize: cur = [NSCursor crosshairCursor]; break;
		case kCursorCopy:
		{
			if ([NSCursor respondsToSelector:@selector(dragCopyCursor)])
				cur = [NSCursor performSelector:@selector(dragCopyCursor)];
			else
				cur = [NSCursor performSelector:@selector(_copyDragCursor)];
			break;
		}
		case kCursorNotAllowed: cur = [NSCursor performSelector:@selector(operationNotAllowedCursor)]; break;
		case kCursorHand: cur = [NSCursor openHandCursor]; break;
		default: cur = [NSCursor arrowCursor]; break;
	}
	if (cur)
	{
		[cur set];
		return true;
	}
	} @catch(...) { [[NSCursor arrowCursor] set]; }
	return false;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::invalidRect (const CRect& rect)
{
	NSRect r = nsRectFromCRect (rect);
	[nsView setNeedsDisplayInRect:r];
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::scrollRect (const CRect& src, const CPoint& distance)
{
	NSRect r = nsRectFromCRect (src);
	NSSize d = NSMakeSize (distance.x, distance.y);
	[nsView scrollRect:r by:d];
	NSRect r2;
	if (d.width > 0)
	{
		r2 = NSMakeRect (r.origin.x, r.origin.y, d.width, r.size.height);
		[nsView setNeedsDisplayInRect:r2];
	}
	else if (d.width < 0)
	{
		r2 = NSMakeRect (r.origin.x + r.size.width + d.width, r.origin.y, -d.width, r.size.height);
		[nsView setNeedsDisplayInRect:r2];
	}
	if (d.height > 0)
	{
		r2 = NSMakeRect (r.origin.x, r.origin.y, r.size.width, d.height);
		[nsView setNeedsDisplayInRect:r2];
	}
	else if (d.height < 0)
	{
		r2 = NSMakeRect (r.origin.x, r.origin.y + r.size.height + d.height, r.size.width, -d.height);
		[nsView setNeedsDisplayInRect:r2];
	}
	return true;
}

//-----------------------------------------------------------------------------
unsigned long NSViewFrame::getTicks () const
{
	return (TickCount () * 1000) / 60;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::showTooltip (const CRect& rect, const char* utf8Text)
{
	if (tooltipWindow == 0)
		tooltipWindow = new CocoaTooltipWindow;
	tooltipWindow->set (this, rect, utf8Text);
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::hideTooltip ()
{
	if (tooltipWindow)
	{
		tooltipWindow->hide ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
IPlatformTextEdit* NSViewFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return new CocoaTextEdit (nsView, textEdit);
}

//-----------------------------------------------------------------------------
IPlatformOptionMenu* NSViewFrame::createPlatformOptionMenu ()
{
	return new NSViewOptionMenu ();
}

//-----------------------------------------------------------------------------
COffscreenContext* NSViewFrame::createOffscreenContext (CCoord width, CCoord height)
{
	CGOffscreenBitmap* bitmap = new CGOffscreenBitmap (CPoint (width, height));
	CGDrawContext* context = new CGDrawContext (bitmap);
	bitmap->forget ();
	return context;
}

//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent)
{
	#if MAC_CARBON
	if (CFrame::getCocoaMode () == false)
		return new HIViewFrame (frame, size, parent);
	#endif
	return new NSViewFrame (frame, size, parent);
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
CocoaTooltipWindow::CocoaTooltipWindow ()
: timer (0)
, frame (0)
, window (0)
, textfield (0)
{
}

//------------------------------------------------------------------------------------
CocoaTooltipWindow::~CocoaTooltipWindow ()
{
	if (timer)
		timer->forget ();
	if (window)
	{
		[window orderOut:nil];
		[window release];
	}
}

//------------------------------------------------------------------------------------
void CocoaTooltipWindow::set (NSViewFrame* nsViewFrame, const CRect& rect, const char* tooltip)
{
	if (timer)
	{
		timer->forget ();
		timer = 0;
	}
	NSView* nsView = nsViewFrame->getPlatformControl ();
	if (!window)
	{
		window = [[NSWindow alloc] initWithContentRect:NSMakeRect (0, 0, 10, 10) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
		[window setReleasedWhenClosed:NO];
		[window setOpaque:NO];
		[window setHasShadow:YES];
		[window setLevel:NSStatusWindowLevel];
		[window setHidesOnDeactivate:YES];
		[window setIgnoresMouseEvents:YES];
		[window setBackgroundColor: [NSColor colorWithDeviceRed:1.0 green:0.96 blue:0.76 alpha:1.0]];
		textfield = [[[NSTextField alloc] initWithFrame:[[window contentView] frame]] autorelease];
		[textfield setEditable:NO];
		[textfield setSelectable:NO];
		[textfield setBezeled:NO];
		[textfield setBordered:NO];
		[textfield setDrawsBackground:NO];
		[window setContentView:textfield];
	}
	NSString* string = [NSString stringWithCString:tooltip encoding:NSUTF8StringEncoding];
	[textfield setStringValue: string];
	[textfield sizeToFit];
	NSSize textSize = [textfield bounds].size;

	CPoint p;
	p.x = rect.left;
	p.y = rect.bottom;
	NSPoint nsp = nsPointFromCPoint (p);
	nsp = [nsView convertPoint:nsp toView:nil];
	nsp = [[nsView window] convertBaseToScreen:nsp];
	nsp.y -= (textSize.height + 4);
	nsp.x += (rect.getWidth () - textSize.width) / 2;
	
	NSRect frameRect = { nsp, [textfield bounds].size };
	[window setFrame:frameRect display:NO];
	[window setAlphaValue:0.95];
	[window orderFront:nil];
}

//------------------------------------------------------------------------------------
void CocoaTooltipWindow::hide ()
{
	if (timer == 0)
	{
		timer = new CVSTGUITimer (this, 10);
		timer->start ();
		notify (timer, CVSTGUITimer::kMsgTimer);
	}
}

//------------------------------------------------------------------------------------
CMessageResult CocoaTooltipWindow::notify (CBaseObject* sender, const char* message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		CGFloat newAlpha = [window alphaValue] - 0.05;
		if (newAlpha <= 0)
		{
			[window orderOut:nil];
		}
		else
			[window setAlphaValue:newAlpha];
		return kMessageNotified;
	}
	return kMessageUnknown;
}
END_NAMESPACE_VSTGUI

#endif // MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION
