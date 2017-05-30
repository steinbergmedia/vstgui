// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32menu.h"
#include "../../../../lib/platform/win32/winstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace {

//------------------------------------------------------------------------
const WCHAR* getWideString (const UTF8String& str)
{
	if (auto winStr = dynamic_cast<WinString*> (str.getPlatformString ()))
	{
		return winStr->getWideString ();
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Win32Menu* Win32Menu::fromHMENU (HMENU menu)
{
	MENUINFO info {};
	info.cbSize = sizeof (MENUINFO);
	info.fMask = MIM_MENUDATA;
	GetMenuInfo (menu, &info);
	return reinterpret_cast<Win32Menu*> (info.dwMenuData);
}

//------------------------------------------------------------------------
Win32Menu::Win32Menu (UTF8StringView name)
{
	title = name;
	menu = CreateMenu ();
	MENUINFO info {};
	info.cbSize = sizeof (MENUINFO);
	info.dwStyle = MNS_NOTIFYBYPOS;
	info.dwMenuData = reinterpret_cast<ULONG_PTR> (this);
	info.fMask = MIM_STYLE | MIM_MENUDATA;
	SetMenuInfo (menu, &info);
}

//------------------------------------------------------------------------
Win32Menu::~Win32Menu () noexcept
{
	if (menu)
		DestroyMenu (menu);
}

//------------------------------------------------------------------------
auto Win32Menu::itemAtIndex (size_t index) const -> ItemPtr
{
	if (index < items.size ())
		return items[index];
	return nullptr;
}

//------------------------------------------------------------------------
size_t Win32Menu::addSeparator ()
{
	auto item = std::make_shared<Win32MenuItem> ();
	item->flags = Win32MenuItem::Flags::separator;
	return addItem (std::move (item));
}

//------------------------------------------------------------------------
size_t Win32Menu::addItem (ItemPtr&& item)
{
	items.emplace_back (item);
	auto& i = items.back ();
	if (i->isSeparator ())
		AppendMenu (menu, MF_SEPARATOR, 0, 0);
	else
	{
		if (i->key != 0)
		{
			auto title = i->title.getString ();
			auto upper = toupper (i->key);
			title += "\tCtrl+";
			if (upper == i->key)
				title += "Shift+";
			title += static_cast<char> (upper);
			UTF8String titleStr (title.data ());
			AppendMenu (menu, MF_STRING, i->id, getWideString (titleStr));
		}
		else
			AppendMenu (menu, MF_STRING, i->id, getWideString (i->title));
	}
	return items.size ();
}

//------------------------------------------------------------------------
size_t Win32Menu::addItem (UTF8StringView title, char16_t key, uint32_t id)
{
	auto item = std::make_shared<Win32MenuItem> ();
	item->title = title;
	item->key = key;
	item->id = id;
	return addItem (std::move (item));
}

//------------------------------------------------------------------------
size_t Win32Menu::addSubMenu (const SubMenuPtr& subMenu)
{
	HMENU platformSubMenu = *subMenu;
	AppendMenu (menu, MF_STRING | MF_POPUP | MF_ENABLED, reinterpret_cast<UINT_PTR> (platformSubMenu),
			    getWideString (subMenu->title));
	items.push_back (subMenu);
	return items.size ();
}

//------------------------------------------------------------------------
void Win32Menu::validateMenuItems (const ValidateFunc& func)
{
	for (auto& item : items)
	{
		if (item->id && func (*item))
		{
			// update item, currently only the enabled state
			EnableMenuItem (*this, item->id, (item->isDisabled () ? MF_DISABLED : MF_ENABLED));
		}
	}
}

//------------------------------------------------------------------------
} // Platform
} // Standalone
} // VSTGUI