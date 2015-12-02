//
//  platform_helper_mac.cpp
//  vstgui
//
//  Created by Arne Scheffler on 22/11/15.
//
//

#include "platform_helper.h"
#import <Cocoa/Cocoa.h>

namespace VSTGUI {
namespace UnitTest {

struct MacParentHandle : PlatformParentHandle
{
	NSWindow* window {nil};

	MacParentHandle ()
	{
		window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 100, 100) styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
	}
	
	~MacParentHandle ()
	{
		[window release];
	}
	
	PlatformType getType () const override
	{
		return kNSView;
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

