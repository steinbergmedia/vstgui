// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11timer.h"
#include "x11platform.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
SharedPointer<IPlatformTimer> IPlatformTimer::create (IPlatformTimerCallback* callback)
{
	return owned (new X11::Timer (callback));
}

//------------------------------------------------------------------------
namespace X11 {

//------------------------------------------------------------------------
Timer::Timer (IPlatformTimerCallback* _callback)
{
	callback = _callback;
}

//------------------------------------------------------------------------
Timer::~Timer () noexcept
{
	stop ();
}

//------------------------------------------------------------------------
bool Timer::start (uint32_t periodMs)
{
	auto runLoop = RunLoop::get ();
	vstgui_assert (runLoop, "Timer only works of run loop was set");
	if (!runLoop)
		return false;
	return runLoop->registerTimer (periodMs, this);
}

//------------------------------------------------------------------------
bool Timer::stop ()
{
	auto runLoop = RunLoop::get ();
	vstgui_assert (runLoop, "Timer only works of run loop was set");
	if (!runLoop)
		return false;
	return runLoop->unregisterTimer (this);
}

//------------------------------------------------------------------------
void Timer::onTimer ()
{
	if (callback)
		callback->fire ();
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
