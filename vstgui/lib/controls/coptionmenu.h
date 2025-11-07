// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cparamdisplay.h"
#include "icommandmenuitemtarget.h"
#include "ioptionmenulistener.h"
#include "../cstring.h"
#include "../dispatchlist.h"
#include "../cbitmap.h"
#include <vector>
#include <functional>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CMenuItem Declaration
//! @brief a menu item
//-----------------------------------------------------------------------------
class CMenuItem : public CBaseObject
{
public:
	enum Flags {
		kNoFlags	= 0,
		/** item is gray and not selectable */
		kDisabled	= 1 << 0,
		/** item indicates a title and is not selectable */
		kTitle		= 1 << 1,
		/** item has a checkmark */
		kChecked	= 1 << 2,
		/** item is a separator */
		kSeparator	= 1 << 3
	};

	CMenuItem (const UTF8String& title, const UTF8String& keycode = "", int32_t keyModifiers = 0,
			   const SharedPointer<CBitmap>& icon = {}, int32_t flags = kNoFlags);
	CMenuItem (const UTF8String& title, const SharedPointer<COptionMenu>& submenu,
			   const SharedPointer<CBitmap>& icon = {});
	CMenuItem (const UTF8String& title, int32_t tag);
	CMenuItem (const CMenuItem& item);

	//-----------------------------------------------------------------------------
	/// @name CMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set title of menu item */
	void setTitle (const UTF8String& title);
	/** set submenu of menu item */
	void setSubmenu (const SharedPointer<COptionMenu>& submenu);
	/** remove the submenu of the item */
	void removeSubmenu ();
	/** set keycode and key modifiers of menu item */
	void setKey (const UTF8String& keyCode, int32_t keyModifiers = 0);
	/** set virtual key and key modifiers of menu item */
	void setVirtualKey (VirtualKey virtualKey, int32_t keyModifiers = 0);
	/** set menu item enabled state */
	void setEnabled (bool state = true);
	/** set menu item checked state */
	void setChecked (bool state = true);
	/** set menu item title state */
	void setIsTitle (bool state = true);
	/** set menu item separator state */
	void setIsSeparator (bool state = true);
	/** set menu item icon */
	void setIcon (const SharedPointer<CBitmap>& icon);
	/** set menu item tag */
	void setTag (int32_t tag);

	/** returns whether the item is enabled or not */
	bool isEnabled () const;
	/** returns whether the item is checked or not */
	bool isChecked () const;
	/** returns whether the item is a title item or not */
	bool isTitle () const;
	/** returns whether the item is a separator or not */
	bool isSeparator () const;

	/** returns the title of the item */
	const UTF8String& getTitle () const;
	/** returns the key modifiers of the item */
	int32_t getKeyModifiers () const;
	/** returns the keycode of the item */
	const UTF8String& getKeycode () const;
	/** returns the virtual key of the item */
	VirtualKey getVirtualKey () const;
	/** returns the submenu of the item */
	SharedPointer<COptionMenu> getSubmenu () const;
	/** returns the icon of the item */
	SharedPointer<CBitmap> getIcon () const;
	/** returns the tag of the item */
	int32_t getTag () const;
	//@}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	void setSubmenu (COptionMenu* submenu);
	int32_t getVirtualKeyCode () const;
	virtual void setVirtualKey (int32_t virtualKeyCode, int32_t keyModifiers = 0);
#endif
//------------------------------------------------------------------------
protected:
	CMenuItem ();
	~CMenuItem () noexcept override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
// CCommandMenuItem Declaration
/// @brief a command menu item
/// @ingroup new_in_4_1
//-----------------------------------------------------------------------------
class CCommandMenuItem : public CMenuItem
{
public:
	struct Desc
	{
		UTF8String title;
		UTF8String commandCategory;
		UTF8String commandName;
		UTF8String keycode;
		SharedPointer<ICommandMenuItemTarget> target;
		SharedPointer<CBitmap> icon;
		int32_t keyModifiers {0};
		int32_t flags {kNoFlags};
		int32_t tag {-1};

		Desc () = default;
		~Desc () noexcept = default;

