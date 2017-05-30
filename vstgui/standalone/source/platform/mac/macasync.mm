// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "macasync.h"
#import "../../../include/iasync.h"
#import <Cocoa/Cocoa.h>
#import <atomic>
#import <dispatch/dispatch.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Async {

//------------------------------------------------------------------------
static std::atomic<uint32_t> gBackgroundTaskCount {};

//------------------------------------------------------------------------
void waitAllTasksDone ()
{
	while (gBackgroundTaskCount != 0)
	{
		[[NSRunLoop mainRunLoop] runMode:NSRunLoopCommonModes beforeDate:[NSDate distantFuture]];
	}
}

//------------------------------------------------------------------------
void perform (Context context, Task&& task)
{
	dispatch_queue_t queue;
	switch (context)
	{
		case Context::Main:
		{
			queue = dispatch_get_main_queue ();
			break;
		}
		case Context::Background:
		{
			queue = dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_LOW, 0);
			++gBackgroundTaskCount;
			break;
		}
	}
	dispatch_async (queue, [context, task = std::move (task)] () {
		// execute
		task ();
		if (context == Context::Background)
			--gBackgroundTaskCount;
	});
}

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI
