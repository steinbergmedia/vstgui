//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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

#if MAC

#include "uicolorchooserpanel.h"
#include "../lib/platform/mac/cocoa/nsviewframe.h"
#include "../lib/platform/mac/cocoa/cocoahelpers.h"
#include "../lib/platform/mac/cgbitmap.h"
#include "../lib/platform/mac/macglobals.h"
#include <Cocoa/Cocoa.h>

using namespace VSTGUI;

#define NATIVE_COLOR_CHOOSER 1

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

#if NATIVE_COLOR_CHOOSER
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
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CocoaWindow : public PlatformWindow
{
public:
	CocoaWindow (const CRect& size, const char* title, WindowType type, int32_t styleFlags, IPlatformWindowDelegate* delegate);
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
	int32_t styleFlags;
};

//-----------------------------------------------------------------------------
PlatformWindow* PlatformWindow::create (const CRect& size, const char* title, WindowType type, int32_t styleFlags, IPlatformWindowDelegate* delegate, void* parentWindow)
{
	return new CocoaWindow (size, title, type, styleFlags, delegate);
}

//-----------------------------------------------------------------------------
CocoaWindow::CocoaWindow (const CRect& size, const char* title, WindowType type, int32_t styleFlags, IPlatformWindowDelegate* delegate)
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
	[window autorelease];
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
	[window selectNextKeyView:nil];
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
void PlatformUtilities::gatherResourceBitmaps (std::list<std::string>& filenames)
{
	CFBundleRef bundle = getBundleRef ();
	if (bundle)
	{
		static CFStringRef bitmapTypes [] = { CFSTR("png"), CFSTR("bmp"), CFSTR("jpg"), NULL };
		CFStringRef type = bitmapTypes[0];
		int32_t index = 0;
		while (type != NULL)
		{
			CFArrayRef urls = CFBundleCopyResourceURLsOfType (bundle, type, NULL);
			if (urls)
			{
				for (CFIndex i = 0; i < CFArrayGetCount (urls); i++)
				{
					CFURLRef url = (CFURLRef)CFArrayGetValueAtIndex (urls, i);
					if (url)
					{
						CFStringRef str = CFURLCopyLastPathComponent (url);
						if (str)
						{
							char cstr[PATH_MAX];
							if (CFStringGetCString (str, cstr, PATH_MAX, kCFStringEncodingUTF8))
								filenames.push_back (cstr);
							CFRelease (str);
						}
					}
				}
				CFRelease (urls);
			}
			type = bitmapTypes[++index];
		}
	}
}

#if !NATIVE_COLOR_CHOOSER
class ColorChooserWindowOwner : public CBaseObject
{
public:
	static ColorChooserWindowOwner* instance () { return &ccwo; }

	UIColorChooserPanel* getPanel (bool create = true)
	{
		if (panel == 0 && create)
		{
			panel = new UIColorChooserPanel (this);
		}
		return panel;
	}
	
	void closePanel ()
	{
		if (panel)
			panel->forget ();
		panel = 0;
	}
	
protected:
	static ColorChooserWindowOwner ccwo; 

	ColorChooserWindowOwner () : panel (0) {}
	~ColorChooserWindowOwner ()
	{
		closePanel ();
	}
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message)
	{
		if (message == UIColorChooserPanel::kMsgWindowClosed)
			panel = 0;
		return kMessageNotified;
	}
	
	UIColorChooserPanel* panel;
};
ColorChooserWindowOwner ColorChooserWindowOwner::ccwo; 
#endif

//-----------------------------------------------------------------------------
void PlatformUtilities::colorChooser (const CColor* oldColor, IPlatformColorChangeCallback* callback)
{
	#if NATIVE_COLOR_CHOOSER
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
	#else
	if (oldColor)
	{
		ColorChooserWindowOwner* owner = ColorChooserWindowOwner::instance ();
		UIColorChooserPanel* panel = owner->getPanel ();
		panel->setColor (*oldColor);
		panel->setColorChangeCallback (callback);
	}
	else
	{
		ColorChooserWindowOwner* owner = ColorChooserWindowOwner::instance ();
		UIColorChooserPanel* panel = owner->getPanel (false);
		if (panel)
		{
			panel->setColorChangeCallback (0);
			owner->closePanel ();
		}
	}
	#endif
}

} // namespace

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

#if NATIVE_COLOR_CHOOSER
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
#endif // NATIVE_COLOR_CHOOSER

#endif // MAC

#endif // VSTGUI_LIVE_EDITING
