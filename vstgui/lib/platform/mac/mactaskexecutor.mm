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

//------------------------------------------------------------------------
struct MacTaskExecutor::DispatchQueue
{
	DispatchQueue (dispatch_queue_t q, uint64_t queueId) : queue (q), taskQueueID ({queueId})
	{
		dispatch_retain (queue);
		group = dispatch_group_create ();
	}

	~DispatchQueue ()
	{
		waitAllTasksExecuted ();
		dispatch_release (group);
		dispatch_release (queue);
	}

	void schedule (Tasks::Task&& task) const
	{
		++taskCount;
		dispatch_group_async (group, queue, [this, task = std::move (task)] () {
			task ();
			--taskCount;
		});
	}

	bool empty () const { return taskCount == 0; }

	void waitAllTasksExecuted () const { dispatch_group_wait (group, DISPATCH_TIME_FOREVER); }

	const Tasks::Queue& getQueueID () const { return taskQueueID; }

private:
	dispatch_queue_t queue;
	dispatch_group_t group;
	mutable Tasks::Queue taskQueueID;
	mutable std::atomic<uint64_t> taskCount {0u};
};

//------------------------------------------------------------------------
MacTaskExecutor::MacTaskExecutor ()
{
	mainQueue = std::make_unique<DispatchQueue> (dispatch_get_main_queue (), 0);
	backgroundQueue = std::make_unique<DispatchQueue> (
		dispatch_get_global_queue (QOS_CLASS_USER_INITIATED, 0), 1);
}

//------------------------------------------------------------------------
MacTaskExecutor::~MacTaskExecutor () noexcept
{
	waitAllTasksExecuted (getBackgroundQueue ());
	waitAllTasksExecuted (getMainQueue ());
	vstgui_assert (serialQueues.empty (), "Serial queues must all be destroyed at this point");
}

//------------------------------------------------------------------------
const Tasks::Queue& MacTaskExecutor::getMainQueue () const { return mainQueue->getQueueID (); }

//------------------------------------------------------------------------
const Tasks::Queue& MacTaskExecutor::getBackgroundQueue () const
{
	return backgroundQueue->getQueueID ();
}

//------------------------------------------------------------------------
Tasks::Queue MacTaskExecutor::makeSerialQueue (const char* name) const
{
	std::lock_guard<std::mutex> guard (serialQueueLock);
	auto dq = dispatch_queue_create (name, DISPATCH_QUEUE_SERIAL);
	auto queue = std::make_unique<DispatchQueue> (dq, nextSerialQueueId++);
	auto queueId = queue->getQueueID ();
	serialQueues.emplace_back (std::move (queue));
	dispatch_release (dq);
	return queueId;
}

//------------------------------------------------------------------------
void MacTaskExecutor::releaseSerialQueue (const Tasks::Queue& queue) const
{
	serialQueueLock.lock ();
	auto it = std::find_if (serialQueues.begin (), serialQueues.end (), [&] (const auto& el) {
		return el->getQueueID ().identifier == queue.identifier;
	});
	if (it != serialQueues.end ())
	{
		auto dispatchQueue = std::move (*it);
		serialQueues.erase (it);
		serialQueueLock.unlock ();
		dispatchQueue->waitAllTasksExecuted ();
	}
	else
	{
		serialQueueLock.unlock ();
	}
}

//------------------------------------------------------------------------
auto MacTaskExecutor::dispatchQueueFromQueueID (const Tasks::Queue& queue) const
	-> const DispatchQueue*
{
	const DispatchQueue* dq = nullptr;
	if (queue.identifier == mainQueue->getQueueID ().identifier)
		dq = mainQueue.get ();
	else if (queue.identifier == backgroundQueue->getQueueID ().identifier)
		dq = backgroundQueue.get ();
	else
	{
		serialQueueLock.lock ();
		auto it = std::find_if (serialQueues.begin (), serialQueues.end (), [&] (const auto& el) {
			return el->getQueueID ().identifier == queue.identifier;
		});
		if (it != serialQueues.end ())
			dq = it->get ();
		serialQueueLock.unlock ();
	}
	return dq;
}

//------------------------------------------------------------------------
void MacTaskExecutor::schedule (const Tasks::Queue& queue, Tasks::Task&& task) const
{
	if (auto dispatchQueue = dispatchQueueFromQueueID (queue))
		dispatchQueue->schedule (std::move (task));
}

//------------------------------------------------------------------------
void MacTaskExecutor::waitAllTasksExecuted (const Tasks::Queue& queue) const
{
	vstgui_assert (pthread_main_np () != 0, "Must be called from the main thread");
	if (auto dispatchQueue = dispatchQueueFromQueueID (queue))
	{
		auto isMainQueue = queue.identifier == mainQueue->getQueueID ().identifier;
		if (isMainQueue)
		{
			while (!dispatchQueue->empty ())
			{
				[[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode
									  beforeDate:[NSDate distantFuture]];
			}
		}
		else
		{
			dispatchQueue->waitAllTasksExecuted ();
		}
	}
}

//------------------------------------------------------------------------
void MacTaskExecutor::waitAllTasksExecuted () const
{
	while (!backgroundQueue->empty () || !mainQueue->empty ())
	{
		[[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
	}
}

//------------------------------------------------------------------------
} // VSTGUI
