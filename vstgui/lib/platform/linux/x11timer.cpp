//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2016, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "x11timer.h"
#include "x11platform.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
IPlatformTimer* IPlatformTimer::create (IPlatformTimerCallback* callback)
{
	return new X11::Timer (callback);
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
