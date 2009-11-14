#ifndef __cocoahelpers__
#define __cocoahelpers__

#import "../../../crect.h"
#import "../../../cpoint.h"
#import "../../../ccolor.h"

#if MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION

#import <objc/runtime.h>
#import <objc/message.h>
#import <Cocoa/Cocoa.h>
struct VstKeyCode;

#define HIDDEN __attribute__((__visibility__("hidden")))

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
#define SUPER	&__os
#define OBJC_GET_VALUE(x,y) get_Objc_Value (x, #y)
#define OBJC_SET_VALUE(x,y,z) set_Objc_Value (x, #y, (id)z)

extern HIDDEN Class generateUniqueClass (NSMutableString* className, Class baseClass);
extern HIDDEN VstKeyCode CreateVstKeyCodeFromNSEvent (NSEvent* theEvent);
extern HIDDEN long eventButton (NSEvent* theEvent);

//------------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------------
HIDDEN inline NSRect nsRectFromCRect (const VSTGUI::CRect& rect)
{
	NSRect r;
	r.origin.x = rect.left;
	r.origin.y = rect.top;
	r.size.width = rect.getWidth ();
	r.size.height = rect.getHeight ();
	return r;
}

//------------------------------------------------------------------------------------
HIDDEN inline NSPoint nsPointFromCPoint (const VSTGUI::CPoint& point)
{
	NSPoint p = { point.x, point.y };
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
	return [NSColor colorWithDeviceRed:color.red/255. green:color.green/255. blue:color.blue/255. alpha:color.alpha/255.];
}

//------------------------------------------------------------------------------------
HIDDEN inline NSImage* imageFromCGImageRef (CGImageRef image)
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

#endif // MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION
#endif // __cocoahelpers__
