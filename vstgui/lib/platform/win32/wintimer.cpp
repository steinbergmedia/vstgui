// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "wintimer.h"

#if WINDOWS
#include <map>
#include <windows.h>

namespace VSTGUI {
namespace WinTimerPrivate {

using TimerMap = std::map<UINT_PTR, IPlatformTimerCallback*>;
static TimerMap gTimerMap;

static VOID CALLBACK TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

} // WinTimerPrivate

//-----------------------------------------------------------------------------
WinTimer::WinTimer (IPlatformTimerCallback* callback) : timer (0), callback (callback)
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

	timer = SetTimer (nullptr, (UINT_PTR)0, fireTime, WinTimerPrivate::TimerProc);
	if (timer)
		WinTimerPrivate::gTimerMap.emplace (static_cast<UINT_PTR> (timer), callback);

	return false;
}

//-----------------------------------------------------------------------------
bool WinTimer::stop ()
{
	if (timer)
	{
		KillTimer ((HWND) nullptr, static_cast<UINT_PTR> (timer));
		if (!WinTimerPrivate::gTimerMap.empty ())
		{
			auto it = WinTimerPrivate::gTimerMap.find (static_cast<UINT_PTR> (timer));
			if (it != WinTimerPrivate::gTimerMap.end ())
				WinTimerPrivate::gTimerMap.erase (it);
		}
		timer = 0;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
VOID CALLBACK WinTimerPrivate::TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	TimerMap::const_iterator it = gTimerMap.find (idEvent);
	if (it != gTimerMap.end ())
		(*it).second->fire ();
}
}
#endif
