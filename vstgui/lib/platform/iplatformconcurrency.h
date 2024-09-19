// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <functional>
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Concurrency {

struct Queue;
using QueuePtr = std::shared_ptr<Queue>;

using Task = std::function<void ()>;

//------------------------------------------------------------------------
} // Concurrency

//------------------------------------------------------------------------
class IPlatformConcurrency
{
public:
	virtual ~IPlatformConcurrency () noexcept = default;

	virtual const Concurrency::Queue& getMainQueue () const = 0;
	virtual const Concurrency::Queue& getBackgroundQueue () const = 0;
	virtual Concurrency::QueuePtr makeSerialQueue (const char* name) const = 0;
	virtual void schedule (const Concurrency::Queue& queue, Concurrency::Task&& task) const = 0;
	virtual void waitAllTasksExecuted (const Concurrency::Queue& queue) const = 0;
	virtual void waitAllTasksExecuted () const = 0;
};

//------------------------------------------------------------------------
} // VSTGUI
