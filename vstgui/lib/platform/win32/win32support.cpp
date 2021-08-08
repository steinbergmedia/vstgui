// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32support.h"

#if WINDOWS

#include "../../vstkeycode.h"
#include "../common/fileresourceinputstream.h"
#include "../platform_win32.h"
#include "win32factory.h"

#include <shlwapi.h>
#include "direct2d/d2ddrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
HINSTANCE GetInstance ()
{
	if (auto wf = getPlatformFactory ().asWin32Factory ())
		return wf->getInstance ();
	return nullptr;
}

//-----------------------------------------------------------------------------
ID2D1Factory* getD2DFactory ()
{
	return getPlatformFactory ().asWin32Factory ()->getD2DFactory ();
}

//-----------------------------------------------------------------------------
IWICImagingFactory* getWICImageingFactory ()
{
	return getPlatformFactory ().asWin32Factory ()->getWICImagingFactory ();
}

//-----------------------------------------------------------------------------
IDWriteFactory* getDWriteFactory ()
{
	return getPlatformFactory ().asWin32Factory ()->getDirectWriteFactory ();
}

//-----------------------------------------------------------------------------
void useD2D () {}

//-----------------------------------------------------------------------------
void unuseD2D () {}

//-----------------------------------------------------------------------------
CDrawContext* createDrawContext (HWND window, HDC device, const CRect& surfaceRect)
{
	auto context = new D2DDrawContext (window, surfaceRect);
	if (!context->usable ())
	{
		context->forget ();
		return nullptr;
	}
	return context;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
Optional<VstKeyCode> keyMessageToKeyCode (WPARAM wParam, LPARAM lParam)
{
	static std::map<WPARAM, VstVirtualKey> vMap = {
		{VK_BACK, VKEY_BACK}, 
		{VK_TAB, VKEY_TAB}, 
		{VK_CLEAR, VKEY_CLEAR}, 
		{VK_RETURN, VKEY_RETURN}, 
		{VK_PAUSE, VKEY_PAUSE}, 
		{VK_ESCAPE, VKEY_ESCAPE}, 
		{VK_SPACE, VKEY_SPACE}, 
		{VK_NEXT, VKEY_NEXT}, 
		{VK_END, VKEY_END}, 
		{VK_HOME, VKEY_HOME}, 
		{VK_LEFT, VKEY_LEFT}, 
		{VK_UP, VKEY_UP}, 
		{VK_RIGHT, VKEY_RIGHT}, 
		{VK_DOWN, VKEY_DOWN}, 
		// {VK_PAGEUP, VKEY_PAGEUP}, 
		// {VK_PAGEDOWN, VKEY_PAGEDOWN}, 
		{VK_SELECT, VKEY_SELECT}, 
		{VK_PRINT, VKEY_PRINT}, 
		// {VK_ENTER, VKEY_ENTER}, 
		{VK_SNAPSHOT, VKEY_SNAPSHOT}, 
		{VK_INSERT, VKEY_INSERT}, 
		{VK_DELETE, VKEY_DELETE}, 
		{VK_HELP, VKEY_HELP}, 
		{VK_NUMPAD0, VKEY_NUMPAD0}, 
		{VK_NUMPAD1, VKEY_NUMPAD1}, 
		{VK_NUMPAD2, VKEY_NUMPAD2}, 
		{VK_NUMPAD3, VKEY_NUMPAD3}, 
		{VK_NUMPAD4, VKEY_NUMPAD4}, 
		{VK_NUMPAD5, VKEY_NUMPAD5}, 
		{VK_NUMPAD6, VKEY_NUMPAD6}, 
		{VK_NUMPAD7, VKEY_NUMPAD7}, 
		{VK_NUMPAD8, VKEY_NUMPAD8}, 
		{VK_NUMPAD9, VKEY_NUMPAD9}, 
		{VK_MULTIPLY, VKEY_MULTIPLY}, 
		{VK_ADD, VKEY_ADD}, 
		{VK_SEPARATOR, VKEY_SEPARATOR}, 
		{VK_SUBTRACT, VKEY_SUBTRACT}, 
		{VK_DECIMAL, VKEY_DECIMAL}, 
		{VK_DIVIDE, VKEY_DIVIDE}, 
		{VK_F1, VKEY_F1}, 
		{VK_F2, VKEY_F2}, 
		{VK_F3, VKEY_F3}, 
		{VK_F4, VKEY_F4}, 
		{VK_F5, VKEY_F5}, 
		{VK_F6, VKEY_F6}, 
		{VK_F7, VKEY_F7}, 
		{VK_F8, VKEY_F8}, 
		{VK_F9, VKEY_F9}, 
		{VK_F10, VKEY_F10}, 
		{VK_F11, VKEY_F11}, 
		{VK_F12, VKEY_F12}, 
		{VK_NUMLOCK, VKEY_NUMLOCK}, 
		{VK_SCROLL, VKEY_SCROLL},
		{VK_SHIFT, VKEY_SHIFT},
		{VK_CONTROL, VKEY_CONTROL},
		// {VK_ALT, VKEY_ALT},
		{VK_OEM_NEC_EQUAL, VKEY_EQUALS} // TODO: verify
	};
	auto it = vMap.find (wParam);
	if (it != vMap.end ())
	{
		VstKeyCode res {};
		res.virt = it->second;
		return Optional<VstKeyCode> (res);
	}
	return {};
}


/// @endcond ignore

} // VSTGUI

#endif // WINDOWS
