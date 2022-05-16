// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "nsviewframe.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "objcclassbuilder.h"
#import "cocoatextedit.h"
#import "nsviewoptionmenu.h"
#import "cocoaopenglview.h"
#import "autoreleasepool.h"
#import "../macclipboard.h"
#import "../macfactory.h"
#import "../cgdrawcontext.h"
#import "../cgbitmap.h"
#import "../quartzgraphicspath.h"
#import "../caviewlayer.h"
#import "../../../cvstguitimer.h"
#import "../../common/genericoptionmenu.h"
#import "../../../cframe.h"
#import "../../../events.h"

#include <QuartzCore/QuartzCore.h>

#import <iostream>

#ifndef MAC_OS_X_VERSION_10_14
#define MAC_OS_X_VERSION_10_14      101400
#endif

using namespace VSTGUI;

#if DEBUG
//------------------------------------------------------------------------
struct VSTGUI_DebugRedrawAnimDelegate
{
	static id alloc () { return [instance ().objcClass alloc]; }

private:
	static constexpr const auto layerVarName = "layer";

	Class objcClass {nullptr};

	VSTGUI_DebugRedrawAnimDelegate ()
	{
		objcClass = ObjCClassBuilder ()
						.init ("VSTGUI_DebugRedrawAnimDelegate", [NSObject class])
						.addProtocol ("CAAnimationDelegate")
						.addMethod (@selector (dealloc), dealloc)
						.addMethod (@selector (animationDidStop:finished:), animationDidStop)
						.addMethod (@selector (setLayer:), setLayer)
						.addIvar<CALayer*> (layerVarName)
						.finalize ();
	}
	~VSTGUI_DebugRedrawAnimDelegate () { objc_disposeClassPair (objcClass); }

	static VSTGUI_DebugRedrawAnimDelegate& instance ()
	{
		static VSTGUI_DebugRedrawAnimDelegate gInstance;
		return gInstance;
	}

	static Ivar getVar (id self, const char* name)
	{
		return class_getInstanceVariable ([self class], name);
	}

	static CALayer* getLayer (id self)
	{
		if (auto ivar = getVar (self, layerVarName))
			if (id oldValue = object_getIvar (self, ivar))
				return oldValue;
		return nullptr;
	}

	static void dealloc (id self, SEL _cmd) { setLayer (self, @selector (setLayer:), nullptr); }

	static void animationDidStop (id self, SEL _cmd, CAAnimation* anim, BOOL flag)
	{
		if (flag)
		{
			if (auto layer = getLayer (self))
			{
				[layer removeFromSuperlayer];
				setLayer (self, @selector (setLayer:), nullptr);
			}
		}
	}

	static void setLayer (id self, SEL _cmd, CALayer* layer)
	{
		if (auto ivar = getVar (self, layerVarName))
		{
			if (id oldValue = object_getIvar (self, ivar))
				[oldValue release];
			if (layer)
				[layer retain];
			object_setIvar (self, ivar, layer);
		}
	}
};

#endif // DEBUG

//------------------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSView)
- (id)initWithNSViewFrame:(NSViewFrame*)frame parent:(NSView*)parent andSize:(const CRect*)size;
- (BOOL)onMouseDown:(NSEvent*)event;
- (BOOL)onMouseUp:(NSEvent*)event;
- (BOOL)onMouseMoved:(NSEvent*)event;
@end

//------------------------------------------------------------------------------------
static void mapModifiers (NSUInteger nsEventModifiers, CButtonState& buttonState)
{
	if (nsEventModifiers & MacEventModifier::ShiftKeyMask)
		buttonState |= kShift;
	if (nsEventModifiers & MacEventModifier::CommandKeyMask)
		buttonState |= kControl;
	if (nsEventModifiers & MacEventModifier::AlternateKeyMask)
		buttonState |= kAlt;
	if (nsEventModifiers & MacEventModifier::ControlKeyMask)
		buttonState |= kApple;
}

