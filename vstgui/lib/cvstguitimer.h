// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "platform/iplatformtimer.h"
#include <functional>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CVSTGUITimer Declaration
//! A timer class, which posts timer messages to CBaseObjects or calls a lambda function (c++11 only).
//-----------------------------------------------------------------------------
class CVSTGUITimer final : public CBaseObject, public IPlatformTimerCallback
{
public:
	using CallbackFunc = std::function<void(CVSTGUITimer*)>;

	CVSTGUITimer (const CallbackFunc& callback, uint32_t fireTime = 100, bool doStart = true);
	CVSTGUITimer (CallbackFunc&& callback, uint32_t fireTime = 100, bool doStart = true);
	CVSTGUITimer (CBaseObject* timerObject, uint32_t fireTime = 100, bool doStart = false);
	
	/** starts the timer */
	bool start ();
	/** stops the timer, returns whether timer was running or not */
	bool stop ();

	/** set fire time in milliseconds */
	bool setFireTime (uint32_t newFireTime);
	/** get fire time in milliseconds*/
	uint32_t getFireTime () const { return fireTime; }

//-----------------------------------------------------------------------------
	/** message string posted to CBaseObject's notify method */
	static IdStringPtr kMsgTimer;
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CVSTGUITimer, CBaseObject)
protected:
	~CVSTGUITimer () noexcept override;

	void beforeDelete () override;
	void fire () override;
	
	uint32_t fireTime;
	CallbackFunc callbackFunc;

	PlatformTimerPtr platformTimer;
};

namespace Call
{
	using FunctionCallback = std::function<void ()>;

	/** Trigger a function call at a later timer */
	inline void later (FunctionCallback callback, uint32_t delayInMilliseconds = 10)
	{
		new CVSTGUITimer ([callback] (CVSTGUITimer* timer) {
			timer->stop ();
			callback ();
			timer->forget ();
		}, delayInMilliseconds, true);
	}
};

} // VSTGUI
