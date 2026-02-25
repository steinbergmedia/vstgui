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
	Tasks::Queue queue;

	Queue (const Tasks::Queue& q) : queue (q) {}
	virtual ~Queue () noexcept = default;
	const Tasks::Queue& get () const { return queue; }
};

//------------------------------------------------------------------------
struct SerialQueue : Queue
{
	using Queue::Queue;
	~SerialQueue () noexcept { Tasks::releaseSerialQueue (get ()); }
};

//------------------------------------------------------------------------
const QueuePtr& mainQueue ()
{
	static QueuePtr q = std::make_shared<Queue> (Tasks::mainQueue ());
	return q;
}

//------------------------------------------------------------------------
const QueuePtr& backgroundQueue ()
{
	static QueuePtr q = std::make_shared<Queue> (Tasks::backgroundQueue ());
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
