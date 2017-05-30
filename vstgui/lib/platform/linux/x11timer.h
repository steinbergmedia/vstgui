// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtimer.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
class Timer : public IPlatformTimer
{
public:
	Timer (IPlatformTimerCallback* callback);

	bool start (uint32_t periodMs) override;
	bool stop () override;

	bool fire ();
	uint32_t getPeriodMs () const { return periodMs; }
	uint64_t getNextFireTime () const { return nextFireTime; }
	void setNextFireTime (uint64_t t) { nextFireTime = t; }

	static void checkAndFireTimers ();

private:
	IPlatformTimerCallback* callback = nullptr;
	uint32_t periodMs = 0;
	uint64_t nextFireTime = 0;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
