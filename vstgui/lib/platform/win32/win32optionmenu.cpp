// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

	COptionMenu *menu = nullptr;
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
void Win32OptionMenu::popup (COptionMenu* optionMenu, const Callback& callback)
{
	vstgui_assert (optionMenu && callback, "arguments are required");

	PlatformOptionMenuResult result = {};
	
	//---Transform local coordinates to global coordinates
	CRect rect = optionMenu->translateToGlobal (optionMenu->getViewSize ());
	rect.offset (-optionMenu->getFrame ()->getViewSize ().getTopLeft ());

	int32_t offset;

	if (optionMenu->isPopupStyle ())
		offset = 0;
	else
		offset = static_cast<int32_t> (rect.getHeight ());

	POINT p;
	p.x = static_cast<LONG> (rect.left);
	p.y = static_cast<LONG> (rect.top + offset);
	ClientToScreen (windowHandle, &p);

	int32_t offsetIndex = 0;
	if (HMENU menu = createMenu (optionMenu, offsetIndex))
	{
		UINT flags = TPM_LEFTALIGN;

// do we need the following ?
//		if (lastButton & kRButton)
//			flags |= TPM_RIGHTBUTTON;

		if (TrackPopupMenu (menu, flags, p.x, p.y, 0, windowHandle, nullptr))
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
	callback (optionMenu, result);
}

//-----------------------------------------------------------------------------
HMENU Win32OptionMenu::createMenu (COptionMenu* _menu, int32_t& offsetIdx)
{
	HMENU menu = CreatePopupMenu ();

	bool multipleCheck = _menu->isMultipleCheckStyle ();

	MENUINFO mi = {};
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
			AppendMenu (menu, MF_SEPARATOR, 0, nullptr);
		}
		else
		{
			char* titleWithPrefixNumbers = nullptr;
			if (_menu->getPrefixNumbers ())
			{
				auto bufferSize = strlen (item->getTitle ()) + 50;
				titleWithPrefixNumbers = (char*)std::malloc (bufferSize);
				switch (_menu->getPrefixNumbers ())
				{
					case 2:
					{
						snprintf (titleWithPrefixNumbers, bufferSize, "%1d %s", inc + 1,
								  item->getTitle ().data ());
						break;
					}
					case 3:
					{
						snprintf (titleWithPrefixNumbers, bufferSize, "%02d %s", inc + 1,
								  item->getTitle ().data ());
						break;
					}
					case 4:
					{
						snprintf (titleWithPrefixNumbers, bufferSize, "%03d %s", inc + 1,
								  item->getTitle ().data ());
						break;
					}
				}
			}
			UTF8StringHelper entryText (titleWithPrefixNumbers ? titleWithPrefixNumbers : item->getTitle ().data ());
			flags = MF_STRING;
			if (nbEntries < 160 && _menu->getNbItemsPerColumn () > 0 && inc && !(inc % _menu->getNbItemsPerColumn ()))
				flags |= MF_MENUBARBREAK;

			if (item->getSubmenu ())
			{
				if (HMENU submenu = createMenu (item->getSubmenu (), offsetIdx))
				{
					if (multipleCheck && item->isChecked())
					{
						flags |= MF_CHECKED;
					}
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
				if (multipleCheck)
				{
					if (item->isChecked ())
						flags |= MF_CHECKED;
				}
				else if (_menu->isCheckStyle () && inc == _menu->getCurrentIndex (true))
					flags |= MF_CHECKED;

				if (!(flags & MF_CHECKED))
					flags |= MF_UNCHECKED;

				AppendMenu (menu, flags, offset + inc, entryText);
				IPlatformBitmap* platformBitmap = item->getIcon () ? item->getIcon ()->getPlatformBitmap () : nullptr;
				if (platformBitmap)
				{
					if (auto* win32Bitmap = dynamic_cast<Win32BitmapBase*> (platformBitmap))
					{
						MENUITEMINFO mInfo = {};
						mInfo.cbSize = sizeof (MENUITEMINFO);
						mInfo.fMask = MIIM_BITMAP;
						if (HBITMAP hBmp = win32Bitmap->createHBitmap ())
						{
							mInfo.hbmpItem = hBmp;
							SetMenuItemInfo (menu, offset + inc, TRUE, &mInfo);
							bitmaps.emplace_back (hBmp);
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

} // VSTGUI

#endif // WINDOWS
