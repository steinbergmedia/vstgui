//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 3.6
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2008, Steinberg Media Technologies, All Rights Reserved
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
#import "vstkeycode.h"
#import "cfileselector.h"
#import <objc/runtime.h>
#import <objc/message.h>

#define HIDDEN __attribute__((__visibility__("hidden")))

//------------------------------------------------------------------------------------
static Class menuClass = 0;
static Class fileSelectorDelegateClass = 0;
static Class textFieldClass = 0;
static Class viewClass = 0;
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
class GenerateUniqueVSTGUIClasses
{
public:
	GenerateUniqueVSTGUIClasses ();
	~GenerateUniqueVSTGUIClasses ();
};

//------------------------------------------------------------------------------------
inline HIDDEN id get_Objc_Value (id obj, const char* name)
{
	Ivar ivar = class_getInstanceVariable ([obj class], name);
	if (ivar)
	{
		id value = object_getIvar (obj, ivar);
		return value;
	}
	return nil;
}

//------------------------------------------------------------------------------------
inline HIDDEN void set_Objc_Value (id obj, const char* name, id value)
{
	Ivar ivar = class_getInstanceVariable ([obj class], name);
	if (ivar)
	{
		object_setIvar (obj, ivar, value);
	}
}

#define OBJC_SUPER(x) objc_super __os; __os.receiver = x; __os.super_class = class_getSuperclass ([x class]);
#define SUPER	&__os
#define OBJC_GET_VALUE(x,y) get_Objc_Value (x, #y)
#define OBJC_SET_VALUE(x,y,z) set_Objc_Value (x, #y, (id)z)


//------------------------------------------------------------------------------------
@interface VSTGUI_NSView : NSView {
	CFrame* _vstguiframe;
	NSTrackingArea* _trackingArea;
}

- (id) initWithFrame: (CFrame*) frame andSize: (const CRect*) size;
- (void) setTooltip: (const char*)tooltip forCView: (CView*)view;
- (void) removeTooltip;
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
HIDDEN NSRect nsRectFromCRect (const CRect& rect)
{
	NSRect r;
	r.origin.x = rect.left;
	r.origin.y = rect.top;
	r.size.width = rect.getWidth ();
	r.size.height = rect.getHeight ();
	return r;
}

//------------------------------------------------------------------------------------
HIDDEN NSPoint nsPointFromCPoint (const CPoint& point)
{
	NSPoint p = { point.x, point.y };
	return p;
}

//------------------------------------------------------------------------------------
HIDDEN CRect rectFromNSRect (const NSRect& rect)
{
	CRect r (rect.origin.x, rect.origin.y, 0, 0);
	r.setWidth (rect.size.width);
	r.setHeight (rect.size.height);
	return r;
}

//------------------------------------------------------------------------------------
HIDDEN CPoint pointFromNSPoint (const NSPoint& point)
{
	CPoint p (point.x, point.y);
	return p;
}

//------------------------------------------------------------------------------------
HIDDEN NSColor* nsColorFromCColor (const CColor& color)
{
	return [NSColor colorWithDeviceRed:color.red/255. green:color.green/255. blue:color.blue/255. alpha:color.alpha/255.];
}

//------------------------------------------------------------------------------------
static NSImage* imageFromCGImageRef (CGImageRef image)
{
    NSRect imageRect = NSMakeRect (0.0, 0.0, 0.0, 0.0);
    CGContextRef imageContext = nil;
    NSImage* newImage = nil;
 
    // Get the image dimensions.
    imageRect.size.height = CGImageGetHeight (image);
    imageRect.size.width = CGImageGetWidth (image);
 
    // Create a new image to receive the Quartz image data.
    newImage = [[NSImage alloc] initWithSize:imageRect.size];
    [newImage lockFocus];
 
    // Get the Quartz context and draw.
    imageContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    CGContextDrawImage(imageContext, NSRectToCGRect (imageRect), image);
    [newImage unlockFocus];
 
    return newImage;
}

//------------------------------------------------------------------------------------
//-- The following function are called from either vstgui.cpp or vstcontrols.cpp
//------------------------------------------------------------------------------------
HIDDEN void* createNSView (CFrame* frame, const CRect& size)
{
	VSTGUI_NSView* view = [[VSTGUI_NSView alloc] initWithFrame: frame andSize: &size];
	return view;
}

