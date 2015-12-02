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
#import "../../../idatapackage.h"
#import "../../../cbitmap.h"

#if MAC_CARBON
	#import "../carbon/hiviewframe.h"
#endif

#import <Carbon/Carbon.h>

using namespace VSTGUI;

//------------------------------------------------------------------------------------
HIDDEN inline IPlatformFrameCallback* getFrame (id obj)
{
	NSViewFrame* nsViewFrame = (NSViewFrame*)OBJC_GET_VALUE(obj, _nsViewFrame);
	if (nsViewFrame)
		return nsViewFrame->getFrame ();
	return 0;
}

//------------------------------------------------------------------------------------
HIDDEN inline NSViewFrame* getNSViewFrame (id obj)
{
	return (NSViewFrame*)OBJC_GET_VALUE(obj, _nsViewFrame);
}

static Class viewClass = 0;
static IDataPackage* gCocoaDragContainer = 0;

#ifndef __MAC_10_7
//------------------------------------------------------------------------------------
@interface NSEvent (VSTGUI_NSEvent_Private)
- (BOOL)isDirectionInvertedFromDevice;
@end
#endif

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

extern NSString* const NSWindowDidChangeBackingPropertiesNotification;

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_viewDidMoveToWindow (id self, SEL _cmd)
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	NSWindow* window = [self window];
	if (window)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowKeyStateChanged:) name:NSWindowDidBecomeKeyNotification object:window];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowKeyStateChanged:) name:NSWindowDidResignKeyNotification object:window];
	#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_6
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidChangeBackingProperties:) name:NSWindowDidChangeBackingPropertiesNotification object:window];
	#endif
		IPlatformFrameCallback* frame = getFrame (self);
		if (frame)
			frame->platformOnActivate ([window isKeyWindow] ? true : false);
	}
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_windowDidChangeBackingProperties (id self, SEL _cmd, NSNotification* notification)
{
	IPlatformFrameCallback* frame = getFrame (self);
	if (frame)
		frame->platformScaleFactorChanged ();
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSView_windowKeyStateChanged (id self, SEL _cmd, NSNotification* notification)
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
				frame->platformOnActivate ([[notification name] isEqualToString:NSWindowDidBecomeKeyNotification] ? true : false);
		}
	}
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
	float distanceX = static_cast<float> ([theEvent deltaX]);
	float distanceY = static_cast<float> ([theEvent deltaY]);
	if ([theEvent respondsToSelector:@selector(isDirectionInvertedFromDevice)])
	{
		BOOL inverted = [theEvent isDirectionInvertedFromDevice];
		if (inverted)
		{
			distanceX *= -1;
			distanceY *= -1;
			buttons |= kMouseWheelInverted;
		}
	}
	CPoint p = pointFromNSPoint (nsPoint);
	if (distanceX != 0.f)
		_vstguiframe->platformOnMouseWheel (p, kMouseWheelAxisX, distanceX, buttons);
	if (distanceY != 0.f)
		_vstguiframe->platformOnMouseWheel (p, kMouseWheelAxisY, distanceY, buttons);
}

//------------------------------------------------------------------------------------
static NSPoint getGlobalMouseLocation (NSView* view)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_6
	NSRect r = {};
	r.origin = [NSEvent mouseLocation];
	r = [[view window] convertRectFromScreen:r];
	return [view convertPoint:r.origin fromView:nil];
#else
	NSPoint nsPoint = [NSEvent mouseLocation];
	nsPoint = [[view window] convertScreenToBase:nsPoint];
	return [view convertPoint:nsPoint fromView:nil];
#endif
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
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return NSDragOperationNone;

    NSPasteboard *pboard = [sender draggingPasteboard];

	gCocoaDragContainer = MacClipboard::createDragDataPackage (pboard);

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);

	getNSViewFrame (self)->setMouseCursor (kCursorNotAllowed);

	_vstguiframe->platformOnDragEnter (gCocoaDragContainer, where);
	
	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
static NSDragOperation VSTGUI_NSView_draggingUpdated (id self, SEL _cmd, id sender)
{
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
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
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
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
	IPlatformFrameCallback* _vstguiframe = getFrame (self);
	if (!_vstguiframe)
		return NO;

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	bool result = _vstguiframe->platformOnDrop (gCocoaDragContainer, where);
	getNSViewFrame (self)->setMouseCursor (kCursorDefault);
	gCocoaDragContainer->forget ();
	gCocoaDragContainer = 0;
	return result;
}

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


namespace VSTGUI {

//------------------------------------------------------------------------------------
class CocoaTooltipWindow : public CBaseObject
{
public:
	CocoaTooltipWindow ();
	~CocoaTooltipWindow ();

