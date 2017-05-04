// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "nsviewframe.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "cocoatextedit.h"
#import "nsviewoptionmenu.h"
#import "cocoaopenglview.h"
#import "autoreleasepool.h"
#import "../macclipboard.h"
#import "../cgdrawcontext.h"
#import "../cgbitmap.h"
#import "../quartzgraphicspath.h"
#import "../caviewlayer.h"
#import "../../../cvstguitimer.h"
#import "../../../cbitmap.h"

#if MAC_CARBON
	#import "../carbon/hiviewframe.h"
#endif

#import <Carbon/Carbon.h>

#define VSTGUI_NSVIEW_USE_DRAGGING_SESSION 0

using namespace VSTGUI;

//------------------------------------------------------------------------------------
HIDDEN inline IPlatformFrameCallback* getFrame (id obj)
{
	NSViewFrame* nsViewFrame = (NSViewFrame*)OBJC_GET_VALUE(obj, _nsViewFrame);
	if (nsViewFrame)
		return nsViewFrame->getFrame ();
	return nullptr;
}

//------------------------------------------------------------------------------------
HIDDEN inline NSViewFrame* getNSViewFrame (id obj)
{
	return (NSViewFrame*)OBJC_GET_VALUE(obj, _nsViewFrame);
}

static Class viewClass = nullptr;

//------------------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSView)
- (id) initWithNSViewFrame: (NSViewFrame*) frame parent: (NSView*) parent andSize: (const CRect*) size;
- (BOOL) onMouseDown: (NSEvent*) event;
- (BOOL) onMouseUp: (NSEvent*) event;
- (BOOL) onMouseMoved: (NSEvent*) event;
@end

//------------------------------------------------------------------------------------
static void mapModifiers (NSUInteger nsEventModifiers, CButtonState& buttonState)
{
	if (nsEventModifiers & NSShiftKeyMask)
		buttonState |= kShift;
	if (nsEventModifiers & NSCommandKeyMask)
		buttonState |= kControl;
	if (nsEventModifiers & NSAlternateKeyMask)
		buttonState |= kAlt;
	if (nsEventModifiers & NSControlKeyMask)
		buttonState |= kApple;
}

//------------------------------------------------------------------------------------
static bool nsViewGetCurrentMouseLocation (void* nsView, CPoint& where)
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
	NSViewFrame* frame = (NSViewFrame*)_frame;
	NSRect nsSize = nsRectFromCRect (*size);

	__OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(initWithFrame:), nsSize); // self = [super initWithFrame: nsSize];
	if (self)
	{
		OBJC_SET_VALUE(self, _nsViewFrame, frame); //		_vstguiframe = frame;

		[parentView addSubview: self];

		[self registerForDraggedTypes:[NSArray arrayWithObjects:NSStringPboardType, NSFilenamesPboardType, NSColorPboardType, [NSString stringWithCString:MacClipboard::getPasteboardBinaryType () encoding:NSASCIIStringEncoding], nil]];
		
		[self setFocusRingType:NSFocusRingTypeNone];
	}
	return self;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_isFlipped (id self, SEL _cmd) { return YES; }
