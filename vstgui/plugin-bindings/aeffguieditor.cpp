// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __aeffguieditor__
#include "aeffguieditor.h"
#endif

#ifndef __audioeffectx__
#include "public.sdk/source/vst2.x/audioeffectx.h"
#endif

#define kIdleRate    100 // host idle rate in ms
#define kIdleRate2    50
#define kIdleRateMin   4 // minimum time between 2 idles in ms

#if WINDOWS
#include <windows.h>
#include "../lib/platform/win32/win32support.h"
#endif

#if MAC
#include <Carbon/Carbon.h>
static void InitMachOLibrary ();
static void ExitMachOLibrary ();
#endif

//-----------------------------------------------------------------------------
// AEffGUIEditor Implementation
//-----------------------------------------------------------------------------
AEffGUIEditor::AEffGUIEditor (void* pEffect) 
: AEffEditor ((AudioEffect*)pEffect)
, inIdleStuff (false)
{
	((AudioEffect*)pEffect)->setEditor (this);
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
AEffGUIEditor::~AEffGUIEditor () 
{
	#if WINDOWS
	OleUninitialize ();
	#endif
	#if MAC
	ExitMachOLibrary ();
	#endif
}

//-----------------------------------------------------------------------------
#if VST_2_1_EXTENSIONS
bool AEffGUIEditor::onKeyDown (VstKeyCode& keyCode)
{
	return frame ? frame->onKeyDown (keyCode) > 0 : false;
}

//-----------------------------------------------------------------------------
bool AEffGUIEditor::onKeyUp (VstKeyCode& keyCode)
{
	return frame ? frame->onKeyUp (keyCode) > 0 : false;
}
#endif

//-----------------------------------------------------------------------------
void AEffGUIEditor::draw (ERect* ppErect)
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
bool AEffGUIEditor::open (void* ptr)
{
	return AEffEditor::open (ptr);
}

//-----------------------------------------------------------------------------
void AEffGUIEditor::idle ()
{
	if (inIdleStuff)
		return;

	AEffEditor::idle ();
	if (frame)
		frame->idle ();
}

//-----------------------------------------------------------------------------
int32_t AEffGUIEditor::knobMode = kCircularMode;

//-----------------------------------------------------------------------------
bool AEffGUIEditor::setKnobMode (int32_t val) 
{
	AEffGUIEditor::knobMode = val;
	return true;
}

//-----------------------------------------------------------------------------
bool AEffGUIEditor::onWheel (float distance)
{
	#if VSTGUI_ENABLE_DEPRECATED_METHODS
	if (frame)
	{
		CPoint where;
		frame->getCurrentMouseLocation (where);
		return frame->onWheel (where, distance, frame->getCurrentMouseButtons ());
	}
	#endif	
	return false;
}

//-----------------------------------------------------------------------------
void AEffGUIEditor::wait (uint32_t ms)
{
	#if MAC
	RunCurrentEventLoop (kEventDurationMillisecond * ms);
	
	#elif WINDOWS
	Sleep (ms);

	#endif
}

//-----------------------------------------------------------------------------
uint32_t AEffGUIEditor::getTicks ()
{
	#if MAC
	return (TickCount () * 1000) / 60;
	
	#elif WINDOWS
	return (uint32_t)GetTickCount ();
	
	#endif

	return 0;
}

//-----------------------------------------------------------------------------
void AEffGUIEditor::doIdleStuff ()
{
	// get the current time
	uint32_t currentTicks = getTicks ();

	if (currentTicks < lLastTicks)
	{
		wait (kIdleRateMin);
		currentTicks += kIdleRateMin;
		if (currentTicks < lLastTicks - kIdleRate2)
			return;
	}

	idle ();

	#if WINDOWS
	struct tagMSG windowsMessage;
	if (PeekMessage (&windowsMessage, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
		DispatchMessage (&windowsMessage);

	#endif

	// save the next time
 	lLastTicks = currentTicks + kIdleRate;

	inIdleStuff = true;

	if (effect)
		effect->masterIdle ();

	inIdleStuff = false;
}

//-----------------------------------------------------------------------------
bool AEffGUIEditor::getRect (ERect **ppErect)
{
	*ppErect = &rect;
	return true;
}

// -----------------------------------------------------------------------------
bool AEffGUIEditor::beforeSizeChange (const CRect& newSize, const CRect& oldSize)
{
	AudioEffectX* eX = (AudioEffectX*)effect;
	if (eX && eX->canHostDo ((char*)"sizeWindow"))
	{
		if (eX->sizeWindow ((VstInt32)newSize.getWidth (), (VstInt32)newSize.getHeight ()))
		{
			return true;
		}
		return false;
	}
#if WINDOWS
	// old hack to size the window of the host. Very ugly stuff ...

	if (getFrame () == 0)
		return true;

	RECT  rctTempWnd, rctParentWnd;
	HWND  hTempWnd;
	long   iFrame = (2 * GetSystemMetrics (SM_CYFIXEDFRAME));
	
	long diffWidth  = 0;
	long diffHeight = 0;
	
	hTempWnd = (HWND)getFrame ()->getPlatformFrame ()->getPlatformRepresentation ();
	
	while ((diffWidth != iFrame) && (hTempWnd != NULL)) // look for FrameWindow
	{
		HWND hTempParentWnd = GetParent (hTempWnd);
		TCHAR buffer[1024];
		GetClassName (hTempParentWnd, buffer, 1024);
		if (!hTempParentWnd || !VSTGUI_STRCMP (buffer, TEXT("MDIClient")))
			break;
		GetWindowRect (hTempWnd, &rctTempWnd);
		GetWindowRect (hTempParentWnd, &rctParentWnd);
		
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, (int)newSize.getWidth () + diffWidth, (int)newSize.getHeight () + diffHeight, SWP_NOMOVE);
		
		diffWidth  += (rctParentWnd.right - rctParentWnd.left) - (rctTempWnd.right - rctTempWnd.left);
		diffHeight += (rctParentWnd.bottom - rctParentWnd.top) - (rctTempWnd.bottom - rctTempWnd.top);
		
		if ((diffWidth > 80) || (diffHeight > 80)) // parent belongs to host
			return true;

		if (diffWidth < 0)
			diffWidth = 0;
        if (diffHeight < 0)
			diffHeight = 0;
		
		hTempWnd = hTempParentWnd;
	}
	
	if (hTempWnd)
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, (int)newSize.getWidth () + diffWidth, (int)newSize.getHeight () + diffHeight, SWP_NOMOVE);
#endif
	return true;
}

#if MAC
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#include "getpluginbundle.h"

namespace VSTGUI {

void* gBundleRef = 0;

} // namespace VSTGUI

#define VSTGUI_BUNDLEREF VSTGUI::gBundleRef

// -----------------------------------------------------------------------------
void InitMachOLibrary ()
{
	VSTGUI_BUNDLEREF = GetPluginBundle ();
}

// -----------------------------------------------------------------------------
void ExitMachOLibrary ()
{
	if (VSTGUI_BUNDLEREF)
		CFRelease (VSTGUI_BUNDLEREF);
}

#endif
