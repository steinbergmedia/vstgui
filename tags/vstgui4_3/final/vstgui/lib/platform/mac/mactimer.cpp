//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#include "../iplatformtimer.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class MacTimer : public IPlatformTimer
{
public:
	MacTimer (IPlatformTimerCallback* callback);
	~MacTimer ();

	bool start (uint32_t fireTime) VSTGUI_OVERRIDE_VMETHOD;
	bool stop () VSTGUI_OVERRIDE_VMETHOD;
private:
	static void timerCallback (CFRunLoopTimerRef timer, void *info);

	IPlatformTimerCallback* callback;
	CFRunLoopTimerRef timer;
};

//-----------------------------------------------------------------------------
IPlatformTimer* IPlatformTimer::create (IPlatformTimerCallback* callback)
{
	return new MacTimer (callback);
}

//-----------------------------------------------------------------------------
MacTimer::MacTimer (IPlatformTimerCallback* callback)
: callback (callback)
, timer (0)
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
	CFRunLoopTimerContext timerContext = {0};
	timerContext.info = callback;
	timer = CFRunLoopTimerCreate (kCFAllocatorDefault, CFAbsoluteTimeGetCurrent () + fireTime * 0.001f, fireTime * 0.001f, 0, 0, timerCallback, &timerContext);
	if (timer)
	{
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_8
	#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAX_OS_X_VERSION_10_8
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
		timer = 0;
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
