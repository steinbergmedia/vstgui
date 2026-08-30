// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cvstguitimer.h"
#include "platform/platformfactory.h"
#include "platform/iplatformtimer.h"

#if DEBUG
#define DEBUGLOG	0
#endif

namespace VSTGUI {

//------------------------------------------------------------------------
struct CVSTGUITimer::PlatformCallbackImpl final : public IPlatformTimerCallback
{
	PlatformCallbackImpl (CVSTGUITimer& timer) : timer (timer) {}
	void fire () override { timer.fire (); }

	CVSTGUITimer& timer;
};

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer () { platformCallback = std::make_unique<PlatformCallbackImpl> (*this); }

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (const CallbackFunc& callback, uint32_t inFireTime, bool doStart)
: CVSTGUITimer ()
{
	fireTime = inFireTime;
	callbackFunc = callback;
	if (doStart)
		start ();
}

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (CallbackFunc&& callback, uint32_t inFireTime, bool doStart)
: CVSTGUITimer ()
{
	fireTime = inFireTime;
	callbackFunc = std::move (callback);
	if (doStart)
		start ();
}

//-----------------------------------------------------------------------------
CVSTGUITimer::~CVSTGUITimer () noexcept = default;

//-----------------------------------------------------------------------------
void CVSTGUITimer::beforeDelete ()
{
	stop ();
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::start ()
{
	if (platformTimer == nullptr)
	{
		platformTimer = getPlatformFactory ().createTimer (platformCallback.get ());
		if (platformTimer)
		{
			platformTimer->start (fireTime);
		#if DEBUGLOG
			DebugPrint ("Timer started (0x%x)\n", timerObject);
		#endif
		}
	}
	return (platformTimer != nullptr);
}

//------------------------------------------------------------------------
bool CVSTGUITimer::start (uint32_t inFireTime, CallbackFunc&& inCallback)
{
	if (platformTimer)
		platformTimer->stop ();
	fireTime = inFireTime;
	callbackFunc = std::move (inCallback);
	if (platformTimer == nullptr)
		platformTimer = getPlatformFactory ().createTimer (platformCallback.get ());
	if (platformTimer)
		platformTimer->start (fireTime);
	return (platformTimer != nullptr);
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::stop ()
{
	if (platformTimer)
	{
		platformTimer->stop ();
		platformTimer = nullptr;

		#if DEBUGLOG
		DebugPrint ("Timer stopped (0x%x)\n", timerObject);
		#endif
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::setFireTime (uint32_t newFireTime)
{
	if (fireTime != newFireTime)
	{
		bool wasRunning = stop ();
		fireTime = newFireTime;
		if (wasRunning)
			return start ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CVSTGUITimer::fire ()
{
	if (callbackFunc)
	{
		auto guard = makeLifeGuard (this);
		callbackFunc (this);
	}
}

//------------------------------------------------------------------------
} // VSTGUI
