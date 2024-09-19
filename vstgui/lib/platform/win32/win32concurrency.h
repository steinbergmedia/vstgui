// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformconcurrency.h"
#include "win32support.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct Win32Concurrency : IPlatformConcurrency
{
	Win32Concurrency ();
	~Win32Concurrency () noexcept;

	void init (HINSTANCE instance);

	const Concurrency::Queue& getMainQueue () const final;
	const Concurrency::Queue& getBackgroundQueue () const final;
	Concurrency::QueuePtr makeSerialQueue (const char* name) const final;
	void schedule (const Concurrency::Queue& queue, Concurrency::Task&& task) const final;
	void waitAllTasksExecuted (const Concurrency::Queue& queue) const final;
	void waitAllTasksExecuted () const final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