static BOOL VSTGUI_NSView_acceptsFirstResponder (id self, SEL _cmd) { return YES; }
static BOOL VSTGUI_NSView_canBecomeKeyView (id self, SEL _cmd) { return YES; }
static BOOL VSTGUI_NSView_wantsDefaultClipping (id self, SEL _cmd) { return NO; }
static NSFocusRingType VSTGUI_NSView_focusRingType (id self) { return NSFocusRingTypeNone; }

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_makeSubViewFirstResponder (id self, SEL _cmd, NSResponder* newFirstResponder)
{
	NSViewFrame* nsFrame = getNSViewFrame (self);
	if (nsFrame)
	{
		nsFrame->setIgnoreNextResignFirstResponder (true);
		[[self window] makeFirstResponder:newFirstResponder];
		nsFrame->setIgnoreNextResignFirstResponder (false);
	}
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_becomeFirstResponder (id self, SEL _cmd)
{
	if ([[self window] isKeyWindow])
	{
		IPlatformFrameCallback* frame = getFrame (self);
		if (frame)
			frame->platformOnActivate (true);
	}
	return YES;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_resignFirstResponder (id self, SEL _cmd)
{
	NSView* firstResponder = (NSView*)[[self window] firstResponder];
	if (![firstResponder isKindOfClass:[NSView class]])
		firstResponder = nil;
	if (firstResponder)
	{
		NSViewFrame* nsFrame = getNSViewFrame (self);
		if (nsFrame && nsFrame->getIgnoreNextResignFirstResponder ())
		{
			while (firstResponder != self && firstResponder != nil)
				firstResponder = [firstResponder superview];
			if (firstResponder == self && [[self window] isKeyWindow])
			{
				return YES;
			}
		}
		IPlatformFrameCallback* frame = getFrame (self);
		if (frame)
			frame->platformOnActivate (false);
	}
	return YES;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_updateTrackingAreas (id self, SEL _cmd)
{
	__OBJC_SUPER(self)

	NSViewFrame* viewFrame = getNSViewFrame (self);
	if (viewFrame)
		viewFrame->initTrackingArea ();
		
	objc_msgSendSuper (SUPER, @selector(updateTrackingAreas));
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_windowDidChangeBackingProperties (id self, SEL _cmd, NSNotification* notification)
{
	double scaleFactor = 1.;
	if (auto window = [self window])
		scaleFactor = [window backingScaleFactor];
	
	IPlatformFrameCallback* frame = getFrame (self);
	if (frame)
		frame->platformScaleFactorChanged (scaleFactor);
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
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidChangeBackingProperties:) name:NSWindowDidChangeBackingPropertiesNotification object:window];
		IPlatformFrameCallback* frame = getFrame (self);
		if (frame)
			frame->platformOnActivate ([window isKeyWindow] ? true : false);
		VSTGUI_NSView_windowDidChangeBackingProperties (self, _cmd, nil);
	}
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_windowKeyStateChanged (id self, SEL _cmd, NSNotification* notification)
{
	IPlatformFrameCallback* frame = getFrame (self);
	auto active = [[notification name] isEqualToString:NSWindowDidBecomeKeyNotification] ? true : false;
	NSView* firstResponder = (NSView*)[[self window] firstResponder];
	if (![firstResponder isKindOfClass:[NSView class]])
		firstResponder = nil;
	if (firstResponder)
	{
		while (firstResponder != self && firstResponder != nil)
			firstResponder = [firstResponder superview];
		if (firstResponder == self)
		{
			if (frame)
				frame->platformOnActivate (active);
		}
	}
	frame->platformOnWindowActivate (active);
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_isOpaque (id self, SEL _cmd)
{
	return NO;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_drawRect (id self, SEL _cmd, NSRect rect)
{
	NSViewFrame* frame = getNSViewFrame (self);
	if (frame)
		frame->drawRect (&rect);
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_onMouseDown (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return NO;

	CButtonState buttons = eventButton (theEvent);
	[[self window] makeFirstResponder:self];
	NSUInteger modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	mapModifiers (modifiers, buttons);
	if ([theEvent clickCount] == 2)
		buttons |= kDoubleClick;
	CPoint p = pointFromNSPoint (nsPoint);
	CMouseEventResult result = _vstguiframe->platformOnMouseDown (p, buttons);
	return (result != kMouseEventNotHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_onMouseUp (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return NO;

	CButtonState buttons = eventButton (theEvent);
	NSUInteger modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	mapModifiers (modifiers, buttons);
	CPoint p = pointFromNSPoint (nsPoint);
	CMouseEventResult result = _vstguiframe->platformOnMouseUp (p, buttons);
	return (result != kMouseEventNotHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_onMouseMoved (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return NO;

	CButtonState buttons = eventButton (theEvent);
	NSUInteger modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	mapModifiers (modifiers, buttons);
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
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return;

	CButtonState buttons = 0;
	NSUInteger modifiers = [theEvent modifierFlags];
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [self convertPoint:nsPoint fromView:nil];
	mapModifiers (modifiers, buttons);
	auto distanceX = [theEvent scrollingDeltaX];
	auto distanceY = [theEvent scrollingDeltaY];
	if ([theEvent hasPreciseScrollingDeltas])
	{
		distanceX *= 0.1;
		distanceY *= 0.1;
	}
	if ([theEvent isDirectionInvertedFromDevice])
	{
		distanceX *= -1;
		distanceY *= -1;
		buttons |= kMouseWheelInverted;
	}
	CPoint p = pointFromNSPoint (nsPoint);
	if (distanceX != 0.)
		_vstguiframe->platformOnMouseWheel (p, kMouseWheelAxisX, static_cast<float> (distanceX), buttons);
	if (distanceY != 0.)
		_vstguiframe->platformOnMouseWheel (p, kMouseWheelAxisY, static_cast<float> (distanceY), buttons);
}

//------------------------------------------------------------------------------------
static NSPoint getGlobalMouseLocation (NSView* view)
{
	NSRect r = {};
	r.origin = [NSEvent mouseLocation];
	r = [[view window] convertRectFromScreen:r];
	return [view convertPoint:r.origin fromView:nil];
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseEntered (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return;
	CButtonState buttons = 0; //eventButton (theEvent);
	NSUInteger modifiers = [theEvent modifierFlags];
	CPoint p = pointFromNSPoint (getGlobalMouseLocation (self));

	mapModifiers (modifiers, buttons);
	_vstguiframe->platformOnMouseMoved (p, buttons);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_mouseExited (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return;
	CButtonState buttons = 0; //eventButton (theEvent);
	NSUInteger modifiers = [theEvent modifierFlags];
	mapModifiers (modifiers, buttons);
	CPoint p = pointFromNSPoint (getGlobalMouseLocation (self));
	_vstguiframe->platformOnMouseExited (p, buttons);
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_cursorUpdate (id self, SEL _cmd, NSEvent* theEvent)
{
	NSViewFrame* frame = getNSViewFrame(self);
	if (frame)
	{
		frame->cursorUpdate ();
	}
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_acceptsFirstMouse (id self, SEL _cmd, NSEvent* event)
{
	return YES; // click through
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_performKeyEquivalent (id self, SEL _cmd, NSEvent* theEvent)
{
	NSView* firstResponder = (NSView*)[[self window] firstResponder];
	if (![firstResponder isKindOfClass:[NSView class]])
		firstResponder = nil;
	if (firstResponder)
	{
		while (firstResponder != self && firstResponder != nil)
			firstResponder = [firstResponder superview];
		if (firstResponder == self)
		{
			IPlatformFrameCallback* frame = getFrame (self);
			if (frame)
			{
				VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);
				if (frame->platformOnKeyDown (keyCode))
					return YES;
			}
		}
	}
	return NO;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_keyDown (id self, SEL _cmd, NSEvent* theEvent)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
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
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
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
	NSViewFrame* frame = getNSViewFrame (self);
	if (!frame)
		return NSDragOperationNone;

    NSPasteboard *pboard = [sender draggingPasteboard];

	frame->setDragDataPackage (MacClipboard::createDragDataPackage (pboard));

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);

	getNSViewFrame (self)->setMouseCursor (kCursorNotAllowed);

	frame->getFrame ()->platformOnDragEnter (frame->getDragDataPackage (), where);
	
	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
static NSDragOperation VSTGUI_NSView_draggingUpdated (id self, SEL _cmd, id sender)
{
	NSViewFrame* frame = getNSViewFrame (self);
	if (!frame)
		return NSDragOperationNone;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	frame->getFrame ()->platformOnDragMove (frame->getDragDataPackage (), where);

	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_draggingExited (id self, SEL _cmd, id sender)
{
	NSViewFrame* frame = getNSViewFrame (self);
	if (!frame || !frame->getDragDataPackage ())
		return;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	frame->getFrame ()->platformOnDragLeave (frame->getDragDataPackage (), where);
	[[NSCursor arrowCursor] set];

	frame->setDragDataPackage (nullptr);
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSView_performDragOperation (id self, SEL _cmd, id sender)
{
	NSViewFrame* frame = getNSViewFrame (self);
	if (!frame || !frame->getDragDataPackage ())
		return NO;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	bool result = frame->getFrame ()->platformOnDrop (frame->getDragDataPackage (), where);
	frame->setMouseCursor (kCursorDefault);
	frame->setDragDataPackage (nullptr);
	return result;
}

#if VSTGUI_NSVIEW_USE_DRAGGING_SESSION
//------------------------------------------------------------------------------------
static NSDragOperation VSTGUI_NSView_draggingSessionSourceOperationMaskForDraggingContext (id self, SEL _cmd, NSDraggingSession* session, NSDraggingContext context)
{
	if (context == NSDraggingContextOutsideApplication)
	{
		if (auto dataPackage = MacClipboard::createDragDataPackage (session.draggingPasteboard))
		{
			for (auto index = 0u, count = dataPackage->getCount (); index < count; ++index)
			{
				if (dataPackage->getDataType (index) == IDataPackage::kBinary)
					return NSDragOperationPrivate;
			}
		}
		return NSDragOperationCopy;
	}
	return NSDragOperationGeneric;
}
#else
//------------------------------------------------------------------------------------
static void VSTGUI_NSView_draggedImageEndedAtOperation (id self, SEL _cmd, NSImage* image, NSPoint aPoint, NSDragOperation operation)
{
	NSViewFrame* frame = getNSViewFrame (self);
	if (frame)
	{
		if (operation == NSDragOperationNone)
		{
			frame->setLastDragOperationResult (kDragRefused);
		}
		else if (operation == NSDragOperationMove)
		{
			frame->setLastDragOperationResult (kDragMoved);
		}
		else
			frame->setLastDragOperationResult (kDragCopied);
	}
}
#endif

//------------------------------------------------------------------------------------
static id VSTGUI_NSView_makeTouchbar (id self)
{
	NSViewFrame* frame = getNSViewFrame (self);
	if (frame)
		return reinterpret_cast<id> (frame->makeTouchBar ());
	return nil;
}

namespace VSTGUI {

//------------------------------------------------------------------------------------
class CocoaTooltipWindow : public NonAtomicReferenceCounted
{
public:
	CocoaTooltipWindow () = default;
	~CocoaTooltipWindow () noexcept override;

	void set (NSViewFrame* nsViewFrame, const CRect& rect, const char* tooltip);
	void hide ();

	void onTimer ();
protected:
	SharedPointer<CVSTGUITimer> timer;
	CFrame* frame {nullptr};
	NSWindow* window {nullptr};
	NSTextField* textfield {nullptr};
};

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_NSView ()
{
	if (viewClass)
		objc_disposeClassPair (viewClass);
}

//-----------------------------------------------------------------------------
void NSViewFrame::initClass ()
{
	if (viewClass == nullptr)
	{
		AutoreleasePool ap;

		const char* nsUIntegerEncoded = @encode(NSUInteger);
		const char* nsRectEncoded = @encode(NSRect);
		char funcSig[100];

		NSMutableString* viewClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSView"] autorelease];
		viewClass = generateUniqueClass (viewClassName, [NSView class]);
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(initWithNSViewFrame:parent:andSize:), IMP (VSTGUI_NSView_Init), "@@:@:^:^:^:"))
	//	VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(dealloc), IMP (VSTGUI_NSView_Dealloc), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(updateTrackingAreas), IMP (VSTGUI_NSView_updateTrackingAreas), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(viewDidMoveToWindow), IMP (VSTGUI_NSView_viewDidMoveToWindow), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(windowKeyStateChanged:), IMP (VSTGUI_NSView_windowKeyStateChanged), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(windowDidChangeBackingProperties:), IMP (VSTGUI_NSView_windowDidChangeBackingProperties), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(isFlipped), IMP (VSTGUI_NSView_isFlipped), "B@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(acceptsFirstResponder), IMP (VSTGUI_NSView_acceptsFirstResponder), "B@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(becomeFirstResponder), IMP (VSTGUI_NSView_becomeFirstResponder), "B@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(resignFirstResponder), IMP (VSTGUI_NSView_resignFirstResponder), "B@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(canBecomeKeyView), IMP (VSTGUI_NSView_canBecomeKeyView), "B@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(wantsDefaultClipping), IMP (VSTGUI_NSView_wantsDefaultClipping), "B@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(isOpaque), IMP (VSTGUI_NSView_isOpaque), "B@:@:"))
		sprintf (funcSig, "v@:@:%s:", nsRectEncoded);
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(drawRect:), IMP (VSTGUI_NSView_drawRect), funcSig))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(onMouseDown:), IMP (VSTGUI_NSView_onMouseDown), "B@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(onMouseUp:), IMP (VSTGUI_NSView_onMouseUp), "B@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(onMouseMoved:), IMP (VSTGUI_NSView_onMouseMoved), "B@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(mouseDown:), IMP (VSTGUI_NSView_mouseDown), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(rightMouseDown:), IMP (VSTGUI_NSView_rightMouseDown), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(otherMouseDown:), IMP (VSTGUI_NSView_otherMouseDown), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(mouseUp:), IMP (VSTGUI_NSView_mouseUp), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(rightMouseUp:), IMP (VSTGUI_NSView_rightMouseUp), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(otherMouseUp:), IMP (VSTGUI_NSView_otherMouseUp), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(mouseMoved:), IMP (VSTGUI_NSView_mouseMoved), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(mouseDragged:), IMP (VSTGUI_NSView_mouseDragged), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(rightMouseDragged:), IMP (VSTGUI_NSView_rightMouseDragged), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(otherMouseDragged:), IMP (VSTGUI_NSView_otherMouseDragged), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(scrollWheel:), IMP (VSTGUI_NSView_scrollWheel), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(mouseEntered:), IMP (VSTGUI_NSView_mouseEntered), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(mouseExited:), IMP (VSTGUI_NSView_mouseExited), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(cursorUpdate:), IMP (VSTGUI_NSView_cursorUpdate), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(acceptsFirstMouse:), IMP (VSTGUI_NSView_acceptsFirstMouse), "B@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(performKeyEquivalent:), IMP (VSTGUI_NSView_performKeyEquivalent), "B@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(keyDown:), IMP (VSTGUI_NSView_keyDown), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(keyUp:), IMP (VSTGUI_NSView_keyUp), "v@:@:^:"))

		sprintf (funcSig, "%s@:@:", @encode(NSFocusRingType));
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(focusRingType), IMP (VSTGUI_NSView_focusRingType), funcSig))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(makeSubViewFirstResponder:), IMP (VSTGUI_NSView_makeSubViewFirstResponder), "v@:@:^:"))

		sprintf (funcSig, "%s@:@:^:", nsUIntegerEncoded);
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(draggingEntered:), IMP (VSTGUI_NSView_draggingEntered), funcSig))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(draggingUpdated:), IMP (VSTGUI_NSView_draggingUpdated), funcSig))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(draggingExited:), IMP (VSTGUI_NSView_draggingExited), "v@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(performDragOperation:), IMP (VSTGUI_NSView_performDragOperation), "B@:@:^:"))

#if VSTGUI_NSVIEW_USE_DRAGGING_SESSION
		sprintf (funcSig, "%s@:@:^:%s", @encode(NSDragOperation), @encode(NSDraggingContext));
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(draggingSession:sourceOperationMaskForDraggingContext:), IMP (VSTGUI_NSView_draggingSessionSourceOperationMaskForDraggingContext), funcSig))
		Protocol* nsDraggingSourceProtocol = objc_getProtocol ("NSDraggingSource");
		class_addProtocol (viewClass, nsDraggingSourceProtocol);
#else
		sprintf (funcSig, "v@:@:^:%s:%s", @encode(NSPoint), nsUIntegerEncoded);
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(draggedImage:endedAt:operation:), IMP (VSTGUI_NSView_draggedImageEndedAtOperation), funcSig))
#endif

// optional touchbar support
		if (auto protocol = objc_getProtocol ("NSTouchBarProvider"))
		{
			class_addProtocol (viewClass, protocol);
			sprintf (funcSig, "%s@:@:", @encode(NSObject*));
			class_addMethod (viewClass, @selector(makeTouchBar), IMP (VSTGUI_NSView_makeTouchbar), funcSig);
		}

		VSTGUI_CHECK_YES(class_addIvar (viewClass, "_nsViewFrame", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))
		objc_registerClassPair (viewClass);
	}
}

