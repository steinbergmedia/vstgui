// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "../../../include/iasync.h"
#import "macasync.h"
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
struct Queue
{
	Queue (dispatch_queue_t q) : queue (q)
	{
		isBackgroundQueue = queue != dispatch_get_main_queue ();
	}

	void schedule (Task&& task)
	{
		if (isBackgroundQueue)
			++gBackgroundTaskCount;
		dispatch_async (queue, [task = std::move (task), backgroundQueue = isBackgroundQueue] () {
			task ();
			if (backgroundQueue)
				--gBackgroundTaskCount;
		});
	}

private:
	dispatch_queue_t queue;
	bool isBackgroundQueue;
};

//------------------------------------------------------------------------
const QueuePtr& mainQueue ()
{
	static QueuePtr q = std::make_shared<Queue> (dispatch_get_main_queue ());
	return q;
}

//------------------------------------------------------------------------
const QueuePtr& backgroundQueue ()
{
	static QueuePtr q =
	    std::make_shared<Queue> (dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
	return q;
}

//------------------------------------------------------------------------
QueuePtr makeSerialQueue (const char* name)
{
	auto q = dispatch_queue_create (name, DISPATCH_QUEUE_SERIAL);
	return std::make_shared<Queue> (q);
}

//------------------------------------------------------------------------
void schedule (QueuePtr queue, Task&& task)
{
	queue->schedule (std::move (task));
}

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI
