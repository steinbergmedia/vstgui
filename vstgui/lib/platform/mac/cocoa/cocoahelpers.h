// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#import "../../../crect.h"
#import "../../../cpoint.h"
#import "../../../ccolor.h"
#import "../../../vstguifwd.h"

#if MAC_COCOA && defined (__OBJC__)

#import <objc/runtime.h>
#import <objc/message.h>
#import <Cocoa/Cocoa.h>
struct VstKeyCode;

#define HIDDEN __attribute__((__visibility__("hidden")))

#if DEBUG
#define VSTGUI_CHECK_YES(x) { BOOL res = x; vstgui_assert (res == YES); }
#else
#define VSTGUI_CHECK_YES(x) x;
#endif

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

#define __OBJC_SUPER(x) objc_super __os; __os.receiver = x; __os.super_class = class_getSuperclass ([x class]);
#define SUPER	static_cast<id> (&__os)
#define OBJC_GET_VALUE(x,y) get_Objc_Value (x, #y)
#define OBJC_SET_VALUE(x,y,z) set_Objc_Value (x, #y, (id)z)

//------------------------------------------------------------------------------------
static id (*SuperInit) (id, SEL) = (id (*) (id, SEL))objc_msgSendSuper;
static id (*SuperInitWithFrame) (id, SEL, NSRect) = (id (*) (id, SEL, NSRect))objc_msgSendSuper;
static void (*SuperDealloc) (id, SEL) = (void (*) (id, SEL))objc_msgSendSuper;
static void (*SuperRemoveFromSuperview) (id, SEL) = SuperDealloc;
static void (*SuperEventMsg) (id, SEL, NSEvent*) = (void (*) (id, SEL, NSEvent*))objc_msgSendSuper;
static void (*SuperUpdateTrackingAreas) (id, SEL) = (void (*) (id, SEL))objc_msgSendSuper;
static void (*SuperTextDidChange) (id, SEL, NSNotification*) = (void (*) (id, SEL, NSNotification*))objc_msgSendSuper;
static void (*SuperSetNeedsDisplayInRect) (id, SEL,
										   NSRect) = (void (*) (id, SEL, NSRect))objc_msgSendSuper;
static void (*SuperViewWillRedraw) (id, SEL) = SuperDealloc;

//------------------------------------------------------------------------------------
extern HIDDEN Class generateUniqueClass (NSMutableString* className, Class baseClass);
extern HIDDEN VstKeyCode CreateVstKeyCodeFromNSEvent (NSEvent* theEvent);
extern HIDDEN NSString* GetVirtualKeyCodeString (int32_t virtualKeyCode);
extern HIDDEN int32_t eventButton (NSEvent* theEvent);
extern HIDDEN void convertPointToGlobal (NSView* view, NSPoint& p);
extern HIDDEN NSImage* bitmapToNSImage (VSTGUI::CBitmap* bitmap);

//------------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------------
HIDDEN inline NSRect nsRectFromCRect (const VSTGUI::CRect& rect)
{
	NSRect r;
	r.origin.x = static_cast<CGFloat> (rect.left);
	r.origin.y = static_cast<CGFloat> (rect.top);
	r.size.width = static_cast<CGFloat> (rect.getWidth ());
	r.size.height = static_cast<CGFloat> (rect.getHeight ());
	return r;
}

//------------------------------------------------------------------------------------
HIDDEN inline NSPoint nsPointFromCPoint (const VSTGUI::CPoint& point)
{
	NSPoint p = { static_cast<CGFloat>(point.x), static_cast<CGFloat>(point.y) };
	return p;
}

//------------------------------------------------------------------------------------
HIDDEN inline VSTGUI::CRect rectFromNSRect (const NSRect& rect)
{
	VSTGUI::CRect r (rect.origin.x, rect.origin.y, 0, 0);
	r.setWidth (rect.size.width);
	r.setHeight (rect.size.height);
	return r;
}

//------------------------------------------------------------------------------------
HIDDEN inline VSTGUI::CPoint pointFromNSPoint (const NSPoint& point)
{
	VSTGUI::CPoint p (point.x, point.y);
	return p;
}

//------------------------------------------------------------------------------------
HIDDEN inline NSColor* nsColorFromCColor (const VSTGUI::CColor& color)
{
	return [NSColor colorWithDeviceRed:color.normRed<CGFloat> ()
	                             green:color.normGreen<CGFloat> ()
	                              blue:color.normBlue<CGFloat> ()
	                             alpha:color.normAlpha<CGFloat> ()];
}

//------------------------------------------------------------------------------------
struct MacEventModifier
{
	enum mask
	{
#ifdef MAC_OS_X_VERSION_10_12
		ShiftKeyMask = NSEventModifierFlagShift,
		CommandKeyMask = NSEventModifierFlagCommand,
		AlternateKeyMask = NSEventModifierFlagOption,
		ControlKeyMask = NSEventModifierFlagControl
#else
		ShiftKeyMask = NSShiftKeyMask,
		CommandKeyMask = NSCommandKeyMask,
		AlternateKeyMask = NSAlternateKeyMask,
		ControlKeyMask = NSControlKeyMask
#endif
	};
};

//------------------------------------------------------------------------------------
namespace MacEventType
{
#ifdef MAC_OS_X_VERSION_10_12
	static constexpr auto LeftMouseDown = ::NSEventTypeLeftMouseDown;
	static constexpr auto LeftMouseDragged = ::NSEventTypeLeftMouseDragged;
	static constexpr auto MouseMoved = ::NSEventTypeMouseMoved;
	static constexpr auto KeyDown = ::NSEventTypeKeyDown;
	static constexpr auto KeyUp = ::NSEventTypeKeyUp;
#else
	static constexpr auto LeftMouseDown = ::NSLeftMouseDown;
	static constexpr auto LeftMouseDragged = ::NSLeftMouseDragged;
	static constexpr auto MouseMoved = ::NSMouseMoved;
	static constexpr auto KeyDown = ::NSKeyDown;
	static constexpr auto KeyUp = ::NSKeyUp;
#endif
}

//------------------------------------------------------------------------------------
namespace MacWindowStyleMask
{
#ifdef MAC_OS_X_VERSION_10_12
	static constexpr auto Borderless = ::NSWindowStyleMaskBorderless;
	static constexpr auto Titled = ::NSWindowStyleMaskTitled;
	static constexpr auto Resizable = ::NSWindowStyleMaskResizable;
	static constexpr auto Miniaturizable = ::NSWindowStyleMaskMiniaturizable;
	static constexpr auto Closable = ::NSWindowStyleMaskClosable;
	static constexpr auto Utility = ::NSWindowStyleMaskUtilityWindow;
	static constexpr auto FullSizeContentView = ::NSWindowStyleMaskFullSizeContentView;
#else
	static constexpr auto Borderless = ::NSBorderlessWindowMask;
	static constexpr auto Titled = ::NSTitledWindowMask;
	static constexpr auto Resizable = ::NSResizableWindowMask;
	static constexpr auto Miniaturizable = ::NSMiniaturizableWindowMask;
	static constexpr auto Closable = ::NSClosableWindowMask;
	static constexpr auto Utility = ::NSUtilityWindowMask;
	static constexpr auto FullSizeContentView = ::NSFullSizeContentViewWindowMask;
#endif
}

#endif // MAC_COCOA