//-----------------------------------------------------------------------------
NSViewFrame::NSViewFrame (IPlatformFrameCallback* frame, const CRect& size, NSView* parent, IPlatformFrameConfig* config)
: IPlatformFrame (frame)
, nsView (nullptr)
, tooltipWindow (nullptr)
, ignoreNextResignFirstResponder (false)
, trackingAreaInitialized (false)
, inDraw (false)
, cursor (kCursorDefault)
{
	auto cocoaConfig = dynamic_cast<CocoaFrameConfig*> (config);
	initClass ();
	
	nsView = [[viewClass alloc] initWithNSViewFrame: this parent: parent andSize: &size];

	if (cocoaConfig && cocoaConfig->flags & CocoaFrameConfig::kNoCALayer)
		return;

	auto processInfo = [NSProcessInfo processInfo];
	if ([processInfo respondsToSelector:@selector(operatingSystemVersion)])
	{
		auto systemVersion = processInfo.operatingSystemVersion;
		// on Mac OS X 10.11 we activate layer drawing as this fixes a few issues like that only a
		// few parts of a window are updated permanently when scrolling or manipulating a control
		// while other parts are only updated when the malipulation ended, or CNinePartTiledBitmap
		// are drawn incorrectly when scaled.
		if (systemVersion.majorVersion >= 10 && systemVersion.minorVersion > 10)
		{
			[nsView setWantsLayer:YES];
		}
	}
}

