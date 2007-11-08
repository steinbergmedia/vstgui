//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 3.6       $Date: 2007-11-08 14:13:28 $
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

/// \cond ignore

#import "cocoasupport.h"

#if MAC_COCOA

#import <Cocoa/Cocoa.h>
#import "cvstguitimer.h"
#import "vstkeycode.h"

//------------------------------------------------------------------------------------
@interface VSTGUI_NSView : NSView {
	CFrame* _vstguiframe;
	NSTimer* _idleTimer;
	BOOL _windowAcceptsMouseMoveEvents;
	NSTrackingRectTag _trackingRectTag;
}

- (id) initWithFrame: (CFrame*) frame andSize: (const CRect&) size;
- (void) clearCFrame;
- (void) setTooltip: (const char*)tooltip forCView: (CView*)view;
- (void) removeTooltip;
@end

//------------------------------------------------------------------------------------
@interface VSTGUI_NSTimer : NSObject {
	NSTimer* _timer;
	CBaseObject* _timerObject;
}
- (id) initWithTimerObject: (CBaseObject*) obj fireTime: (int) ms;
- (void) stop;
@end

//------------------------------------------------------------------------------------
@interface VSTGUI_NSMenu : NSMenu {
	COptionMenu* _optionMenu;
	COptionMenu* _selectedMenu;
	long _selectedItem;
}
- (id) initWithOptionMenu: (COptionMenu*) menu;
- (COptionMenu*) optionMenu;
- (COptionMenu*) selectedMenu;
- (long) selectedItem;
@end

//------------------------------------------------------------------------------------
@interface VSTGUI_NSTextField : NSTextField {
	CTextEdit* _textEdit;
}
- (id) initWithTextEdit: (CTextEdit*) textEdit;
- (void) syncSize;
@end

//------------------------------------------------------------------------------------
class CocoaDragContainer : public CDragContainer
{
public:
	CocoaDragContainer (NSPasteboard* platformDrag);
	~CocoaDragContainer ();
	
	void* first (long& size, long& type);
	void* next (long& size, long& type);
	long getType (long idx) const;
	long getCount () const { return nbItems; }

protected:
	NSPasteboard* pb;

	long nbItems;
	
	long iterator;
	void* lastItem;
};
static CocoaDragContainer* gDragContainer = 0;

//------------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------------
NSRect nsRectFromCRect (const CRect& rect)
{
	NSRect r = { {rect.left, rect.top}, {rect.getWidth (), rect.getHeight ()} };
	return r;
}

//------------------------------------------------------------------------------------
NSPoint nsPointFromCPoint (const CPoint& point)
{
	NSPoint p = { point.x, point.y };
	return p;
}

//------------------------------------------------------------------------------------
CRect rectFromNSRect (const NSRect& rect)
{
	CRect r (rect.origin.x, rect.origin.y, 0, 0);
	r.setWidth (rect.size.width);
	r.setHeight (rect.size.height);
	return r;
}

//------------------------------------------------------------------------------------
CPoint pointFromNSPoint (const NSPoint& point)
{
	CPoint p (point.x, point.y);
	return p;
}

//------------------------------------------------------------------------------------
NSColor* nsColorFromCColor (const CColor& color)
{
	return [NSColor colorWithDeviceRed:color.red/255. green:color.green/255. blue:color.blue/255. alpha:color.alpha/255.];
}

