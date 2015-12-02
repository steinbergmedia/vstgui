/*  VSTGUI Tester Application

 *  Created by Arne Scheffler on Sat Mar 6 2004.
 *  Copyright (c) 2004 Arne Scheffler. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#if TARGET_API_MAC_CARBON
#define __CF_USE_FRAMEWORK_INCLUDES__
#include <Carbon/Carbon.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <iostream.h>

#define MAC 1
#define CARBON 1

#define LIB_INSTANCE	CFBundleRef
#define mainEntryPoint	"main_macho"
#define __stdcall

static LIB_INSTANCE LoadLibrary (const char* name);
static void* GetProcAddress (LIB_INSTANCE instance, const char* functionName);
static void FreeLibrary (LIB_INSTANCE instance);
pascal void EventLoopTimerProc (EventLoopTimerRef inTimer, void *inUserData);
#endif

#include "aeffectx.h"

//---------------------------------------------------------------------------------------------------------------------
static bool debugOutput = false;

//---------------------------------------------------------------------------------------------------------------------
class VSTGUITester
{
public:
	VSTGUITester (const char* plugPath = 0);
	~VSTGUITester ();

	void runEventLoop ();

	bool getPluginPath (char* pathPtr);

	// window methods	
	void createWindow ();
	void showWindow ();
	void closeWindow ();
	void destroyWindow ();
	void setWindowSize (short top, short left, short bottom, short right);

	// AEffect
	bool createInstance (const char* plugPath);
	void releaseInstance ();

	void idleEffect ();

	void openEffectEditor ();
	void closeEffectEditor ();

	static long VSTCALLBACK hostCallback (AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

protected:
	#if MAC
	static pascal OSStatus carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	#endif
	
	AEffect* effect;
	void* window;
	LIB_INSTANCE lib;
};

//---------------------------------------------------------------------------------------------------------------------
VSTGUITester::VSTGUITester (const char* plugPath)
: effect (0)
, window (0)
, lib (0)
{
	createInstance (plugPath);

	createWindow ();
	openEffectEditor ();
	showWindow ();
}

//---------------------------------------------------------------------------------------------------------------------
VSTGUITester::~VSTGUITester ()
{
	closeWindow ();
	closeEffectEditor ();
	destroyWindow ();
	releaseInstance ();
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::runEventLoop ()
{
	#if MAC
	EventLoopTimerRef timer;
	InstallEventLoopTimer (GetCurrentEventLoop (), 0, kEventDurationMillisecond * 20, EventLoopTimerProc, this, &timer);
	RunApplicationEventLoop ();
	RemoveEventLoopTimer (timer);
	#else
	#endif
}

//---------------------------------------------------------------------------------------------------------------------
bool VSTGUITester::getPluginPath (char* pathPtr)
{
	bool result = false;
	#if MAC
	NavDialogCreationOptions dialogOptions;
	NavGetDefaultDialogCreationOptions (&dialogOptions);
	dialogOptions.windowTitle = CFSTR ("Select a VST Plugin to Open");
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.optionFlags |= kNavSupportPackages;
	NavDialogRef dialogRef;
	if (NavCreateGetFileDialog (&dialogOptions, NULL, NULL, NULL, NULL, NULL, &dialogRef) == noErr)
	{
		NavDialogRun (dialogRef);
		NavReplyRecord navReply;
		if (NavDialogGetReply (dialogRef, &navReply) == noErr)
		{
			FSRef parentFSRef;
			AEKeyword theAEKeyword;
			DescType typeCode;
			Size actualSize;
		    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
			{
				if (FSRefMakePath (&parentFSRef, (unsigned char*)pathPtr, 1024) == noErr)
					result = true;
				
			}
		}
		NavDialogDispose (dialogRef);
	}

	#else
	#endif
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::createWindow ()
{
	#if MAC
	WindowRef macWindow;
	Rect r = {0,0,200,200};
	WindowAttributes attributes = kWindowStandardHandlerAttribute | kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute;
	if (CreateNewWindow (kDocumentWindowClass, attributes, &r, &macWindow) == noErr)
	{
		window = (void*)macWindow;
		const EventTypeSpec eventTypes[] = {	{ kEventClassWindow, kEventWindowClosed } };
		InstallWindowEventHandler (macWindow, VSTGUITester::carbonEventHandler, GetEventTypeCount (eventTypes), eventTypes, this, NULL);
	}
	#else
	#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::showWindow ()
{
	#if MAC
	if (window)
	{
		RepositionWindow ((WindowRef)window, NULL, kWindowCenterOnMainScreen);
		ShowWindow ((WindowRef)window);
	}
	#else
	#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::closeWindow ()
{
	#if MAC
	if (window)
		HideWindow ((WindowRef)window);
	#else
	#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::destroyWindow ()
{
	#if MAC
	if (window)
		ReleaseWindow ((WindowRef)window);
	#else
	#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::setWindowSize (short top, short left, short bottom, short right)
{
	#if MAC
	if (window)
		SizeWindow ((WindowRef)window, right - left, bottom - top, false);
	#else
	#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::openEffectEditor ()
{
	if (effect && window)
	{
		effect->dispatcher (effect, effEditOpen, 0, 0, window, 0.0f);
		short *rect = 0;
		effect->dispatcher (effect, effEditGetRect, 0, 0, &rect, 0.f);
		if (rect != 0)
			setWindowSize (rect[0], rect[1], rect[2], rect[3]);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::closeEffectEditor ()
{
	if (effect && window)
	{
		effect->dispatcher (effect, effEditClose, 0, 0, NULL, 0.0f);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::idleEffect ()
{
	if (effect)
		effect->dispatcher (effect, effEditIdle, 0, 0, NULL, 0.0f);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSTGUITester::createInstance (const char* plugPath)
{
	if (plugPath == 0)
	{
		char path[1024];
		if (getPluginPath (path))
			return createInstance (path);
		return false;
	}
	#if MAC
	struct stat fs;
	if (stat (plugPath, &fs) != 0)
		return createInstance (0);
	lib = LoadLibrary (plugPath);
	if (lib)
	{
		AEffect* (__stdcall* GetNewPlugInstance)(audioMasterCallback);
		GetNewPlugInstance=(AEffect*(__stdcall*)(audioMasterCallback))GetProcAddress (lib, mainEntryPoint);
		if (GetNewPlugInstance)
		{
			effect = GetNewPlugInstance (VSTGUITester::hostCallback);
			if (effect == NULL || effect->magic != kEffectMagic || !(effect->flags & effFlagsHasEditor))
			{
				FreeLibrary (lib);
				lib = 0;
				return false;
			}
			effect->dispatcher (effect, effSetSampleRate, 0, 0, NULL, 44100.f);
			return true;
		}
	}
	#else
	#endif
	return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSTGUITester::releaseInstance ()
{
	if (effect)
		effect->dispatcher (effect, effClose, 0, 0, NULL, 0.0f);
	effect = 0;
	if (lib)
		FreeLibrary (lib);
	lib = 0;
}

//---------------------------------------------------------------------------------------------------------------------
long VSTCALLBACK VSTGUITester::hostCallback (AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
	long retval=0;

	switch (opcode)
	{
		//VST 1.0 opcodes
		case audioMasterVersion:
			//Input values:
			//none

			//Return Value:
			//0 or 1 for old version
			//2 or higher for VST2.0 host?
			if (debugOutput)
				cout << "plug called audioMasterVersion" << endl;
			retval=2;
			break;

		case audioMasterAutomate:
			//Input values:
			//<index> parameter that has changed
			//<opt> new value

			//Return value:
			//not tested, always return 0

			//NB - this is called when the plug calls
			//setParameterAutomated

			if (debugOutput)
				cout << "plug called audioMasterAutomate" << endl;
			break;

		case audioMasterCurrentId:
			//Input values:
			//none

			//Return Value
			//the unique id of a plug that's currently loading
			//zero is a default value and can be safely returned if not known
			if (debugOutput)
				cout << "plug called audioMasterCurrentId" << endl;
			break;

		case audioMasterIdle:
			//Input values:
			//none

			//Return Value
			//not tested, always return 0

			//NB - idle routine should also call effEditIdle for all open editors
			if (debugOutput)
				cout << "plug called audioMasterIdle" << endl;
			break;

		case audioMasterPinConnected:
			//Input values:
			//<index> pin to be checked
			//<value> 0=input pin, non-zero value=output pin

			//Return values:
			//0=true, non-zero=false
			if (debugOutput)
				cout << "plug called audioMasterPinConnected" << endl;
			break;

		//VST 2.0 opcodes
		case audioMasterWantMidi:
			//Input Values:
			//<value> filter flags (which is currently ignored, no defined flags?)

			//Return Value:
			//not tested, always return 0
			if (debugOutput)
				cout << "plug called audioMasterWantMidi" << endl;
			break;

		case audioMasterGetTime:
			//Input Values:
			//<value> should contain a mask indicating which fields are required
			//...from the following list?
			//kVstNanosValid
			//kVstPpqPosValid
			//kVstTempoValid
			//kVstBarsValid
			//kVstCyclePosValid
			//kVstTimeSigValid
			//kVstSmpteValid
			//kVstClockValid

			//Return Value:
			//ptr to populated const VstTimeInfo structure (or 0 if not supported)

			//NB - this structure will have to be held in memory for long enough
			//for the plug to safely make use of it
			if (debugOutput)
				cout << "plug called audioMasterGetTime" << endl;
			break;

		case audioMasterProcessEvents:
			//Input Values:
			//<ptr> Pointer to a populated VstEvents structure

			//Return value:
			//0 if error
			//1 if OK
			if (debugOutput)
				cout << "plug called audioMasterProcessEvents" << endl;
			break;

		case audioMasterSetTime:
			//IGNORE!
			break;

		case audioMasterTempoAt:
			//Input Values:
			//<value> sample frame location to be checked

			//Return Value:
			//tempo (in bpm * 10000)
			if (debugOutput)
				cout << "plug called audioMasterTempoAt" << endl;
			break;

		case audioMasterGetNumAutomatableParameters:
			//Input Values:
			//None

			//Return Value:
			//number of automatable parameters
			//zero is a default value and can be safely returned if not known

			//NB - what exactly does this mean? can the host set a limit to the
			//number of parameters that can be automated?
			if (debugOutput)
				cout << "plug called audioMasterGetNumAutomatableParameters" << endl;
			break;

		case audioMasterGetParameterQuantization:
			//Input Values:
			//None

			//Return Value:
			//integer value for +1.0 representation,
			//or 1 if full single float precision is maintained
			//in automation.

			//NB - ***possibly bugged***
			//Steinberg notes say "parameter index in <value> (-1: all, any)"
			//but in aeffectx.cpp no parameters are taken or passed
			if (debugOutput)
				cout << "plug called audioMasterGetParameterQuantization" << endl;
			break;

		case audioMasterIOChanged:
			//Input Values:
			//None

			//Return Value:
			//0 if error
			//non-zero value if OK
			if (debugOutput)
				cout << "plug called audioMasterIOChanged" << endl;
			break;

		case audioMasterNeedIdle:
			//Input Values:
			//None

			//Return Value:
			//0 if error
			//non-zero value if OK

			//NB plug needs idle calls (outside its editor window)
			//this means that effIdle must be dispatched to the plug
			//during host idle process and not effEditIdle calls only when its
			//editor is open
			//Check despatcher notes for any return codes from effIdle
			if (debugOutput)
				cout << "plug called audioMasterNeedIdle" << endl;
			break;

		case audioMasterSizeWindow:
			//Input Values:
			//<index> width
			//<value> height

			//Return Value:
			//0 if error
			//non-zero value if OK
			if (debugOutput)
				cout << "plug called audioMasterSizeWindow" << endl;
			break;

		case audioMasterGetSampleRate:
			//Input Values:
			//None

			//Return Value:
			//not tested, always return 0

			//NB - Host must despatch effSetSampleRate to the plug in response
			//to this call
			//Check despatcher notes for any return codes from effSetSampleRate
			if (debugOutput)
				cout << "plug called audioMasterGetSampleRate" << endl;
			effect->dispatcher(effect,effSetSampleRate,0,0,NULL,44100.);
			break;

		case audioMasterGetBlockSize:
			//Input Values:
			//None

			//Return Value:
			//not tested, always return 0

			//NB - Host must despatch effSetBlockSize to the plug in response
			//to this call
			//Check despatcher notes for any return codes from effSetBlockSize
			if (debugOutput)
				cout << "plug called audioMasterGetBlockSize" << endl;
			effect->dispatcher(effect,effSetBlockSize,0,512,NULL,0.0f);

			break;

		case audioMasterGetInputLatency:
			//Input Values:
			//None

			//Return Value:
			//input latency (in sampleframes?)
			if (debugOutput)
				cout << "plug called audioMasterGetInputLatency" << endl;
			break;

		case audioMasterGetOutputLatency:
			//Input Values:
			//None

			//Return Value:
			//output latency (in sampleframes?)
			if (debugOutput)
				cout << "plug called audioMasterGetOutputLatency" << endl;
			break;

		case audioMasterGetPreviousPlug:
			//Input Values:
			//None

			//Return Value:
			//pointer to AEffect structure or NULL if not known?

			//NB - ***possibly bugged***
			//Steinberg notes say "input pin in <value> (-1: first to come)"
			//but in aeffectx.cpp no parameters are taken or passed
			if (debugOutput)
				cout << "plug called audioMasterGetPreviousPlug" << endl;
			break;

		case audioMasterGetNextPlug:
			//Input Values:
			//None

			//Return Value:
			//pointer to AEffect structure or NULL if not known?

			//NB - ***possibly bugged***
			//Steinberg notes say "output pin in <value> (-1: first to come)"
			//but in aeffectx.cpp no parameters are taken or passed
			if (debugOutput)
				cout << "plug called audioMasterGetNextPlug" << endl;
			break;

		case audioMasterWillReplaceOrAccumulate:
			//Input Values:
			//None

			//Return Value:
			//0: not supported
			//1: replace
			//2: accumulate
			if (debugOutput)
				cout << "plug called audioMasterWillReplaceOrAccumulate" << endl;
			break;

		case audioMasterGetCurrentProcessLevel:
			//Input Values:
			//None

			//Return Value:
			//0: not supported,
			//1: currently in user thread (gui)
			//2: currently in audio thread (where process is called)
			//3: currently in 'sequencer' thread (midi, timer etc)
			//4: currently offline processing and thus in user thread
			//other: not defined, but probably pre-empting user thread.
			if (debugOutput)
				cout << "plug called audioMasterGetCurrentProcessLevel" << endl;
			break;

		case audioMasterGetAutomationState:
			//Input Values:
			//None

			//Return Value:
			//0: not supported
			//1: off
			//2:read
			//3:write
			//4:read/write
			if (debugOutput)
				cout << "plug called audioMasterGetAutomationState" << endl;
			break;

		case audioMasterGetVendorString:
			//Input Values:
			//<ptr> string (max 64 chars) to be populated

			//Return Value:
			//0 if error
			//non-zero value if OK
			if (debugOutput)
				cout << "plug called audioMasterGetVendorString" << endl;
			break;

		case audioMasterGetProductString:
			//Input Values:
			//<ptr> string (max 64 chars) to be populated

			//Return Value:
			//0 if error
			//non-zero value if OK
			if (debugOutput)
				cout << "plug called audioMasterGetProductString" << endl;
			break;

		case audioMasterGetVendorVersion:
			//Input Values:
			//None

			//Return Value:
			//Vendor specific host version as integer
			if (debugOutput)
				cout << "plug called audioMasterGetVendorVersion" << endl;
			break;

		case audioMasterVendorSpecific:
			//Input Values:
			//<index> lArg1
			//<value> lArg2
			//<ptr> ptrArg
			//<opt>	floatArg

			//Return Values:
			//Vendor specific response as integer
			if (debugOutput)
				cout << "plug called audioMasterVendorSpecific" << endl;
			break;

		case audioMasterSetIcon:
			//IGNORE
			break;

		case audioMasterCanDo:
			//Input Values:
			//<ptr> predefined "canDo" string

			//Return Value:
			//0 = Not Supported
			//non-zero value if host supports that feature

			//NB - Possible Can Do strings are:
			//"sendVstEvents",
			//"sendVstMidiEvent",
			//"sendVstTimeInfo",
			//"receiveVstEvents",
			//"receiveVstMidiEvent",
			//"receiveVstTimeInfo",
			//"reportConnectionChanges",
			//"acceptIOChanges",
			//"sizeWindow",
			//"asyncProcessing",
			//"offline",
			//"supplyIdle",
			//"supportShell"
			if (debugOutput)
				cout << "plug called audioMasterCanDo" << endl;

			if (strcmp((char*)ptr,"sendVstEvents")==0 ||
				strcmp((char*)ptr,"sendVstMidiEvent")==0 ||
				strcmp((char*)ptr,"supplyIdle")==0)
			{
				retval=1;
			}
			else
			{
				retval=0;
			}

			break;

		case audioMasterGetLanguage:
			//Input Values:
			//None

			//Return Value:
			//kVstLangEnglish
			//kVstLangGerman
			//kVstLangFrench
			//kVstLangItalian
			//kVstLangSpanish
			//kVstLangJapanese
			if (debugOutput)
				cout << "plug called audioMasterGetLanguage" << endl;
			retval=1;
			break;
/*
		MAC SPECIFIC?

		case audioMasterOpenWindow:
			//Input Values:
			//<ptr> pointer to a VstWindow structure

			//Return Value:
			//0 if error
			//else platform specific ptr
			cout << "plug called audioMasterOpenWindow" << endl;
			break;

		case audioMasterCloseWindow:
			//Input Values:
			//<ptr> pointer to a VstWindow structure

			//Return Value:
			//0 if error
			//Non-zero value if OK
			cout << "plug called audioMasterCloseWindow" << endl;
			break;
*/
		case audioMasterGetDirectory:
			//Input Values:
			//None

			//Return Value:
			//0 if error
			//FSSpec on MAC, else char* as integer

			//NB Refers to which directory, exactly?
			if (debugOutput)
				cout << "plug called audioMasterGetDirectory" << endl;
			break;

		case audioMasterUpdateDisplay:
			//Input Values:
			//None

			//Return Value:
			//Unknown
			if (debugOutput)
				cout << "plug called audioMasterUpdateDisplay" << endl;
			break;
		default:
			if (debugOutput)
				cout << "opcode not handled" << endl;
			break;
	}

	return retval;
}

