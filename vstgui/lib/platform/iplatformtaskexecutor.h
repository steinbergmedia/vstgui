// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <functional>
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Tasks {

//------------------------------------------------------------------------
struct Queue final
{
	const uint64_t identifier;
};
static constexpr Queue InvalidQueue = Queue {std::numeric_limits<uint64_t>::max ()};

//------------------------------------------------------------------------
inline bool operator== (const Queue& q1, const Queue& q2) noexcept
{
	return q1.identifier == q2.identifier;
}

//------------------------------------------------------------------------
inline bool operator!= (const Queue& q1, const Queue& q2) noexcept
{
	return q1.identifier != q2.identifier;
}

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
	virtual Tasks::Queue makeSerialQueue (const char* name) const = 0;
	virtual void releaseSerialQueue (const Tasks::Queue& queue) const = 0;
	virtual void schedule (const Tasks::Queue& queue, Tasks::Task&& task) const = 0;
	virtual void waitAllTasksExecuted (const Tasks::Queue& queue) const = 0;
	virtual void waitAllTasksExecuted () const = 0;
};

using PlatformTaskExecutorPtr = std::unique_ptr<IPlatformTaskExecutor>;

//------------------------------------------------------------------------
} // VSTGUI