//------------------------------------------------------------------------------------
static NSImage* imageFromCGImageRef (CGImageRef image)
{
    NSRect imageRect = NSMakeRect(0.0, 0.0, 0.0, 0.0);
    CGContextRef imageContext = nil;
    NSImage* newImage = nil;
 
    // Get the image dimensions.
    imageRect.size.height = CGImageGetHeight(image);
    imageRect.size.width = CGImageGetWidth(image);
 
    // Create a new image to receive the Quartz image data.
    newImage = [[NSImage alloc] initWithSize:imageRect.size];
    [newImage lockFocus];
 
    // Get the Quartz context and draw.
    imageContext = (CGContextRef)[[NSGraphicsContext currentContext]
                                         graphicsPort];
    CGContextDrawImage(imageContext, *(CGRect*)&imageRect, image);
    [newImage unlockFocus];
 
    return newImage;
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void* createNSView (CFrame* frame, const CRect& size)
{
	VSTGUI_NSView* view = [[VSTGUI_NSView alloc] initWithFrame: frame andSize: size];
	return view;
}

//------------------------------------------------------------------------------------
void destroyNSView (void* ptr)
{
	VSTGUI_NSView* view = (VSTGUI_NSView*)ptr;
	[view clearCFrame];
	[view removeFromSuperview];
	[view autorelease];
}

//------------------------------------------------------------------------------------
void invalidNSViewRect (void* nsView, const CRect& size)
{
	VSTGUI_NSView* view = (VSTGUI_NSView*)nsView;
	NSRect r = nsRectFromCRect (size);
	[view setNeedsDisplayInRect:r];
}

//------------------------------------------------------------------------------------
void resizeNSView (void* nsView, const CRect& newSize)
{
	VSTGUI_NSView* view = (VSTGUI_NSView*)nsView;
	unsigned int oldResizeMask = [view autoresizingMask];
	NSRect oldFrame = [view frame];
	[view setAutoresizingMask: 0];
	NSRect r = nsRectFromCRect (newSize);
	r.origin.x += oldFrame.origin.x;
	r.origin.y += oldFrame.origin.y;
	[view setFrameSize: r.size];
	[view setAutoresizingMask: oldResizeMask];
}

//------------------------------------------------------------------------------------
void getSizeOfNSView (void* nsView, CRect* rect)
{
	VSTGUI_NSView* view = (VSTGUI_NSView*)nsView;
	*rect = rectFromNSRect ([view frame]);
}

//------------------------------------------------------------------------------------
bool nsViewGetCurrentMouseLocation (void* nsView, CPoint& where)
{
	VSTGUI_NSView* view = (VSTGUI_NSView*)nsView;
	NSPoint p = [[view window] mouseLocationOutsideOfEventStream];
	p = [view convertPoint:p fromView:nil];
	where = pointFromNSPoint (p);
	return true;
}

//------------------------------------------------------------------------------------
void nsViewSetMouseCursor (CCursorType type)
{
	NSCursor* cur = 0;
	switch (type)
	{
		case kCursorWait: cur = [NSCursor arrowCursor]; break;
		case kCursorHSize: cur = [NSCursor resizeLeftRightCursor]; break;
		case kCursorVSize: cur = [NSCursor resizeUpDownCursor]; break;
		case kCursorSizeAll: cur = [NSCursor crosshairCursor]; break;
		case kCursorNESWSize: cur = [NSCursor arrowCursor]; break;
		case kCursorNWSESize: cur = [NSCursor arrowCursor]; break;
		case kCursorCopy: cur = [NSCursor _copyDragCursor]; break;
		case kCursorNotAllowed: cur = [NSCursor operationNotAllowedCursor]; break;
		case kCursorHand: cur = [NSCursor openHandCursor]; break;
		default: cur = [NSCursor arrowCursor]; break;
	}
	if (cur)
		[cur set];
}

//------------------------------------------------------------------------------------
void* addNSTextField (CFrame* frame, CTextEdit* edit)
{
	return [[VSTGUI_NSTextField alloc] initWithTextEdit:edit];
}

//------------------------------------------------------------------------------------
void moveNSTextField (void* control, CTextEdit* edit)
{
	VSTGUI_NSTextField* textField = (VSTGUI_NSTextField*)control;
	[textField syncSize];
}

//------------------------------------------------------------------------------------
void removeNSTextField (void* control)
{
	VSTGUI_NSTextField* textField = (VSTGUI_NSTextField*)control;
	[textField removeFromSuperview];
	[textField autorelease];
}

//------------------------------------------------------------------------------------
bool getNSTextFieldText (void* control, char* text, long maxSize)
{
	NSTextField* textField = (NSTextField*)control;
	[[textField stringValue] getCString:text maxLength:maxSize encoding:NSUTF8StringEncoding];
	return true;
}

//------------------------------------------------------------------------------------
long showNSContextMenu (COptionMenu* menu, COptionMenu** usedMenu)
{
	bool multipleCheck = menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
	VSTGUI_NSView* view = (VSTGUI_NSView*)menu->getFrame ()->getNSView ();
	VSTGUI_NSMenu* nsMenu = [[VSTGUI_NSMenu alloc] initWithOptionMenu: menu];
	CPoint p (menu->getViewSize ().left, menu->getViewSize ().top);
	menu->localToFrame (p);
	NSRect cellFrameRect = {0};
	cellFrameRect.origin = nsPointFromCPoint (p);
	cellFrameRect.size.width = menu->getViewSize ().getWidth ();
	cellFrameRect.size.height = menu->getViewSize ().getHeight ();
	if (!(menu->getStyle () & kPopupStyle))
		[nsMenu insertItemWithTitle:@"" action:nil keyEquivalent:@"" atIndex:0];
	if (!multipleCheck && menu->getStyle () & kCheckStyle)
		[[nsMenu itemWithTag:menu->getValue ()] setState:NSOnState];

	NSPopUpButtonCell* cell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:menu->getStyle () & kPopupStyle ? NO : YES];
	[cell setAltersStateOfSelectedItem: NO];
	[cell setAutoenablesItems:NO];
	[cell setMenu:nsMenu];
	if (menu->getStyle () & kPopupStyle)
		[cell selectItemWithTag:menu->getValue ()];
	[cell performClickWithFrame:cellFrameRect inView:view];
	*usedMenu = [nsMenu selectedMenu];
	return [nsMenu selectedItem];
}

