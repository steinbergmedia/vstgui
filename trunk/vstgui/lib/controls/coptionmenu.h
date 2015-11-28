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

#ifndef __coptionmenu__
#define __coptionmenu__

#include "cparamdisplay.h"
#include <vector>
#if VSTGUI_HAS_FUNCTIONAL
#include <functional>
#endif

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

	CMenuItem (UTF8StringPtr title, UTF8StringPtr keycode = 0, int32_t keyModifiers = 0, CBitmap* icon = 0, int32_t flags = kNoFlags);
	CMenuItem (UTF8StringPtr title, COptionMenu* submenu, CBitmap* icon = 0);
	CMenuItem (UTF8StringPtr title, int32_t tag);
	CMenuItem (const CMenuItem& item);

	//-----------------------------------------------------------------------------
	/// @name CMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTitle (UTF8StringPtr title);							///< set title of menu item
	virtual void setSubmenu (COptionMenu* submenu);							///< set submenu of menu item
	virtual void setKey (UTF8StringPtr keyCode, int32_t keyModifiers = 0);	///< set keycode and key modifiers of menu item
	virtual void setVirtualKey (int32_t virtualKeyCode, int32_t keyModifiers = 0);///< set virtual keycode and key modifiers of menu item
	virtual void setEnabled (bool state = true);							///< set menu item enabled state
	virtual void setChecked (bool state = true);							///< set menu item checked state
	virtual void setIsTitle (bool state = true);							///< set menu item title state
	virtual void setIsSeparator (bool state = true);						///< set menu item separator state
	virtual void setIcon (CBitmap* icon);									///< set menu item icon
	virtual void setTag (int32_t tag);										///< set menu item tag

	bool isEnabled () const { return !(flags & kDisabled); }				///< returns whether the item is enabled or not
	bool isChecked () const { return (flags & kChecked) != 0; }				///< returns whether the item is checked or not
	bool isTitle () const { return (flags & kTitle) != 0; }					///< returns whether the item is a title item or not
	bool isSeparator () const { return (flags & kSeparator) != 0; }			///< returns whether the item is a separator or not

	UTF8StringPtr getTitle () const { return title; }						///< returns the title of the item
	int32_t getKeyModifiers () const { return keyModifiers; }				///< returns the key modifiers of the item
	UTF8StringPtr getKeycode () const { return keyCode; }					///< returns the keycode of the item
	int32_t getVirtualKeyCode () const { return virtualKeyCode; }			///< returns the virtual keycode of the item
	COptionMenu* getSubmenu () const { return submenu; }					///< returns the submenu of the item
	CBitmap* getIcon () const { return icon; }								///< returns the icon of the item
	int32_t getTag () const { return tag; }									///< returns the tag of the item
	//@}

//------------------------------------------------------------------------
protected:
	~CMenuItem ();

	UTF8StringBuffer title;
	UTF8StringBuffer keyCode;
	COptionMenu* submenu;
	CBitmap* icon;
	int32_t flags;
	int32_t keyModifiers;
	int32_t virtualKeyCode;
	int32_t tag;
};

//-----------------------------------------------------------------------------
// CCommandMenuItem Declaration
/// @brief a command menu item
/// @ingroup new_in_4_1
//-----------------------------------------------------------------------------
class CCommandMenuItem : public CMenuItem
{
public:
	CCommandMenuItem (UTF8StringPtr title, UTF8StringPtr keycode = 0, int32_t keyModifiers = 0, CBitmap* icon = 0, int32_t flags = kNoFlags, CBaseObject* target = 0, IdStringPtr commandCategory = 0, IdStringPtr commandName = 0);
	CCommandMenuItem (UTF8StringPtr title, COptionMenu* submenu, CBitmap* icon = 0, CBaseObject* target = 0, IdStringPtr commandCategory = 0, IdStringPtr commandName = 0);
	CCommandMenuItem (UTF8StringPtr title, int32_t tag, CBaseObject* target = 0, IdStringPtr commandCategory = 0, IdStringPtr commandName = 0);
	CCommandMenuItem (UTF8StringPtr title, CBaseObject* target = 0, IdStringPtr commandCategory = 0, IdStringPtr commandName = 0);
	CCommandMenuItem (const CCommandMenuItem& item);
	~CCommandMenuItem ();

