// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <functional>
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Tasks {

struct Queue;
using QueuePtr = std::shared_ptr<Queue>;

using Task = std::function<void ()>;

//------------------------------------------------------------------------
} // Tasks

//------------------------------------------------------------------------
class IPlatformTaskExecutor
{
public:
	virtual ~IPlatformTaskExecutor () noexcept = default;

	virtual const Tasks::Queue& getMainQueue () const = 0;
	virtual const Tasks::Queue& getBackgroundQueue () const = 0;
	virtual Tasks::QueuePtr makeSerialQueue (const char* name) const = 0;
	virtual void schedule (const Tasks::Queue& queue, Tasks::Task&& task) const = 0;
	virtual void waitAllTasksExecuted (const Tasks::Queue& queue) const = 0;
	virtual void waitAllTasksExecuted () const = 0;
};

//------------------------------------------------------------------------
} // VSTGUI
