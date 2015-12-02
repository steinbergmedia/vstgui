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

#if WINDOWS
#include <windows.h>
#include <list>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class WinTimer : public IPlatformTimer
{
public:
	WinTimer (IPlatformTimerCallback* callback);
	~WinTimer ();

	bool start (uint32_t fireTime) VSTGUI_OVERRIDE_VMETHOD;
	bool stop () VSTGUI_OVERRIDE_VMETHOD;
private:
	typedef std::map<UINT_PTR, IPlatformTimerCallback*> TimerMap;
	static TimerMap gTimerMap;

	static VOID CALLBACK TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	UINT_PTR timer;
	IPlatformTimerCallback* callback;
};

//-----------------------------------------------------------------------------
IPlatformTimer* IPlatformTimer::create (IPlatformTimerCallback* callback)
{
	return new WinTimer (callback);
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
WinTimer::~WinTimer ()
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
		gTimerMap.insert (std::make_pair (timer, callback));

	return false;
}

//-----------------------------------------------------------------------------
bool WinTimer::stop ()
{
	if (timer)
	{
		KillTimer ((HWND)NULL, timer);
		TimerMap::const_iterator it = gTimerMap.find (timer);
		if (it != gTimerMap.end ())
			gTimerMap.erase (it);

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
