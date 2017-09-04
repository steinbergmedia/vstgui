// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __coptionmenu__
#define __coptionmenu__

#include "cparamdisplay.h"
#include "../cstring.h"
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
		kDisabled	= 1 << 0,	///< item is gray and not selectable
		kTitle		= 1 << 1,	///< item indicates a title and is not selectable
		kChecked	= 1 << 2,	///< item has a checkmark
		kSeparator	= 1 << 3	///< item is a separator
	};

	CMenuItem (const UTF8String& title, const UTF8String& keycode = "", int32_t keyModifiers = 0, CBitmap* icon = nullptr, int32_t flags = kNoFlags);
	CMenuItem (const UTF8String& title, COptionMenu* submenu, CBitmap* icon = nullptr);
	CMenuItem (const UTF8String& title, int32_t tag);
	CMenuItem (const CMenuItem& item);

	//-----------------------------------------------------------------------------
	/// @name CMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTitle (const UTF8String& title);							///< set title of menu item
	virtual void setSubmenu (COptionMenu* submenu);							///< set submenu of menu item
	virtual void setKey (const UTF8String& keyCode, int32_t keyModifiers = 0);	///< set keycode and key modifiers of menu item
	virtual void setVirtualKey (int32_t virtualKeyCode, int32_t keyModifiers = 0);///< set virtual keycode and key modifiers of menu item
	virtual void setEnabled (bool state = true);							///< set menu item enabled state
	virtual void setChecked (bool state = true);							///< set menu item checked state
	virtual void setIsTitle (bool state = true);							///< set menu item title state
	virtual void setIsSeparator (bool state = true);						///< set menu item separator state
	virtual void setIcon (CBitmap* icon);									///< set menu item icon
	virtual void setTag (int32_t tag);										///< set menu item tag

	bool isEnabled () const { return !hasBit (flags, kDisabled); }				///< returns whether the item is enabled or not
	bool isChecked () const { return hasBit (flags, kChecked); }				///< returns whether the item is checked or not
	bool isTitle () const { return hasBit (flags, kTitle); }					///< returns whether the item is a title item or not
	bool isSeparator () const { return hasBit (flags, kSeparator); }			///< returns whether the item is a separator or not

	const UTF8String& getTitle () const { return title; }					///< returns the title of the item
	int32_t getKeyModifiers () const { return keyModifiers; }				///< returns the key modifiers of the item
	const UTF8String& getKeycode () const { return keyCode; }				///< returns the keycode of the item
	int32_t getVirtualKeyCode () const { return virtualKeyCode; }			///< returns the virtual keycode of the item
	COptionMenu* getSubmenu () const { return submenu; }					///< returns the submenu of the item
	CBitmap* getIcon () const { return icon; }								///< returns the icon of the item
	int32_t getTag () const { return tag; }									///< returns the tag of the item
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
	CCommandMenuItem (const UTF8String& title, const UTF8String& keycode = nullptr, int32_t keyModifiers = 0, CBitmap* icon = nullptr, int32_t flags = kNoFlags, CBaseObject* target = nullptr, const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr);
	CCommandMenuItem (const UTF8String& title, COptionMenu* submenu, CBitmap* icon = nullptr, CBaseObject* target = nullptr, const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr);
	CCommandMenuItem (const UTF8String& title, int32_t tag, CBaseObject* target = nullptr, const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr);
	CCommandMenuItem (const UTF8String& title, CBaseObject* target, const UTF8String& commandCategory = nullptr, const UTF8String& commandName = nullptr);
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

	void setTarget (CBaseObject* target);
	CBaseObject* getTarget () const { return target; }

	using ValidateCallbackFunction = std::function<void(CCommandMenuItem* item)>;
	using SelectedCallbackFunction = std::function<void(CCommandMenuItem* item)>;

	void setActions (SelectedCallbackFunction&& selected, ValidateCallbackFunction&& validate = [](CCommandMenuItem*){});
	//@}

	void execute ();
	void validate ();

	static IdStringPtr kMsgMenuItemValidate;	///< message send to the target before the item is shown
	static IdStringPtr kMsgMenuItemSelected;	///< message send to the target when this item was selected
protected:
	ValidateCallbackFunction validateFunc;
	SelectedCallbackFunction selectedFunc;
	SharedPointer<CBaseObject> target;
	UTF8String commandCategory;
	UTF8String commandName;
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
public:
	COptionMenu ();
	COptionMenu (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background = nullptr, CBitmap* bgWhenClick = nullptr, const int32_t style = 0);
	COptionMenu (const COptionMenu& menu);
	~COptionMenu () noexcept override;

	//-----------------------------------------------------------------------------
	/// @name COptionMenu Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMenuItem* addEntry (CMenuItem* item, int32_t index = -1);											///< add a new entry
	virtual CMenuItem* addEntry (COptionMenu* submenu, const UTF8String& title);									///< add a new submenu entry
	virtual CMenuItem* addEntry (const UTF8String& title, int32_t index = -1, int32_t itemFlags = CMenuItem::kNoFlags);	///< add a new entry
	virtual CMenuItem* addSeparator (int32_t index = -1);														///< add a new separator entry
	virtual CMenuItem* getCurrent () const;																	///< get current entry
	virtual int32_t getCurrentIndex (bool countSeparator = false) const;
	virtual CMenuItem* getEntry (int32_t index) const;															///< get entry at index position
	virtual int32_t getNbEntries () const;																		///< get number of entries
	virtual	bool setCurrent (int32_t index, bool countSeparator = true);										///< set current entry
	virtual	bool removeEntry (int32_t index);																	///< remove an entry
	virtual	bool removeAllEntry ();																			///< remove all entries

	virtual bool checkEntry (int32_t index, bool state);														///< change check state of entry at index
	virtual bool checkEntryAlone (int32_t index);																///< check entry at index and uncheck every other item
	virtual bool isCheckEntry (int32_t index) const;															///< get check state of entry at index
	virtual void setNbItemsPerColumn (int32_t val) { nbItemsPerColumn = val; }									///< Windows only
	virtual int32_t getNbItemsPerColumn () const { return nbItemsPerColumn; }									///< Windows only

	int32_t getLastResult () const { return lastResult; }														///< get last index of choosen entry
	COptionMenu* getLastItemMenu (int32_t& idxInMenu) const;													///< get last menu and index of choosen entry

	virtual void setPrefixNumbers (int32_t preCount);															///< set prefix numbering
	int32_t getPrefixNumbers () const { return prefixNumbers; }												///< get prefix numbering

	COptionMenu* getSubMenu (int32_t idx) const;																///< get a submenu

	bool popup ();																							///< pops up menu
	bool popup (CFrame* frame, const CPoint& frameLocation);												///< pops up menu at frameLocation

	CMenuItemList* getItems () const { return menuItems; }
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

	static IdStringPtr kMsgBeforePopup;
	
	CLASS_METHODS(COptionMenu, CParamDisplay)
protected:
	bool doPopup ();
	void beforePopup ();

	CMenuItemList* menuItems;

	bool inPopup {false};
	int32_t currentIndex {-1};
	CButtonState lastButton {0};
	int32_t nbItemsPerColumn {-1};
	int32_t lastResult {-1};
	int32_t prefixNumbers {0};
	SharedPointer<CBitmap> bgWhenClick;
	COptionMenu* lastMenu {nullptr};
};

} // namespace

#endif
