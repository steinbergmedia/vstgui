// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __plugguieditor__
#include "plugguieditor.h"
#endif

#define kIdleRate    100 // host idle rate in ms
#define kIdleRate2    50
#define kIdleRateMin   4 // minimum time between 2 idles in ms

#if WINDOWS
#include <windows.h>
#include <ole2.h>
#endif

#if MAC
#include <Carbon/Carbon.h>
#include "getpluginbundle.h"

namespace VSTGUI {
static void InitMachOLibrary ();
static void ExitMachOLibrary ();
} // VSTGUI
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
// PluginGUIEditor Implementation
//-----------------------------------------------------------------------------
/*! @class PluginGUIEditor
This is the same as the AEffGUIEditor class except that this one allows
the VSTGUI lib to build without VST dependencies.
*/
PluginGUIEditor::PluginGUIEditor (void *pEffect) 
	: effect (pEffect)
{
	systemWindow = nullptr;
	lLastTicks   = getTicks ();

	#if WINDOWS
	OleInitialize (nullptr);
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
	if (PeekMessage (&windowsMessage, nullptr, WM_PAINT, WM_PAINT, PM_REMOVE))
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

void* gBundleRef = 0;

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

} // VSTGUI
