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

#include "win32optionmenu.h"

#if WINDOWS

#include "win32support.h"
#include "win32bitmapbase.h"
#include "../../controls/coptionmenu.h"
#include "../../cframe.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
Win32OptionMenu::Win32OptionMenu (HWND windowHandle)
: windowHandle (windowHandle)
{
}

//------------------------------------------------------------------------
COptionMenu* getItemMenu (int32_t idx, int32_t &idxInMenu, int32_t &offsetIdx, COptionMenu* _menu)
{
	int32_t oldIDx = offsetIdx;
	offsetIdx += _menu->getNbEntries ();

	if (idx < offsetIdx)
	{
		idxInMenu = idx - oldIDx;
		return _menu;
	}
	

	COptionMenu *menu = 0;
	CMenuItemIterator it = _menu->getItems ()->begin ();
	while (it != _menu->getItems ()->end ())
	{
		if ((*it)->getSubmenu ())
		{
			menu = getItemMenu (idx, idxInMenu, offsetIdx, (*it)->getSubmenu ());
			if (menu)
				break;
		}
		it++;
	}
	return menu;
}

//-----------------------------------------------------------------------------
PlatformOptionMenuResult Win32OptionMenu::popup (COptionMenu* optionMenu)
{
	PlatformOptionMenuResult result = {0};
	
	//---Transform local coordinates to global coordinates
	CRect rect = optionMenu->translateToGlobal (optionMenu->getViewSize ());

	int32_t offset;

	if (optionMenu->getStyle () & kPopupStyle)
		offset = 0;
	else
		offset = static_cast<int32_t> (rect.getHeight ());

	CCoord gx = 0, gy = 0;
	optionMenu->getFrame ()->getPosition (gx, gy);
	gx += rect.left;
	gy += rect.top + offset;

	int32_t offsetIndex = 0;
	HMENU menu = createMenu (optionMenu, offsetIndex);
	if (menu)
	{
		int flags = TPM_LEFTALIGN;

// do we need the following ?
//		if (lastButton & kRButton)
//			flags |= TPM_RIGHTBUTTON;

		if (TrackPopupMenu (menu, flags, (int)gx, (int)gy, 0, windowHandle, 0))
		{
			MSG msg;
			if (PeekMessage (&msg, windowHandle, WM_COMMAND, WM_COMMAND, PM_REMOVE))
			{
				if (HIWORD (msg.wParam) == 0)
				{
					int32_t res = LOWORD (msg.wParam);
					if (res != -1)
					{
						int32_t idx = 0;
						offsetIndex = 0;
						COptionMenu* resultMenu = getItemMenu (res, idx, offsetIndex, optionMenu);
						if (resultMenu)
						{
							result.menu = resultMenu;
							result.index = idx;
						}
					}
				}
			}
		}
		std::list<HBITMAP>::iterator it = bitmaps.begin ();
		while (it != bitmaps.end ())
		{
			DeleteObject (*it);
			it++;
		}
		DestroyMenu (menu);
	}
	return result;
}

//-----------------------------------------------------------------------------
HMENU Win32OptionMenu::createMenu (COptionMenu* _menu, int32_t& offsetIdx)
{
	HMENU menu = CreatePopupMenu ();

	bool multipleCheck = _menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);

#if 0
	if (!multipleCheck && !(_menu->getStyle () & kCheckStyle))
	{
		MENUINFO mi = {0};
		mi.cbSize = sizeof (MENUINFO);
		mi.fMask = MIM_STYLE;
		mi.dwStyle = MNS_NOCHECK;
		SetMenuInfo (menu, &mi);
	}
#endif

	MENUINFO mi = {0};
	mi.cbSize = sizeof (MENUINFO);
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo (menu, &mi);

	int flags = 0;
	int32_t offset = offsetIdx;
	int32_t nbEntries = _menu->getNbEntries ();
	offsetIdx += nbEntries;
	int32_t inc = 0;
	CMenuItemIterator it = _menu->getItems ()->begin ();
	while (it != _menu->getItems ()->end ())
	{
		CMenuItem* item = (*it);
		if (item->isSeparator ())
		{
			AppendMenu (menu, MF_SEPARATOR, 0, 0);
		}
		else
		{
			char* titleWithPrefixNumbers = 0;
			if (_menu->getPrefixNumbers ())
			{
				titleWithPrefixNumbers = (char*)std::malloc (strlen (item->getTitle ()) + 50);
				switch (_menu->getPrefixNumbers ())
				{
					case 2:
					{
						sprintf (titleWithPrefixNumbers, "%1d %s", inc+1, item->getTitle ());
						break;
					}
					case 3:
					{
						sprintf (titleWithPrefixNumbers, "%02d %s", inc+1, item->getTitle ());
						break;
					}
					case 4:
					{
						sprintf (titleWithPrefixNumbers, "%03d %s", inc+1, item->getTitle ());
						break;
					}
				}
			}
			UTF8StringHelper entryText (titleWithPrefixNumbers ? titleWithPrefixNumbers : item->getTitle ());
			flags = MF_STRING;
			if (nbEntries < 160 && _menu->getNbItemsPerColumn () > 0 && inc && !(inc % _menu->getNbItemsPerColumn ()))
				flags |= MF_MENUBARBREAK;

			if (item->getSubmenu ())
			{
				HMENU submenu = createMenu (item->getSubmenu (), offsetIdx);
				if (submenu)
				{
					AppendMenu (menu, flags|MF_POPUP|MF_ENABLED, (UINT_PTR)submenu, (const TCHAR*)entryText);
				}
			}
			else
			{
				if (item->isEnabled ())
					flags |= MF_ENABLED;
				else
					flags |= MF_GRAYED;
				if (item->isTitle ())
					flags |= MF_DISABLED;
				if (multipleCheck && item->isChecked ())
					flags |= MF_CHECKED;
				if (_menu->getStyle () & kCheckStyle && inc == _menu->getCurrentIndex ())
					flags |= MF_CHECKED;
				if (!(flags & MF_CHECKED))
					flags |= MF_UNCHECKED;
				AppendMenu (menu, flags, offset + inc, entryText);
				IPlatformBitmap* platformBitmap = item->getIcon () ? item->getIcon ()->getPlatformBitmap () : 0;
				if (platformBitmap)
				{
					Win32BitmapBase* win32Bitmap = dynamic_cast<Win32BitmapBase*> (platformBitmap);
					if (win32Bitmap)
					{
						MENUITEMINFO mInfo = {0};
						mInfo.cbSize = sizeof (MENUITEMINFO);
						mInfo.fMask = MIIM_BITMAP;
						HBITMAP hBmp = win32Bitmap->createHBitmap ();
						if (hBmp)
						{
							mInfo.hbmpItem = hBmp;
							SetMenuItemInfo (menu, offset + inc, TRUE, &mInfo);
							bitmaps.push_back (hBmp);
						}
					}
				}
			}
			if (titleWithPrefixNumbers)
				std::free (titleWithPrefixNumbers);
		}
		inc++;
		it++;
	}
	return menu;
}

} // namespace

#endif // WINDOWS
