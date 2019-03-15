// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../include/iasync.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
} // GDK
} // Platform

//------------------------------------------------------------------------
namespace Async {

//------------------------------------------------------------------------
struct Queue
{
	void schedule (Task&& task)
	{
		// TODO: scheduling not implemented yet
		task ();
	}
};

//------------------------------------------------------------------------
const QueuePtr& mainQueue ()
{
	static QueuePtr q = std::make_shared<Queue> ();
	return q;
}

//------------------------------------------------------------------------
const QueuePtr& backgroundQueue ()
{
	static QueuePtr q = std::make_shared<Queue> ();
	return q;
}

//------------------------------------------------------------------------
QueuePtr makeSerialQueue (const char* name)
{
	return std::make_shared<Queue> ();
}

//------------------------------------------------------------------------
void schedule (QueuePtr queue, Task&& task)
{
	queue->schedule (std::move (task));
}

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI
