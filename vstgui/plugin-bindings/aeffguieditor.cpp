// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __aeffguieditor__
#include "aeffguieditor.h"
#endif

#ifndef __audioeffectx__
#include "public.sdk/source/vst2.x/audioeffectx.h"
#endif

#include "../lib/platform/iplatformframe.h"

#if WINDOWS
#include <windows.h>
#include "../lib/platform/win32/win32support.h"
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
// AEffGUIEditor Implementation
//-----------------------------------------------------------------------------
AEffGUIEditor::AEffGUIEditor (void* pEffect)
: AEffEditor ((AudioEffect*)pEffect)
{
	((AudioEffect*)pEffect)->setEditor (this);
	systemWindow = 0;

	#if WINDOWS
	OleInitialize (nullptr);
	#endif
}

//-----------------------------------------------------------------------------
AEffGUIEditor::~AEffGUIEditor ()
{
	#if WINDOWS
	OleUninitialize ();
	#endif
}

//-----------------------------------------------------------------------------
#if VST_2_1_EXTENSIONS
bool AEffGUIEditor::onKeyDown (VstKeyCode& keyCode)
{
	return false;
}

//-----------------------------------------------------------------------------
bool AEffGUIEditor::onKeyUp (VstKeyCode& keyCode)
{
	return false;
}
#endif

//-----------------------------------------------------------------------------
bool AEffGUIEditor::open (void* ptr)
{
	return AEffEditor::open (ptr);
}

//-----------------------------------------------------------------------------
void AEffGUIEditor::idle ()
{
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
	return false;
}

//-----------------------------------------------------------------------------
void AEffGUIEditor::doIdleStuff ()
{
	vstgui_assert (false, "unexpected call");
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

	while ((diffWidth != iFrame) && (hTempWnd != nullptr)) // look for FrameWindow
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

} // VSTGUI