	//-----------------------------------------------------------------------------
	/// @name CCommandMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	void setCommandCategory (IdStringPtr category);
	IdStringPtr getCommandCategory () const { return commandCategory; }
	bool isCommandCategory (IdStringPtr category) const;
	
	void setCommandName (IdStringPtr name);
	IdStringPtr getCommandName () const { return commandName; }
	bool isCommandName (IdStringPtr name) const;

	void setTarget (CBaseObject* target);
	CBaseObject* getTarget () const { return target; }

#if VSTGUI_HAS_FUNCTIONAL
	typedef std::function<void(CCommandMenuItem* item)> ValidateCallbackFunction;
	typedef std::function<void(CCommandMenuItem* item)> SelectedCallbackFunction;

	void setActions (SelectedCallbackFunction&& selected, ValidateCallbackFunction&& validate = [](CCommandMenuItem*){});
#endif
	//@}

	void execute ();
	void validate ();

	static IdStringPtr kMsgMenuItemValidate;	///< message send to the target before the item is shown
	static IdStringPtr kMsgMenuItemSelected;	///< message send to the target when this item was selected
protected:
#if VSTGUI_HAS_FUNCTIONAL
	ValidateCallbackFunction validateFunc;
	SelectedCallbackFunction selectedFunc;
#endif
	CBaseObject* target;
	char* commandCategory;
	char* commandName;
};

typedef std::vector<OwningPointer<CMenuItem> > CMenuItemList;
typedef CMenuItemList::iterator CMenuItemIterator;
typedef CMenuItemList::const_iterator CConstMenuItemIterator;


//-----------------------------------------------------------------------------
// COptionMenu Declaration
//! @brief a popup menu control
/// @ingroup controls
//-----------------------------------------------------------------------------
class COptionMenu : public CParamDisplay
{
public:
	COptionMenu ();
	COptionMenu (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background = 0, CBitmap* bgWhenClick = 0, const int32_t style = 0);
	COptionMenu (const COptionMenu& menu);
	~COptionMenu ();

	//-----------------------------------------------------------------------------
	/// @name COptionMenu Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMenuItem* addEntry (CMenuItem* item, int32_t index = -1);											///< add a new entry
	virtual CMenuItem* addEntry (COptionMenu* submenu, UTF8StringPtr title);									///< add a new submenu entry
	virtual CMenuItem* addEntry (UTF8StringPtr title, int32_t index = -1, int32_t itemFlags = CMenuItem::kNoFlags);	///< add a new entry
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
	virtual void setValue (float val) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setMin (float val) VSTGUI_OVERRIDE_VMETHOD {}
	virtual float getMin () const VSTGUI_OVERRIDE_VMETHOD { return 0; }
	virtual void setMax (float val) VSTGUI_OVERRIDE_VMETHOD {}
	virtual float getMax () const VSTGUI_OVERRIDE_VMETHOD { return (float)(menuItems->size () - 1); }

	virtual	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual	void takeFocus () VSTGUI_OVERRIDE_VMETHOD;
	virtual	void looseFocus () VSTGUI_OVERRIDE_VMETHOD;

	static IdStringPtr kMsgBeforePopup;
	
	CLASS_METHODS(COptionMenu, CParamDisplay)
protected:
	void beforePopup ();

	CMenuItemList* menuItems;

	bool     inPopup;
	int32_t     currentIndex;
	CButtonState     lastButton;
	int32_t     nbItemsPerColumn;
	int32_t	 lastResult;
	int32_t	 prefixNumbers;
	CBitmap* bgWhenClick;
	COptionMenu* lastMenu;
};

} // namespace

#endif
