// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../../lib/cstring.h"
#include <Windows.h>
#include <functional>
#include <vector>
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {

struct Win32Menu;

//------------------------------------------------------------------------
struct Win32MenuItem
{
	UTF8String title;
	char16_t key {0};
	uint32_t flags {0};
	uint32_t id {0};

	enum Flags
	{
		disabled = 1 << 0,
		separator = 1 << 1,
		submenu = 1 << 2,
	};

	bool isDisabled () const { return (flags & Flags::disabled) != 0; }
	bool isSeparator () const { return (flags & Flags::separator) != 0; }
	bool isSubmenu () const { return (flags & Flags::submenu) != 0; }

	void disable () { flags |= Flags::disabled; }
	void enable () { flags &= ~Flags::disabled; }

	virtual Win32Menu* asMenu () { return nullptr; } 
};

//------------------------------------------------------------------------
struct Win32Menu : Win32MenuItem
{
	using SubMenuPtr = std::shared_ptr<Win32Menu>;
	using ItemPtr = std::shared_ptr<Win32MenuItem>;
	using Items = std::vector<ItemPtr>;

	Win32Menu (UTF8StringView name);
	~Win32Menu () noexcept;

	size_t addSeparator ();
	size_t addItem (ItemPtr&& item);
	size_t addItem (UTF8StringView title, char16_t key = 0, uint32_t id = 0);
	size_t addSubMenu (const SubMenuPtr& subMenu);

	ItemPtr itemAtIndex (size_t index) const;

	using ValidateFunc = std::function<bool (Win32MenuItem& item)>;
	void validateMenuItems (const ValidateFunc& func);

	operator HMENU () const { return menu; }
	Win32Menu* asMenu () override { return this; } 

	static Win32Menu* fromHMENU (HMENU menu);
//------------------------------------------------------------------------
private:
	HMENU menu {nullptr};
	Items items;
};

//------------------------------------------------------------------------
} // Platform
} // Standalone
} // VSTGUI