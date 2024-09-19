// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "concurrency.h"
#include "platform/platformfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Concurrency {

//------------------------------------------------------------------------
const Queue& mainQueue () { return getPlatformFactory ().getConcurrency ().getMainQueue (); }

//------------------------------------------------------------------------
const Queue& backgroundQueue ()
{
	return getPlatformFactory ().getConcurrency ().getBackgroundQueue ();
}

//------------------------------------------------------------------------
QueuePtr makeSerialQueue (const char* name)
{
	return getPlatformFactory ().getConcurrency ().makeSerialQueue (name);
}

//------------------------------------------------------------------------
void schedule (const Queue& queue, Task&& task)
{
	return getPlatformFactory ().getConcurrency ().schedule (queue, std::move (task));
}

//------------------------------------------------------------------------
} // Concurrency
} // VSTGUI