		Desc (const UTF8String& title, const UTF8String& keycode = nullptr,
			  int32_t keyModifiers = 0, const SharedPointer<CBitmap>& icon = {},
			  int32_t flags = kNoFlags, ICommandMenuItemTarget* target = nullptr,
			  const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr)
		: title (title)
		, commandCategory (commandCategory)
		, commandName (commandName)
		, keycode (keycode)
		, target (target)
		, icon (icon)
		, keyModifiers (keyModifiers)
		, flags (flags)
		{
		}

		Desc (const UTF8String& title, int32_t tag,
			  const SharedPointer<ICommandMenuItemTarget>& target = {},
			  const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr)
		: title (title)
		, commandCategory (commandCategory)
		, commandName (commandName)
		, target (target)
		, tag (tag)
		{
		}

		Desc (const UTF8String& title, const SharedPointer<ICommandMenuItemTarget>& target,
			  const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr)
		: title (title)
		, commandCategory (commandCategory)
		, commandName (commandName)
		, target (target)
		{
		}
	};

	CCommandMenuItem (Desc&& args);
	CCommandMenuItem (const Desc& args);
	CCommandMenuItem (const CCommandMenuItem& item);
	~CCommandMenuItem () noexcept override = default;

	//-----------------------------------------------------------------------------
	/// @name CCommandMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	void setCommandCategory (const UTF8String& category);
	const UTF8String& getCommandCategory () const { return commandCategory; }
	bool isCommandCategory (const UTF8String& category) const;
	
	void setCommandName (const UTF8String& name);
	const UTF8String& getCommandName () const { return commandName; }
	bool isCommandName (const UTF8String& name) const;

	void setItemTarget (const SharedPointer<ICommandMenuItemTarget>& target);
	SharedPointer<ICommandMenuItemTarget> getItemTarget () const { return itemTarget; }

	using ValidateCallbackFunction = std::function<void (SharedPointer<CCommandMenuItem> item)>;
	using SelectedCallbackFunction = std::function<void (SharedPointer<CCommandMenuItem> item)>;

	void setActions (SelectedCallbackFunction&& selected, ValidateCallbackFunction&& validate = [](CCommandMenuItem*){});
	//@}

	void execute ();
	void validate ();

protected:
	ValidateCallbackFunction validateFunc;
	SelectedCallbackFunction selectedFunc;
	UTF8String commandCategory;
	UTF8String commandName;
	SharedPointer<ICommandMenuItemTarget> itemTarget;
};

using CMenuItemList = std::vector<SharedPointer<CMenuItem>>;
using CMenuItemIterator = CMenuItemList::iterator;
using CConstMenuItemIterator = CMenuItemList::const_iterator;

//-----------------------------------------------------------------------------
// COptionMenu Declaration
//! @brief a popup menu control
/// @ingroup controls
//-----------------------------------------------------------------------------
class COptionMenu : public CParamDisplay
{
private:
	enum StyleEnum
	{
		StylePopup = CParamDisplay::LastStyle,
		StyleCheck,
		StyleMultipleCheck,
	};
public:
	COptionMenu ();
	COptionMenu (const CRect& size, IControlListener* listener, int32_t tag,
				 const SharedPointer<CBitmap>& background = {},
				 const SharedPointer<CBitmap>& bgWhenClick = {}, const int32_t style = 0);
	COptionMenu (const COptionMenu& menu);
	~COptionMenu () noexcept override;

	enum Style
	{
		kPopupStyle = 1 << StylePopup,
		kCheckStyle = 1 << StyleCheck,
		kMultipleCheckStyle = 1 << StyleMultipleCheck
	};

	bool isPopupStyle () const { return hasBit (getStyle (), kPopupStyle); }
	bool isCheckStyle () const { return hasBit (getStyle (), kCheckStyle); }
	bool isMultipleCheckStyle () const { return hasBit (getStyle (), kMultipleCheckStyle); }

