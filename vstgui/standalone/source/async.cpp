// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../include/iasync.h"
#include "../../lib/tasks.h"

//------------------------------------------------------------------------
namespace VSTGUI::Standalone::Async {

// Compatibility Layer to support previous API

//------------------------------------------------------------------------
struct Queue
{
	virtual ~Queue () noexcept = default;

	virtual const Tasks::Queue& get () const = 0;
};

//------------------------------------------------------------------------
struct SimpleQueue : Queue
{
	const Tasks::Queue* queue {nullptr};

	SimpleQueue (const Tasks::Queue& q) : queue (&q) {}
	const Tasks::Queue& get () const { return *queue; }
};

//------------------------------------------------------------------------
struct SerialQueue : Queue
{
	Tasks::QueuePtr queue;

	SerialQueue (Tasks::QueuePtr&& queue) : queue (std::move (queue)) {}
	const Tasks::Queue& get () const { return *queue.get (); }
};

//------------------------------------------------------------------------
const QueuePtr& mainQueue ()
{
	static QueuePtr q = std::make_shared<SimpleQueue> (Tasks::mainQueue ());
	return q;
}

//------------------------------------------------------------------------
const QueuePtr& backgroundQueue ()
{
	static QueuePtr q = std::make_shared<SimpleQueue> (Tasks::backgroundQueue ());
	return q;
}

//------------------------------------------------------------------------
QueuePtr makeSerialQueue (const char* name)
{
	return std::make_shared<SerialQueue> (Tasks::makeSerialQueue (name));
}

//------------------------------------------------------------------------
void schedule (QueuePtr queue, Task&& task) { Tasks::schedule (queue->get (), std::move (task)); }

//------------------------------------------------------------------------
} // namespace VSTGUI::Standalone::Async
