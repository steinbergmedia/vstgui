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

#ifndef __cvstguitimer__
#define __cvstguitimer__

#include "vstguibase.h"
#include "platform/iplatformtimer.h"

#if VSTGUI_HAS_FUNCTIONAL
#include <functional>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CVSTGUITimer Declaration
//! A timer class, which posts timer messages to CBaseObjects or calls a lambda function (c++11 only).
//-----------------------------------------------------------------------------
class CVSTGUITimer : public CBaseObject, public IPlatformTimerCallback
{
public:
#if VSTGUI_HAS_FUNCTIONAL
	typedef std::function<void(CVSTGUITimer*)> CallbackFunc;

	CVSTGUITimer (const CallbackFunc& callback, uint32_t fireTime = 100, bool doStart = true);
	CVSTGUITimer (CallbackFunc&& callback, uint32_t fireTime = 100, bool doStart = true);
#endif
	CVSTGUITimer (CBaseObject* timerObject, uint32_t fireTime = 100, bool doStart = false);
	
	virtual bool start ();							///< starts the timer
	virtual bool stop ();							///< stops the timer, returns whether timer was running or not

	virtual bool setFireTime (uint32_t newFireTime);		///< in milliseconds
	uint32_t getFireTime () const { return fireTime; }		///< in milliseconds

//-----------------------------------------------------------------------------
	static IdStringPtr kMsgTimer;					///< message string posted to CBaseObject's notify method
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CVSTGUITimer, CBaseObject)
protected:
	~CVSTGUITimer ();
	
	void fire () VSTGUI_OVERRIDE_VMETHOD;
	
	uint32_t fireTime;
#if VSTGUI_HAS_FUNCTIONAL
	CallbackFunc callbackFunc;
#else
	CBaseObject* timerObject;
#endif

	SharedPointer<IPlatformTimer> platformTimer;
};

#if VSTGUI_HAS_FUNCTIONAL
namespace Call
{
	typedef std::function<void ()> FunctionCallback;

	/** Trigger a function call at a later timer */
	inline void later (FunctionCallback callback, uint32_t delayInMilliseconds = 10)
	{
		new CVSTGUITimer ([callback] (CVSTGUITimer* timer) {
			callback ();
			timer->forget ();
		}, delayInMilliseconds, true);
	}
};
#endif

} // namespace

#endif
