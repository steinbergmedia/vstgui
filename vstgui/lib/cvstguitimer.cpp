// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cvstguitimer.h"

#if DEBUG
#define DEBUGLOG	0
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
IdStringPtr CVSTGUITimer::kMsgTimer = "timer fired";

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (CBaseObject* timerObject, uint32_t fireTime, bool doStart)
: fireTime (fireTime)
, platformTimer (nullptr)
{
	callbackFunc = [timerObject](CVSTGUITimer* timer) {
		timerObject->notify (timer, kMsgTimer);
	};
	if (doStart)
		start ();
}

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (const CallbackFunc& callback, uint32_t fireTime, bool doStart)
: fireTime (fireTime)
, callbackFunc (callback)
, platformTimer (nullptr)
{
	if (doStart)
		start ();
}

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (CallbackFunc&& callback, uint32_t fireTime, bool doStart)
: fireTime (fireTime)
, callbackFunc (std::move (callback))
, platformTimer (nullptr)
{
	if (doStart)
		start ();
}

//-----------------------------------------------------------------------------
CVSTGUITimer::~CVSTGUITimer () noexcept = default;

//-----------------------------------------------------------------------------
void CVSTGUITimer::beforeDelete ()
{
	stop ();
	CBaseObject::beforeDelete ();
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::start ()
{
	if (platformTimer == nullptr)
	{
		platformTimer = IPlatformTimer::create (this);
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
	CBaseObjectGuard guard (this);
	if (callbackFunc)
		callbackFunc (this);
}

} // namespace

