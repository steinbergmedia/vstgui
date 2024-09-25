// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "linuxtaskexecutor.h"

//------------------------------------------------------------------------
namespace VSTGUI {

// TODO: This is currently a dummy implementation which executes the tasks directly

//------------------------------------------------------------------------
struct LinuxTaskExecutor::Queue
{
	Queue (uint64_t qId) : queueID ({qId}) {}
	virtual ~Queue () noexcept = default;
	virtual void schedule (Tasks::Task&& task) const = 0;

	Tasks::Queue queueID;
};

//------------------------------------------------------------------------
struct LinuxTaskExecutor::MainQueue : LinuxTaskExecutor::Queue
{
	using Queue::Queue;
	void schedule (Tasks::Task&& task) const final { task (); }
};

//------------------------------------------------------------------------
struct LinuxTaskExecutor::BackgroundQueue : LinuxTaskExecutor::Queue
{
	using Queue::Queue;
	void schedule (Tasks::Task&& task) const final { task (); }
};

//------------------------------------------------------------------------
LinuxTaskExecutor::LinuxTaskExecutor ()
{
	mainQueue = std::make_unique<MainQueue> (0u);
	backgroundQueue = std::make_unique<BackgroundQueue> (1u);
}

//------------------------------------------------------------------------
LinuxTaskExecutor::~LinuxTaskExecutor () noexcept {}

//------------------------------------------------------------------------
const Tasks::Queue& LinuxTaskExecutor::getMainQueue () const { return mainQueue->queueID; }

//------------------------------------------------------------------------
const Tasks::Queue& LinuxTaskExecutor::getBackgroundQueue () const
{
	return backgroundQueue->queueID;
}

//------------------------------------------------------------------------
Tasks::Queue LinuxTaskExecutor::makeSerialQueue (const char* name) const { return {3}; }

void LinuxTaskExecutor::releaseSerialQueue (const Tasks::Queue& queue) const {}

//------------------------------------------------------------------------
void LinuxTaskExecutor::schedule (const Tasks::Queue& queue, Tasks::Task&& task) const
{
	const Queue* q = nullptr;
	if (queue.identifier == mainQueue->queueID.identifier)
		q = mainQueue.get ();
	else if (queue.identifier == backgroundQueue->queueID.identifier)
		q = backgroundQueue.get ();
	if (q)
		q->schedule (std::move (task));
}

//------------------------------------------------------------------------
void LinuxTaskExecutor::waitAllTasksExecuted (const Tasks::Queue& queue) const {}

//------------------------------------------------------------------------
void LinuxTaskExecutor::waitAllTasksExecuted () const {}

//------------------------------------------------------------------------
} // VSTGUI
