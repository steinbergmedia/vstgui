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
QueuePtr makeSerialQueue (const char* name)
{
	return getPlatformFactory ().getTaskExecutor ().makeSerialQueue (name);
}

//------------------------------------------------------------------------
void schedule (const Queue& queue, Task&& task)
{
	return getPlatformFactory ().getTaskExecutor ().schedule (queue, std::move (task));
}

//------------------------------------------------------------------------
} // Tasks
} // VSTGUI