//-----------------------------------------------------------------------------
NSViewFrame::~NSViewFrame () noexcept
{
	if (tooltipWindow)
		tooltipWindow->forget ();
	[nsView unregisterDraggedTypes]; // this is neccessary otherwise AppKit will crash if the plug-in is unloaded from the process
	[nsView removeFromSuperview];
	[nsView release];
}

//-----------------------------------------------------------------------------
void NSViewFrame::initTrackingArea ()
{
	if (trackingAreaInitialized == false)
	{
		NSPoint p = getGlobalMouseLocation (nsView);
		NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingActiveInActiveApp|NSTrackingInVisibleRect;
		if ([nsView hitTest:p])
		{
			options |= NSTrackingAssumeInside;
			CPoint cp = pointFromNSPoint (p);
			frame->platformOnMouseMoved (cp, 0);
		}
		NSTrackingArea* trackingArea = [[[NSTrackingArea alloc] initWithRect:[nsView frame] options:options owner:nsView userInfo:nil] autorelease];
		[nsView addTrackingArea: trackingArea];
		trackingAreaInitialized = true;
	}
}

//-----------------------------------------------------------------------------
void NSViewFrame::cursorUpdate ()
{
	setMouseCursor (cursor);
}

//-----------------------------------------------------------------------------
void NSViewFrame::drawRect (NSRect* rect)
{
	inDraw = true;
	NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];
	
	CGDrawContext drawContext ((CGContextRef)[nsContext graphicsPort], rectFromNSRect ([nsView bounds]));
	drawContext.beginDraw ();
	const NSRect* dirtyRects;
	NSInteger numDirtyRects;
	[nsView getRectsBeingDrawn:&dirtyRects count:&numDirtyRects];
	for (NSInteger i = 0; i < numDirtyRects; i++)
	{
		frame->platformDrawRect (&drawContext, rectFromNSRect (dirtyRects[i]));
	}
	drawContext.endDraw ();
	inDraw = false;
}

