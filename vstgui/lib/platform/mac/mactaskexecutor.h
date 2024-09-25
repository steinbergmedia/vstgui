// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtaskexecutor.h"
#include <vector>
#include <mutex>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct MacTaskExecutor final : IPlatformTaskExecutor
{
	MacTaskExecutor ();
	~MacTaskExecutor () noexcept override;

	const Tasks::Queue& getMainQueue () const final;
	const Tasks::Queue& getBackgroundQueue () const final;
	Tasks::Queue makeSerialQueue (const char* name) const final;
	void releaseSerialQueue (const Tasks::Queue& queue) const final;
	void schedule (const Tasks::Queue& queue, Tasks::Task&& task) const final;
	void waitAllTasksExecuted (const Tasks::Queue& queue) const final;
	void waitAllTasksExecuted () const final;

private:
	struct DispatchQueue;
	using DispatchQueuePtr = std::unique_ptr<DispatchQueue>;

	const DispatchQueue* dispatchQueueFromQueueID (const Tasks::Queue& queue) const;

	DispatchQueuePtr mainQueue;
	DispatchQueuePtr backgroundQueue;
	mutable std::vector<DispatchQueuePtr> serialQueues;
	mutable std::mutex serialQueueLock;
	mutable uint64_t nextSerialQueueId {2};
};

//------------------------------------------------------------------------
} // VSTGUI
