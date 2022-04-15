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

#define HIDDEN __attribute__((__visibility__("hidden")))

//------------------------------------------------------------------------------------
extern HIDDEN bool CreateKeyboardEventFromNSEvent (NSEvent* theEvent, VSTGUI::KeyboardEvent& event);
extern HIDDEN NSString* GetVirtualKeyCodeString (VSTGUI::VirtualKey virtualKey);
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
