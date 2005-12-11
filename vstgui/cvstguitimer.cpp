//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// CVSTGUITimer written 2005 by Arne Scheffler
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cvstguitimer.h"
#include "vstgui.h"

#if WINDOWS
#include <windows.h>
#include <list>
BEGIN_NAMESPACE_VSTGUI
static std::list<CVSTGUITimer*> gTimerList;
END_NAMESPACE_VSTGUI
#endif

//-----------------------------------------------------------------------------
const char* CVSTGUITimer::kMsgTimer = "timer fired";

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (CBaseObject* timerObject, int fireTime)
: fireTime (fireTime)
, timerObject (timerObject)
, platformTimer (0)
{
}

//-----------------------------------------------------------------------------
CVSTGUITimer::~CVSTGUITimer ()
{
	stop ();
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::start ()
{
	if (platformTimer == 0)
	{
		#if MAC
		InstallEventLoopTimer (GetMainEventLoop (), kEventDurationMillisecond * fireTime, kEventDurationMillisecond * fireTime, timerProc, this, (EventLoopTimerRef*)&platformTimer);
		#elif WINDOWS
		platformTimer = (void*)SetTimer ((HWND)NULL, (UINT_PTR)this, fireTime, TimerProc);
		if (platformTimer)
			gTimerList.push_back (this);
		#endif
	}
	return (platformTimer != 0);
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::stop ()
{
	if (platformTimer)
	{
		#if MAC
		RemoveEventLoopTimer ((EventLoopTimerRef)platformTimer);
		#elif WINDOWS
		KillTimer ((HWND)NULL, (UINT_PTR)platformTimer);
		std::list<CVSTGUITimer*>::iterator it = gTimerList.begin ();
		while (it != gTimerList.end ())
		{
			if ((*it) == this)
			{
				gTimerList.remove (*it);
				break;
			}
			it++;
		}
		#endif
		platformTimer = 0;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::setFireTime (int newFireTime)
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

#if MAC
//-----------------------------------------------------------------------------
pascal void CVSTGUITimer::timerProc (EventLoopTimerRef inTimer, void *inUserData)
{
	CVSTGUITimer* timer = (CVSTGUITimer*)inUserData;
	if (timer->timerObject)
		timer->timerObject->notify (timer, kMsgTimer);
}

#elif WINDOWS
//------------------------------------------------------------------------
VOID CALLBACK CVSTGUITimer::TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	std::list<CVSTGUITimer*>::iterator it = gTimerList.begin ();
	while (it != gTimerList.end ())
	{
		if ((UINT_PTR)((*it)->platformTimer) == idEvent)
		{
			(*it)->timerObject->notify ((*it), kMsgTimer);
			break;
		}
		it++;
	}
}
#endif

