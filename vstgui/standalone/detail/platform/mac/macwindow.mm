#import <Cocoa/Cocoa.h>

#import "macwindow.h"
#import "VSTGUICommand.h"
#import "../iplatformwindow.h"
#import "../../../../lib/platform/mac/macstring.h"
#import "../../../../lib/cvstguitimer.h"
#import "../../../../lib/cframe.h"

#ifndef MAC_OS_X_VERSION_10_11
#define MAC_OS_X_VERSION_10_11 101100
#endif

//------------------------------------------------------------------------
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_11
@interface NSWindow (BackwardsCompatibility)
- (void)performWindowDragWithEvent:(NSEvent*)event;
@end
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

class Window;

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI

//------------------------------------------------------------------------
@interface VSTGUIWindowDelegate : NSObject <NSWindowDelegate>
@property VSTGUI::Standalone::Platform::Mac::Window* macWindow;
@end

//------------------------------------------------------------------------
@interface VSTGUIWindow : NSWindow
@property BOOL supportMovableByWindowBackground;
@property BOOL nonClosable;
@end

//------------------------------------------------------------------------
@interface VSTGUIPopup : NSPanel
@property BOOL supportMovableByWindowBackground;
@property BOOL inSendEvent;
@property NSInteger doResignKey;
@property NSInteger doResignKeyStackDepth;
@end

//------------------------------------------------------------------------
@interface VSTGUIPopupDelegate : VSTGUIWindowDelegate
@end

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
class Window : public IWindow, public IMacWindow
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
	void activate () override;
	void center () override;

	PlatformType getPlatformType () const override { return kNSView; };
	void* getPlatformHandle () const override
	{
		return static_cast<void*> ((__bridge void*)nsWindow.contentView);
	}
	void onSetContentView (CFrame* newFrame) override;

	void windowDidResize (const CPoint& newSize);
	void windowWillClose ();
	IWindowDelegate& getDelegate () const { return *delegate; }
	NSWindow* getNSWindow () const override { return nsWindow; }
	bool isPopup () const override;

private:
	NSWindow* nsWindow {nullptr};
	VSTGUIWindowDelegate* nsWindowDelegate {nullptr};
	IWindowDelegate* delegate {nullptr};
	CFrame* frame {nullptr};
};

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, IWindowDelegate& inDelegate)
{
	NSUInteger styleMask = 0;
	if (config.style.hasBorder ())
		styleMask |= NSTitledWindowMask;
	if (config.style.canSize ())
		styleMask |= NSResizableWindowMask;
	if (config.style.canClose ())
		styleMask |= NSClosableWindowMask;

	delegate = &inDelegate;

	NSRect contentRect = NSMakeRect (0, 0, config.size.x, config.size.y);
	if (config.type == WindowType::Popup)
	{
		styleMask |= NSUtilityWindowMask;

		VSTGUIPopup* popup = [[VSTGUIPopup alloc] initWithContentRect:contentRect
		                                                    styleMask:styleMask
		                                                      backing:NSBackingStoreBuffered
		                                                        defer:YES];

		popup.becomesKeyOnlyIfNeeded = NO;
		popup.level = NSFloatingWindowLevel;
		popup.supportMovableByWindowBackground = config.style.isMovableByWindowBackground ();

		nsWindow = popup;
		nsWindowDelegate = [VSTGUIPopupDelegate new];
		nsWindowDelegate.macWindow = this;
		[nsWindow setAnimationBehavior:NSWindowAnimationBehaviorUtilityWindow];
		nsWindow.collectionBehavior =
		    NSWindowCollectionBehaviorFullScreenAuxiliary | nsWindow.collectionBehavior;
	}
	else
	{
		VSTGUIWindow* window = [[VSTGUIWindow alloc] initWithContentRect:contentRect
		                                                       styleMask:styleMask
		                                                         backing:NSBackingStoreBuffered
		                                                           defer:YES];
		window.supportMovableByWindowBackground = config.style.isMovableByWindowBackground ();
		nsWindow = window;

		nsWindowDelegate = [VSTGUIWindowDelegate new];
		nsWindowDelegate.macWindow = this;
		[nsWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];
		if (!config.style.canClose ())
			window.nonClosable = true;
		nsWindow.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary |
		                              NSWindowCollectionBehaviorFullScreenAuxiliary |
		                              nsWindow.collectionBehavior;
	}
	[nsWindow setDelegate:nsWindowDelegate];

	if (config.style.isTransparent ())
	{
		nsWindow.backgroundColor = [NSColor clearColor];
		nsWindow.opaque = NO;
	}

	auto titleMacStr = dynamic_cast<MacString*> (config.title.getPlatformString ());
	if (titleMacStr && titleMacStr->getCFString ())
	{
		nsWindow.title = (__bridge NSString*)titleMacStr->getCFString ();
	}
	[nsWindow setReleasedWhenClosed:NO];
	[nsWindow center];

	return true;
}

//------------------------------------------------------------------------
bool Window::isPopup () const { return [nsWindow isKindOfClass:[NSPanel class]]; }

//------------------------------------------------------------------------
void Window::onSetContentView (CFrame* newFrame) { frame = newFrame; }

//------------------------------------------------------------------------
void Window::windowDidResize (const CPoint& newSize)
{
	delegate->onSizeChanged (newSize);
	if (frame)
		frame->setSize (newSize.x, newSize.y);
}

//------------------------------------------------------------------------
void Window::windowWillClose ()
{
	NSWindow* temp = nsWindow;
	nsWindowDelegate = nil;
	delegate->onClosed ();
	// we are now destroyed ! at least we should !
	temp.delegate = nil;
	temp = nil;
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
static NSRect getMainScreenRect ()
{
	NSScreen* mainScreen = [NSScreen screens][0];
	return mainScreen.frame;
}

//------------------------------------------------------------------------
CPoint Window::getPosition () const
{
	CPoint p;
	NSRect windowRect = [nsWindow contentRectForFrameRect:nsWindow.frame];
	p.x = windowRect.origin.x;
	p.y = windowRect.origin.y;
	p.y = getMainScreenRect ().size.height - (p.y + windowRect.size.height);
	return p;
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	NSRect r = [nsWindow contentRectForFrameRect:nsWindow.frame];
	CGFloat diff = newSize.y - r.size.height;
	r.size.width = newSize.x;
	r.size.height = newSize.y;
	r.origin.y -= diff;
	[nsWindow setFrame:[nsWindow frameRectForContentRect:r]
	           display:[nsWindow isVisible]
	           animate:NO];
	if (!nsWindow.opaque)
		[nsWindow invalidateShadow];
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	NSRect r = [nsWindow contentRectForFrameRect:nsWindow.frame];
	r.origin.x = newPosition.x;
	r.origin.y = getMainScreenRect ().size.height - (newPosition.y + r.size.height);
	[nsWindow setFrame:[nsWindow frameRectForContentRect:r]
	           display:[nsWindow isVisible]
	           animate:NO];
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
	auto titleMacStr = dynamic_cast<MacString*> (newTitle.getPlatformString ());
	if (titleMacStr && titleMacStr->getCFString ())
	{
		nsWindow.title = (__bridge NSString*)titleMacStr->getCFString ();
	}
}

//------------------------------------------------------------------------
void Window::show ()
{
	if (![nsWindow isVisible])
	{
		delegate->onShow ();
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
void Window::close () { [nsWindow performClose:nil]; }

//------------------------------------------------------------------------
void Window::activate () { [nsWindow makeKeyAndOrderFront:nil]; }

//------------------------------------------------------------------------
void Window::center () { [nsWindow center]; }

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
		id delegate = [NSApp delegate];
		if ([delegate respondsToSelector:@selector (processCommand:)])
			return [delegate processCommand:sender];
	}
}

//------------------------------------------------------------------------
- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
	BOOL res = NO;
	if (VSTGUICommand* command = menuItem.representedObject)
		res = self.macWindow->getDelegate ().canHandleCommand ([command command]);
	if (!res)
	{
		id delegate = [NSApp delegate];
		if ([delegate respondsToSelector:@selector (validateMenuItem:)])
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
	self.macWindow->windowDidResize (size);
}

//------------------------------------------------------------------------
- (void)windowDidMove:(NSNotification*)notification
{
	self.macWindow->getDelegate ().onPositionChanged (self.macWindow->getPosition ());
}

//------------------------------------------------------------------------
- (void)windowWillClose:(NSNotification*)notification { self.macWindow->windowWillClose (); }

//------------------------------------------------------------------------
- (BOOL)windowShouldClose:(id)sender { return self.macWindow->getDelegate ().canClose (); }

//------------------------------------------------------------------------
- (void)windowDidBecomeKey:(NSNotification*)notification
{
	self.macWindow->getDelegate ().onActivated ();
}

//------------------------------------------------------------------------
- (void)windowDidResignKey:(NSNotification*)notification
{
	self.macWindow->getDelegate ().onDeactivated ();
}

@end

//------------------------------------------------------------------------
@implementation VSTGUIWindow

//------------------------------------------------------------------------
- (void)mouseDown:(NSEvent*)theEvent
{
	if (self.supportMovableByWindowBackground &&
	    [super respondsToSelector:@selector (performWindowDragWithEvent:)])
	{
		[super performWindowDragWithEvent:theEvent];
	}
}

//------------------------------------------------------------------------
- (BOOL)canBecomeKeyWindow { return YES; }

//------------------------------------------------------------------------
- (void)performClose:(nullable id)sender
{
	VSTGUIWindow* window = self;
	VSTGUI::Call::later ([=] () { [window close]; });
}

//------------------------------------------------------------------------
- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
	if ([menuItem action] == @selector (performClose:))
		return !self.nonClosable;
	return [super validateMenuItem:menuItem];
}

