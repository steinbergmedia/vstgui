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

#include "hiviewoptionmenu.h"

#if MAC_CARBON

#include "../../../controls/coptionmenu.h"
#include "../../../cframe.h"
#include "../../../cbitmap.h"
#include "../cgbitmap.h"

namespace VSTGUI {

#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
typedef UInt32 URefCon;
#endif

//-----------------------------------------------------------------------------
PlatformOptionMenuResult HIViewOptionMenu::popup (COptionMenu* optionMenu)
{
	PlatformOptionMenuResult popupResult = {0};
	
	//---Transform local coordinates to global coordinates
	CRect rect (optionMenu->getViewSize ());
	CPoint p (0, 0);
	optionMenu->localToFrame (p);
	rect.offset (p.x, p.y);

	int32_t offset;

	if (optionMenu->getStyle () & kPopupStyle)
		offset = 0;
	else
		offset = (int32_t)optionMenu->getViewSize ().getHeight ();

	CCoord gx = 0, gy = 0;
	optionMenu->getFrame()->getPosition (gx, gy);
	gy += rect.top + offset;
	gx += rect.left;

	MenuRef menuRef = createMenu (optionMenu);
	if (menuRef)
	{
		CalcMenuSize (menuRef);
		SInt16 menuWidth = GetMenuWidth (menuRef);
		if (menuWidth < optionMenu->getViewSize ().getWidth ())
			SetMenuWidth (menuRef, optionMenu->getViewSize ().getWidth ());
		int32_t popUpItem = optionMenu->getStyle () & kPopupStyle ? (optionMenu->getValue () + 1) : 1;
		int32_t PopUpMenuItem = PopUpMenuItem = PopUpMenuSelect (menuRef, gy, gx, popUpItem);
		
		short result = LoWord (PopUpMenuItem) - 1;	
		short menuIDResult = HiWord (PopUpMenuItem);
		if (menuIDResult != 0)
		{
			MenuRef usedMenuRef = GetMenuHandle (menuIDResult);
			if (usedMenuRef)
			{
				COptionMenu* resultMenu = 0;
				if (GetMenuItemRefCon (usedMenuRef, 0, (URefCon*)&resultMenu) == noErr)
				{
					popupResult.menu = resultMenu;
					popupResult.index = result;
				}
			}
		}
		CFRelease (menuRef);
	}

	return popupResult;
}

//-----------------------------------------------------------------------------
MenuRef HIViewOptionMenu::createMenu (COptionMenu* menu)
{
	MenuRef menuRef = 0;
	ResID menuID = UniqueID ('MENU');
	CMenuItemList* menuItems = menu->getItems ();
	if (menuItems && CreateNewMenu (menuID, kMenuAttrCondenseSeparators, &menuRef) == noErr)
	{
		bool multipleCheck = menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
		CConstMenuItemIterator it = menuItems->begin ();
		int32_t i = 0;
		while (it != menuItems->end ())
		{
			i++;
			CMenuItem* item = (*it);
			if (item->isSeparator ())
				AppendMenuItemTextWithCFString (menuRef, CFSTR(""), kMenuItemAttrSeparator, 0, NULL);
			else
			{
				CFStringRef itemString = CFStringCreateWithCString (NULL, item->getTitle (), kCFStringEncodingUTF8);
				if (menu->getPrefixNumbers ())
				{
					CFStringRef prefixString = 0;
					switch (menu->getPrefixNumbers ())
					{
						case 2:
							prefixString = CFStringCreateWithFormat (NULL, 0, CFSTR("%1d "),i); break;
						case 3:
							prefixString = CFStringCreateWithFormat (NULL, 0, CFSTR("%02d "),i); break;
						case 4:
							prefixString = CFStringCreateWithFormat (NULL, 0, CFSTR("%03d "),i); break;
					}
					CFMutableStringRef newItemString = CFStringCreateMutable (0, 0);
					CFStringAppend (newItemString, prefixString);
					CFStringAppend (newItemString, itemString);
					CFRelease (itemString);
					CFRelease (prefixString);
					itemString = newItemString;
				}
				if (itemString == 0)
					continue;
				MenuItemAttributes itemAttribs = kMenuItemAttrIgnoreMeta;
				if (!item->isEnabled ())
					itemAttribs |= kMenuItemAttrDisabled;
				if (item->isTitle ())
					itemAttribs |= kMenuItemAttrSectionHeader;

				InsertMenuItemTextWithCFString (menuRef, itemString, i, itemAttribs, 0);

				if (item->isChecked () && multipleCheck)
					CheckMenuItem (menuRef, i, true);
				if (item->getSubmenu ())
				{
					MenuRef submenu = createMenu (item->getSubmenu ());
					if (submenu)
					{
						SetMenuItemHierarchicalMenu (menuRef, i, submenu);
						CFRelease (submenu);
					}
				}
				if (item->getIcon ())
				{
					IPlatformBitmap* platformBitmap = item->getIcon ()->getPlatformBitmap ();
					CGBitmap* cgBitmap = platformBitmap ? dynamic_cast<CGBitmap*> (platformBitmap) : 0;
					CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : 0;
					if (image)
					{
						SetMenuItemIconHandle (menuRef, i, kMenuCGImageRefType, (Handle)image);
					}
				}
				if (item->getKeycode ())
				{
					SetItemCmd (menuRef, i, toupper (item->getKeycode ()[0]));
					UInt8 keyModifiers = 0;
					int32_t itemModifiers = item->getKeyModifiers ();
					if (itemModifiers & kShift)
						keyModifiers |= kMenuShiftModifier;
					if (!(itemModifiers & kControl))
						keyModifiers |= kMenuNoCommandModifier;
					if (itemModifiers & kAlt)
						keyModifiers |= kMenuOptionModifier;
					if (itemModifiers & kApple)
						keyModifiers |= kMenuControlModifier;
					
					SetMenuItemModifiers (menuRef, i, keyModifiers);
				}
				CFRelease (itemString);
			}
			it++;
		}
		if (menu->getStyle () & kCheckStyle && !multipleCheck)
			CheckMenuItem (menuRef, menu->getCurrentIndex (true) + 1, true);
		SetMenuItemRefCon (menuRef, 0, (int32_t)menu);
		InsertMenu (menuRef, kInsertHierarchicalMenu);
	}
	return menuRef;
}

} // namespace

#endif // MAC_CARBON
