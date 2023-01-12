// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "mactimer.h"

#if MAC

namespace VSTGUI {

//-----------------------------------------------------------------------------
MacTimer::MacTimer (IPlatformTimerCallback* callback)
: callback (callback)
, timer (nullptr)
{
}

//-----------------------------------------------------------------------------
MacTimer::~MacTimer ()
{
	stop ();
}

//-----------------------------------------------------------------------------
bool MacTimer::start (uint32_t fireTime)
{
	if (timer)
		return false;
	CFRunLoopTimerContext timerContext = {};
	timerContext.info = callback;
	timer = CFRunLoopTimerCreate (kCFAllocatorDefault, CFAbsoluteTimeGetCurrent () + fireTime * 0.001f, fireTime * 0.001f, 0, 0, timerCallback, &timerContext);
	if (timer)
	{
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_8
	#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_8
		if (CFRunLoopTimerSetTolerance)
	#endif
			CFRunLoopTimerSetTolerance (timer, fireTime * 0.0001f);
#endif
		CFRunLoopAddTimer (CFRunLoopGetCurrent (), timer, kCFRunLoopCommonModes);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool MacTimer::stop ()
{
	if (timer)
	{
		CFRunLoopTimerInvalidate (timer);
		CFRelease (timer);
		timer = nullptr;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void MacTimer::timerCallback (CFRunLoopTimerRef t, void *info)
{
	IPlatformTimerCallback* timer = static_cast<IPlatformTimerCallback*> (info);
	timer->fire ();
}

}

#endif
