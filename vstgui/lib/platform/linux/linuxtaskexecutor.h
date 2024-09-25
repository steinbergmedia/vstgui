// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtaskexecutor.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct LinuxTaskExecutor final : IPlatformTaskExecutor
{
	LinuxTaskExecutor ();
	~LinuxTaskExecutor () noexcept override;

	const Tasks::Queue& getMainQueue () const final;
	const Tasks::Queue& getBackgroundQueue () const final;
	Tasks::Queue makeSerialQueue (const char* name) const final;
	void releaseSerialQueue (const Tasks::Queue& queue) const final;
	void schedule (const Tasks::Queue& queue, Tasks::Task&& task) const final;
	void waitAllTasksExecuted (const Tasks::Queue& queue) const final;
	void waitAllTasksExecuted () const final;

private:
	struct Queue;
	struct MainQueue;
	struct BackgroundQueue;
	std::unique_ptr<Queue> mainQueue;
	std::unique_ptr<Queue> backgroundQueue;
};

//------------------------------------------------------------------------
} // VSTGUI
