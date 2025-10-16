// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "platform/iplatformtaskexecutor.h"

//------------------------------------------------------------------------
namespace VSTGUI {
/** Simple Task Concurrency API.
 *
 *	The Task Concurrency API facilitates scheduling tasks for execution on background threads,
 *	either concurrently or sequentially. This API also enables the scheduling of tasks on the main
 *	thread from any other thread.
 *
 *	This API can be used while the VSTGUI library is initialized. Upon deinitialization, triggered
 *	by the mandatory exit() call, this API ensures that all scheduled tasks are completed prior to
 *	returning.
 *
 *	Please note that once a task has been dispatched, it cannot be canceled.
 */
namespace Tasks {

//------------------------------------------------------------------------
/** Get the main/UI serial queue.
 *
 *	Tasks scheduled on this queue are performed sequentially on the main/ui thread.
 *
 *	Can be called from any thread.
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
 *	Can be called from any thread.
 *
 *	@return the shared background queue
 */
const Queue& backgroundQueue ();

//------------------------------------------------------------------------
/** Make a new serial queue.
 *
 *	Tasks scheduled on this queue are executed sequentially on a background thread.
 *
 *	The caller owns the queue, and needs to release the queue via the releaseSerialQueue() function.
 *
 *	Can be called from any thread.
 *
 *	@param name		the name of the serial queue (optional)
 *	@return 		a new serial queue
 */
Queue makeSerialQueue (const char* name);

//------------------------------------------------------------------------
/** Release a serial queue
 *
 *	This function will block until all tasks of the queue were executed.
 *
 *	Can be called from any thread.
 *
 *	@param queue	the queue which to release
 */
void releaseSerialQueue (const Queue& queue);

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
} // Tasks
} // VSTGUI
