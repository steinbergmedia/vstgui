// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "tasks.h"
#include "platform/platformfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Tasks {

//------------------------------------------------------------------------
const Queue& mainQueue () { return getPlatformFactory ().getTaskExecutor ().getMainQueue (); }

//------------------------------------------------------------------------
const Queue& backgroundQueue ()
{
	return getPlatformFactory ().getTaskExecutor ().getBackgroundQueue ();
}

//------------------------------------------------------------------------
Queue makeSerialQueue (const char* name)
{
	return getPlatformFactory ().getTaskExecutor ().makeSerialQueue (name);
}

//------------------------------------------------------------------------
void releaseSerialQueue (const Queue& queue)
{
	getPlatformFactory ().getTaskExecutor ().releaseSerialQueue (queue);
}

//------------------------------------------------------------------------
void schedule (const Queue& queue, Task&& task)
{
	getPlatformFactory ().getTaskExecutor ().schedule (queue, std::move (task));
}

//------------------------------------------------------------------------
void waitAllTasksExecuted (const Queue& queue)
{
	getPlatformFactory ().getTaskExecutor ().waitAllTasksExecuted (queue);
}

//------------------------------------------------------------------------
} // Tasks
} // VSTGUI