//------------------------------------------------------------------------------------
void* startNSTimer (int ms, CBaseObject* timerObject)
{
	return [[VSTGUI_NSTimer alloc] initWithTimerObject:timerObject fireTime:ms];
}

//------------------------------------------------------------------------------------
void stopNSTimer (void* platformTimer)
{
	VSTGUI_NSTimer* timer = (VSTGUI_NSTimer*)platformTimer;
	[timer stop];
	[timer release];
}

//------------------------------------------------------------------------------------
void nsViewSetTooltip (CView* view, const char* tooltip)
{
	VSTGUI_NSView* nsView = (VSTGUI_NSView*)view->getFrame ()->getNSView ();
	[nsView setTooltip:tooltip forCView:view];
}

//------------------------------------------------------------------------------------
void nsViewRemoveTooltip (CView* view)
{
	VSTGUI_NSView* nsView = (VSTGUI_NSView*)view->getFrame ()->getNSView ();
	[nsView removeTooltip];
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
@implementation VSTGUI_NSView

//------------------------------------------------------------------------------------
- (id) initWithFrame: (CFrame*) frame andSize: (const CRect&) size
{
	NSRect nsSize = { {size.left, size.top}, {size.getWidth (), size.getHeight ()} };
	[super initWithFrame: nsSize];
	
	_vstguiframe = frame;

	NSView* parentView = (NSView*)frame->getSystemWindow ();
	[parentView addSubview: self];

	[self setAutoresizingMask:NSViewMinYMargin];
	[self setAutoresizesSubviews:YES];

	[self registerForDraggedTypes:[NSArray arrayWithObjects:
            NSStringPboardType, NSFilenamesPboardType, nil]];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(frameDidChanged:) name:NSViewFrameDidChangeNotification object:self];

	return self;
}

//------------------------------------------------------------------------------------
- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewFrameDidChangeNotification object:self];

	if (_vstguiframe)
		_vstguiframe->forget ();
	[super dealloc];
}

//------------------------------------------------------------------------------------
- (void) clearCFrame
{
	_vstguiframe = 0;
}

//------------------------------------------------------------------------------------
- (BOOL)isFlipped { return YES; }
- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)becomeFirstResponder { return YES; }
- (BOOL)canBecomeKeyView { return YES; }

//------------------------------------------------------------------------------------
- (BOOL)isOpaque 
{
	BOOL transparent = (_vstguiframe->getTransparency () 
			   || (_vstguiframe->getBackground () && !_vstguiframe->getBackground ()->getNoAlpha ())
			   || (!_vstguiframe->getBackground () && _vstguiframe->getBackgroundColor().alpha != 255)
			      );
	return !transparent;
}

//------------------------------------------------------------------------------------
- (void) frameDidChanged: (NSNotification*) notification
{
	if (_vstguiframe)
	{
		CRect r = rectFromNSRect ([self frame]);
		r.offset (-r.left, -r.top);
		_vstguiframe->setViewSize (r);
	}
}

//------------------------------------------------------------------------------------
- (void)viewDidMoveToWindow
{
	if ([self window])
	{
		_windowAcceptsMouseMoveEvents = [[self window] acceptsMouseMovedEvents];
		_trackingRectTag = [self addTrackingRect:[self bounds] owner:self userData:nil assumeInside:NO];
		_idleTimer = [NSTimer timerWithTimeInterval: 0.01 target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
		[[NSRunLoop currentRunLoop] addTimer:_idleTimer forMode:NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:_idleTimer forMode:NSEventTrackingRunLoopMode];
	}
	else
	{
		[self removeTrackingRect:_trackingRectTag];
		if (_idleTimer)
			[_idleTimer invalidate];
	}
}

//------------------------------------------------------------------------------------
-(void)resetCursorRects
{
	if ([self window])
	{
		[self removeTrackingRect:_trackingRectTag];
		_trackingRectTag = [self addTrackingRect:[self bounds] owner:self userData:nil assumeInside:NO];
	}
}

//------------------------------------------------------------------------------------
- (void)onTimer:(NSTimer*)theTimer
{
	if (_vstguiframe)
		_vstguiframe->idle ();
}

//------------------------------------------------------------------------------------
- (void)drawRect:(NSRect)rect
{
	NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];
	
	CDrawContext drawContext (_vstguiframe, [nsContext graphicsPort]);
	_vstguiframe->drawRect (&drawContext, rectFromNSRect (rect));
}

