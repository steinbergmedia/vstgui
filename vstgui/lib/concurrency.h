// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "platform/iplatformconcurrency.h"

//------------------------------------------------------------------------
namespace VSTGUI {
/** Simple Task Concurrency API.
 *
 *	The task concurrency API allows scheduling tasks either concurrently or sequentially on
 *	background threads. Additionally, tasks running on background threads can be scheduled back to
 *	the main thread, for example, to notify when a task has been completed.
 *
 *	This API can be used while the VSTGUI library is initialized. Upon deinitialization (via the
 *	required exit() call), the exit() function ensures that all dispatched tasks are completed
 *	before returning.
 *
 *	Note that tasks cannot be canceled once they have been dispatched.
 */
namespace Concurrency {

//------------------------------------------------------------------------
/** Get the main/UI serial queue.
 *
 *	Tasks scheduled on this queue are performed sequentially on the main/ui thread.
 *
 *	@return the shared main queue
 */
const Queue& mainQueue ();

//------------------------------------------------------------------------
/** Get the background concurrent queue.
 *
 *	Tasks scheduled on this queue are performed concurrently on background threads.
 *	The number of background threads depends on the system's CPU core count.
 *
 *	@return the shared background queue
 */
const Queue& backgroundQueue ();

//------------------------------------------------------------------------
/** Make a new serial queue.
 *
 *	Tasks scheduled on this queue are executed sequentially on a background thread.
 *
 *	The caller owns the queue, and when the queue is destroyed, all remaining tasks are completed
 *	before the destroy call returns..
 *
 *	@param name		the name of the serial queue (optional)
 *	@return 		a new serial queue
 */
QueuePtr makeSerialQueue (const char* name);

//------------------------------------------------------------------------
/** Schedule a task to be executed asynchronously on a queue.
 *
 *	Can be called from any thread, but should not be called from realtime constraint threads as it
 *	may involves locks and memory allocations
 *
 *	@param queue	on which queue to perform the task
 *	@param task		task to be performed
 */
void schedule (const Queue& queue, Task&& task);

//------------------------------------------------------------------------
/** Wait for all tasks in the queue to complete execution.
 *
 *	Can only be called from the main thread.
 *
 *	@param queue	on which queue to perform the task
 */
void waitAllTasksExecuted (const Queue& queue);

//------------------------------------------------------------------------
} // Concurrency
} // VSTGUI