// IPlatformFrame
//-----------------------------------------------------------------------------
bool NSViewFrame::getGlobalPosition (CPoint& pos) const
{
	NSPoint p = [nsView bounds].origin;
	convertPointToGlobal (nsView, p);
	if ([nsView window] == nil)
	{
		p.y -= [nsView bounds].size.height;
	}
	else
	{
		NSScreen* mainScreen = [[NSScreen screens] objectAtIndex:0];
		NSRect screenRect = [mainScreen frame];
		p.y = screenRect.size.height - (p.y + screenRect.origin.y);
	}
	pos.x = p.x;
	pos.y = p.y;
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::setSize (const CRect& newSize)
{
	NSRect r = nsRectFromCRect (newSize);
	if (NSEqualRects (r, [nsView frame]))
		return true;

	NSUInteger oldResizeMask = [nsView autoresizingMask];
	[nsView setAutoresizingMask: 0];
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
bool NSViewFrame::getCurrentMouseButtons (CButtonState& buttons) const
{
	NSUInteger modifiers = [NSEvent modifierFlags];
	mapModifiers (modifiers, buttons);

	NSUInteger mouseButtons = [NSEvent pressedMouseButtons];
	if (mouseButtons & (1 << 0))
	{
		if (mouseButtons == (1 << 0) && modifiers & NSControlKeyMask)
		{
			buttons = kRButton;
			return true;
		}
		else
			buttons |= kLButton;
	}
	if (mouseButtons & (1 << 1))
		buttons |= kRButton;
	if (mouseButtons & (1 << 2))
		buttons |= kMButton;
	if (mouseButtons & (1 << 3))
		buttons |= kButton4;
	if (mouseButtons & (1 << 4))
		buttons |= kButton5;
	
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::setMouseCursor (CCursorType type)
{
	cursor = type;
	@try {
	NSCursor* cur = nullptr;
	switch (type)
	{
		case kCursorWait: cur = [NSCursor arrowCursor]; break;
		case kCursorHSize: cur = [NSCursor resizeLeftRightCursor]; break;
		case kCursorVSize: cur = [NSCursor resizeUpDownCursor]; break;
		case kCursorSizeAll: cur = [NSCursor crosshairCursor]; break;
		case kCursorNESWSize:
		{
			if ([NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)])
				cur = [NSCursor performSelector:@selector(_windowResizeNorthEastSouthWestCursor)];
			else
				cur = [NSCursor crosshairCursor];
			break;
		}
		case kCursorNWSESize:
		{
			if ([NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)])
				cur = [NSCursor performSelector:@selector(_windowResizeNorthWestSouthEastCursor)];
			else
				cur = [NSCursor crosshairCursor];
			break;
		}
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
	if (inDraw)
		return false;
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
bool NSViewFrame::showTooltip (const CRect& rect, const char* utf8Text)
{
	if (tooltipWindow == nullptr)
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
SharedPointer<IPlatformTextEdit> NSViewFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return makeOwned<CocoaTextEdit> (nsView, textEdit);
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> NSViewFrame::createPlatformOptionMenu ()
{
	return makeOwned<NSViewOptionMenu> ();
}

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
SharedPointer<IPlatformOpenGLView> NSViewFrame::createPlatformOpenGLView ()
{
	return makeOwned<CocoaOpenGLView> (nsView);
}
#endif

//-----------------------------------------------------------------------------
SharedPointer<IPlatformViewLayer> NSViewFrame::createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	auto parentViewLayer = dynamic_cast<CAViewLayer*> (parentLayer);
	if (parentViewLayer == nullptr || parentViewLayer->getLayer () == nullptr)
	{
		// after this is called, 'Quartz Debug' will not work as before. So when using 'Quartz Debug' comment the following two lines.
		[nsView setWantsLayer:YES];
		nsView.layer.actions = nil;
	}
	auto caParentLayer = parentViewLayer ? parentViewLayer->getLayer () : [nsView layer];
	auto layer = makeOwned<CAViewLayer> (caParentLayer);
	layer->init (drawDelegate);
	return std::move (layer);
}

//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> NSViewFrame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
	auto bitmap = makeOwned<CGBitmap> (CPoint (width * scaleFactor, height * scaleFactor));
	bitmap->setScaleFactor (scaleFactor);
	auto context = makeOwned<CGDrawContext> (bitmap);
	if (context->getCGContext ())
		return std::move (context);
	return nullptr;
}

//------------------------------------------------------------------------------------
DragResult NSViewFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	lastDragOperationResult = kDragError;
	if (nsView)
	{
		CGBitmap* cgBitmap = dragBitmap ? dynamic_cast<CGBitmap*> (dragBitmap->getPlatformBitmap ().get ()) : nullptr;
		CGImageRef cgImage = cgBitmap ? cgBitmap->getCGImage () : nullptr;
		NSPoint bitmapOffset = { static_cast<CGFloat>(offset.x), static_cast<CGFloat>(offset.y) };

		NSEvent* event = [NSApp currentEvent];
		if (event == nullptr || !([event type] == NSLeftMouseDown || [event type] == NSLeftMouseDragged))
			return kDragRefused;
		NSPoint nsLocation = [event locationInWindow];
		NSImage* nsImage = nil;
		if (cgImage)
		{
			nsImage = [imageFromCGImageRef (cgImage) autorelease];
			nsLocation = [nsView convertPoint:nsLocation fromView:nil];
			bitmapOffset.x += nsLocation.x;
			bitmapOffset.y += nsLocation.y + [nsImage size].height;
		}
		else
		{
			if (bitmapOffset.x == 0)
				bitmapOffset.x = 1;
			if (bitmapOffset.y == 0)
				bitmapOffset.y = 1;
			nsImage = [[[NSImage alloc] initWithSize:NSMakeSize (fabs (bitmapOffset.x)*2, fabs (bitmapOffset.y)*2)] autorelease];
			bitmapOffset.x += nsLocation.x;
			bitmapOffset.y += nsLocation.y;
		}
#if VSTGUI_NSVIEW_USE_DRAGGING_SESSION
		NSMutableArray* dragItems = [[NSMutableArray new] autorelease];
		for (uint32_t index = 0, count = source->getCount (); index < count; ++index)
		{
			const void* buffer = nullptr;
			IDataPackage::Type type {};
			auto size = source->getData (index, buffer, type);
			if (size == 0)
				continue;
			NSDraggingItem* item = nil;
			switch (type)
			{
				case IDataPackage::kFilePath:
				{
					if (auto fileUrl = [NSURL fileURLWithPath:[NSString stringWithUTF8String:reinterpret_cast<const char*>(buffer)]])
					{
						item = [[[NSDraggingItem alloc] initWithPasteboardWriter:fileUrl] autorelease];
					}
					break;
				}
				case IDataPackage::kText:
				{
					if (auto string = [NSString stringWithUTF8String:reinterpret_cast<const char*>(buffer)])
					{
						item = [[[NSDraggingItem alloc] initWithPasteboardWriter:string] autorelease];
					}
					break;
				}
				case IDataPackage::kBinary:
				{
#if 0
					// TODO: write an object implementing NSPasteboardWriting to provide NSData
					if (auto data = [NSData dataWithBytes:buffer length:size])
					{
						item = [[[NSDraggingItem alloc] initWithPasteboardWriter:data] autorelease];
					}
#endif
					break;
				}
				case IDataPackage::kError:
				{
					continue;
				}
			}
			if (item)
			{
				if (cgImage && [dragItems count] == 0)
				{
					NSRect r;
					r.origin = bitmapOffset;
					r.origin.y -= [nsImage size].height;
					r.size = nsImage.size;
					[item setDraggingFrame:r contents:nsImage];
				}
				else
					item.draggingFrame.origin = bitmapOffset;
				[dragItems addObject:item];
			}
		}
		NSView <NSDraggingSource>* draggingSource = (NSView <NSDraggingSource>*)nsView;
		if (auto session = [nsView beginDraggingSessionWithItems:dragItems event:event source:draggingSource])
		{
			session.animatesToStartingPositionsOnCancelOrFail = YES;
			return kDragAsynchronous;
		}
#else
		NSPasteboard* nsPasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
		IDataPackage::Type type = source->getDataType (0);
		switch (type)
		{
			case IDataPackage::kFilePath:
			{
				NSMutableArray* files = [[[NSMutableArray alloc] init] autorelease];
				// we allow more than one file
				for (uint32_t i = 0; i < source->getCount (); i++)
				{
					const void* buffer = nullptr;
					uint32_t bufferSize = source->getData (i, buffer, type);
					if (type == IDataPackage::kFilePath && bufferSize > 0 && ((const char*)buffer)[bufferSize-1] == 0)
					{
						[files addObject:[NSString stringWithCString:(const char*)buffer encoding:NSUTF8StringEncoding]];
					}
				}
				[nsPasteboard declareTypes:[NSArray arrayWithObject:NSFilenamesPboardType] owner:nil];
				[nsPasteboard setPropertyList:files forType:NSFilenamesPboardType];
				break;
			}
			case IDataPackage::kText:
			{
				const void* buffer = nullptr;
				uint32_t bufferSize = source->getData (0, buffer, type);
				if (bufferSize > 0)
				{
					[nsPasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
					[nsPasteboard setString:[[[NSString alloc] initWithBytes:buffer length:bufferSize encoding:NSUTF8StringEncoding] autorelease] forType:NSStringPboardType];
				}
				break;
			}
			case IDataPackage::kBinary:
			{
				const void* buffer = nullptr;
				uint32_t bufferSize = source->getData (0, buffer, type);
				if (bufferSize > 0)
				{
					[nsPasteboard declareTypes:[NSArray arrayWithObject:[NSString stringWithCString:MacClipboard::getPasteboardBinaryType () encoding:NSASCIIStringEncoding]] owner:nil];
					[nsPasteboard setData:[NSData dataWithBytes:buffer length:bufferSize] forType:[NSString stringWithCString:MacClipboard::getPasteboardBinaryType () encoding:NSASCIIStringEncoding]];
				}
				break;
			}
			case IDataPackage::kError:
			{
				return kDragError;
			}
		}
		[nsView dragImage:nsImage at:bitmapOffset offset:NSMakeSize (0, 0) event:event pasteboard:nsPasteboard source:nsView slideBack:dragBitmap ? YES : NO];
		[nsPasteboard clearContents];
		return lastDragOperationResult;
#endif
	}
	return kDragError;
}

//-----------------------------------------------------------------------------
void NSViewFrame::setClipboard (const SharedPointer<IDataPackage>& data)
{
	MacClipboard::setClipboard (data);
}

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> NSViewFrame::getClipboard ()
{
	return MacClipboard::createClipboardDataPackage ();
}

//-----------------------------------------------------------------------------
void NSViewFrame::setTouchBarCreator (const SharedPointer<ITouchBarCreator>& creator)
{
	touchBarCreator = creator;
	if (!nsView.window || !nsView.window.visible)
		return;
	if (![nsView respondsToSelector:@selector(setTouchBar:)])
		return;
	
	if (!touchBarCreator)
		[nsView performSelector:@selector(setTouchBar:) withObject:nil];
	else
	{
		if ([nsView performSelector:@selector(touchBar) withObject:nil] != nil)
			recreateTouchBar ();
	}
}

//-----------------------------------------------------------------------------
void NSViewFrame::recreateTouchBar ()
{
	if (!touchBarCreator)
		return;
	if (![nsView respondsToSelector:@selector(setTouchBar:)])
		return;
	if (id tb = reinterpret_cast<id> (touchBarCreator->createTouchBar ()))
		[nsView performSelector:@selector(setTouchBar:) withObject:tb];
}

//-----------------------------------------------------------------------------
void* NSViewFrame::makeTouchBar () const
{
	if (touchBarCreator)
		return touchBarCreator->createTouchBar ();
	return nullptr;
}

//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType platformType, IPlatformFrameConfig* config)
{
	#if MAC_CARBON
	if (platformType == kWindowRef || platformType == kDefaultNative)
		return new HIViewFrame (frame, size, reinterpret_cast<WindowRef> (parent));
	#endif
	return new NSViewFrame (frame, size, reinterpret_cast<NSView*> (parent), config);
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
CocoaTooltipWindow::~CocoaTooltipWindow () noexcept
{
	timer = nullptr;
	if (window)
	{
		[window orderOut:nil];
		[window release];
	}
}

//------------------------------------------------------------------------------------
void CocoaTooltipWindow::set (NSViewFrame* nsViewFrame, const CRect& rect, const char* tooltip)
{
	timer = nullptr;
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
	NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont controlContentFontOfSize:0], NSFontAttributeName, nil];
	NSMutableAttributedString* attrString = [[[NSMutableAttributedString alloc] init] autorelease];
	NSArray* lines = [string componentsSeparatedByString:@"\\n"];
	bool first = true;
	for (NSString* str in lines)
	{
		if (!first)
			[attrString appendAttributedString:[[[NSAttributedString alloc] initWithString:@"\n" attributes:attributes] autorelease]];
		else
			first = false;
		[attrString appendAttributedString:[[[NSAttributedString alloc] initWithString:str attributes:attributes] autorelease]];
	}
	[textfield setAttributedStringValue:attrString];
	[textfield sizeToFit];
	NSSize textSize = [textfield bounds].size;

	CPoint p;
	p.x = rect.left;
	p.y = rect.bottom;
	NSPoint nsp = nsPointFromCPoint (p);
	convertPointToGlobal (nsView, nsp);
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
	if (timer == nullptr && [window isVisible])
	{
		timer = makeOwned<CVSTGUITimer> ([this] (CVSTGUITimer*) { onTimer (); }, 17);
		onTimer ();
	}
}

//------------------------------------------------------------------------------------
void CocoaTooltipWindow::onTimer ()
{
	CGFloat newAlpha = [window alphaValue] - 0.05;
	if (newAlpha <= 0)
	{
		[window orderOut:nil];
		timer = nullptr;
	}
	else
		[window setAlphaValue:newAlpha];
}
} // namespace

#endif // MAC_COCOA
