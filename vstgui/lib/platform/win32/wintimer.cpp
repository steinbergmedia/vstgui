// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../iplatformtimer.h"

#if WINDOWS
#include <windows.h>
#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class WinTimer : public IPlatformTimer
{
public:
	WinTimer (IPlatformTimerCallback* callback);
	~WinTimer () noexcept;

	bool start (uint32_t fireTime) override;
	bool stop () override;
private:
	using TimerMap = std::map<UINT_PTR, IPlatformTimerCallback*>;
	static TimerMap gTimerMap;

	static VOID CALLBACK TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	UINT_PTR timer;
	IPlatformTimerCallback* callback;
};

//-----------------------------------------------------------------------------
SharedPointer<IPlatformTimer> IPlatformTimer::create (IPlatformTimerCallback* callback)
{
	return owned<IPlatformTimer> (new WinTimer (callback));
}

//-----------------------------------------------------------------------------
WinTimer::TimerMap WinTimer::gTimerMap;

//-----------------------------------------------------------------------------
WinTimer::WinTimer (IPlatformTimerCallback* callback)
: callback (callback)
, timer (0)
{
}

//-----------------------------------------------------------------------------
WinTimer::~WinTimer () noexcept
{
	stop ();
}

//-----------------------------------------------------------------------------
bool WinTimer::start (uint32_t fireTime)
{
	if (timer)
		return false;

	timer = SetTimer ((HWND)NULL, (UINT_PTR)0, fireTime, TimerProc);
	if (timer)
		gTimerMap.emplace (timer, callback);

	return false;
}

//-----------------------------------------------------------------------------
bool WinTimer::stop ()
{
	if (timer)
	{
		KillTimer ((HWND)NULL, timer);
		if (!gTimerMap.empty ())
		{
			TimerMap::const_iterator it = gTimerMap.find (timer);
			if (it != gTimerMap.end ())
				gTimerMap.erase (it);
		}
		timer = 0;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
VOID CALLBACK WinTimer::TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	TimerMap::const_iterator it = gTimerMap.find (idEvent);
	if (it != gTimerMap.end ())
		(*it).second->fire ();
}

}

#endif
