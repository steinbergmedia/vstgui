// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platform_helper.h"
#import <Cocoa/Cocoa.h>

namespace VSTGUI {
namespace UnitTest {

struct MacParentHandle : PlatformParentHandle
{
	NSWindow* window {nil};

	MacParentHandle ()
	{
		NSWindowStyleMask style;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
		style = NSWindowStyleMaskTitled;
#else
		style = NSTitledWindowMask;
#endif
		window = [[NSWindow alloc] initWithContentRect:NSMakeRect (0, 0, 100, 100)
											 styleMask:style
											   backing:NSBackingStoreBuffered
												 defer:NO];
	}
	
	~MacParentHandle () override
	{
		[window release];
	}
	
	PlatformType getType () const override
	{
		return PlatformType::kNSView;
	}

	void* getHandle () const override
	{
		return window.contentView;
	}

	void forceRedraw () override
	{
		[window displayIfNeeded];
	}

};

SharedPointer<PlatformParentHandle> PlatformParentHandle::create ()
{
	return owned (dynamic_cast<PlatformParentHandle*> (new MacParentHandle ()));
}

} // UnitTest
} // VSTGUI

