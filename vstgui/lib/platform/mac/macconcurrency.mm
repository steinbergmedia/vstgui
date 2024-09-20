// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "macconcurrency.h"
#import "../../vstguidebug.h"
#import <dispatch/dispatch.h>
#import <atomic>
#import <thread>
#import <Foundation/Foundation.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Concurrency {

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
} // Concurrency

//------------------------------------------------------------------------
MacConcurrency::MacConcurrency ()
{
	mainQueue = std::make_unique<Concurrency::Queue> (dispatch_get_main_queue ());
	backgroundQueue = std::make_unique<Concurrency::Queue> (
		dispatch_get_global_queue (QOS_CLASS_USER_INITIATED, 0));
}

//------------------------------------------------------------------------
MacConcurrency::~MacConcurrency () noexcept
{
	waitAllTasksExecuted (getBackgroundQueue ());
	waitAllTasksExecuted (getMainQueue ());
	vstgui_assert (Concurrency::numUserQueues == 0u,
				   "Concurrency Queues must all be destroyed at this point");
}

//------------------------------------------------------------------------
const Concurrency::Queue& MacConcurrency::getMainQueue () const { return *mainQueue.get (); }

//------------------------------------------------------------------------
const Concurrency::Queue& MacConcurrency::getBackgroundQueue () const
{
	return *backgroundQueue.get ();
}

//------------------------------------------------------------------------
Concurrency::QueuePtr MacConcurrency::makeSerialQueue (const char* name) const
{
	auto dq = dispatch_queue_create (name, DISPATCH_QUEUE_SERIAL);
	auto queue = std::shared_ptr<Concurrency::Queue> (new Concurrency::Queue {dq}, [] (auto queue) {
		queue->waitAllTasksDone ();
		--Concurrency::numUserQueues;
		delete queue;
	});
	++Concurrency::numUserQueues;
	dispatch_release (dq);
	return queue;
}

//------------------------------------------------------------------------
void MacConcurrency::schedule (const Concurrency::Queue& queue, Concurrency::Task&& task) const
{
	queue.schedule (std::move (task));
}

//------------------------------------------------------------------------
void MacConcurrency::waitAllTasksExecuted (const Concurrency::Queue& queue) const
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
void MacConcurrency::waitAllTasksExecuted () const
{
	while (!backgroundQueue->empty ())
	{
		[[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
	}
}

//------------------------------------------------------------------------
} // VSTGUI
