// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../include/iasync.h"
#include "../../lib/concurrency.h"

//------------------------------------------------------------------------
namespace VSTGUI::Standalone::Async {

// Compatibility Layer to support previous API

//------------------------------------------------------------------------
struct Queue
{
	virtual ~Queue () noexcept = default;

	virtual const Concurrency::Queue& get () const = 0;
};

//------------------------------------------------------------------------
struct SimpleQueue : Queue
{
	const Concurrency::Queue* queue {nullptr};

	SimpleQueue (const Concurrency::Queue& q) : queue (&q) {}
	const Concurrency::Queue& get () const { return *queue; }
};

//------------------------------------------------------------------------
struct SerialQueue : Queue
{
	Concurrency::QueuePtr queue;

	SerialQueue (Concurrency::QueuePtr&& queue) : queue (std::move (queue)) {}
	const Concurrency::Queue& get () const { return *queue.get (); }
};

//------------------------------------------------------------------------
const QueuePtr& mainQueue ()
{
	static QueuePtr q = std::make_shared<SimpleQueue> (Concurrency::mainQueue ());
	return q;
}

//------------------------------------------------------------------------
const QueuePtr& backgroundQueue ()
{
	static QueuePtr q = std::make_shared<SimpleQueue> (Concurrency::backgroundQueue ());
	return q;
}

//------------------------------------------------------------------------
QueuePtr makeSerialQueue (const char* name)
{
	return std::make_shared<SerialQueue> (Concurrency::makeSerialQueue (name));
}

//------------------------------------------------------------------------
void schedule (QueuePtr queue, Task&& task)
{
	Concurrency::schedule (queue->get (), std::move (task));
}

//------------------------------------------------------------------------
} // namespace VSTGUI::Standalone::Async
