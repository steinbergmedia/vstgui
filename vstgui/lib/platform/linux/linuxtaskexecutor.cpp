// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "linuxconcurrency.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Tasks {

// TODO: This is currently a dummy implementation which executes the tasks directly

//------------------------------------------------------------------------
struct Queue
{
	virtual void schedule (Task&& task) const = 0;
};

//------------------------------------------------------------------------
struct MainQueue : Queue
{
	void schedule (Task&& task) const final { task (); }
};

//------------------------------------------------------------------------
struct BackgroundQueue : Queue
{
	void schedule (Task&& task) const final { task (); }
};

//------------------------------------------------------------------------
} // Tasks

//------------------------------------------------------------------------
LinuxTaskExecutor::LinuxTaskExecutor ()
{
	mainQueue = std::make_unique<Tasks::MainQueue> ();
	backgroundQueue = std::make_unique<Tasks::BackgroundQueue> ();
}

//------------------------------------------------------------------------
LinuxTaskExecutor::~LinuxTaskExecutor () noexcept {}

//------------------------------------------------------------------------
const Tasks::Queue& LinuxTaskExecutor::getMainQueue () const { return *mainQueue.get (); }

//------------------------------------------------------------------------
const Tasks::Queue& LinuxTaskExecutor::getBackgroundQueue () const { return *backgroundQueue.get (); }

//------------------------------------------------------------------------
Tasks::QueuePtr LinuxTaskExecutor::makeSerialQueue (const char* name) const { return nullptr; }

//------------------------------------------------------------------------
void LinuxTaskExecutor::schedule (const Tasks::Queue& queue, Tasks::Task&& task) const
{
	queue.schedule (std::move (task));
}

//------------------------------------------------------------------------
void LinuxTaskExecutor::waitAllTasksExecuted (const Tasks::Queue& queue) const {}

//------------------------------------------------------------------------
void LinuxTaskExecutor::waitAllTasksExecuted () const {}

//------------------------------------------------------------------------
} // VSTGUI