//------------------------------------------------------------------------
- (void)makeKeyAndOrderFront:(nullable id)sender
{
	if (!self.visible && [self.title length] > 0)
	{
		[NSApp addWindowsItem:self title:self.title filename:NO];
	}
	[super makeKeyAndOrderFront:sender];
}

@end

//------------------------------------------------------------------------
@implementation VSTGUIPopup

//------------------------------------------------------------------------
- (BOOL)canBecomeKeyWindow { return YES; }

//------------------------------------------------------------------------
- (void)mouseDown:(NSEvent*)theEvent
{
	if (self.supportMovableByWindowBackground &&
	    [super respondsToSelector:@selector (performWindowDragWithEvent:)])
	{
		[super performWindowDragWithEvent:theEvent];
	}
}

//------------------------------------------------------------------------
- (void)sendEvent:(NSEvent*)theEvent
{
	self.doResignKeyStackDepth++;
	self.inSendEvent = YES;
	[super sendEvent:theEvent];
	self.inSendEvent = NO;
	if (self.doResignKey == self.doResignKeyStackDepth)
	{
		self.doResignKey = NO;
		[super resignKeyWindow];
	}
	self.doResignKeyStackDepth--;
}

//------------------------------------------------------------------------
- (void)resignKeyWindow
{
	if (self.inSendEvent)
	{
		self.doResignKey = self.doResignKeyStackDepth;
	}
	else
	{
		[super resignKeyWindow];
	}
}

//------------------------------------------------------------------------
- (void)cancelOperation:(nullable id)sender { [self resignKeyWindow]; }

//------------------------------------------------------------------------
- (void)performClose:(nullable id)sender
{
	VSTGUIPopup* popup = self;
	VSTGUI::Call::later ([=] () { [popup close]; });
}

@end

//------------------------------------------------------------------------
@implementation VSTGUIPopupDelegate

//------------------------------------------------------------------------
- (void)windowDidResignKey:(NSNotification*)notification { [self.macWindow->getNSWindow () close]; }

@end