//------------------------------------------------------------------------------------
static Modifiers modifiersFromModifierFlags (NSUInteger nsEventModifiers)
{
	Modifiers mods;
	if (nsEventModifiers & MacEventModifier::ShiftKeyMask)
		mods.add (ModifierKey::Shift);
	if (nsEventModifiers & MacEventModifier::CommandKeyMask)
		mods.add (ModifierKey::Control);
	if (nsEventModifiers & MacEventModifier::AlternateKeyMask)
		mods.add (ModifierKey::Alt);
	if (nsEventModifiers & MacEventModifier::ControlKeyMask)
		mods.add (ModifierKey::Super);
	return mods;
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
static NSPoint getGlobalMouseLocation (NSView* view)
{
	NSRect r = {};
	r.origin = [NSEvent mouseLocation];
	r = [[view window] convertRectFromScreen:r];
	return [view convertPoint:r.origin fromView:nil];
}

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
static auto VSTGUI_NSPasteboardTypeFileURL = NSPasteboardTypeFileURL;
#else
static auto VSTGUI_NSPasteboardTypeFileURL = NSFilenamesPboardType;
#endif

//------------------------------------------------------------------------------------
struct VSTGUI_NSView
{
	static id alloc () { return [instance ().viewClass alloc]; }

private:
	static constexpr const auto nsViewFrameVarName = "_nsViewFrame";

	Class viewClass {nullptr};

	//------------------------------------------------------------------------------------
	VSTGUI_NSView ()
	{
		ObjCClassBuilder builder;
		builder.init ("VSTGUI_NSView", [NSView class])
			.addMethod (@selector (initWithNSViewFrame:parent:andSize:), init)
			//		.addMethod (@selector(dealloc), IMP (Dealloc)))
			.addMethod (@selector (updateTrackingAreas), updateTrackingAreas)
			.addMethod (@selector (viewDidMoveToWindow), viewDidMoveToWindow)
			.addMethod (@selector (windowKeyStateChanged:), windowKeyStateChanged)
			.addMethod (@selector (windowDidChangeBackingProperties:),
						windowDidChangeBackingProperties)
			.addMethod (@selector (isFlipped), isFlipped)
			.addMethod (@selector (acceptsFirstResponder), acceptsFirstResponder)
			.addMethod (@selector (becomeFirstResponder), becomeFirstResponder)
			.addMethod (@selector (resignFirstResponder), resignFirstResponder)
			.addMethod (@selector (canBecomeKeyView), canBecomeKeyView)
			.addMethod (@selector (wantsDefaultClipping), wantsDefaultClipping)
			.addMethod (@selector (isOpaque), isOpaque)
			.addMethod (@selector (drawRect:), drawRect)
			.addMethod (@selector (setNeedsDisplayInRect:), setNeedsDisplayInRect)
			.addMethod (@selector (viewWillDraw), viewWillDraw)
			.addMethod (@selector (shouldBeTreatedAsInkEvent:), shouldBeTreatedAsInkEvent)
			.addMethod (@selector (onMouseDown:), onMouseDown)
			.addMethod (@selector (onMouseUp:), onMouseUp)
			.addMethod (@selector (onMouseMoved:), onMouseMoved)
			.addMethod (@selector (mouseDown:), mouseDown)
			.addMethod (@selector (rightMouseDown:), mouseDown)
			.addMethod (@selector (otherMouseDown:), mouseDown)
			.addMethod (@selector (mouseUp:), mouseUp)
			.addMethod (@selector (rightMouseUp:), mouseUp)
			.addMethod (@selector (otherMouseUp:), mouseUp)
			.addMethod (@selector (mouseMoved:), mouseMoved)
			.addMethod (@selector (mouseDragged:), mouseMoved)
			.addMethod (@selector (rightMouseDragged:), mouseMoved)
			.addMethod (@selector (otherMouseDragged:), mouseMoved)
			.addMethod (@selector (scrollWheel:), scrollWheel)
			.addMethod (@selector (mouseEntered:), mouseEntered)
			.addMethod (@selector (mouseExited:), mouseExited)
			.addMethod (@selector (cursorUpdate:), cursorUpdate)
			.addMethod (@selector (acceptsFirstMouse:), acceptsFirstMouse)
			.addMethod (@selector (performKeyEquivalent:), performKeyEquivalent)
			.addMethod (@selector (keyDown:), keyDown)
			.addMethod (@selector (keyUp:), keyUp)
			.addMethod (@selector (magnifyWithEvent:), magnifyWithEvent)
			.addMethod (@selector (focusRingType), focusRingType)
			.addMethod (@selector (makeSubViewFirstResponder:), makeSubViewFirstResponder)
			.addMethod (@selector (draggingEntered:), draggingEntered)
			.addMethod (@selector (draggingUpdated:), draggingUpdated)
			.addMethod (@selector (draggingExited:), draggingExited)
			.addMethod (@selector (performDragOperation:), performDragOperation);

		if (auto nsDraggingSourceProtocol = objc_getProtocol ("NSDraggingSource"))
		{
			builder.addProtocol (nsDraggingSourceProtocol)
				.addMethod (@selector (draggingSession:sourceOperationMaskForDraggingContext:),
							draggingSessionSourceOperationMaskForDraggingContext)
				.addMethod (@selector (draggingSession:willBeginAtPoint:),
							draggingSessionWillBeginAtPoint)
				.addMethod (@selector (draggingSession:movedToPoint:), draggingSessionMovedToPoint)
				.addMethod (@selector (draggingSession:endedAtPoint:operation:),
							draggingSessionEndedAtPoint);
		}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
		builder.addMethod (@selector (draggedImage:endedAt:operation:),
						   draggedImageEndedAtOperation);
#endif

		// optional touchbar support
		if (auto protocol = objc_getProtocol ("NSTouchBarProvider"))
		{
			builder.addProtocol (protocol).addMethod (@selector (makeTouchBar), makeTouchbar);
		}

		builder.addIvar<void*> ("_nsViewFrame");

		viewClass = builder.finalize ();
	}

	//------------------------------------------------------------------------------------
	~VSTGUI_NSView () { objc_disposeClassPair (viewClass); }

	//------------------------------------------------------------------------------------
	static VSTGUI_NSView& instance ()
	{
		static VSTGUI_NSView gInstance;
		return gInstance;
	}

	//------------------------------------------------------------------------------------
	static NSViewFrame* getNSViewFrame (id obj)
	{
		if (auto var = ObjCInstance (obj).getVariable<NSViewFrame*> (nsViewFrameVarName))
			return var->get ();
		return nullptr;
	}

	//------------------------------------------------------------------------------------
	static IPlatformFrameCallback* getFrame (id obj)
	{
		if (auto nsViewFrame = getNSViewFrame (obj))
			return nsViewFrame->getFrame ();
		return nullptr;
	}

	//------------------------------------------------------------------------------------
	static id init (id self, SEL _cmd, void* _frame, NSView* parentView, const void* _size)
	{
		ObjCInstance obj (self);

		const CRect* size = (const CRect*)_size;
		NSViewFrame* frame = (NSViewFrame*)_frame;
		NSRect nsSize = nsRectFromCRect (*size);

		self = obj.callSuper<id (id, SEL, NSRect), id> (@selector (initWithFrame:), nsSize);
		if (self)
		{
			if (auto var = obj.getVariable<NSViewFrame*> (nsViewFrameVarName))
				var->set (frame);

			[parentView addSubview:self];

			[self
				registerForDraggedTypes:
					[NSArray arrayWithObjects:NSPasteboardTypeString,
											  VSTGUI_NSPasteboardTypeFileURL, NSPasteboardTypeColor,
											  [NSString stringWithCString:
															MacClipboard::getPasteboardBinaryType ()
																 encoding:NSASCIIStringEncoding],
											  nil]];

			[self setFocusRingType:NSFocusRingTypeNone];
		}
		return self;
	}

	//------------------------------------------------------------------------------------
	static BOOL isFlipped (id self, SEL _cmd) { return YES; }
	static BOOL acceptsFirstResponder (id self, SEL _cmd) { return YES; }
	static BOOL canBecomeKeyView (id self, SEL _cmd) { return YES; }
	static BOOL wantsDefaultClipping (id self, SEL _cmd) { return NO; }
	static NSFocusRingType focusRingType (id self) { return NSFocusRingTypeNone; }
	static BOOL shouldBeTreatedAsInkEvent (id self, SEL _cmd, NSEvent* event) { return NO; }

	//------------------------------------------------------------------------------------
	static void makeSubViewFirstResponder (id self, SEL _cmd, NSResponder* newFirstResponder)
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
	static BOOL becomeFirstResponder (id self, SEL _cmd)
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
	static BOOL resignFirstResponder (id self, SEL _cmd)
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
	static void updateTrackingAreas (id self, SEL _cmd)
	{
		NSViewFrame* viewFrame = getNSViewFrame (self);
		if (viewFrame)
			viewFrame->initTrackingArea ();

		ObjCInstance (self).callSuper<void (id, SEL)> (_cmd);
	}

	//------------------------------------------------------------------------------------
	static void windowDidChangeBackingProperties (id self, SEL _cmd, NSNotification* notification)
	{
		double scaleFactor = 1.;
		if (auto window = [self window])
			scaleFactor = [window backingScaleFactor];

		NSViewFrame* viewFrame = getNSViewFrame (self);
		if (viewFrame)
			viewFrame->scaleFactorChanged (scaleFactor);
	}

	//------------------------------------------------------------------------------------
	static void viewDidMoveToWindow (id self, SEL _cmd)
	{
		[[NSNotificationCenter defaultCenter] removeObserver:self];
		NSWindow* window = [self window];
		if (window)
		{
			[[NSNotificationCenter defaultCenter] addObserver:self
													 selector:@selector (windowKeyStateChanged:)
														 name:NSWindowDidBecomeKeyNotification
													   object:window];
			[[NSNotificationCenter defaultCenter] addObserver:self
													 selector:@selector (windowKeyStateChanged:)
														 name:NSWindowDidResignKeyNotification
													   object:window];
			[[NSNotificationCenter defaultCenter]
				addObserver:self
				   selector:@selector (windowDidChangeBackingProperties:)
					   name:NSWindowDidChangeBackingPropertiesNotification
					 object:window];
			IPlatformFrameCallback* frame = getFrame (self);
			if (frame)
				frame->platformOnActivate ([window isKeyWindow] ? true : false);
			windowDidChangeBackingProperties (self, _cmd, nil);
		}
	}

	//------------------------------------------------------------------------------------
	static void windowKeyStateChanged (id self, SEL _cmd, NSNotification* notification)
	{
		IPlatformFrameCallback* frame = getFrame (self);
		auto active =
			[[notification name] isEqualToString:NSWindowDidBecomeKeyNotification] ? true : false;
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
	static BOOL isOpaque (id self, SEL _cmd) { return NO; }

	//------------------------------------------------------------------------------------
	static void drawRect (id self, SEL _cmd, NSRect rect)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
			frame->drawRect (&rect);
	}

	//------------------------------------------------------------------------
	static void viewWillDraw (id self, SEL _cmd)
	{
		if (@available (macOS 10.12, *))
		{
			if (auto layer = [self layer])
			{
				layer.contentsFormat = kCAContentsFormatRGBA8Uint;
			}
		}
		ObjCInstance (self).callSuper<void (id, SEL)> (_cmd);
	}

	//------------------------------------------------------------------------
	static void setNeedsDisplayInRect (id self, SEL _cmd, NSRect rect)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
			frame->setNeedsDisplayInRect (rect);
		ObjCInstance (self).callSuper<void (id, SEL, NSRect)> (_cmd, rect);
	}

	//------------------------------------------------------------------------------------
	static BOOL onMouseDown (id self, SEL _cmd, NSEvent* theEvent)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
			return frame->onMouseDown (theEvent) ? YES : NO;
		return NO;
	}

	//------------------------------------------------------------------------------------
	static BOOL onMouseUp (id self, SEL _cmd, NSEvent* theEvent)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
			return frame->onMouseUp (theEvent) ? YES : NO;
		return NO;
	}

	//------------------------------------------------------------------------------------
	static BOOL onMouseMoved (id self, SEL _cmd, NSEvent* theEvent)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
			return frame->onMouseMoved (theEvent) ? YES : NO;
		return NO;
	}

	//------------------------------------------------------------------------------------
	static void mouseDown (id self, SEL _cmd, NSEvent* theEvent)
	{
		if (![self onMouseDown:theEvent])
		{
			ObjCInstance (self).callSuper<void (id, SEL, NSEvent*)> (_cmd, theEvent);
		}
	}

	//------------------------------------------------------------------------------------
	static void mouseUp (id self, SEL _cmd, NSEvent* theEvent)
	{
		if (![self onMouseUp:theEvent])
		{
			ObjCInstance (self).callSuper<void (id, SEL, NSEvent*)> (_cmd, theEvent);
		}
	}

	//------------------------------------------------------------------------------------
	static void mouseMoved (id self, SEL _cmd, NSEvent* theEvent)
	{
		if (![self onMouseMoved:theEvent])
		{
			ObjCInstance (self).callSuper<void (id, SEL, NSEvent*)> (_cmd, theEvent);
		}
	}

	//------------------------------------------------------------------------------------
	static void scrollWheel (id self, SEL _cmd, NSEvent* theEvent)
	{
		IPlatformFrameCallback* _vstguiframe = getFrame (self);
		if (!_vstguiframe)
			return;

		auto distanceX = [theEvent scrollingDeltaX];
		auto distanceY = [theEvent scrollingDeltaY];
		if (std::abs (distanceX) == 0. && std::abs (distanceY) == 0.)
			return;

		MouseWheelEvent event;
		event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);

		NSPoint nsPoint = [theEvent locationInWindow];
		nsPoint = [self convertPoint:nsPoint fromView:nil];

		if ([theEvent hasPreciseScrollingDeltas])
		{
			distanceX *= 0.1;
			distanceY *= 0.1;
			event.flags |= MouseWheelEvent::PreciseDeltas;
		}
		if ([theEvent isDirectionInvertedFromDevice])
		{
			distanceX *= -1;
			distanceY *= -1;
			event.flags |= MouseWheelEvent::DirectionInvertedFromDevice;
		}

		event.mousePosition = pointFromNSPoint (nsPoint);
		event.modifiers = modifiersFromModifierFlags ([theEvent modifierFlags]);
		event.deltaX = distanceX;
		event.deltaY = distanceY;

		_vstguiframe->platformOnEvent (event);
	}

	//------------------------------------------------------------------------------------
	static void mouseEntered (id self, SEL _cmd, NSEvent* theEvent)
	{
		IPlatformFrameCallback* _vstguiframe = getFrame (self);
		if (!_vstguiframe)
			return;

		MouseMoveEvent event;
		event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
		event.modifiers = modifiersFromModifierFlags (theEvent.modifierFlags);
		event.mousePosition = pointFromNSPoint (getGlobalMouseLocation (self));

		_vstguiframe->platformOnEvent (event);
	}

	//------------------------------------------------------------------------------------
	static void mouseExited (id self, SEL _cmd, NSEvent* theEvent)
	{
		IPlatformFrameCallback* _vstguiframe = getFrame (self);
		if (!_vstguiframe)
			return;
		MouseExitEvent event;
		event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
		event.modifiers = modifiersFromModifierFlags (theEvent.modifierFlags);
		event.mousePosition = pointFromNSPoint (getGlobalMouseLocation (self));
		_vstguiframe->platformOnEvent (event);
	}

	//------------------------------------------------------------------------------------
	static void cursorUpdate (id self, SEL _cmd, NSEvent* theEvent)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
		{
			frame->cursorUpdate ();
		}
	}

	//------------------------------------------------------------------------------------
	static BOOL acceptsFirstMouse (id self, SEL _cmd, NSEvent* event)
	{
		return YES; // click through
	}

	//------------------------------------------------------------------------------------
	static BOOL performKeyEquivalent (id self, SEL _cmd, NSEvent* theEvent)
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
				if (auto _vstguiframe = getFrame (self))
				{
					KeyboardEvent keyEvent;
					keyEvent.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
					if (CreateKeyboardEventFromNSEvent (theEvent, keyEvent))
					{
						_vstguiframe->platformOnEvent (keyEvent);
						return keyEvent.consumed ? YES : NO;
					}
				}
			}
		}
		return NO;
	}

	//------------------------------------------------------------------------------------
	static void keyDown (id self, SEL _cmd, NSEvent* theEvent)
	{
		IPlatformFrameCallback* _vstguiframe = getFrame (self);
		if (!_vstguiframe)
			return;

		KeyboardEvent keyEvent;
		keyEvent.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
		if (CreateKeyboardEventFromNSEvent (theEvent, keyEvent))
		{
			_vstguiframe->platformOnEvent (keyEvent);
			if (keyEvent.consumed)
				return;
			if (keyEvent.virt == VirtualKey::Tab)
			{
				if (keyEvent.modifiers.has (ModifierKey::Shift))
					[[self window] selectKeyViewPrecedingView:self];
				else
					[[self window] selectKeyViewFollowingView:self];
				return;
			}
		}
		[[self nextResponder] keyDown:theEvent];
	}

	//------------------------------------------------------------------------------------
	static void keyUp (id self, SEL _cmd, NSEvent* theEvent)
	{
		IPlatformFrameCallback* _vstguiframe = getFrame (self);
		if (!_vstguiframe)
			return;

		KeyboardEvent keyEvent;
		keyEvent.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
		if (CreateKeyboardEventFromNSEvent (theEvent, keyEvent))
		{
			_vstguiframe->platformOnEvent (keyEvent);
			if (keyEvent.consumed)
				return;
		}
		[[self nextResponder] keyUp:theEvent];
	}

	//------------------------------------------------------------------------------------
	static void magnifyWithEvent (id self, SEL _cmd, NSEvent* theEvent)
	{
		IPlatformFrameCallback* _vstguiframe = getFrame (self);
		if (!_vstguiframe)
			return;

		NSPoint nsPoint = [theEvent locationInWindow];
		nsPoint = [self convertPoint:nsPoint fromView:nil];

		ZoomGestureEvent event;
		event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
		switch (theEvent.phase)
		{
			case NSEventPhaseBegan:
				event.phase = ZoomGestureEvent::Phase::Begin;
				break;
			case NSEventPhaseChanged:
				event.phase = ZoomGestureEvent::Phase::Changed;
				break;
			case NSEventPhaseCancelled:
				[[fallthrough]];
			case NSEventPhaseEnded:
				event.phase = ZoomGestureEvent::Phase::End;
				break;
			default:
				return;
		}

		event.mousePosition = pointFromNSPoint (nsPoint);
		event.modifiers = modifiersFromModifierFlags (theEvent.modifierFlags);
		event.zoom = theEvent.magnification;

		_vstguiframe->platformOnEvent (event);
	}

	//------------------------------------------------------------------------------------
	static NSDragOperation draggingEntered (id self, SEL _cmd, id sender)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame)
			return NSDragOperationNone;

		NSPasteboard* pboard = [sender draggingPasteboard];

		frame->setDragDataPackage (MacClipboard::createDragDataPackage (pboard));

		CPoint where;
		nsViewGetCurrentMouseLocation (self, where);
		auto modifiers = modifiersFromModifierFlags (NSEvent.modifierFlags);

		DragEventData data {frame->getDragDataPackage (), where, modifiers};
		auto result = frame->getFrame ()->platformOnDragEnter (data);
		if (result == DragOperation::Copy)
			return NSDragOperationCopy;
		if (result == DragOperation::Move)
			return NSDragOperationMove;

		return NSDragOperationGeneric;
	}

	//------------------------------------------------------------------------------------
	static NSDragOperation draggingUpdated (id self, SEL _cmd, id sender)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame)
			return NSDragOperationNone;

		CPoint where;
		nsViewGetCurrentMouseLocation (self, where);
		auto modifiers = modifiersFromModifierFlags (NSEvent.modifierFlags);

		DragEventData data {frame->getDragDataPackage (), where, modifiers};
		auto result = frame->getFrame ()->platformOnDragMove (data);
		if (result == DragOperation::Copy)
			return NSDragOperationCopy;
		if (result == DragOperation::Move)
			return NSDragOperationMove;

		return NSDragOperationNone;
	}

	//------------------------------------------------------------------------------------
	static void draggingExited (id self, SEL _cmd, id sender)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame || !frame->getDragDataPackage ())
			return;

		CPoint where;
		nsViewGetCurrentMouseLocation (self, where);
		auto modifiers = modifiersFromModifierFlags (NSEvent.modifierFlags);

		DragEventData data {frame->getDragDataPackage (), where, modifiers};
		frame->getFrame ()->platformOnDragLeave (data);
		frame->setDragDataPackage (nullptr);

		// we may should remember the cursor via [NSCursor currentCursor]
		[[NSCursor arrowCursor] set];
	}

	//------------------------------------------------------------------------------------
	static BOOL performDragOperation (id self, SEL _cmd, id sender)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame || !frame->getDragDataPackage ())
			return NO;

		CPoint where;
		nsViewGetCurrentMouseLocation (self, where);
		auto modifiers = modifiersFromModifierFlags (NSEvent.modifierFlags);

		DragEventData data {frame->getDragDataPackage (), where, modifiers};
		bool result = frame->getFrame ()->platformOnDrop (data);
		frame->setMouseCursor (kCursorDefault);
		frame->setDragDataPackage (nullptr);
		return result;
	}

	//------------------------------------------------------------------------------------
	static NSDragOperation draggingSessionSourceOperationMaskForDraggingContext (
		id self, SEL _cmd, NSDraggingSession* session, NSDraggingContext context)
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

	//------------------------------------------------------------------------------------
	static void draggingSessionWillBeginAtPoint (id self, SEL _cmd, NSDraggingSession* session,
												 NSPoint position)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame)
			return;
		if (auto dragSession = frame->getDraggingSession ())
		{
			auto r = [[self window] convertRectFromScreen: {position, NSMakeSize (0, 0)}];
			auto pos = pointFromNSPoint ([self convertPoint:r.origin fromView:nil]);
			dragSession->dragWillBegin (pos);
		}
	}

	//------------------------------------------------------------------------------------
	static void draggingSessionMovedToPoint (id self, SEL _cmd, NSDraggingSession* session,
											 NSPoint position)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame)
			return;
		if (auto dragSession = frame->getDraggingSession ())
		{
			auto r = [[self window] convertRectFromScreen: {position, NSMakeSize (0, 0)}];
			auto pos = pointFromNSPoint ([self convertPoint:r.origin fromView:nil]);
			dragSession->dragMoved (pos);
		}
	}

	//------------------------------------------------------------------------------------
	static void draggingSessionEndedAtPoint (id self, SEL _cmd, NSDraggingSession* session,
											 NSPoint position, NSDragOperation operation)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (!frame)
			return;
		if (auto dragSession = frame->getDraggingSession ())
		{
			DragOperation result;
			switch (operation)
			{
				case NSDragOperationNone:
					result = DragOperation::None;
					break;
				case NSDragOperationMove:
					result = DragOperation::Move;
					break;
				default:
					result = DragOperation::Copy;
					break;
			}
			auto r = [[self window] convertRectFromScreen: {position, NSMakeSize (0, 0)}];
			auto pos = pointFromNSPoint ([self convertPoint:r.origin fromView:nil]);
			dragSession->dragEnded (pos, result);
			frame->clearDraggingSession ();
		}
	}

	//------------------------------------------------------------------------------------
	static id makeTouchbar (id self)
	{
		NSViewFrame* frame = getNSViewFrame (self);
		if (frame)
			return reinterpret_cast<id> (frame->makeTouchBar ());
		return nil;
	}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	//------------------------------------------------------------------------------------
	static void draggedImageEndedAtOperation (id self, SEL _cmd, NSImage* image, NSPoint aPoint,
											  NSDragOperation operation)
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
}; // VSTGUI_NSView