//------------------------------------------------------------------------------------
HIDDEN void destroyNSView (void* ptr)
{
	NSView* view = (NSView*)ptr;
	[view removeFromSuperview];
	[view release];
}

//------------------------------------------------------------------------------------
HIDDEN void invalidNSViewRect (void* nsView, const CRect& size)
{
	NSView* view = (NSView*)nsView;
	NSRect r = nsRectFromCRect (size);
	[view setNeedsDisplayInRect:r];
}

//------------------------------------------------------------------------------------
HIDDEN void resizeNSView (void* nsView, const CRect& newSize)
{
	NSView* view = (NSView*)nsView;
	unsigned int oldResizeMask = [view autoresizingMask];
	NSRect oldFrame = [view frame];
	[view setAutoresizingMask: 0];
	NSRect r = nsRectFromCRect (newSize);
	[view setFrame: r];
	[view setAutoresizingMask: oldResizeMask];
}

//------------------------------------------------------------------------------------
HIDDEN void getSizeOfNSView (void* nsView, CRect* rect)
{
	NSView* view = (NSView*)nsView;
	*rect = rectFromNSRect ([view frame]);
}

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
HIDDEN void nsViewSetMouseCursor (CCursorType type)
{
	@try {
	NSCursor* cur = 0;
	switch (type)
	{
		case kCursorWait: cur = [NSCursor arrowCursor]; break;
		case kCursorHSize: cur = [NSCursor resizeLeftRightCursor]; break;
		case kCursorVSize: cur = [NSCursor resizeUpDownCursor]; break;
		case kCursorSizeAll: cur = [NSCursor crosshairCursor]; break;
		case kCursorNESWSize: cur = [NSCursor arrowCursor]; break;
		case kCursorNWSESize: cur = [NSCursor arrowCursor]; break;
		case kCursorCopy: cur = [NSCursor performSelector:@selector(_copyDragCursor)]; break;
		case kCursorNotAllowed: cur = [NSCursor performSelector:@selector(operationNotAllowedCursor)]; break;
		case kCursorHand: cur = [NSCursor openHandCursor]; break;
		default: cur = [NSCursor arrowCursor]; break;
	}
	if (cur)
		[cur set];
	} @catch(...) { [[NSCursor arrowCursor] set]; }
}

//------------------------------------------------------------------------------------
HIDDEN void* addNSTextField (CFrame* frame, CTextEdit* edit)
{
	return [[textFieldClass alloc] performSelector:@selector (initWithTextEdit:) withObject:(id)edit];
}

//------------------------------------------------------------------------------------
HIDDEN void moveNSTextField (void* control, CTextEdit* edit)
{
	[(id)control performSelector:@selector(syncSize)];
}

//------------------------------------------------------------------------------------
HIDDEN void removeNSTextField (void* control)
{
	[(id)control performSelector:@selector(removeFromSuperview)];
	[(id)control performSelector:@selector(autorelease)];
}

//------------------------------------------------------------------------------------
HIDDEN bool getNSTextFieldText (void* control, char* text, long maxSize)
{
	[[(id)control stringValue] getCString:text maxLength:maxSize encoding:NSUTF8StringEncoding];
	return true;
}

//------------------------------------------------------------------------------------
HIDDEN long showNSContextMenu (COptionMenu* menu, COptionMenu** usedMenu)
{
	bool multipleCheck = menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
	NSView* view = (NSView*)menu->getFrame ()->getNSView ();
	NSMenu* nsMenu = [[menuClass alloc] performSelector:@selector(initWithOptionMenu:) withObject:(id)menu];
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
	*usedMenu = (COptionMenu*)[nsMenu performSelector:@selector(selectedMenu)];
	return (long)[nsMenu performSelector:@selector(selectedItem)];
}

//------------------------------------------------------------------------------------
HIDDEN void nsViewSetTooltip (CView* view, const char* tooltip)
{
	if (view->getFrame ())
	{
		VSTGUI_NSView* nsView = (VSTGUI_NSView*)view->getFrame ()->getNSView ();
		[nsView setTooltip:tooltip forCView:view];
	}
}