//---------------------------------------------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
	char* pathPtr = 0;
	if (argc > 1)
		pathPtr = (char*)argv[1];

	VSTGUITester tester (pathPtr);
	tester.runEventLoop ();
	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#if MAC
pascal OSStatus VSTGUITester::carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	VSTGUITester* tester = (VSTGUITester*)inUserData;
	OSStatus result = eventNotHandledErr;
	EventClass eventClass = GetEventClass (inEvent);
	EventKind eventKind = GetEventKind (inEvent);

	switch (eventClass)
	{
		case kEventClassWindow:
		{
			switch (eventKind)
			{
				case kEventWindowClosed:
				{
					QuitApplicationEventLoop ();
					break;
				}
			}
			break;
		}
	} 
	
	return result;
}

pascal void EventLoopTimerProc (EventLoopTimerRef inTimer, void *inUserData)
{
	VSTGUITester* tester = (VSTGUITester*)inUserData;
	tester->idleEffect ();
}

LIB_INSTANCE LoadLibrary (const char* name)
{
	LIB_INSTANCE instance = 0;
	CFURLRef url = CFURLCreateFromFileSystemRepresentation (kCFAllocatorDefault, (const unsigned char*)name, strlen (name), false);
	if (url)
	{
		instance = CFBundleCreate (kCFAllocatorDefault, url);
		if (instance)
		{
			short resRef = CFBundleOpenBundleResourceMap (instance);
			UseResFile (resRef);
			if (CFBundleLoadExecutable (instance) == false)
			{
				CFRelease (instance);
				instance = 0;
			}
		}
		CFRelease (url);
	}
	return instance;
}

void* GetProcAddress (LIB_INSTANCE instance, const char* functionName)
{
	void* ptr = 0;
	CFStringRef str = CFStringCreateWithCString (kCFAllocatorDefault, functionName, kCFStringEncodingMacRoman);
	if (str)
	{
		ptr = CFBundleGetFunctionPointerForName (instance, str);
		CFRelease (str);
	}
	return ptr;
}

void FreeLibrary (LIB_INSTANCE instance)
{
	CFBundleUnloadExecutable (instance);
	CFRelease (instance);
}
#endif
