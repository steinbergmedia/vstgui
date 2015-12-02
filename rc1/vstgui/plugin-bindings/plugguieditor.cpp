//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
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

#ifndef __plugguieditor__
#include "plugguieditor.h"
#endif

#define kIdleRate    100 // host idle rate in ms
#define kIdleRate2    50
#define kIdleRateMin   4 // minimum time between 2 idles in ms

#if WINDOWS
#include <Windows.h>
#endif

#if MAC
#include <Carbon/Carbon.h>
static void InitMachOLibrary ();
static void ExitMachOLibrary ();
#endif

//-----------------------------------------------------------------------------
// PluginGUIEditor Implementation
//-----------------------------------------------------------------------------
/*! @class PluginGUIEditor
This is the same as the AEffGUIEditor class except that this one allows
the VSTGUI lib to build without VST dependencies.
*/
PluginGUIEditor::PluginGUIEditor (void *pEffect) 
	: effect (pEffect), inIdleStuff (false)
{
	systemWindow = 0;
	lLastTicks   = getTicks ();

	#if WINDOWS
	OleInitialize (0);
	#endif
	#if MAC
	InitMachOLibrary ();
	#endif
}

//-----------------------------------------------------------------------------
PluginGUIEditor::~PluginGUIEditor () 
{
	#if WINDOWS
	OleUninitialize ();
	#endif
	#if MAC
	ExitMachOLibrary ();
	#endif
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::draw (ERect *ppErect)
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	if (frame)
	{
		CRect r;
		if (ppErect)
			r (ppErect->left, ppErect->top, ppErect->right, ppErect->bottom);
		else
			r = frame->getViewSize ();
		CDrawContext* context = frame->createDrawContext ();
		if (context)
		{
			frame->drawRect (context, r);
			context->forget();
		}
	}
#endif
}

//-----------------------------------------------------------------------------
bool PluginGUIEditor::open (void *ptr)
{
	systemWindow = ptr;
	return true;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::idle ()
{
	if (frame)
		frame->idle ();
}

//-----------------------------------------------------------------------------
int32_t PluginGUIEditor::knobMode = kCircularMode;

//-----------------------------------------------------------------------------
int32_t PluginGUIEditor::setKnobMode (int32_t val) 
{
	PluginGUIEditor::knobMode = val;
	return 1;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::wait (uint32_t ms)
{
	#if MAC
	RunCurrentEventLoop (kEventDurationMillisecond * ms);
	
	#elif WINDOWS
	Sleep (ms);

	#endif
}

//-----------------------------------------------------------------------------
uint32_t PluginGUIEditor::getTicks ()
{
	#if MAC
	return (TickCount () * 1000) / 60;
	
	#elif WINDOWS
	return (uint32_t)GetTickCount ();
	
	#endif

	return 0;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::doIdleStuff ()
{
	// get the current time
	uint32_t currentTicks = getTicks ();

	// YG TEST idle ();
	if (currentTicks < lLastTicks)
	{
		#if (MAC && TARGET_API_MAC_CARBON)
		RunCurrentEventLoop (kEventDurationMillisecond * kIdleRateMin);
		#else
		wait (kIdleRateMin);
		#endif
		currentTicks += kIdleRateMin;
		if (currentTicks < lLastTicks - kIdleRate2)
			return;
	}
	idle (); // TEST

	#if WINDOWS
	struct tagMSG windowsMessage;
	if (PeekMessage (&windowsMessage, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
		DispatchMessage (&windowsMessage);

	#elif MAC && !__LP64__
	EventRef event;
	EventTypeSpec eventTypes[] = { {kEventClassWindow, kEventWindowUpdate}, {kEventClassWindow, kEventWindowDrawContent} };
	if (ReceiveNextEvent (GetEventTypeCount (eventTypes), eventTypes, kEventDurationNoWait, true, &event) == noErr)
	{
		SendEventToEventTarget (event, GetEventDispatcherTarget ());
		ReleaseEvent (event);
	}
	#endif

	// save the next time
 	lLastTicks = currentTicks + kIdleRate;
}

//-----------------------------------------------------------------------------
bool PluginGUIEditor::getRect (ERect **ppErect)
{
	*ppErect = &rect;
	return true;
}

#if MAC
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#include "getpluginbundle.h"

namespace VSTGUI {
	void* gBundleRef = 0;
}

// -----------------------------------------------------------------------------
void InitMachOLibrary ()
{
	VSTGUI::gBundleRef = GetPluginBundle ();
}

// -----------------------------------------------------------------------------
void ExitMachOLibrary ()
{
	if (VSTGUI::gBundleRef)
		CFRelease (VSTGUI::gBundleRef);
}

#endif
