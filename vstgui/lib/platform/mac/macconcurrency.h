// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformconcurrency.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct MacConcurrency final : IPlatformConcurrency
{
	MacConcurrency ();
	~MacConcurrency () noexcept override;

	const Concurrency::Queue& getMainQueue () const final;
	const Concurrency::Queue& getBackgroundQueue () const final;
	Concurrency::QueuePtr makeSerialQueue (const char* name) const final;
	void schedule (const Concurrency::Queue& queue, Concurrency::Task&& task) const final;
	void waitAllTasksExecuted (const Concurrency::Queue& queue) const final;
	void waitAllTasksExecuted () const final;

private:
	std::unique_ptr<Concurrency::Queue> mainQueue;
	std::unique_ptr<Concurrency::Queue> backgroundQueue;
};

//------------------------------------------------------------------------
} // VSTGUI
