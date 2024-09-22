// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "mactaskexecutor.h"
#import "../../vstguidebug.h"
#import <dispatch/dispatch.h>
#import <atomic>
#import <thread>
#import <Foundation/Foundation.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Tasks {

//------------------------------------------------------------------------
struct Queue
{
	Queue (dispatch_queue_t q) : queue (q)
	{
		dispatch_retain (queue);
		group = dispatch_group_create ();
	}

	~Queue ()
	{
		waitAllTasksDone ();
		dispatch_release (group);
		dispatch_release (queue);
	}

	void schedule (Task&& task) const
	{
		++taskCount;
		dispatch_group_async (group, queue, [this, task = std::move (task)] () {
			task ();
			--taskCount;
		});
	}

	bool empty () const { return taskCount == 0; }

	bool waitAllTasksDone () const
	{
		return dispatch_group_wait (group, DISPATCH_TIME_FOREVER) == 0;
	}

private:
	dispatch_queue_t queue;
	dispatch_group_t group;
	mutable std::atomic<uint64_t> taskCount {0u};
};

static std::atomic<uint32_t> numUserQueues {0u};

//------------------------------------------------------------------------
} // Tasks

//------------------------------------------------------------------------
MacTaskExecutor::MacTaskExecutor ()
{
	mainQueue = std::make_unique<Tasks::Queue> (dispatch_get_main_queue ());
	backgroundQueue =
		std::make_unique<Tasks::Queue> (dispatch_get_global_queue (QOS_CLASS_USER_INITIATED, 0));
}

//------------------------------------------------------------------------
MacTaskExecutor::~MacTaskExecutor () noexcept
{
	waitAllTasksExecuted (getBackgroundQueue ());
	waitAllTasksExecuted (getMainQueue ());
	vstgui_assert (Tasks::numUserQueues == 0u, "Serial queues must all be destroyed at this point");
}

//------------------------------------------------------------------------
const Tasks::Queue& MacTaskExecutor::getMainQueue () const { return *mainQueue.get (); }

//------------------------------------------------------------------------
const Tasks::Queue& MacTaskExecutor::getBackgroundQueue () const { return *backgroundQueue.get (); }

//------------------------------------------------------------------------
Tasks::QueuePtr MacTaskExecutor::makeSerialQueue (const char* name) const
{
	auto dq = dispatch_queue_create (name, DISPATCH_QUEUE_SERIAL);
	auto queue = std::shared_ptr<Tasks::Queue> (new Tasks::Queue {dq}, [] (auto queue) {
		queue->waitAllTasksDone ();
		--Tasks::numUserQueues;
		delete queue;
	});
	++Tasks::numUserQueues;
	dispatch_release (dq);
	return queue;
}

//------------------------------------------------------------------------
void MacTaskExecutor::schedule (const Tasks::Queue& queue, Tasks::Task&& task) const
{
	queue.schedule (std::move (task));
}

//------------------------------------------------------------------------
void MacTaskExecutor::waitAllTasksExecuted (const Tasks::Queue& queue) const
{
	vstgui_assert (pthread_main_np () != 0, "Must be called from the main thread");
	auto isMainQueue = &queue == &getMainQueue ();
	if (isMainQueue)
	{
		while (!queue.empty ())
		{
			[[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode
								  beforeDate:[NSDate distantFuture]];
		}
	}
	else
	{
		queue.waitAllTasksDone ();
	}
}

//------------------------------------------------------------------------
void MacTaskExecutor::waitAllTasksExecuted () const
{
	while (!backgroundQueue->empty ())
	{
		[[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
	}
}

//------------------------------------------------------------------------
} // VSTGUI
