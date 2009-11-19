//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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

#if VSTGUI_LIVE_EDITING

#include "platformsupport.h"
#if VSTGUI_PLATFORM_ABSTRACTION
#include "../lib/platform/mac/cocoa/nsviewframe.h"
#include "../lib/platform/mac/cocoa/cocoahelpers.h"
#include "../lib/platform/mac/cgbitmap.h"
#else
#include "../lib/cocoasupport.h"
#endif
#include <Cocoa/Cocoa.h>

using namespace VSTGUI;

//-----------------------------------------------------------------------------
@interface VSTGUI_CocoaWindowDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
<NSWindowDelegate>
#endif
{
	IPlatformWindowDelegate* delegate;
	PlatformWindow* platformWindow;
}
- (id) initWithDelegate:(IPlatformWindowDelegate*) delegate platformWindow: (PlatformWindow*) platformWindow;
@end

//-----------------------------------------------------------------------------
@interface VSTGUI_CocoaDraggingSource : NSObject {
	BOOL localOnly;
}
- (void) setLocalOnly:(BOOL)flag;
@end

//-----------------------------------------------------------------------------
@interface VSTGUI_ColorChangeCallback : NSObject {
	IPlatformColorChangeCallback* callback;
}
- (void) setCallback: (IPlatformColorChangeCallback*) _callback;
@end
static VSTGUI_ColorChangeCallback* gColorChangeCallback = 0;

//-----------------------------------------------------------------------------
static __attribute__((__destructor__)) void colorChangeCallbackDestructor () 
{
	if (gColorChangeCallback)
		[gColorChangeCallback release];
}

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class CocoaWindow : public PlatformWindow
{
public:
	CocoaWindow (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate);
	~CocoaWindow ();

	void* getPlatformHandle () const { return [window contentView]; }
	void show ();
	void center ();
	CRect getSize ();
	void setSize (const CRect& size);
	void runModal ();
	void stopModal ();
protected:
	NSWindow* window;
	IPlatformWindowDelegate* delegate;
	VSTGUI_CocoaWindowDelegate* cocoaDelegate;
	WindowType type;
	long styleFlags;
};

//-----------------------------------------------------------------------------
PlatformWindow* PlatformWindow::create (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate, void* parentWindow)
{
	return new CocoaWindow (size, title, type, styleFlags, delegate);
}

//-----------------------------------------------------------------------------
CocoaWindow::CocoaWindow (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate)
: window (0)
, delegate (delegate)
, cocoaDelegate (0)
, type (type)
, styleFlags (styleFlags)
{
	NSRect contentRect = NSMakeRect (size.left, size.top, size.getWidth (), size.getHeight ());
	NSUInteger style = NSTitledWindowMask;
	if (styleFlags & kClosable)
		style |= NSClosableWindowMask;
	if (styleFlags & kResizable)
		style |= NSResizableWindowMask;
	if (type == kPanelType)
	{
		window = [[NSPanel alloc] initWithContentRect:contentRect styleMask:style|NSUtilityWindowMask|NSHUDWindowMask backing:NSBackingStoreBuffered defer:YES];
		[(NSPanel*)window setBecomesKeyOnlyIfNeeded:NO];
		[window setMovableByWindowBackground:NO];
	}
	else if (type == kWindowType)
	{
		window = [[NSWindow alloc] initWithContentRect:contentRect styleMask:style backing:NSBackingStoreBuffered defer:YES];
	}
	[window setReleasedWhenClosed:NO];
	if (title)
		[window setTitle:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
	if (delegate)
	{
		cocoaDelegate = [[VSTGUI_CocoaWindowDelegate alloc] initWithDelegate:delegate platformWindow:this];
		[window setDelegate:cocoaDelegate];
	}
}

//-----------------------------------------------------------------------------
CocoaWindow::~CocoaWindow ()
{
	if (cocoaDelegate)
	{
		[window setDelegate:nil];
		[cocoaDelegate release];
	}
	[window orderOut:nil];
	[window release];
}

//-----------------------------------------------------------------------------
void CocoaWindow::show ()
{
	if (type == kPanelType)
		[window orderFront:nil];
	else
		[window makeKeyAndOrderFront:nil];
}

//-----------------------------------------------------------------------------
void CocoaWindow::center ()
{
	[window center];
}

//-----------------------------------------------------------------------------
CRect CocoaWindow::getSize ()
{
	NSRect size = [window contentRectForFrameRect:[window frame]];
	CRect r (size.origin.x, size.origin.y, 0, 0);
	r.setWidth (size.size.width);
	r.setHeight (size.size.height);
	return r;
}

//-----------------------------------------------------------------------------
void CocoaWindow::setSize (const CRect& size)
{
	NSRect r = NSMakeRect (size.left, size.top, size.getWidth (), size.getHeight ());
	r = [window frameRectForContentRect:r];
	[window setFrame:r display:YES];
}

//-----------------------------------------------------------------------------
void CocoaWindow::runModal ()
{
	[NSApp runModalForWindow:window];
}

//-----------------------------------------------------------------------------
void CocoaWindow::stopModal ()
{
	[NSApp abortModal];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool PlatformUtilities::collectPlatformFontNames (std::list<std::string*>& fontNames)
{
	NSArray* fonts = [[NSFontManager sharedFontManager] availableFontFamilies];
	for (NSString* font in fonts)
	{
		fontNames.push_back (new std::string ([font UTF8String]));
	}
	return true;
}

//-----------------------------------------------------------------------------
bool PlatformUtilities::startDrag (CFrame* frame, const CPoint& location, const char* string, CBitmap* dragBitmap, bool localOnly)
{
	CGImageRef cgImage = 0;
#if VSTGUI_PLATFORM_ABSTRACTION
	if (!frame->getPlatformFrame ())
		return false;

	NSViewFrame* nsViewFrame = dynamic_cast<NSViewFrame*> (frame->getPlatformFrame ());
	NSView* nsView = nsViewFrame ? nsViewFrame->getPlatformControl () : 0;
	CGBitmap* cgBitmap = dragBitmap ? dynamic_cast<CGBitmap*> (dragBitmap->getPlatformBitmap ()) : 0;
	cgImage = cgBitmap ? cgBitmap->getCGImage () : 0;
#else
	NSView* nsView = (NSView*)frame->getNSView ();
	CGImageRef cgImage = dragBitmap ? dragBitmap->createCGImage (false) : 0;
#endif
	if (nsView)
	{
		NSPoint bitmapOffset = { location.x, location.y };
		NSPasteboard* nsPasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
		NSImage* nsImage = nil;
		if (cgImage)
		{
			nsImage = [imageFromCGImageRef (cgImage) autorelease];
			bitmapOffset.x -= [nsImage size].width/2;
			bitmapOffset.y += [nsImage size].height/2;
#if !VSTGUI_PLATFORM_ABSTRACTION
			CFRelease (cgImage);
#endif
		}
		else
		{
			nsImage = [[[NSImage alloc] initWithSize:NSMakeSize (2, 2)] autorelease];
		}
		VSTGUI_CocoaDraggingSource* sourceObj = [[VSTGUI_CocoaDraggingSource alloc] init];
		[sourceObj setLocalOnly:localOnly];
		
		[nsPasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:sourceObj];
		[nsPasteboard setString:[NSString stringWithCString:string encoding:NSUTF8StringEncoding] forType:NSStringPboardType];
		
		[nsView dragImage:nsImage at:bitmapOffset offset:NSMakeSize (0, 0) event:[NSApp currentEvent] pasteboard:nsPasteboard source:sourceObj slideBack:YES];
		[sourceObj release];
	}
	return false;
}

//-----------------------------------------------------------------------------
void PlatformUtilities::colorChooser (const CColor* oldColor, IPlatformColorChangeCallback* callback)
{
	if (gColorChangeCallback == 0)
		gColorChangeCallback = [[VSTGUI_ColorChangeCallback alloc] init];
	[gColorChangeCallback setCallback:callback];
	NSColorPanel* colorPanel = [NSColorPanel sharedColorPanel];
	[colorPanel setTarget:nil];
	if (oldColor)
	{
		[colorPanel setShowsAlpha:YES];
		NSColor* nsColor = [NSColor colorWithDeviceRed:(float)oldColor->red/255.f green:(float)oldColor->green/255.f blue:(float)oldColor->blue/255.f alpha:(float)oldColor->alpha/255.f];
		[colorPanel setColor:nsColor];
		[colorPanel setTarget:gColorChangeCallback];
		[colorPanel setAction:@selector(colorChanged:)];
		[colorPanel makeKeyAndOrderFront:nil];
	}
}

END_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
@implementation VSTGUI_CocoaWindowDelegate
//-----------------------------------------------------------------------------
- (id) initWithDelegate:(IPlatformWindowDelegate*) _delegate platformWindow:(PlatformWindow*) _platformWindow
{
	self = [super init];
	if (self)
	{
		delegate = _delegate;
		platformWindow = _platformWindow;
	}
	return self;
}

//-----------------------------------------------------------------------------
- (void)windowDidResize:(NSNotification *)notification
{
	NSWindow* window = [notification object];
	NSRect size = [window contentRectForFrameRect:[window frame]];
	CRect r (size.origin.x, size.origin.y, 0, 0);
	r.setWidth (size.size.width);
	r.setHeight (size.size.height);
	delegate->windowSizeChanged (r, platformWindow);
}

//-----------------------------------------------------------------------------
- (void)windowWillClose:(NSNotification *)notification
{
	delegate->windowClosed (platformWindow);
}

//-----------------------------------------------------------------------------
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
	NSRect r = {{0, 0}, frameSize };
	r = [sender contentRectForFrameRect:r];
	CPoint p (r.size.width, r.size.height);
	delegate->checkWindowSizeConstraints (p, platformWindow);
	r.size.width = p.x;
	r.size.height = p.y;
	r = [sender frameRectForContentRect:r];
	return r.size;
}

@end

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
@implementation VSTGUI_CocoaDraggingSource
//-----------------------------------------------------------------------------
- (id) init
{
	self = [super init];
	if (self)
	{
		localOnly = NO;
	}
	return self;
}

//-----------------------------------------------------------------------------
- (void) setLocalOnly:(BOOL)flag
{
	localOnly = flag;
}

//-----------------------------------------------------------------------------
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)flag
{
	if (localOnly && !flag)
		return NSDragOperationNone;
	return NSDragOperationGeneric;
}

//-----------------------------------------------------------------------------
- (BOOL)ignoreModifierKeysWhileDragging
{
	return YES;
}

@end

//-----------------------------------------------------------------------------
@implementation VSTGUI_ColorChangeCallback
- (void) setCallback: (IPlatformColorChangeCallback*) _callback
{
	callback = _callback;
}
- (void) colorChanged: (id) sender
{
	if (callback)
	{
		NSColor* nsColor = [sender color];
		nsColor = [nsColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if (nsColor)
		{
			CColor newColor = MakeCColor ([nsColor redComponent] * 255., [nsColor greenComponent] * 255., [nsColor blueComponent] * 255., [nsColor alphaComponent] * 255.);
			callback->colorChanged (newColor);
		}
	}
}
@end

#endif // VSTGUI_LIVE_EDITING
