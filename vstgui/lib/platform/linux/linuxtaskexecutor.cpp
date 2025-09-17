// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "linuxtaskexecutor.h"
#include "../common/threadpooltaskexecutor.h"
#include "../../vstguidebug.h"

//------------------------------------------------------------------------
namespace VSTGUI {

struct LinuxTaskExecutor::Impl
{
	using SerialQueueVector = std::vector<std::unique_ptr<Tasks::Detail::SerialQueue>>;

	Tasks::Detail::ThreadPool threadPool {std::thread::hardware_concurrency ()};
	uint64_t queueIdentifierCounter {};
	SerialQueueVector serialQueues;
	std::mutex serialQueueMutex;
	ScheduleMainQueueTaskFunc scheduleMainQueueTaskFunc;
	Tasks::Queue mainQueue {0u};
	Tasks::Queue backgroundQueue {1u};

	void waitAllTasksExecuted (SerialQueueVector::const_iterator it) const
	{
		while ((*it)->empty () == false)
			std::this_thread::sleep_for (std::chrono::milliseconds (1));
	}

	SerialQueueVector::const_iterator findQueue (uint64_t identifier) const
	{
		return std::find_if (serialQueues.begin (), serialQueues.end (),
							 [&] (const auto& el) { return el->getIdentifier (); });
	}
};

//------------------------------------------------------------------------
LinuxTaskExecutor::LinuxTaskExecutor () { impl = std::make_unique<Impl> (); }

//------------------------------------------------------------------------
LinuxTaskExecutor::~LinuxTaskExecutor () noexcept {}

//------------------------------------------------------------------------
void LinuxTaskExecutor::setScheduleMainQueueTaskFunc (ScheduleMainQueueTaskFunc&& func)
{
	impl->scheduleMainQueueTaskFunc = std::move (func);
}

//------------------------------------------------------------------------
const Tasks::Queue& LinuxTaskExecutor::getMainQueue () const { return impl->mainQueue; }

//------------------------------------------------------------------------
const Tasks::Queue& LinuxTaskExecutor::getBackgroundQueue () const { return impl->backgroundQueue; }

//------------------------------------------------------------------------
Tasks::Queue LinuxTaskExecutor::makeSerialQueue (const char* name) const
{
	std::lock_guard<std::mutex> lock (impl->serialQueueMutex);
	impl->serialQueues.emplace_back (std::make_unique<Tasks::Detail::SerialQueue> (
		impl->threadPool, ++impl->queueIdentifierCounter, name));
	return {impl->queueIdentifierCounter};
}

void LinuxTaskExecutor::releaseSerialQueue (const Tasks::Queue& queue) const
{
	std::lock_guard<std::mutex> lock (impl->serialQueueMutex);
	auto it = impl->findQueue (queue.identifier);
	if (it != impl->serialQueues.end ())
	{
		impl->waitAllTasksExecuted (it);
		impl->serialQueues.erase (it);
	}
}

//------------------------------------------------------------------------
void LinuxTaskExecutor::schedule (const Tasks::Queue& queue, Tasks::Task&& task) const
{
	if (queue == getMainQueue ())
	{
		if (impl->scheduleMainQueueTaskFunc)
			impl->scheduleMainQueueTaskFunc (std::move (task));
	}
	else if (queue == getBackgroundQueue ())
	{
		impl->threadPool.enqueue (std::move (task));
	}
	else
	{
		std::lock_guard<std::mutex> lock (impl->serialQueueMutex);
		auto it = impl->findQueue (queue.identifier);
		if (it != impl->serialQueues.end ())
			(*it)->schedule (std::move (task));
	}
}

//------------------------------------------------------------------------
void LinuxTaskExecutor::waitAllTasksExecuted (const Tasks::Queue& queue) const
{
	if (queue == impl->backgroundQueue)
	{
		while (!impl->threadPool.empty ())
			std::this_thread::sleep_for (std::chrono::milliseconds (1));
	}
	else
	{
		std::lock_guard<std::mutex> lock (impl->serialQueueMutex);
		auto it = impl->findQueue (queue.identifier);
		if (it != impl->serialQueues.end ())
			impl->waitAllTasksExecuted (it);
	}
}

//------------------------------------------------------------------------
void LinuxTaskExecutor::waitAllTasksExecuted () const
{
	{
		std::lock_guard<std::mutex> lock (impl->serialQueueMutex);
		for (auto it = impl->serialQueues.begin (); it != impl->serialQueues.end (); ++it)
			impl->waitAllTasksExecuted (it);
	}
	waitAllTasksExecuted (impl->backgroundQueue);
}

//------------------------------------------------------------------------
} // VSTGUI
