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

//-----------------------------------------------------------------------------
const char* CVSTGUITimer::kMsgTimer = "timer fired";

//-----------------------------------------------------------------------------
CVSTGUITimer::CVSTGUITimer (CView* timerView, int fireTime)
: fireTime (fireTime)
, timerView (timerView)
{
	#if MAC
	timerRef = 0;
	#elif WINDOWS
	// todo
	#endif
}

//-----------------------------------------------------------------------------
CVSTGUITimer::~CVSTGUITimer ()
{
	stop ();
}

//-----------------------------------------------------------------------------
void CVSTGUITimer::start ()
{
	#if MAC
	if (!timerRef)
		InstallEventLoopTimer (GetMainEventLoop (), kEventDurationMillisecond * fireTime, kEventDurationMillisecond * fireTime, timerProc, this, &timerRef);
	#elif WINDOWS
	// todo
	#endif
}

//-----------------------------------------------------------------------------
bool CVSTGUITimer::stop ()
{
	#if MAC
	if (timerRef)
	{
		RemoveEventLoopTimer (timerRef);
		timerRef = 0;
		return true;
	}
	#elif WINDOWS
	// todo
	#endif
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
			start ();
	}
	return true;
}

#if MAC
//-----------------------------------------------------------------------------
pascal void CVSTGUITimer::timerProc (EventLoopTimerRef inTimer, void *inUserData)
{
	CVSTGUITimer* timer = (CVSTGUITimer*)inUserData;
	if (timer->timerView)
		timer->timerView->notify (NULL, kMsgTimer);
}

#endif

