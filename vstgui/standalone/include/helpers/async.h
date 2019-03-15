// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iasync.h"
#include <atomic>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Async {

//------------------------------------------------------------------------
/** Group of asynchronous tasks. */
struct Group final : std::enable_shared_from_this<Group>
{
	/** Create a new group.
	 *
	 *	Note that all calls to the group must be from one thread. If you want to call them from
	 *	different threads, you have to lock the access of it with a mutex yourself.
	 *
	 *	@param queue	the queue where to schedule the groups tasks
	 *	@return			a shared pointer to the new group
	 */
	static GroupPtr make (QueuePtr queue) { return GroupPtr (new Group (queue)); }

	/** Add a task to the group.
	 *
	 *	If the group was started, new tasks cannot be added.
	 *
	 *	@param task the task to add
	 *	@return true on success
	 */
	template <typename T>
	bool add (T&& task)
	{
		if (started == true)
			return false;
		unscheduledTasks.emplace_back (std::forward<T> (task));
		return true;
	}

	/** Start the groups tasks
	 *
	 *	A group can only be started once. The optional finishTask is performed after all tasks in
	 *	this group have executed. The finish task will execute on the same queue as the tasks.
	 *
	 *	@param finishTask an optional task to run after all group tasks were executed.
	 *	@return true on success
	 */
	bool start (Task&& finishTask = nullptr)
	{
		if (started == true)
			return false;
		started = true;
		finalizerTask = std::move (finishTask);
		if (unscheduledTasks.empty ())
		{
			if (finalizerTask)
				schedule (queue, std::move (finalizerTask));
			return true;
		}
		taskCounter.store (unscheduledTasks.size ());
		for (auto& t : unscheduledTasks)
		{
			schedule (queue, [task = std::move (t), g = shared_from_this ()] () {
				task ();
				g->taskDone ();
			});
		}
		unscheduledTasks.clear ();
		return true;
	}

private:
	Group (QueuePtr queue) : queue (queue) {}

	void taskDone ()
	{
		if (--taskCounter == 0 && finalizerTask)
		{
			finalizerTask ();
			finalizerTask = nullptr;
		}
	}

	QueuePtr queue;
	Task finalizerTask;
	std::vector<Task> unscheduledTasks;
	std::atomic<size_t> taskCounter {0};
	bool started {false};
};

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI
