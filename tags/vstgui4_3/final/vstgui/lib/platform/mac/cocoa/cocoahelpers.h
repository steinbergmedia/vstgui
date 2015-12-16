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

#ifndef __cocoahelpers__
#define __cocoahelpers__

#import "../../../crect.h"
#import "../../../cpoint.h"
#import "../../../ccolor.h"

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
#define SUPER	&__os
#define OBJC_GET_VALUE(x,y) get_Objc_Value (x, #y)
#define OBJC_SET_VALUE(x,y,z) set_Objc_Value (x, #y, (id)z)

extern HIDDEN Class generateUniqueClass (NSMutableString* className, Class baseClass);
extern HIDDEN VstKeyCode CreateVstKeyCodeFromNSEvent (NSEvent* theEvent);
extern HIDDEN NSString* GetVirtualKeyCodeString (int32_t virtualKeyCode);
extern HIDDEN int32_t eventButton (NSEvent* theEvent);
extern HIDDEN void convertPointToGlobal (NSView* view, NSPoint& p);

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
	return [NSColor colorWithDeviceRed:color.red/255. green:color.green/255. blue:color.blue/255. alpha:color.alpha/255.];
}

//------------------------------------------------------------------------------------
HIDDEN inline NSImage* imageFromCGImageRef (CGImageRef image)
{
	#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
	return [[NSImage alloc] initWithCGImage:image size:NSZeroSize];

	#else
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
	#endif
}

#endif // MAC_COCOA
#endif // __cocoahelpers__