//------------------------------------------------------------------------------------
- (BOOL)onMouseDown: (NSEvent*)theEvent buttons: (long)buttons
{
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
	CMouseEventResult result = _vstguiframe->onMouseDown (p, buttons);
	return (result == kMouseEventHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
- (BOOL)onMouseUp: (NSEvent*)theEvent buttons: (long)buttons
{
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
	CMouseEventResult result = _vstguiframe->onMouseUp (p, buttons);
	return (result == kMouseEventHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
- (BOOL)onMouseMoved: (NSEvent*)theEvent buttons: (long)buttons
{
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
	CMouseEventResult result = _vstguiframe->onMouseMoved (p, buttons);
	return (result == kMouseEventHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
- (void)mouseDown:(NSEvent *)theEvent
{
	long buttons = kLButton;
	if (![self onMouseDown: theEvent buttons: buttons])
		[super mouseDown: theEvent];
}

//------------------------------------------------------------------------------------
- (void)rightMouseDown:(NSEvent *)theEvent
{
	long buttons = kRButton;
	if (![self onMouseDown: theEvent buttons: buttons])
		[super mouseDown: theEvent];
}

//------------------------------------------------------------------------------------
- (void)otherMouseDown:(NSEvent *)theEvent
{
	long buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 3: buttons = kMButton; break;
		case 4: buttons = kButton4; break;
		case 5: buttons = kButton5; break;
	}
	if (![self onMouseDown: theEvent buttons: buttons])
		[super mouseDown: theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseUp:(NSEvent *)theEvent
{
	long buttons = kLButton;
	if (![self onMouseUp: theEvent buttons: buttons])
		[super mouseUp: theEvent];
}

//------------------------------------------------------------------------------------
- (void)rightMouseUp:(NSEvent *)theEvent
{
	long buttons = kRButton;
	if (![self onMouseUp: theEvent buttons: buttons])
		[super mouseUp: theEvent];
}

//------------------------------------------------------------------------------------
- (void)otherMouseUp:(NSEvent *)theEvent
{
	long buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 3: buttons = kMButton; break;
		case 4: buttons = kButton4; break;
		case 5: buttons = kButton5; break;
	}
	if (![self onMouseUp: theEvent buttons: buttons])
		[super mouseUp: theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseMoved:(NSEvent *)theEvent
{
	long buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 1: buttons = kLButton; break;
		case 2: buttons = kRButton; break;
		case 3: buttons = kMButton; break;
		case 4: buttons = kButton4; break;
		case 5: buttons = kButton5; break;
	}
	if (![self onMouseMoved: theEvent buttons: buttons])
		[super mouseMoved: theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseDragged:(NSEvent *)theEvent
{
	long buttons = kLButton;
	if (![self onMouseMoved: theEvent buttons: buttons])
		[super mouseDragged: theEvent];
}

//------------------------------------------------------------------------------------
- (void)rightMouseDragged:(NSEvent *)theEvent
{
	long buttons = kRButton;
	if (![self onMouseMoved: theEvent buttons: buttons])
		[super mouseDragged: theEvent];
}

//------------------------------------------------------------------------------------
- (void)otherMouseDragged:(NSEvent *)theEvent
{
	long buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 3: buttons = kMButton; break;
		case 4: buttons = kButton4; break;
		case 5: buttons = kButton5; break;
	}
	if (![self onMouseMoved: theEvent buttons: buttons])
		[super mouseDragged: theEvent];
}

//------------------------------------------------------------------------------------
- (void)scrollWheel:(NSEvent *)theEvent
{
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
		_vstguiframe->onWheel (p, kMouseWheelAxisX, [theEvent deltaX], buttons);
	if ([theEvent deltaY])
		_vstguiframe->onWheel (p, kMouseWheelAxisY, [theEvent deltaY], buttons);
}

//------------------------------------------------------------------------------------
- (void)mouseEntered:(NSEvent *)theEvent
{
	if (_windowAcceptsMouseMoveEvents == NO)
	{
		if ([self window])
			[[self window] setAcceptsMouseMovedEvents: YES];
	}
}

//------------------------------------------------------------------------------------
- (void)mouseExited:(NSEvent *)theEvent
{
	if (_windowAcceptsMouseMoveEvents == NO)
	{
		if ([self window])
			[[self window] setAcceptsMouseMovedEvents: NO];
	}
}

//------------------------------------------------------------------------------------
- (VstKeyCode)createVstKeyCode: (NSEvent*) theEvent
{
	VstKeyCode kc = {0};
    NSString *s = [theEvent charactersIgnoringModifiers];
    if ([s length] == 1)
	{
		unichar c = [s characterAtIndex:0];
		switch (c)
		{
			case 8: case 0x7f:				kc.virt = VKEY_BACK; break;
			case 9:	case 0x19:				kc.virt = VKEY_TAB; break;
			case NSClearLineFunctionKey:	kc.virt = VKEY_CLEAR; break;
			case 0xd:						kc.virt = VKEY_RETURN; break;
			case NSPauseFunctionKey:		kc.virt = VKEY_PAUSE; break;
			case 0x1b:						kc.virt = VKEY_ESCAPE; break;
			case ' ':						kc.virt = VKEY_SPACE; break;
			case NSNextFunctionKey:			kc.virt = VKEY_NEXT; break;
			case NSEndFunctionKey:			kc.virt = VKEY_END; break;
			case NSHomeFunctionKey:			kc.virt = VKEY_HOME; break;

			case NSLeftArrowFunctionKey:	kc.virt = VKEY_LEFT; break;
			case NSUpArrowFunctionKey:		kc.virt = VKEY_UP; break;
			case NSRightArrowFunctionKey:	kc.virt = VKEY_RIGHT; break;
			case NSDownArrowFunctionKey:	kc.virt = VKEY_DOWN; break;
			case NSPageUpFunctionKey:		kc.virt = VKEY_PAGEUP; break;
			case NSPageDownFunctionKey:		kc.virt = VKEY_PAGEDOWN; break;
			
			case NSSelectFunctionKey:		kc.virt = VKEY_SELECT; break;
			case NSPrintFunctionKey:		kc.virt = VKEY_PRINT; break;
			// VKEY_ENTER
			// VKEY_SNAPSHOT
			case NSInsertFunctionKey:		kc.virt = VKEY_INSERT; break;
			case NSDeleteFunctionKey:		kc.virt = VKEY_DELETE; break;
			case NSHelpFunctionKey:			kc.virt = VKEY_HELP; break;


			case NSF1FunctionKey:			kc.virt = VKEY_F1; break;
			case NSF2FunctionKey:			kc.virt = VKEY_F2; break;
			case NSF3FunctionKey:			kc.virt = VKEY_F3; break;
			case NSF4FunctionKey:			kc.virt = VKEY_F4; break;
			case NSF5FunctionKey:			kc.virt = VKEY_F5; break;
			case NSF6FunctionKey:			kc.virt = VKEY_F6; break;
			case NSF7FunctionKey:			kc.virt = VKEY_F7; break;
			case NSF8FunctionKey:			kc.virt = VKEY_F8; break;
			case NSF9FunctionKey:			kc.virt = VKEY_F9; break;
			case NSF10FunctionKey:			kc.virt = VKEY_F10; break;
			case NSF11FunctionKey:			kc.virt = VKEY_F11; break;
			case NSF12FunctionKey:			kc.virt = VKEY_F12; break;
			default:
			{
				switch ([theEvent keyCode])
				{
					case 82:				kc.virt = VKEY_NUMPAD0; break;
					case 83:				kc.virt = VKEY_NUMPAD1; break;
					case 84:				kc.virt = VKEY_NUMPAD2; break;
					case 85:				kc.virt = VKEY_NUMPAD3; break;
					case 86:				kc.virt = VKEY_NUMPAD4; break;
					case 87:				kc.virt = VKEY_NUMPAD5; break;
					case 88:				kc.virt = VKEY_NUMPAD6; break;
					case 89:				kc.virt = VKEY_NUMPAD7; break;
					case 91:				kc.virt = VKEY_NUMPAD8; break;
					case 92:				kc.virt = VKEY_NUMPAD9; break;
					case 67:				kc.virt = VKEY_MULTIPLY; break;
					case 69:				kc.virt = VKEY_ADD; break;
					case 78:				kc.virt = VKEY_SUBTRACT; break;
					case 65:				kc.virt = VKEY_DECIMAL; break;
					case 75:				kc.virt = VKEY_DIVIDE; break;
					case 76:				kc.virt = VKEY_ENTER; break;
					default:
						kc.character = c; break;
				}
			}
		}
    }

	unsigned int modifiers = [theEvent modifierFlags];
	if (modifiers & NSShiftKeyMask)
		kc.modifier |= MODIFIER_SHIFT;
	if (modifiers & NSCommandKeyMask)
		kc.modifier |= MODIFIER_COMMAND;
	if (modifiers & NSAlternateKeyMask)
		kc.modifier |= MODIFIER_ALTERNATE;
	if (modifiers & NSControlKeyMask)
		kc.modifier |= MODIFIER_CONTROL;

	return kc;
}

//------------------------------------------------------------------------------------
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
	VstKeyCode keyCode = [self createVstKeyCode: theEvent];
	if (_vstguiframe->onKeyDown (keyCode) == 1)
		return YES;

	return [super performKeyEquivalent: theEvent];
}

//------------------------------------------------------------------------------------
- (void)keyDown:(NSEvent *)theEvent
{
	VstKeyCode keyCode = [self createVstKeyCode: theEvent];
	
	if (_vstguiframe->onKeyDown (keyCode) != 1 && keyCode.virt == VKEY_TAB)
	{
		if (keyCode.modifier & MODIFIER_SHIFT)
			[[self window] selectKeyViewPrecedingView:self];
		else
			[[self window] selectKeyViewFollowingView:self];
	}
}

//------------------------------------------------------------------------------------
- (void)keyUp:(NSEvent *)theEvent
{
	VstKeyCode keyCode = [self createVstKeyCode: theEvent];

	_vstguiframe->onKeyUp (keyCode);
}

//------------------------------------------------------------------------------------
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pboard = [sender draggingPasteboard];

	gDragContainer = new CocoaDragContainer (pboard);

	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);

	_vstguiframe->setCursor (kCursorNotAllowed);
	_vstguiframe->onDragEnter (gDragContainer, where);

	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	_vstguiframe->onDragMove (gDragContainer, where);

	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
- (void)draggingExited:(id <NSDraggingInfo>)sender
{
	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	_vstguiframe->onDragLeave (gDragContainer, where);
	_vstguiframe->setCursor (kCursorDefault);

	gDragContainer->forget ();
	gDragContainer = 0;
}

//------------------------------------------------------------------------------------
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	bool result = _vstguiframe->onDrop (gDragContainer, where);
	_vstguiframe->setCursor (kCursorDefault);
	gDragContainer->forget ();
	gDragContainer = 0;
	return result;
}

//------------------------------------------------------------------------------------
- (void) setTooltip: (const char*)tooltip forCView: (CView*)view
{
}

//------------------------------------------------------------------------------------
- (void) removeTooltip
{
}

@end

//------------------------------------------------------------------------------------
@implementation VSTGUI_NSTimer

//------------------------------------------------------------------------------------
- (id) initWithTimerObject: (CBaseObject*) obj fireTime: (int) ms
{
	[super init];
	_timerObject = obj;
	_timer = [NSTimer timerWithTimeInterval: ((double)ms)* (1./1000.) target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:_timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:_timer forMode:NSEventTrackingRunLoopMode];
	return self;
}

//------------------------------------------------------------------------------------
- (void) stop
{
	[_timer invalidate];
	_timer = 0;
}

//------------------------------------------------------------------------------------
- (void) onTimer:(NSTimer*)theTimer
{
	if (_timerObject)
		_timerObject->notify (0, CVSTGUITimer::kMsgTimer);
}

@end

//------------------------------------------------------------------------------------
@implementation VSTGUI_NSMenu

//------------------------------------------------------------------------------------
- (id) initWithOptionMenu: (COptionMenu*) menu
{
	bool multipleCheck = menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
	_optionMenu = menu;
	_selectedMenu = 0;
	_selectedItem = -1;
	[super init];

	for (long i = 0; i < menu->getNbEntries (); i++)
	{
		NSMenuItem* nsItem = 0;
		CMenuItem* item = menu->getEntry (i);
		NSMutableString* itemTitle = [[[NSMutableString alloc] initWithCString:item->getTitle () encoding:NSUTF8StringEncoding] autorelease];
		if (menu->getPrefixNumbers ())
		{
			NSString* prefixString = 0;
			switch (menu->getPrefixNumbers ())
			{
				case 2:	prefixString = [NSString stringWithFormat:@"%1d ", i+1]; break;
				case 3: prefixString = [NSString stringWithFormat:@"%02d ", i+1]; break;
				case 4: prefixString = [NSString stringWithFormat:@"%03d ", i+1]; break;
			}
			[itemTitle insertString:prefixString atIndex:0];
		}
		if (item->getSubmenu ())
		{
			nsItem = [self addItemWithTitle:itemTitle action:nil keyEquivalent:@""];
			VSTGUI_NSMenu* subMenu = [[VSTGUI_NSMenu alloc] initWithOptionMenu: item->getSubmenu ()];
			[self setSubmenu: subMenu forItem:nsItem];
		}
		else if (item->isSeparator ())
		{
			[self addItem:[NSMenuItem separatorItem]];
		}
		else
		{
			nsItem = [self addItemWithTitle:itemTitle action:@selector(menuItemSelected:) keyEquivalent:@""];
			if (item->isTitle ())
				[nsItem setIndentationLevel:1];
			[nsItem setTarget:self];
			[nsItem setTag: i];
			if (multipleCheck && item->isChecked ())
				[nsItem setState:NSOnState];
			else
				[nsItem setState:NSOffState];
			if (item->getKeycode ())
			{
				[nsItem setKeyEquivalent:[NSString stringWithCString:item->getKeycode () encoding:NSUTF8StringEncoding]];
				unsigned int keyModifiers = 0;
				if (item->getKeyModifiers () & kControl)
					keyModifiers |= NSCommandKeyMask;
				if (item->getKeyModifiers () & kShift)
					keyModifiers |= NSShiftKeyMask;
				if (item->getKeyModifiers () & kAlt)
					keyModifiers |= NSAlternateKeyMask;
				if (item->getKeyModifiers () & kApple)
					keyModifiers |= NSControlKeyMask;
				[nsItem setKeyEquivalentModifierMask:keyModifiers];
			}
		}
		if (nsItem && item->getIcon ())
		{
			CGImageRef cgImage = item->getIcon ()->createCGImage ();
			if (cgImage)
			{
				NSImage* nsImage = imageFromCGImageRef (cgImage);
				if (nsImage)
				{
					[nsItem setImage:nsImage];
					[nsImage release];
				}
				CGImageRelease (cgImage);
			}
		}
	}
	return self;
}

//------------------------------------------------------------------------------------
- (BOOL) validateMenuItem:(id)item
{
	CMenuItem* menuItem = _optionMenu->getEntry ([item tag]);
	if (!menuItem->isEnabled () || menuItem->isTitle ())
		return NO;
	return YES;
}

//------------------------------------------------------------------------------------
- (COptionMenu*) optionMenu
{
	return _optionMenu;
}

//------------------------------------------------------------------------------------
- (COptionMenu*) selectedMenu
{
	return _selectedMenu;
}

//------------------------------------------------------------------------------------
- (void) setSelectedMenu: (COptionMenu*) menu
{
	_selectedMenu = menu;
}

//------------------------------------------------------------------------------------
- (long) selectedItem
{
	return _selectedItem;
}

//------------------------------------------------------------------------------------
- (void) setSelectedItem: (long) item
{
	_selectedItem = item;
}

//------------------------------------------------------------------------------------
- (IBAction) menuItemSelected: (id) item
{
	id menu = self;
	while ([menu supermenu]) menu = [menu supermenu];
	[menu setSelectedMenu: _optionMenu];
	[menu setSelectedItem: [item tag]];
}

@end

@implementation VSTGUI_NSTextField

//------------------------------------------------------------------------------------
- (id) initWithTextEdit: (CTextEdit*) textEdit
{
	_textEdit = textEdit;

	NSView* frameView = (NSView*)_textEdit->getFrame ()->getNSView ();
	CPoint p (_textEdit->getViewSize ().left, _textEdit->getViewSize ().top);
	_textEdit->localToFrame (p);
	NSRect editFrameRect = {0};
	editFrameRect.origin = nsPointFromCPoint (p);
	editFrameRect.size.width = _textEdit->getViewSize ().getWidth ();
	editFrameRect.size.height = _textEdit->getViewSize ().getHeight ();
	NSView* containerView = [[NSView alloc] initWithFrame:editFrameRect];
	[containerView setAutoresizesSubviews:YES];

	editFrameRect.origin.x = 0;
	editFrameRect.origin.y = 0;
	[super initWithFrame:editFrameRect];

	#if __LP64__
	CTFontRef fontRef = (CTFontRef)_textEdit->getFont()->getPlatformFont ();
	CTFontDescriptorRef fontDesc = CTFontCopyFontDescriptor (fontRef);
	
	[self setFont:[NSFont fontWithDescriptor:(NSFontDescriptor *)fontDesc size:_textEdit->getFont ()->getSize ()]];
	CFRelease (fontDesc);
	#else
	
	#endif

	NSString* text = [NSString stringWithCString:_textEdit->getText () encoding:NSUTF8StringEncoding];

	[self setBackgroundColor:nsColorFromCColor (_textEdit->getBackColor ())];
	[self setTextColor:nsColorFromCColor (_textEdit->getFontColor ())];
	[self setAllowsEditingTextAttributes:NO];
	[self setImportsGraphics:NO];
	[self setStringValue:text];
	
	[containerView addSubview:self];
	[self syncSize];
	[frameView addSubview:containerView];

	NSTextFieldCell* cell = [self cell];
	[cell setLineBreakMode: NSLineBreakByClipping];
	[cell setScrollable:YES];
	if (_textEdit->getHoriAlign () == kCenterText)
		[cell setAlignment:NSCenterTextAlignment];
	else if (_textEdit->getHoriAlign () == kRightText)
		[cell setAlignment:NSRightTextAlignment];

	[self setDelegate:self];
	[[self window] makeFirstResponder: self];
	return self;
}

//------------------------------------------------------------------------------------
- (void) syncSize
{
	NSView* containerView = [self superview];
	CRect rect (_textEdit->getVisibleSize ());
	CRect viewSize = _textEdit->getViewSize ();
	CPoint p (0, 0);
	_textEdit->localToFrame (p);
	rect.offset (p.x, p.y);
	viewSize.offset (p.x, p.y);

	[containerView setFrame:nsRectFromCRect (rect)];

	CRect rect2 (viewSize);
	rect2.offset (-rect2.left, -rect2.top);
	CPoint offset;
	offset.x = viewSize.left - rect.left;
	offset.y = rect.bottom - viewSize.bottom;
	rect2.offset (offset.x, offset.y);
	[self setFrame:nsRectFromCRect (rect2)];

	rect.inset (-15, -15);
	[[containerView superview] setNeedsDisplayInRect:nsRectFromCRect (rect)];
}

//------------------------------------------------------------------------------------
- (void) removeFromSuperview
{
	NSView* containerView = [self superview];
	[containerView removeFromSuperview];
	[super removeFromSuperview];
	[containerView release];
}

//------------------------------------------------------------------------------------
- (BOOL) control:(NSControl*)control textView:(NSTextView*)textView doCommandBySelector:(SEL)commandSelector
{
	if (commandSelector == @selector (insertNewline:))
	{
		_textEdit->bWasReturnPressed = true;
		_textEdit->looseFocus ();
	}
	else if (commandSelector == @selector (insertTab:))
	{
		if (!_textEdit->getFrame ()->advanceNextFocusView (_textEdit, false))
		{
			[[self window] makeFirstResponder:(NSView*)_textEdit->getFrame ()->getNSView ()];
			[[self window] selectKeyViewFollowingView:(NSView*)_textEdit->getFrame ()->getNSView ()];
		}
		return YES;
	}
	else if (commandSelector == @selector (insertBacktab:))
	{
		if (!_textEdit->getFrame ()->advanceNextFocusView (_textEdit, true))
		{
			[[self window] makeFirstResponder:(NSView*)_textEdit->getFrame ()->getNSView ()];
			[[self window] selectKeyViewPrecedingView:(NSView*)_textEdit->getFrame ()->getNSView ()];
		}
		return YES;
	}
	else if (commandSelector == @selector (cancelOperation:))
	{
		_textEdit->looseFocus ();
		return YES; // return YES, otherwise it beeps !!!
	}
	return NO;
}

@end

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
CocoaDragContainer::CocoaDragContainer (NSPasteboard* platformDrag)
: pb (platformDrag)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
		nbItems = 1;
	else
	{
		supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
		NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
		if (hasFilenames)
		{
			NSArray* fileNames = [pb propertyListForType:hasFilenames];
			nbItems = [fileNames count];
		}
	}
}

//------------------------------------------------------------------------------------
CocoaDragContainer::~CocoaDragContainer ()
{
	if (lastItem)
		free (lastItem);
}

//-----------------------------------------------------------------------------
void* CocoaDragContainer::first (long& size, long& type)
{
	iterator = 0;
	return next (size, type);
}

//------------------------------------------------------------------------------------
void* CocoaDragContainer::next (long& size, long& type)
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
	size = 0;
	iterator++;
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
	{
		if (iterator-1 == 0)
		{
			NSString* unicodeText = [pb stringForType:NSStringPboardType];
			if (unicodeText)
			{
				const char* utf8Text = [unicodeText UTF8String];
				char* data = (char*)malloc (strlen (utf8Text) + 1);
				strcpy (data, utf8Text);
				type = CDragContainer::kUnicodeText;
				size = strlen (utf8Text);
				lastItem = data;
				return data;
			}
		}
		type = CDragContainer::kError;
		return 0;
	}
		
	supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
	NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
	if (hasFilenames)
	{
		NSArray* fileNames = [pb propertyListForType:hasFilenames];
		if (iterator-1 < [fileNames count])
		{
			NSString* filename = [fileNames objectAtIndex:iterator-1];
			if (filename)
			{
				const char* utf8Text = [filename UTF8String];
				char* data = (char*)malloc (strlen (utf8Text) + 1);
				strcpy (data, utf8Text);
				type = CDragContainer::kFile;
				size = strlen (utf8Text);
				lastItem = data;
				return data;
			}
		}
		type = CDragContainer::kError;
		return 0;
	}
	
	type = CDragContainer::kError;
	return 0;
}

//------------------------------------------------------------------------------------
long CocoaDragContainer::getType (long idx) const
{
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
		return idx == 0 ? CDragContainer::kUnicodeText : CDragContainer::kError;
		
	supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
	NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
	if (hasFilenames)
	{
		NSArray* fileNames = [pb propertyListForType:hasFilenames];
		if ([fileNames count] > idx)
			return CDragContainer::kFile;
	}

	return CDragContainer::kUnknown;
}

#endif
/// \endcond
