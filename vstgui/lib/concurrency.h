// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "platform/iplatformconcurrency.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Concurrency {

//------------------------------------------------------------------------
/** Get main/UI serial queue.
 *
 *	Tasks scheduled on this queue are performed serially on the main/ui thread.
 */
const Queue& mainQueue ();

//------------------------------------------------------------------------
/** Get background concurrent queue.
 *
 *	Tasks scheduled on this queue are performed concurrently on background threads.
 *	The number of background threads are depending on the systems number of CPU cores.
 */
const Queue& backgroundQueue ();

//------------------------------------------------------------------------
/** Make a new serial queue.
 *
 *	Tasks scheduled on this queue are performed serially on a background thread.
 *
 *	@param name		the name of the serial queue (optional)
 *	@return 		a new serial queue
 */
QueuePtr makeSerialQueue (const char* name);

//------------------------------------------------------------------------
/** Schedule a task to be performed asynchronous on a queue.
 *
 *	Can be called from any thread, but should not be called from realtime constraint threads as it
 *	may involves locks and memory allocations
 *
 *	@param queue	on which queue to perform the task
 *	@param task		task to be performed
 */
void schedule (const Queue& queue, Task&& task);

//------------------------------------------------------------------------
/** Wait for all tasks to be executed on a queue
 *
 *	Can only be called from the main thread.
 *
 *	@param queue	on which queue to perform the task
 */
void waitAllTasksExecuted (const Queue& queue);

//------------------------------------------------------------------------
} // Concurrency
} // VSTGUI
