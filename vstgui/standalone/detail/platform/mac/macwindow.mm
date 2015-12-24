//
//  window.m
//  vstgui
//
//  Created by Arne Scheffler on 21.12.15.
//
//

#import <Cocoa/Cocoa.h>
#import "../iplatformwindow.h"
#import "../../../../lib/platform/mac/macstring.h"
#import "VSTGUICommand.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {
class Window;
}}}}

//------------------------------------------------------------------------
@interface VSTGUIWindowDelegate : NSObject <NSWindowDelegate>
@property VSTGUI::Standalone::Platform::Mac::Window* macWindow;
@end

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
class Window : public IWindow
{
public:
	bool init (const WindowConfiguration& config, IWindowDelegate& delegate);
	CPoint getSize () const override;
	CPoint getPosition () const override;
	
	void setSize (const CPoint& newSize) override;
	void setPosition (const CPoint& newPosition) override;
	void setTitle (const UTF8String& newTitle) override;
	
	void show () override;
	void hide () override;
	void close () override;
	
	PlatformType getPlatformType () const override { return kNSView; };
	void* getPlatformHandle () const override { return static_cast<void*> ((__bridge void*) nsWindow.contentView); }

	void windowWillClose ();
	IWindowDelegate& getDelegate () const { return *delegate; }
	NSWindow* getNSWindow () const { return nsWindow; }
private:
	NSWindow* nsWindow {nullptr};
	VSTGUIWindowDelegate* nsWindowDelegate {nullptr};
	IWindowDelegate* delegate {nullptr};
};
	
//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, IWindowDelegate& inDelegate)
{
	NSUInteger styleMask = 0;
	if (config.flags.hasBorder ())
		styleMask |= NSTitledWindowMask;
	if (config.flags.canSize ())
		styleMask |= NSResizableWindowMask;
	if (config.flags.canClose ())
		styleMask |= NSClosableWindowMask;

	delegate = &inDelegate;

	NSRect contentRect = NSMakeRect (0, 0,
									 config.size.x, config.size.y);
	nsWindow = [[NSWindow alloc] initWithContentRect:contentRect
										 styleMask:styleMask
										   backing:NSBackingStoreBuffered
											 defer:YES];

	nsWindowDelegate = [VSTGUIWindowDelegate new];
	nsWindowDelegate.macWindow = this;
	[nsWindow setDelegate:nsWindowDelegate];
	[nsWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];
	
	auto titleMacStr = dynamic_cast<MacString*> (config.title.getPlatformString ());
	if (titleMacStr && titleMacStr->getCFString ())
	{
		nsWindow.title = (__bridge NSString*)titleMacStr->getCFString();
	}
	[nsWindow setReleasedWhenClosed:NO];

	return true;
}

//------------------------------------------------------------------------
void Window::windowWillClose ()
{
	nsWindow.delegate = nil;
	nsWindowDelegate = nil;
	nsWindow = nil;
	delegate->onClosed ();
	// we are now destroyed ! at least we should !
}

//------------------------------------------------------------------------
CPoint Window::getSize () const
{
	CPoint p;
	NSSize size = [nsWindow contentRectForFrameRect:nsWindow.frame].size;
	p.x = size.width;
	p.y = size.height;
	return p;
}

//------------------------------------------------------------------------
CPoint Window::getPosition () const
{
	CPoint p;
	NSPoint origin = [nsWindow contentRectForFrameRect:nsWindow.frame].origin;
	p.x = origin.x;
	p.y = origin.y;
	return p;
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	NSRect r = [nsWindow contentRectForFrameRect:nsWindow.frame];
	CGFloat diff = newSize.y - r.size.height;
	r.size.width = newSize.x;
	r.size.height = newSize.y;
	if ([nsWindow isVisible])
		r.origin.y -= diff;
	[nsWindow setFrame:[nsWindow frameRectForContentRect:r] display:YES animate:NO];
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	assert (false && "Not yet implemented");
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
	auto titleMacStr = dynamic_cast<MacString*> (newTitle.getPlatformString ());
	if (titleMacStr && titleMacStr->getCFString ())
	{
		nsWindow.title = (__bridge NSString*)titleMacStr->getCFString();
	}
}

//------------------------------------------------------------------------
void Window::show ()
{
	if (![nsWindow isVisible])
	{
		delegate->onShow ();
		if (nsWindow.frame.origin.y == 0)
			[nsWindow center];
		[nsWindow makeKeyAndOrderFront:nil];
	}
}

//------------------------------------------------------------------------
void Window::hide ()
{
	delegate->onHide ();
	[nsWindow orderOut:nil];
}

//------------------------------------------------------------------------
void Window::close ()
{
	[nsWindow performClose:nil];
}

} // Mac


//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, IWindowDelegate& delegate)
{
	auto window = std::make_shared<Mac::Window> ();
	if (window->init (config, delegate))
		return window;
	return nullptr;
}

} // Platform
} // Standalone
} // VSTGUI

//------------------------------------------------------------------------
@implementation VSTGUIWindowDelegate

//------------------------------------------------------------------------
- (IBAction)processCommand:(id)sender
{
	bool res = false;
	VSTGUICommand* command = [sender representedObject];
	if (command)
		res = self.macWindow->getDelegate ().handleCommand ([command command]);
	if (!res)
	{
		id delegate = NSApp.delegate;
		if ([delegate respondsToSelector:@selector(processCommand:)])
			return [delegate processCommand:sender];
	}
}

//------------------------------------------------------------------------
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	BOOL res = NO;
	if (VSTGUICommand* command = menuItem.representedObject)
		res = self.macWindow->getDelegate ().canHandleCommand ([command command]);
	if (!res)
	{
		id delegate = NSApp.delegate;
		if ([delegate respondsToSelector:@selector(validateMenuItem:)])
			return [delegate validateMenuItem:menuItem];
	}
	return res;
}

//------------------------------------------------------------------------
- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
	NSRect r {};
	r.size = frameSize;
	r = [sender contentRectForFrameRect:r];
	VSTGUI::CPoint p (r.size.width, r.size.height);
	p = self.macWindow->getDelegate ().constraintSize (p);
	r.size.width = p.x;
	r.size.height = p.y;
	r = [sender frameRectForContentRect:r];
	return r.size;
}

//------------------------------------------------------------------------
- (void)windowDidResize:(NSNotification*)notification
{
	NSRect r = [[notification object] frame];
	r = [self.macWindow->getNSWindow () contentRectForFrameRect:r];
	VSTGUI::CPoint size;
	size.x = r.size.width;
	size.y = r.size.height;
	self.macWindow->getDelegate ().onSizeChanged (size);
}

//------------------------------------------------------------------------
- (void)windowWillClose:(NSNotification*)notification
{
	self.macWindow->windowWillClose ();
}

//------------------------------------------------------------------------
- (BOOL)windowShouldClose:(id)sender
{
	return self.macWindow->getDelegate ().canClose ();
}

@end