//------------------------------------------------------------------------------------
HIDDEN void nsViewRemoveTooltip (CView* view)
{
	if (view->getFrame ())
	{
		VSTGUI_NSView* nsView = (VSTGUI_NSView*)view->getFrame ()->getNSView ();
		[nsView removeTooltip];
	}
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
static VstKeyCode CreateVstKeyCodeFromNSEvent (NSEvent* theEvent)
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
static long eventButton (NSEvent* theEvent)
{
	if ([theEvent type] == NSMouseMoved)
		return 0;
	long buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 0: buttons = kLButton; break;
		case 1: buttons = kRButton; break;
		case 2: buttons = kMButton; break;
		case 3: buttons = kButton4; break;
		case 4: buttons = kButton5; break;
	}
	return buttons;
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
@implementation VSTGUI_NSView

//------------------------------------------------------------------------------------
- (id) initWithFrame: (CFrame*) frame andSize: (const CRect*) size
{
	NSRect nsSize = nsRectFromCRect (*size);
	self = [super initWithFrame: nsSize];
	if (self)
	{
		_vstguiframe = frame;

		NSView* parentView = (NSView*)_vstguiframe->getSystemWindow ();
		if (parentView)
			[parentView addSubview: self];

		[self registerForDraggedTypes:[NSArray arrayWithObjects:NSStringPboardType, NSFilenamesPboardType, nil]];
		
		_trackingArea = [[NSTrackingArea alloc] initWithRect:[self frame] options:NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingActiveInActiveApp|NSTrackingInVisibleRect owner:self userInfo:nil];
		[self addTrackingArea: _trackingArea];
	}
	return self;
}

//------------------------------------------------------------------------------------
- (void) dealloc
{
	[self removeTrackingArea:_trackingArea];
	[_trackingArea release];
	[super dealloc];
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
- (void)drawRect:(NSRect)rect
{
	NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];
	
	CDrawContext drawContext (_vstguiframe, [nsContext graphicsPort]);
	_vstguiframe->drawRect (&drawContext, rectFromNSRect (rect));
}

//------------------------------------------------------------------------------------
- (BOOL)onMouseDown: (NSEvent*)theEvent
{
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
	CMouseEventResult result = _vstguiframe->onMouseDown (p, buttons);
	return (result == kMouseEventHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
- (BOOL)onMouseUp: (NSEvent*)theEvent
{
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
	CMouseEventResult result = _vstguiframe->onMouseUp (p, buttons);
	return (result == kMouseEventHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
- (BOOL)onMouseMoved: (NSEvent*)theEvent
{
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
	CMouseEventResult result = _vstguiframe->onMouseMoved (p, buttons);
	return (result == kMouseEventHandled) ? YES : NO;
}

//------------------------------------------------------------------------------------
- (void)mouseDown:(NSEvent *)theEvent
{
	if (![self onMouseDown: theEvent])
		[super mouseDown: theEvent];
}

//------------------------------------------------------------------------------------
- (void)rightMouseDown:(NSEvent *)theEvent
{
	if (![self onMouseDown: theEvent])
		[super mouseDown: theEvent];
}

//------------------------------------------------------------------------------------
- (void)otherMouseDown:(NSEvent *)theEvent
{
	if (![self onMouseDown: theEvent])
		[super mouseDown: theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseUp:(NSEvent *)theEvent
{
	if (![self onMouseUp: theEvent])
		[super mouseUp: theEvent];
}

//------------------------------------------------------------------------------------
- (void)rightMouseUp:(NSEvent *)theEvent
{
	if (![self onMouseUp: theEvent])
		[super mouseUp: theEvent];
}

//------------------------------------------------------------------------------------
- (void)otherMouseUp:(NSEvent *)theEvent
{
	if (![self onMouseUp: theEvent])
		[super mouseUp: theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseMoved:(NSEvent *)theEvent
{
	if (![self onMouseMoved: theEvent])
		[super mouseMoved: theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseDragged:(NSEvent *)theEvent
{
	if (![self onMouseMoved: theEvent])
		[super mouseDragged: theEvent];
}

//------------------------------------------------------------------------------------
- (void)rightMouseDragged:(NSEvent *)theEvent
{
	if (![self onMouseMoved: theEvent])
		[super mouseDragged: theEvent];
}

//------------------------------------------------------------------------------------
- (void)otherMouseDragged:(NSEvent *)theEvent
{
	if (![self onMouseMoved: theEvent])
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
	[self mouseMoved:theEvent];
}

//------------------------------------------------------------------------------------
- (void)mouseExited:(NSEvent *)theEvent
{
	[self mouseMoved:theEvent];
}

//------------------------------------------------------------------------------------
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
	VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);
	if (_vstguiframe->onKeyDown (keyCode) == 1)
		return YES;

	return [super performKeyEquivalent: theEvent];
}

//------------------------------------------------------------------------------------
- (void)keyDown:(NSEvent *)theEvent
{
	VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);
	
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
	VstKeyCode keyCode = CreateVstKeyCodeFromNSEvent (theEvent);

	_vstguiframe->onKeyUp (keyCode);
}

//------------------------------------------------------------------------------------
- (NSDragOperation)draggingEntered:(id)sender
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
- (NSDragOperation)draggingUpdated:(id)sender
{
	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	_vstguiframe->onDragMove (gDragContainer, where);

	return NSDragOperationGeneric;
}

//------------------------------------------------------------------------------------
- (void)draggingExited:(id)sender
{
	CPoint where;
	nsViewGetCurrentMouseLocation (self, where);
	_vstguiframe->onDragLeave (gDragContainer, where);
	_vstguiframe->setCursor (kCursorDefault);

	gDragContainer->forget ();
	gDragContainer = 0;
}

//------------------------------------------------------------------------------------
- (BOOL)performDragOperation:(id)sender
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
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
struct VSTGUI_NSMenu_Var
{
	COptionMenu* _optionMenu;
	COptionMenu* _selectedMenu;
	long _selectedItem;
};

//------------------------------------------------------------------------------------
static id VSTGUI_NSMenu_Init (id self, SEL _cmd, void* _menu)
{
	OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(init)); // self = [super init];
	if (self)
	{
		NSMenu* nsMenu = (NSMenu*)self;
		COptionMenu* menu = (COptionMenu*)_menu;
		VSTGUI_NSMenu_Var* var = new VSTGUI_NSMenu_Var;
		var->_optionMenu = menu;
		var->_selectedItem = 0;
		var->_selectedMenu = 0;
		OBJC_SET_VALUE(self, _private, var);

		bool multipleCheck = menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
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
				nsItem = [nsMenu addItemWithTitle:itemTitle action:nil keyEquivalent:@""];
				NSMenu* subMenu = [[menuClass alloc] performSelector:@selector(initWithOptionMenu:) withObject:(id)item->getSubmenu ()];
				[nsMenu setSubmenu: subMenu forItem:nsItem];
			}
			else if (item->isSeparator ())
			{
				[nsMenu addItem:[NSMenuItem separatorItem]];
			}
			else
			{
				nsItem = [nsMenu addItemWithTitle:itemTitle action:@selector(menuItemSelected:) keyEquivalent:@""];
				if (item->isTitle ())
					[nsItem setIndentationLevel:1];
				[nsItem setTarget:nsMenu];
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
	}
	return self;
}

//-----------------------------------------------------------------------------
static void VSTGUI_NSMenu_Dealloc (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
		delete var;
	OBJC_SUPER(self)
	objc_msgSendSuper (SUPER, @selector(dealloc)); // [super dealloc];
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSMenu_ValidateMenuItem (id self, SEL _cmd, id item)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var && var->_optionMenu)
	{
		CMenuItem* menuItem = var->_optionMenu->getEntry ([item tag]);
		if (!menuItem->isEnabled () || menuItem->isTitle ())
			return NO;
	}
	return YES;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSMenu_MenuItemSelected (id self, SEL _cmd, id item)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
	{
		id menu = self;
		while ([menu supermenu]) menu = [menu supermenu];
		[menu performSelector:@selector (setSelectedMenu:) withObject: (id)var->_optionMenu];
		[menu performSelector:@selector (setSelectedItem:) withObject: (id)[item tag]];
	}
}

//------------------------------------------------------------------------------------
static void* VSTGUI_NSMenu_OptionMenu (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_optionMenu : 0;
}

//------------------------------------------------------------------------------------
static void* VSTGUI_NSMenu_SelectedMenu (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_selectedMenu : 0;
}

//------------------------------------------------------------------------------------
static long VSTGUI_NSMenu_SelectedItem (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_selectedItem : 0;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSMenu_SetSelectedMenu (id self, SEL _cmd, void* menu)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
		var->_selectedMenu = (COptionMenu*)menu;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSMenu_SetSelectedItem (id self, SEL _cmd, long item)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
		var->_selectedItem = item;
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
static id VSTGUI_NSTextField_Init (id self, SEL _cmd, void* textEdit)
{
	OBJC_SUPER(self)
	if (self)
	{
		CTextEdit* te = (CTextEdit*)textEdit;
		NSView* frameView = (NSView*)te->getFrame ()->getNSView ();
		CPoint p (te->getViewSize ().left, te->getViewSize ().top);
		te->localToFrame (p);
		NSRect editFrameRect = {0};
		editFrameRect.origin = nsPointFromCPoint (p);
		editFrameRect.size.width = te->getViewSize ().getWidth ();
		editFrameRect.size.height = te->getViewSize ().getHeight ();
		NSView* containerView = [[NSView alloc] initWithFrame:editFrameRect];
		[containerView setAutoresizesSubviews:YES];

		editFrameRect.origin.x = 0;
		editFrameRect.origin.y = 0;
		self = objc_msgSendSuper (SUPER, @selector(initWithFrame:), editFrameRect);
		if (!self)
		{
			[containerView release];
			return nil;
		}
		OBJC_SET_VALUE (self, _textEdit, textEdit);

		CTFontRef fontRef = (CTFontRef)te->getFont()->getPlatformFont ();
		CTFontDescriptorRef fontDesc = CTFontCopyFontDescriptor (fontRef);
		
		[self setFont:[NSFont fontWithDescriptor:(NSFontDescriptor *)fontDesc size:te->getFont ()->getSize ()]];
		CFRelease (fontDesc);

		NSString* text = [NSString stringWithCString:te->getText () encoding:NSUTF8StringEncoding];

		[self setBackgroundColor:nsColorFromCColor (te->getBackColor ())];
		[self setTextColor:nsColorFromCColor (te->getFontColor ())];
		[self setAllowsEditingTextAttributes:NO];
		[self setImportsGraphics:NO];
		[self setStringValue:text];
		
		[containerView addSubview:self];
		[self performSelector:@selector(syncSize)];
		[frameView addSubview:containerView];

		NSTextFieldCell* cell = [self cell];
		[cell setLineBreakMode: NSLineBreakByClipping];
		[cell setScrollable:YES];
		if (te->getHoriAlign () == kCenterText)
			[cell setAlignment:NSCenterTextAlignment];
		else if (te->getHoriAlign () == kRightText)
			[cell setAlignment:NSRightTextAlignment];

		[self setDelegate:self];
		[[self window] makeFirstResponder: self];
	}
	return self;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSTextField_SyncSize (id self, SEL _cmd)
{
	CTextEdit* te = (CTextEdit*)OBJC_GET_VALUE(self, _textEdit);
	if (!te)
		return;
	NSView* containerView = [self superview];
	CRect rect (te->getVisibleSize ());
	CRect viewSize = te->getViewSize ();
	CPoint p (0, 0);
	te->localToFrame (p);
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
static void VSTGUI_NSTextField_RemoveFromSuperview (id self, SEL _cmd)
{
	NSView* containerView = [self superview];
	if (containerView)
	{
		[containerView removeFromSuperview];
		OBJC_SUPER(self)
		objc_msgSendSuper (SUPER, @selector(removeFromSuperview)); // [super removeFromSuperview];
		[containerView release];
	}
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSTextField_DoCommandBySelector (id self, SEL _cmd, NSControl* control, NSTextView* textView, SEL commandSelector)
{
	CTextEdit* te = (CTextEdit*)OBJC_GET_VALUE(self, _textEdit);
	if (!te)
		return NO;
	if (commandSelector == @selector (insertNewline:))
	{
		te->bWasReturnPressed = true;
		te->looseFocus ();
	}
	else if (commandSelector == @selector (insertTab:))
	{
		if (!te->getFrame ()->advanceNextFocusView (te, false))
		{
			[[self window] makeFirstResponder:(NSView*)te->getFrame ()->getNSView ()];
			[[self window] selectKeyViewFollowingView:(NSView*)te->getFrame ()->getNSView ()];
		}
		return YES;
	}
	else if (commandSelector == @selector (insertBacktab:))
	{
		if (!te->getFrame ()->advanceNextFocusView (te, true))
		{
			[[self window] makeFirstResponder:(NSView*)te->getFrame ()->getNSView ()];
			[[self window] selectKeyViewPrecedingView:(NSView*)te->getFrame ()->getNSView ()];
		}
		return YES;
	}
	else if (commandSelector == @selector (cancelOperation:))
	{
		te->looseFocus ();
		return YES; // return YES, otherwise it beeps !!!
	}
	return NO;
}

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

//------------------------------------------------------------------------------------
static id VSTGUI_FileSelector_Delegate_Init (id self, SEL _cmd, void* fileSelector);
static void VSTGUI_FileSelector_Delegate_Dealloc (id self, SEL _cmd);
static void VSTGUI_FileSelector_Delegate_OpenPanelDidEnd (id self, SEL _cmd, NSOpenPanel* openPanel, int returnCode, void* contextInfo);
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
// Creating Objective-C classes at runtime, to prevent name clashes with different VSTGUI versions.
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
static Class generateUniqueClass (NSMutableString* className, Class baseClass)
{
	NSString* _className = [NSString stringWithString:className];
	NSInteger iteration = 0;
	id cl = nil;
	while ((cl = objc_lookUpClass ([className UTF8String])) != nil)
	{
		iteration++;
		[className setString:[NSString stringWithFormat:@"%@_%d", _className, iteration]];
	}
	Class resClass = objc_allocateClassPair (baseClass, [className UTF8String], 0);
	return resClass;
}

//------------------------------------------------------------------------------------
GenerateUniqueVSTGUIClasses _gGenerateUniqueVSTGUIClasses;
GenerateUniqueVSTGUIClasses* gGenerateUniqueVSTGUIClasses = &_gGenerateUniqueVSTGUIClasses;

//------------------------------------------------------------------------------------
GenerateUniqueVSTGUIClasses::GenerateUniqueVSTGUIClasses ()
{
	BOOL res;
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	// generating VSTGUI_NSView
	NSMutableString* viewClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSView"] autorelease];
	viewClass = generateUniqueClass (viewClassName, [NSView class]);

	// generating VSTGUI_NSMenu
	NSMutableString* menuClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSMenu"] autorelease];
	menuClass = generateUniqueClass (menuClassName, [NSMenu class]);
	res = class_addMethod (menuClass, @selector(initWithOptionMenu:), IMP (VSTGUI_NSMenu_Init), "@@:@:^:");
	res = class_addMethod (menuClass, @selector(dealloc), IMP (VSTGUI_NSMenu_Dealloc), "v@:@:");
	res = class_addMethod (menuClass, @selector(validateMenuItem:), IMP (VSTGUI_NSMenu_ValidateMenuItem), "B@:@:@:");
	res = class_addMethod (menuClass, @selector(menuItemSelected:), IMP (VSTGUI_NSMenu_MenuItemSelected), "v@:@:@:");
	res = class_addMethod (menuClass, @selector(optionMenu), IMP (VSTGUI_NSMenu_OptionMenu), "^@:@:");
	res = class_addMethod (menuClass, @selector(selectedMenu), IMP (VSTGUI_NSMenu_SelectedMenu), "^@:@:");
	res = class_addMethod (menuClass, @selector(selectedItem), IMP (VSTGUI_NSMenu_SelectedItem), "l@:@:");
	res = class_addMethod (menuClass, @selector(setSelectedMenu:), IMP (VSTGUI_NSMenu_SetSelectedMenu), "^@:@:^:");
	res = class_addMethod (menuClass, @selector(setSelectedItem:), IMP (VSTGUI_NSMenu_SetSelectedItem), "^@:@:l:");
	res = class_addIvar (menuClass, "_private", sizeof (VSTGUI_NSMenu_Var*), log2(sizeof(VSTGUI_NSMenu_Var*)), @encode(VSTGUI_NSMenu_Var*));
	objc_registerClassPair (menuClass);

	// generating VSTGUI_NSTextField
	NSMutableString* textFieldClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSTextField"] autorelease];
	textFieldClass = generateUniqueClass (textFieldClassName, [NSTextField class]);
	res = class_addMethod (textFieldClass, @selector(initWithTextEdit:), IMP (VSTGUI_NSTextField_Init), "@@:@:^:");
	res = class_addMethod (textFieldClass, @selector(syncSize), IMP (VSTGUI_NSTextField_SyncSize), "v@:@:");
	res = class_addMethod (textFieldClass, @selector(removeFromSuperview), IMP (VSTGUI_NSTextField_RemoveFromSuperview), "v@:@:");
	res = class_addMethod (textFieldClass, @selector(control:textView:doCommandBySelector:), IMP (VSTGUI_NSTextField_DoCommandBySelector), "B@:@:@:@::");
	res = class_addIvar (textFieldClass, "_textEdit", sizeof (void*), log2(sizeof(void*)), @encode(void*));
	objc_registerClassPair (textFieldClass);

	// generating VSTGUI_FileSelector_Delegate
	NSMutableString* fileSelectorDelegateClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_FileSelector_Delegate"] autorelease];
	fileSelectorDelegateClass = generateUniqueClass (fileSelectorDelegateClassName, [NSObject class]);
	res = class_addMethod (fileSelectorDelegateClass, @selector(initWithFileSelector:), IMP (VSTGUI_FileSelector_Delegate_Init), "@@:@:^:");
	res = class_addMethod (fileSelectorDelegateClass, @selector(dealloc), IMP (VSTGUI_FileSelector_Delegate_Dealloc), "v@:@:");
	res = class_addMethod (fileSelectorDelegateClass, @selector(openPanelDidEnd:returnCode:contextInfo:), IMP (VSTGUI_FileSelector_Delegate_OpenPanelDidEnd), "v@:@:@:I:@:");
	res = class_addIvar (fileSelectorDelegateClass, "_fileSelector", sizeof (void*), log2(sizeof(void*)), @encode(void*));
	objc_registerClassPair (fileSelectorDelegateClass);

	[pool release];
}

//------------------------------------------------------------------------------------
GenerateUniqueVSTGUIClasses::~GenerateUniqueVSTGUIClasses ()
{
	objc_disposeClassPair (menuClass);
	objc_disposeClassPair (viewClass);
	objc_disposeClassPair (textFieldClass);
	objc_disposeClassPair (fileSelectorDelegateClass);
}
#endif // MAC_COCOA

#if !MAC_COCOA
// the cocoa fileselector is also used for carbon
#import <Cocoa/Cocoa.h>
#import "cfileselector.h"
#endif
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CocoaFileSelector : public CNewFileSelector
{
public:
	CocoaFileSelector (CFrame* frame, Style style);
	~CocoaFileSelector ();

	void openPanelDidEnd (NSOpenPanel* panel, int resultCode);
protected:
	bool runInternal (CBaseObject* delegate);
	bool runModalInternal ();
	void cancelInternal ();

	Style style;
	CBaseObject* delegate;
	NSSavePanel* savePanel;
};

//-----------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* frame, Style style)
{
	return new CocoaFileSelector (frame, style);
}

//-----------------------------------------------------------------------------
CocoaFileSelector::CocoaFileSelector (CFrame* frame, Style style)
: CNewFileSelector (frame)
, style (style)
, delegate (0)
{
	savePanel = nil;
}

//-----------------------------------------------------------------------------
CocoaFileSelector::~CocoaFileSelector ()
{
	if (delegate)
		delegate->forget ();
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::openPanelDidEnd (NSOpenPanel* openPanel, int res)
{
	if (res == NSFileHandlingPanelOKButton)
	{
		if (style == kSelectSaveFile)
		{
			NSURL* url = [openPanel URL];
			const char* utf8Path = url ? [[url path] UTF8String] : 0;
			if (utf8Path)
			{
				char* path = (char*)malloc (strlen (utf8Path) + 1);
				strcpy (path, utf8Path);
				result.push_back (path);
			}
		}
		else
		{
			NSArray* urls = [openPanel URLs];
			for (int i = 0; i < [urls count]; i++)
			{
				NSURL* url = [urls objectAtIndex:i];
				if (url == 0 || [url path] == 0)
					continue;
				const char* utf8Path = [[url path] UTF8String];
				if (utf8Path)
				{
					char* path = (char*)malloc (strlen (utf8Path) + 1);
					strcpy (path, utf8Path);
					result.push_back (path);
				}
			}
		}
	}
	if (delegate)
		delegate->notify (this, CNewFileSelector::kSelectEndMessage);
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::cancelInternal ()
{
	if (savePanel)
		[savePanel cancel:nil];
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::runInternal (CBaseObject* _delegate)
{
	remember ();
	NSWindow* parentWindow = nil;
	if (_delegate)
	{
		#if MAC_COCOA
		if (frame && frame->getSystemWindow () && frame->getNSView ())
			parentWindow = [((NSView*)frame->getSystemWindow ()) window];
		#endif
		delegate = _delegate;
		delegate->remember ();
	}
	NSOpenPanel* openPanel = nil;
	NSMutableArray* typesArray = nil;
	if (extensions.size () > 0)
	{
		typesArray = [[[NSMutableArray alloc] init] autorelease];
		std::list<CFileExtension>::const_iterator it = extensions.begin ();
		while (it != extensions.end ())
		{
			NSString* uti = 0;
			if ((*it).getMimeType ())
				uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassMIMEType, (CFStringRef)[NSString stringWithCString: (*it).getMimeType () encoding:NSUTF8StringEncoding], NULL);
			if (uti == 0 && (*it).getMacType ())
			{
				NSString* osType = (NSString*)UTCreateStringForOSType ((*it).getMacType ());
				if (osType)
				{
					uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassOSType, (CFStringRef)osType, NULL);
					[osType release];
				}
			}
			if (uti == 0 && (*it).getExtension ())
				uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassFilenameExtension, (CFStringRef)[NSString stringWithCString: (*it).getExtension () encoding:NSUTF8StringEncoding], NULL);
			if (uti)
			{
				[typesArray addObject:uti];
				[uti release];
			}
			it++;
		}
	}
	if (style == kSelectSaveFile)
	{
		savePanel = [NSSavePanel savePanel];
		if (typesArray)
			[savePanel setAllowedFileTypes:typesArray];
	}
	else
	{
		savePanel = openPanel = [NSOpenPanel openPanel];
		if (style == kSelectFile)
		{
			[openPanel setAllowsMultipleSelection:allowMultiFileSelection ? YES : NO];
		}
		else
		{
			[openPanel setCanChooseDirectories:YES];
		}
	}
	if (title && savePanel)
		[savePanel setTitle:[NSString stringWithCString: title encoding:NSUTF8StringEncoding]];
	if (openPanel)
	{
		#if MAC_COCOA
		if (parentWindow)
		{
			id fsdelegate = [[fileSelectorDelegateClass alloc] performSelector:@selector(initWithFileSelector:) withObject: (id)this];
			[openPanel beginSheetForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding] : nil file:nil types:typesArray modalForWindow:parentWindow modalDelegate:fsdelegate didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
		}
		else
		#endif
		{
			int res = [openPanel runModalForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding] : nil file:nil types:typesArray];
			openPanelDidEnd (openPanel, res);
		}
	}
	else if (savePanel)
	{
		#if MAC_COCOA
		if (parentWindow)
		{
			id fsdelegate = [[fileSelectorDelegateClass alloc] performSelector:@selector(initWithFileSelector:) withObject: (id)this];
			[savePanel beginSheetForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding] : nil file:defaultSaveName ? [NSString stringWithCString:defaultSaveName encoding:NSUTF8StringEncoding] : nil modalForWindow:parentWindow modalDelegate:fsdelegate didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
		}
		else
		#endif
		{
			int res = [savePanel runModalForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding]:nil file:defaultSaveName ? [NSString stringWithCString:defaultSaveName encoding:NSUTF8StringEncoding] : nil];
			openPanelDidEnd (savePanel, res);
		}
	}
	
	forget ();
	return true;
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::runModalInternal ()
{
	return runInternal (0);
}

#if MAC_COCOA
//-----------------------------------------------------------------------------
id VSTGUI_FileSelector_Delegate_Init (id self, SEL _cmd, void* fileSelector)
{
	OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(init)); // self = [super init];
	if (self)
	{
		((CocoaFileSelector*)fileSelector)->remember ();
		OBJC_SET_VALUE (self, _fileSelector, (id)fileSelector);
	}
	return self;
}

//-----------------------------------------------------------------------------
void VSTGUI_FileSelector_Delegate_Dealloc (id self, SEL _cmd)
{
	id fileSelector = OBJC_GET_VALUE(self, _fileSelector);
	if (fileSelector)
		((CocoaFileSelector*)fileSelector)->forget ();
	OBJC_SUPER(self)
	objc_msgSendSuper (SUPER, @selector(dealloc)); // [super dealloc];
}

//-----------------------------------------------------------------------------
void VSTGUI_FileSelector_Delegate_OpenPanelDidEnd (id self, SEL _cmd, NSOpenPanel* openPanel, int returnCode, void* contextInfo)
{
	id fileSelector = OBJC_GET_VALUE(self, _fileSelector);
	if (fileSelector)
	{
		((CocoaFileSelector*)fileSelector)->openPanelDidEnd (openPanel, returnCode);
	}
	[self autorelease];
}
#endif
/// \endcond