	//-----------------------------------------------------------------------------
	/// @name COptionMenu Methods
	//-----------------------------------------------------------------------------
	//@{
#if VSTGUI_EXPLICIT_SHARED_POINTER_CONSTRUCTOR
	/** add a new entry */
	SharedPointer<CMenuItem> addEntry (const SharedPointer<CMenuItem>& item, int32_t index = -1);
	/** add a new entry */
	template<typename T>
	SharedPointer<T> addEntry (const SharedPointer<T>& item, int32_t index = -1)
	{
		static_assert (std::is_base_of_v<CMenuItem, T>, "Needs CMenuItems");
		return addEntry (item.template cast<CMenuItem> (), index).template cast<T> ();
	}
	/** add a new submenu entry */
	SharedPointer<CMenuItem> addEntry (const SharedPointer<COptionMenu>& submenu,
									   const UTF8String& title);
#endif
	/** add a new entry */
	virtual SharedPointer<CMenuItem> addEntry (const UTF8String& title, int32_t index = -1,
											   int32_t itemFlags = CMenuItem::kNoFlags);
	/** add a new separator entry */
	virtual SharedPointer<CMenuItem> addSeparator (int32_t index = -1);
	/** get current entry */
	virtual SharedPointer<CMenuItem> getCurrent () const;
	/** TODO: Doc */
	virtual int32_t getCurrentIndex (bool countSeparator = false) const;
	/** get entry at index position */
	virtual SharedPointer<CMenuItem> getEntry (int32_t index) const;
	/** get number of entries */
	virtual int32_t getNbEntries () const;
	/** set current entry */
	virtual	bool setCurrent (int32_t index, bool countSeparator = true);
	/** remove an entry */
	virtual	bool removeEntry (int32_t index);
	/** remove all entries */
	virtual	bool removeAllEntry ();

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	virtual SharedPointer<CMenuItem> addEntry (CMenuItem* item, int32_t index = -1);
	virtual SharedPointer<CMenuItem> addEntry (COptionMenu* submenu, const UTF8String& title);
#endif
	/** change check state of entry at index */
	virtual bool checkEntry (int32_t index, bool state);
	/** check entry at index and uncheck every other item */
	virtual bool checkEntryAlone (int32_t index);
	/** get check state of entry at index */
	virtual bool isCheckEntry (int32_t index) const;
	/** Windows only */
	virtual void setNbItemsPerColumn (int32_t val) { nbItemsPerColumn = val; }
	/** Windows only */
	virtual int32_t getNbItemsPerColumn () const { return nbItemsPerColumn; }

	/** get last index of choosen entry */
	int32_t getLastResult () const { return lastResult; }
	/** get last menu and index of choosen entry */
	SharedPointer<COptionMenu> getLastItemMenu (int32_t& idxInMenu) const;

	/** set prefix numbering */
	virtual void setPrefixNumbers (int32_t preCount);
	/** get prefix numbering */
	int32_t getPrefixNumbers () const { return prefixNumbers; }

	/** get a submenu */
	SharedPointer<COptionMenu> getSubMenu (int32_t idx) const;

	/** popup callback function */
	using PopupCallback = std::function<void (SharedPointer<COptionMenu> menu)>;

	/** pops up the menu */
	bool popup (const PopupCallback& callback = {});
	/** pops up the menu at frameLocation */
	bool popup (CFrame* frame, const CPoint& frameLocation, const PopupCallback& callback = {});

	VSTGUI_DEPRECATED_MSG (
		CMenuItemList* getItems () const { return const_cast<CMenuItemList*> (&menuItems); },
		"Use COptionMenu::getItemList() instead")
	const CMenuItemList& getItemList () const { return menuItems; }

	/** remove separators as first and last item and double separators */
	void cleanupSeparators (bool deep);

	void registerOptionMenuListener (IOptionMenuListener* listener);
	void unregisterOptionMenuListener (IOptionMenuListener* listener);
	//@}

	// overrides
	void setValue (float val) override;
	void setMin (float val) override {}
	float getMin () const override { return 0; }
	void setMax (float val) override {}
	float getMax () const override;

	void draw (CDrawContext* pContext) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	void takeFocus () override;
	void looseFocus () override;

	CLASS_METHODS(COptionMenu, CParamDisplay)
private:
	bool doPopup ();
	void beforePopup ();
	void afterPopup ();

	CMenuItemList menuItems;

	bool inPopup {false};
	int32_t currentIndex {-1};
	CButtonState lastButton {0};
	int32_t nbItemsPerColumn {-1};
	int32_t lastResult {-1};
	int32_t prefixNumbers {0};
	SharedPointer<CBitmap> bgWhenClick;
	SharedPointer<COptionMenu> lastMenu;
	using MenuListenerList = DispatchList<IOptionMenuListener*>;
	std::unique_ptr<MenuListenerList> listeners;
};

} // VSTGUI