//------------------------------------------------------------------------------------
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

	nsView = [VSTGUI_NSView::alloc () initWithNSViewFrame:this parent:parent andSize:&size];

	if (cocoaConfig && cocoaConfig->flags & CocoaFrameConfig::kNoCALayer)
		return;

	auto processInfo = [NSProcessInfo processInfo];
	if ([processInfo respondsToSelector:@selector(operatingSystemVersion)])
	{
		// on Mac OS X 10.11 we activate layer drawing as this fixes a few issues like that only a
		// few parts of a window are updated permanently when scrolling or manipulating a control
		// while other parts are only updated when the malipulation ended, or CNinePartTiledBitmap
		// are drawn incorrectly when scaled.
		if (@available (macOS 10.11, *))
		{
			[nsView setWantsLayer:YES];
			if (@available (macOS 10.13, *))
			{
				nsView.layer.contentsFormat = kCAContentsFormatRGBA8Uint;
				// asynchronous layer drawing or drawing only dirty rectangles are exclusive as
				// the CoreGraphics engineers decided to be clever and join dirty rectangles without
				// letting us know
				if (getPlatformFactory ().asMacFactory ()->getUseAsynchronousLayerDrawing ())
					nsView.layer.drawsAsynchronously = YES;
				else
					useInvalidRects = true;
			}
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

//------------------------------------------------------------------------------------
void NSViewFrame::scaleFactorChanged (double newScaleFactor)
{
	if (nsView.wantsLayer)
		nsView.layer.contentsScale = newScaleFactor;

	if (frame)
		frame->platformScaleFactorChanged (newScaleFactor);
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

			MouseMoveEvent event;
			event.mousePosition = pointFromNSPoint (p);
			frame->platformOnEvent (event);
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

//------------------------------------------------------------------------
void NSViewFrame::setNeedsDisplayInRect (NSRect r)
{
	if (useInvalidRects)
		invalidRectList.add (rectFromNSRect (r));
}

//-----------------------------------------------------------------------------
void NSViewFrame::addDebugRedrawRect (CRect r, bool isClipBoundingBox)
{
#if DEBUG
	if (getPlatformFactory ().asMacFactory ()->enableVisualizeRedrawAreas () && nsView.layer)
	{
		id delegate = [[VSTGUI_DebugRedrawAnimDelegate::alloc () init] autorelease];
		auto anim = [CABasicAnimation animation];
		anim.fromValue = [NSNumber numberWithDouble:isClipBoundingBox ? 0.2 : 0.8];
		anim.toValue = [NSNumber numberWithDouble:0.];
		anim.keyPath = @"opacity";
		anim.delegate = delegate;
		anim.duration = isClipBoundingBox ? 1. : 1.;

		auto rect = nsRectFromCRect (r);
		for (CALayer* layer in nsView.layer.sublayers)
		{
			if (![layer.name isEqualToString:@"DebugLayer"])
				continue;
			if (CGRectEqualToRect (rect, layer.frame))
			{
				[layer removeAnimationForKey:@"opacity"];
				[layer addAnimation:anim forKey:@"opacity"];
				return;
			}
		}
		auto layer = [[CALayer new] autorelease];
		layer.name = @"DebugLayer";
		layer.backgroundColor = CGColorCreateGenericRGB (isClipBoundingBox ? 0. : 1., 1., 0., 1.);
		layer.opacity = 0.f;
		layer.zPosition = isClipBoundingBox ? 10 : 11;
		layer.frame = nsRectFromCRect (r);
		[nsView.layer addSublayer:layer];

		[delegate setLayer:layer];
		[layer addAnimation:anim forKey:@"opacity"];
	}
#endif
}

//-----------------------------------------------------------------------------
void NSViewFrame::drawRect (NSRect* rect)
{
	inDraw = true;
	NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAX_OS_X_VERSION_10_10
	auto cgContext = static_cast<CGContextRef> ([nsContext graphicsPort]);
#else
	auto cgContext = static_cast<CGContextRef> ([nsContext CGContext]);
#endif

	addDebugRedrawRect (rectFromNSRect (*rect), true);

	CGDrawContext drawContext (cgContext, rectFromNSRect ([nsView bounds]));
	drawContext.beginDraw ();

	if (useInvalidRects)
	{
		joinNearbyInvalidRects (invalidRectList, 24.);
		for (auto r : invalidRectList)
		{
			frame->platformDrawRect (&drawContext, r);
			addDebugRedrawRect (r, false);
		}
		invalidRectList.clear ();
	}
	else
	{
		const NSRect* dirtyRects;
		NSInteger numDirtyRects;
		[nsView getRectsBeingDrawn:&dirtyRects count:&numDirtyRects];
		for (NSInteger i = 0; i < numDirtyRects; i++)
		{
			auto r = rectFromNSRect (dirtyRects[i]);
			frame->platformDrawRect (&drawContext, r);
			addDebugRedrawRect (r, false);
		}
	}
	drawContext.endDraw ();
	inDraw = false;
}

//------------------------------------------------------------------------
static MouseEventButtonState buttonStateFromNSEvent (NSEvent* theEvent)
{
	MouseEventButtonState state;
	if (theEvent.type == MacEventType::MouseMoved)
		return state;
	switch (theEvent.buttonNumber)
	{
		case 0:
		{
			if (theEvent.modifierFlags & MacEventModifier::ControlKeyMask)
				state.add (MouseButton::Right);
			else
				state.add (MouseButton::Left);
			break;
		}
		case 1:
			state.add (MouseButton::Right);
			break;
		case 2:
			state.add (MouseButton::Middle);
			break;
		case 3:
			state.add (MouseButton::Fourth);
			break;
		case 4:
			state.add (MouseButton::Fifth);
			break;
	}
	return state;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::onMouseDown (NSEvent* theEvent)
{
	MouseDownEvent event;
	event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
	event.buttonState = buttonStateFromNSEvent (theEvent);
	event.modifiers = modifiersFromModifierFlags (theEvent.modifierFlags);
	event.clickCount = theEvent.clickCount;
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [nsView convertPoint:nsPoint fromView:nil];
	event.mousePosition = pointFromNSPoint (nsPoint);
	frame->platformOnEvent (event);
	return event.consumed;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::onMouseUp (NSEvent* theEvent)
{
	MouseUpEvent event;
	event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
	event.buttonState = buttonStateFromNSEvent (theEvent);
	event.modifiers = modifiersFromModifierFlags (theEvent.modifierFlags);
	event.clickCount = theEvent.clickCount;
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [nsView convertPoint:nsPoint fromView:nil];
	event.mousePosition = pointFromNSPoint (nsPoint);
	frame->platformOnEvent (event);
	return event.consumed;
}

//-----------------------------------------------------------------------------
bool NSViewFrame::onMouseMoved (NSEvent* theEvent)
{
	MouseMoveEvent event;
	event.timestamp = static_cast<uint64_t> (theEvent.timestamp * 1000.);
	event.buttonState = buttonStateFromNSEvent (theEvent);
	event.modifiers = modifiersFromModifierFlags (theEvent.modifierFlags);
	event.clickCount = theEvent.clickCount;
	NSPoint nsPoint = [theEvent locationInWindow];
	nsPoint = [nsView convertPoint:nsPoint fromView:nil];
	event.mousePosition = pointFromNSPoint (nsPoint);
	frame->platformOnEvent (event);
	return event.consumed;
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
		if (mouseButtons == (1 << 0) && modifiers & MacEventModifier::ControlKeyMask)
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
bool NSViewFrame::getCurrentModifiers (Modifiers& modifiers) const
{
	modifiers = modifiersFromModifierFlags (NSEvent.modifierFlags);
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
		case kCursorIBeam: cur = [NSCursor IBeamCursor]; break;
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
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_14
	if (nsView.wantsLayer)
		return false;
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
#else
	return false;
#endif
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
Optional<UTF8String> NSViewFrame::convertCurrentKeyEventToText ()
{
	auto event = [NSApp currentEvent];
	if (!event)
		return {};
	if (!(event.type == MacEventType::KeyDown || event.type == MacEventType::KeyUp))
		return {};
	if (event.characters.length <= 0)
		return {};
	return Optional<UTF8String> (event.characters.UTF8String);
}

//-----------------------------------------------------------------------------
bool NSViewFrame::setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme)
{
	if (!use)
	{
		genericOptionMenuTheme = nullptr;
	}
	else
	{
		if (theme)
			genericOptionMenuTheme = std::unique_ptr<GenericOptionMenuTheme> (new GenericOptionMenuTheme (*theme));
		else
			genericOptionMenuTheme = std::unique_ptr<GenericOptionMenuTheme> (new GenericOptionMenuTheme);
	}
	return true;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformTextEdit> NSViewFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return makeOwned<CocoaTextEdit> (nsView, textEdit);
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> NSViewFrame::createPlatformOptionMenu ()
{
	if (genericOptionMenuTheme)
	{
		MouseEventButtonState buttonState;
		if (auto event = [NSApp currentEvent])
			buttonState = buttonStateFromNSEvent (event);
		return makeOwned<GenericOptionMenu> (dynamic_cast<CFrame*> (frame), buttonState,
		                                     *genericOptionMenuTheme.get ());
	}
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
	if (parentViewLayer == nullptr || parentViewLayer->getCALayer () == nullptr)
	{
		// after this is called, 'Quartz Debug' will not work as before. So when using 'Quartz Debug' comment the following two lines.
		[nsView setWantsLayer:YES];
		nsView.layer.actions = nil;
	}
	auto caParentLayer = parentViewLayer ? parentViewLayer->getCALayer () : [nsView layer];
	auto layer = makeOwned<CAViewLayer> (caParentLayer);
	layer->init (drawDelegate);
	return std::move (layer);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------------------
DragResult NSViewFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

	lastDragOperationResult = kDragError;
	if (nsView)
	{
		NSPoint bitmapOffset = { static_cast<CGFloat>(offset.x), static_cast<CGFloat>(offset.y) };

		NSEvent* event = [NSApp currentEvent];
		if (event == nullptr || !([event type] == MacEventType::LeftMouseDown ||
		                          [event type] == MacEventType::LeftMouseDragged))
			return kDragRefused;
		NSPoint nsLocation = [event locationInWindow];
		NSImage* nsImage = bitmapToNSImage (dragBitmap);
		if (nsImage)
		{
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
	}
	return kDragError;

#pragma clang diagnostic pop

}
#endif

//-----------------------------------------------------------------------------
bool NSViewFrame::doDrag (const DragDescription& dragDescription,
                          const SharedPointer<IDragCallback>& callback)
{
	if (!nsView)
		return false;

	draggingSession = NSViewDraggingSession::create (nsView, dragDescription, callback);
	return draggingSession != nullptr;
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
	NSView* nsView = nsViewFrame->getNSView ();
	if (!window)
	{
		window = [[NSWindow alloc] initWithContentRect:NSMakeRect (0, 0, 10, 10) styleMask:MacWindowStyleMask::Borderless backing:NSBackingStoreBuffered defer:NO];
		[window setReleasedWhenClosed:NO];
		[window setOpaque:NO];
		[window setHasShadow:YES];
		[window setLevel:NSStatusWindowLevel];
		[window setHidesOnDeactivate:YES];
		[window setIgnoresMouseEvents:YES];
		[window setBackgroundColor: [NSColor textBackgroundColor]];
		textfield = [[[NSTextField alloc] initWithFrame:NSMakeRect (2, 2, 8, 8)] autorelease];
		[textfield setEditable:NO];
		[textfield setSelectable:NO];
		[textfield setBezeled:NO];
		[textfield setBordered:NO];
		[textfield setDrawsBackground:NO];
		[window.contentView addSubview:textfield];
	}
	auto paragrapheStyle = [[NSMutableParagraphStyle new] autorelease];
	[paragrapheStyle setParagraphStyle:[NSParagraphStyle defaultParagraphStyle]];
	paragrapheStyle.alignment = NSTextAlignmentCenter;
	auto string = [NSString stringWithCString:tooltip encoding:NSUTF8StringEncoding];
	auto attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont toolTipsFontOfSize:0], NSFontAttributeName, paragrapheStyle, NSParagraphStyleAttributeName, nil];
	auto attrString = [[[NSMutableAttributedString alloc] init] autorelease];
	auto lines = [string componentsSeparatedByString:@"\\n"];
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
	textSize.width += 4;
	textSize.height += 4;

	CPoint p;
	p.x = rect.left;
	p.y = rect.bottom;
	NSPoint nsp = nsPointFromCPoint (p);
	convertPointToGlobal (nsView, nsp);
	nsp.y -= (textSize.height + 4);
	nsp.x += (rect.getWidth () - textSize.width) / 2;

	NSRect frameRect = { nsp, textSize };
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

} // VSTGUI

#endif // MAC_COCOA
