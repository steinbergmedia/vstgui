// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "linuxconcurrency.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Concurrency {

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
} // Concurrency

//------------------------------------------------------------------------
LinuxConcurrency::LinuxConcurrency ()
{
	mainQueue = std::make_unique<Concurrency::MainQueue> ();
	backgroundQueue = std::make_unique<Concurrency::BackgroundQueue> ();
}

//------------------------------------------------------------------------
LinuxConcurrency::~LinuxConcurrency () noexcept {}

//------------------------------------------------------------------------
const Concurrency::Queue& LinuxConcurrency::getMainQueue () const { return *mainQueue.get (); }

//------------------------------------------------------------------------
const Concurrency::Queue& LinuxConcurrency::getBackgroundQueue () const
{
	return *backgroundQueue.get ();
}

//------------------------------------------------------------------------
Concurrency::QueuePtr LinuxConcurrency::makeSerialQueue (const char* name) const { return nullptr; }

//------------------------------------------------------------------------
void LinuxConcurrency::schedule (const Concurrency::Queue& queue, Concurrency::Task&& task) const
{
	queue.schedule (std::move (task));
}

//------------------------------------------------------------------------
void LinuxConcurrency::waitAllTasksExecuted (const Concurrency::Queue& queue) const {}

//------------------------------------------------------------------------
void LinuxConcurrency::waitAllTasksExecuted () const {}

//------------------------------------------------------------------------
} // VSTGUI
