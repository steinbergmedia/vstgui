//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 3.6
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __plugguieditor__
#include "plugguieditor.h"
#endif

#define kIdleRate    100 // host idle rate in ms
#define kIdleRate2    50
#define kIdleRateMin   4 // minimum time between 2 idles in ms

#if WINDOWS
static struct tagMSG windowsMessage;
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
	void InitMachOLibrary ();
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
	void ExitMachOLibrary ();
	ExitMachOLibrary ();
	#endif
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::draw (ERect *ppErect)
{
	if (frame)
	{
		CDrawContext context (frame, NULL, systemWindow);
		if (ppErect)
		{
			CRect r (ppErect->left, ppErect->top, ppErect->right, ppErect->bottom);
			frame->drawRect (&context, r);
		}
		else
			frame->draw (&context);
	}
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
long PluginGUIEditor::knobMode = kCircularMode;

//-----------------------------------------------------------------------------
long PluginGUIEditor::setKnobMode (int val) 
{
	PluginGUIEditor::knobMode = val;
	return 1;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::wait (unsigned long ms)
{
	#if MAC
	unsigned long ticks;
	Delay (ms * 60 / 1000, &ticks);
	
	#elif WINDOWS
	Sleep (ms);

	#endif
}

//-----------------------------------------------------------------------------
unsigned long PluginGUIEditor::getTicks ()
{
	#if MAC
	return (TickCount () * 1000) / 60;
	
	#elif WINDOWS
	return (unsigned long)GetTickCount ();
	
	#endif

	return 0;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::doIdleStuff ()
{
	// get the current time
	unsigned long currentTicks = getTicks ();

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
long PluginGUIEditor::getRect (ERect **ppErect)
{
	*ppErect = &rect;
	return true;
}

#if MAC
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#include <dlfcn.h>
#include <CoreFoundation/CFBundle.h>
#include <string>

BEGIN_NAMESPACE_VSTGUI

void* gBundleRef = 0;

END_NAMESPACE_VSTGUI

#if USE_NAMESPACE
#define VSTGUI_BUNDLEREF VSTGUI::gBundleRef
#else
#define VSTGUI_BUNDLEREF gBundleRef
#endif

#if PLUGGUI_STANDALONE
void InitMachOLibrary ()
{
	VSTGUI_BUNDLEREF = CFBundleGetMainBundle ();
}

void ExitMachOLibrary () {}
#else
// -----------------------------------------------------------------------------
void InitMachOLibrary ();
void InitMachOLibrary ()
{
	Dl_info info;
	if (dladdr ((const void*)InitMachOLibrary, &info))
	{
		if (info.dli_fname)
		{
			std::string name;
			name.assign (info.dli_fname);
			for (int i = 0; i < 3; i++)
			{
				int delPos = name.find_last_of ('/');
				if (delPos == -1)
				{
					fprintf (stdout, "Could not determine bundle location.\n");
					return; // unexpected
				}
				name.erase (delPos, name.length () - delPos);
			}
			CFURLRef bundleUrl = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*)name.c_str (), name.length (), true);
			if (bundleUrl)
			{
				VSTGUI_BUNDLEREF = CFBundleCreate (0, bundleUrl);
				CFRelease (bundleUrl);
			}
		}
	}
}

// -----------------------------------------------------------------------------
void ExitMachOLibrary ();
void ExitMachOLibrary ()
{
	if (VSTGUI_BUNDLEREF)
		CFRelease (VSTGUI_BUNDLEREF);
}

#endif
#endif