	void set (NSViewFrame* nsViewFrame, const CRect& rect, const char* tooltip);
	void hide ();

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
protected:
	CVSTGUITimer* timer;
	CFrame* frame;
	NSWindow* window;
	NSTextField* textfield;
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
	if (viewClass == 0)
	{
		AutoreleasePool ap;

		const char* nsPointEncoded = @encode(NSPoint);
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

		sprintf (funcSig, "v@:@:^:%s:%s", nsPointEncoded, nsUIntegerEncoded);
		VSTGUI_CHECK_YES(class_addMethod (viewClass, @selector(draggedImage:endedAt:operation:), IMP (VSTGUI_NSView_draggedImageEndedAtOperation), funcSig))

		VSTGUI_CHECK_YES(class_addIvar (viewClass, "_nsViewFrame", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))
		objc_registerClassPair (viewClass);
	}
}

//-----------------------------------------------------------------------------
NSViewFrame::NSViewFrame (IPlatformFrameCallback* frame, const CRect& size, NSView* parent)
: IPlatformFrame (frame)
, nsView (0)
, tooltipWindow (0)
, ignoreNextResignFirstResponder (false)
, trackingAreaInitialized (false)
, inDraw (false)
, cursor (kCursorDefault)
{
	initClass ();
	nsView = [[viewClass alloc] initWithNSViewFrame: this parent: parent andSize: &size];
}

//-----------------------------------------------------------------------------
NSViewFrame::~NSViewFrame ()
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
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
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
	
#else
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
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::setMouseCursor (CCursorType type)
{
	cursor = type;
	@try {
	NSCursor* cur = 0;
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

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
IPlatformOpenGLView* NSViewFrame::createPlatformOpenGLView ()
{
	return new CocoaOpenGLView (nsView);
}
#endif

//-----------------------------------------------------------------------------
IPlatformViewLayer* NSViewFrame::createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	CAViewLayer* parentViewLayer = dynamic_cast<CAViewLayer*> (parentLayer);
	if (parentViewLayer == 0 || parentViewLayer->getLayer () == 0)
	{
		// after this is called, 'Quartz Debug' will not work as before. So when using 'Quartz Debug' comment the following two lines.
		[nsView setWantsLayer:YES];
		nsView.layer.actions = nil;
	}
    CALayer* caParentLayer = parentViewLayer ? parentViewLayer->getLayer () : [nsView layer];
	CAViewLayer* layer = new CAViewLayer (caParentLayer);
	layer->init (drawDelegate);
	return layer;
}

//-----------------------------------------------------------------------------
COffscreenContext* NSViewFrame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
	CGBitmap* bitmap = new CGBitmap (CPoint (width * scaleFactor, height * scaleFactor));
	bitmap->setScaleFactor (scaleFactor);
	CGDrawContext* context = new CGDrawContext (bitmap);
	bitmap->forget ();
	if (context->getCGContext ())
		return context;
	context->forget ();
	return 0;
}

//------------------------------------------------------------------------------------
DragResult NSViewFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	lastDragOperationResult = kDragError;
	CGBitmap* cgBitmap = dragBitmap ? dynamic_cast<CGBitmap*> (dragBitmap->getPlatformBitmap ()) : 0;
	CGImageRef cgImage = cgBitmap ? cgBitmap->getCGImage () : 0;
	if (nsView)
	{
		NSPoint bitmapOffset = { static_cast<CGFloat>(offset.x), static_cast<CGFloat>(offset.y) };
		NSPasteboard* nsPasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
		NSImage* nsImage = nil;
		NSEvent* event = [NSApp currentEvent];
		if (event == 0 || !([event type] == NSLeftMouseDown || [event type] == NSLeftMouseDragged))
			return kDragRefused;
		NSPoint nsLocation = [event locationInWindow];
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

		
		IDataPackage::Type type = source->getDataType (0);
		switch (type)
		{
			case IDataPackage::kFilePath:
			{
				NSMutableArray* files = [[[NSMutableArray alloc] init] autorelease];
				// we allow more than one file
				for (uint32_t i = 0; i < source->getCount (); i++)
				{
					const void* buffer = 0;
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
				const void* buffer = 0;
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
				const void* buffer = 0;
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
	}
	return kDragError;
}

//-----------------------------------------------------------------------------
void NSViewFrame::setClipboard (IDataPackage* data)
{
	MacClipboard::setClipboard (data);
}

//-----------------------------------------------------------------------------
IDataPackage* NSViewFrame::getClipboard ()
{
	return MacClipboard::createClipboardDataPackage ();
}

//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType platformType)
{
	#if MAC_CARBON
	if ((platformType == kWindowRef || platformType == kDefaultNative)
	#if VSTGUI_ENABLE_DEPRECATED_METHODS
	 || (platformType != kNSView && CFrame::getCocoaMode () == false)
	#endif
	)
		return new HIViewFrame (frame, size, (WindowRef)parent);
	#endif
	return new NSViewFrame (frame, size, (NSView*)parent);
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
	if (timer == 0 && [window isVisible])
	{
		timer = new CVSTGUITimer (this, 17);
		timer->start ();
		notify (timer, CVSTGUITimer::kMsgTimer);
	}
}

//------------------------------------------------------------------------------------
CMessageResult CocoaTooltipWindow::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		CGFloat newAlpha = [window alphaValue] - 0.05;
		if (newAlpha <= 0)
		{
			[window orderOut:nil];
			timer->forget ();
			timer = 0;
		}
		else
			[window setAlphaValue:newAlpha];
		return kMessageNotified;
	}
	return kMessageUnknown;
}
} // namespace

#endif // MAC_COCOA
