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

using PeriodicTimer = std::list<SharedPointer<Timer>>;
static PeriodicTimer gPeriodicTimers;

//------------------------------------------------------------------------
void Timer::checkAndFireTimers ()
{
	uint64_t currentTimeMs = Platform::getCurrentTimeMs ();

	for (auto it = gPeriodicTimers.begin (); it != gPeriodicTimers.end ();)
	{
		auto timer = *it;
		if (timer->getNextFireTime () <= currentTimeMs)
		{
			if (timer->fire ())
				timer->setNextFireTime (currentTimeMs + timer->getPeriodMs ());
			else
			{
#if LOG_TIMER
				std::cout << "unregisterTimer " << timer << " " << timer->getPeriodMs () << " "
						  << gPeriodicTimers.size () - 1 << "\n";
#endif
				it = gPeriodicTimers.erase (it);
				continue;
			}
		}
		++it;
	}
}

//------------------------------------------------------------------------
Timer::Timer (IPlatformTimerCallback* _callback)
{
	callback = _callback;
}

//------------------------------------------------------------------------
bool Timer::start (uint32_t _periodMs)
{
	periodMs = _periodMs;
	nextFireTime = Platform::getCurrentTimeMs () + periodMs;

	gPeriodicTimers.push_back (this);

	return true;
}

//------------------------------------------------------------------------
bool Timer::stop ()
{
	callback = nullptr;
	return true;
}

//------------------------------------------------------------------------
bool Timer::fire ()
{
	if (!callback)
		return false;
	callback->fire ();
	return true;
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
