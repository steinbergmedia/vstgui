// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtaskexecutor.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct MacTaskExecutor final : IPlatformTaskExecutor
{
	MacTaskExecutor ();
	~MacTaskExecutor () noexcept override;

	const Tasks::Queue& getMainQueue () const final;
	const Tasks::Queue& getBackgroundQueue () const final;
	Tasks::QueuePtr makeSerialQueue (const char* name) const final;
	void schedule (const Tasks::Queue& queue, Tasks::Task&& task) const final;
	void waitAllTasksExecuted (const Tasks::Queue& queue) const final;
	void waitAllTasksExecuted () const final;

private:
	std::unique_ptr<Tasks::Queue> mainQueue;
	std::unique_ptr<Tasks::Queue> backgroundQueue;
};

//------------------------------------------------------------------------
} // VSTGUI
