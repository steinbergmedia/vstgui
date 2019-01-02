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

	CMenuItem (const UTF8String& title, const UTF8String& keycode = "", int32_t keyModifiers = 0, CBitmap* icon = nullptr, int32_t flags = kNoFlags);
	CMenuItem (const UTF8String& title, COptionMenu* submenu, CBitmap* icon = nullptr);
	CMenuItem (const UTF8String& title, int32_t tag);
	CMenuItem (const CMenuItem& item);

	//-----------------------------------------------------------------------------
	/// @name CMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set title of menu item */
	virtual void setTitle (const UTF8String& title);
	/** set submenu of menu item */
	virtual void setSubmenu (COptionMenu* submenu);
	/** set keycode and key modifiers of menu item */
	virtual void setKey (const UTF8String& keyCode, int32_t keyModifiers = 0);
	/** set virtual keycode and key modifiers of menu item */
	virtual void setVirtualKey (int32_t virtualKeyCode, int32_t keyModifiers = 0);
	/** set menu item enabled state */
	virtual void setEnabled (bool state = true);
	/** set menu item checked state */
	virtual void setChecked (bool state = true);
	/** set menu item title state */
	virtual void setIsTitle (bool state = true);
	/** set menu item separator state */
	virtual void setIsSeparator (bool state = true);
	/** set menu item icon */
	virtual void setIcon (CBitmap* icon);
	/** set menu item tag */
	virtual void setTag (int32_t tag);

	/** returns whether the item is enabled or not */
	bool isEnabled () const { return !hasBit (flags, kDisabled); }
	/** returns whether the item is checked or not */
	bool isChecked () const { return hasBit (flags, kChecked); }
	/** returns whether the item is a title item or not */
	bool isTitle () const { return hasBit (flags, kTitle); }
	/** returns whether the item is a separator or not */
	bool isSeparator () const { return hasBit (flags, kSeparator); }

	/** returns the title of the item */
	const UTF8String& getTitle () const { return title; }
	/** returns the key modifiers of the item */
	int32_t getKeyModifiers () const { return keyModifiers; }
	/** returns the keycode of the item */
	const UTF8String& getKeycode () const { return keyCode; }
	/** returns the virtual keycode of the item */
	int32_t getVirtualKeyCode () const { return virtualKeyCode; }
	/** returns the submenu of the item */
	COptionMenu* getSubmenu () const { return submenu; }
	/** returns the icon of the item */
	CBitmap* getIcon () const { return icon; }
	/** returns the tag of the item */
	int32_t getTag () const { return tag; }
	//@}

//------------------------------------------------------------------------
protected:
	~CMenuItem () noexcept override = default;

	UTF8String title;
	UTF8String keyCode;
	SharedPointer<COptionMenu> submenu;
	SharedPointer<CBitmap> icon;
	int32_t flags {0};
	int32_t keyModifiers {0};
	int32_t virtualKeyCode {0};
	int32_t tag {-1};
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
		
		Desc () = default;
		~Desc () noexcept = default;

		Desc (const UTF8String& title, const UTF8String& keycode = nullptr,
		                 int32_t keyModifiers = 0, CBitmap* icon = nullptr,
		                 int32_t flags = kNoFlags, ICommandMenuItemTarget* target = nullptr,
		                 const UTF8String& commandCategory = nullptr,
		                 const UTF8String& commandName = nullptr)
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
		                 ICommandMenuItemTarget* target = nullptr,
		                 const UTF8String& commandCategory = nullptr,
		                 const UTF8String& commandName = nullptr)
		: title (title)
		, commandCategory (commandCategory)
		, commandName (commandName)
		, target (target)
		{
		}

		Desc (const UTF8String& title, ICommandMenuItemTarget* target,
		                 const UTF8String& commandCategory = nullptr,
		                 const UTF8String& commandName = nullptr)
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

	void setItemTarget (ICommandMenuItemTarget* target);
	ICommandMenuItemTarget* getItemTarget () const { return itemTarget; }

	using ValidateCallbackFunction = std::function<void(CCommandMenuItem* item)>;
	using SelectedCallbackFunction = std::function<void(CCommandMenuItem* item)>;

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
	COptionMenu (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background = nullptr, CBitmap* bgWhenClick = nullptr, const int32_t style = 0);
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
	/** add a new entry */
	virtual CMenuItem* addEntry (CMenuItem* item, int32_t index = -1);
	/** add a new submenu entry */
	virtual CMenuItem* addEntry (COptionMenu* submenu, const UTF8String& title);
	/** add a new entry */
	virtual CMenuItem* addEntry (const UTF8String& title, int32_t index = -1, int32_t itemFlags = CMenuItem::kNoFlags);
	/** add a new separator entry */
	virtual CMenuItem* addSeparator (int32_t index = -1);
	/** get current entry */
	virtual CMenuItem* getCurrent () const;
	/** TODO: Doc */
	virtual int32_t getCurrentIndex (bool countSeparator = false) const;
	/** get entry at index position */
	virtual CMenuItem* getEntry (int32_t index) const;
	/** get number of entries */
	virtual int32_t getNbEntries () const;
	/** set current entry */
	virtual	bool setCurrent (int32_t index, bool countSeparator = true);
	/** remove an entry */
	virtual	bool removeEntry (int32_t index);
	/** remove all entries */
	virtual	bool removeAllEntry ();

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
	COptionMenu* getLastItemMenu (int32_t& idxInMenu) const;

	/** set prefix numbering */
	virtual void setPrefixNumbers (int32_t preCount);
	/** get prefix numbering */
	int32_t getPrefixNumbers () const { return prefixNumbers; }

	/** get a submenu */
	COptionMenu* getSubMenu (int32_t idx) const;

	/** popup callback function */
	using PopupCallback = std::function<void (COptionMenu* menu)>;

	/** pops up the menu */
	bool popup (const PopupCallback& callback = {});
	/** pops up the menu at frameLocation */
	bool popup (CFrame* frame, const CPoint& frameLocation, const PopupCallback& callback = {});

	CMenuItemList* getItems () const { return menuItems; }

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
	float getMax () const override { return (float)(menuItems->size () - 1); }

	void draw (CDrawContext* pContext) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	void takeFocus () override;
	void looseFocus () override;

	CLASS_METHODS(COptionMenu, CParamDisplay)
protected:
	bool doPopup ();
	void beforePopup ();
	void afterPopup ();

	CMenuItemList* menuItems;

	bool inPopup {false};
	int32_t currentIndex {-1};
	CButtonState lastButton {0};
	int32_t nbItemsPerColumn {-1};
	int32_t lastResult {-1};
	int32_t prefixNumbers {0};
	SharedPointer<CBitmap> bgWhenClick;
	COptionMenu* lastMenu {nullptr};
	using MenuListenerList = DispatchList<IOptionMenuListener*>;
	std::unique_ptr<MenuListenerList> listeners;
};

} // VSTGUI
