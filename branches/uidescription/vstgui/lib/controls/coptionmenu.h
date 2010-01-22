//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
#include <list>

#include "../platform/iplatformoptionmenu.h"

namespace VSTGUI {

class COptionMenu;
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
		kSeparator	= 1 << 3,	///< item is a separator
	};

	CMenuItem (const char* title, const char* keycode = 0, long keyModifiers = 0, CBitmap* icon = 0, long flags = kNoFlags);
	CMenuItem (const char* title, COptionMenu* submenu, CBitmap* icon = 0);
	CMenuItem (const char* title, long tag);
	CMenuItem (const CMenuItem& item);

	//-----------------------------------------------------------------------------
	/// @name CMenuItem Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTitle (const char* title);							///< set title of menu item
	virtual void setSubmenu (COptionMenu* submenu);						///< set submenu of menu item
	virtual void setKey (const char* keyCode, long keyModifiers = 0);	///< set keycode and key modifiers of menu item
	virtual void setEnabled (bool state = true);						///< set menu item enabled state
	virtual void setChecked (bool state = true);						///< set menu item checked state
	virtual void setIsTitle (bool state = true);						///< set menu item title state
	virtual void setIsSeparator (bool state = true);					///< set menu item separator state
	virtual void setIcon (CBitmap* icon);								///< set menu item icon
	virtual void setTag (long tag);										///< set menu item tag

	bool isEnabled () const { return !(flags & kDisabled); }			///< returns whether the item is enabled or not
	bool isChecked () const { return (flags & kChecked) != 0; }				///< returns whether the item is checked or not
	bool isTitle () const { return (flags & kTitle) != 0; }					///< returns whether the item is a title item or not
	bool isSeparator () const { return (flags & kSeparator) != 0; }			///< returns whether the item is a separator or not

	const char* getTitle () const { return title; }						///< returns the title of the item
	long getKeyModifiers () const { return keyModifiers; }				///< returns the key modifiers of the item
	const char* getKeycode () const { return keycode; }					///< returns the keycode of the item
	COptionMenu* getSubmenu () const { return submenu; }				///< returns the submenu of the item
	CBitmap* getIcon () const { return icon; }							///< returns the icon of the item
	long getTag () const { return tag; }								///< returns the tag of the item
	//@}

//------------------------------------------------------------------------
protected:
	~CMenuItem ();

	char* title;
	char* keycode;
	COptionMenu* submenu;
	CBitmap* icon;
	long flags;
	long keyModifiers;
	long tag;
};

//-----------------------------------------------------------------------------
class CMenuItemList : public std::list<CMenuItem*>
{
public:
	CMenuItemList () {}
	CMenuItemList (const CMenuItemList& inList) : std::list<CMenuItem*> (inList) {}
};

typedef std::list<CMenuItem*>::iterator CMenuItemIterator;
typedef std::list<CMenuItem*>::const_iterator CConstMenuItemIterator;


//-----------------------------------------------------------------------------
// COptionMenu Declaration
//! @brief a popup menu control
/// @ingroup controls
//-----------------------------------------------------------------------------
class COptionMenu : public CParamDisplay
{
public:
	COptionMenu ();
	COptionMenu (const CRect& size, CControlListener* listener, long tag, CBitmap* background = 0, CBitmap* bgWhenClick = 0, const long style = 0);
	COptionMenu (const COptionMenu& menu);

	//-----------------------------------------------------------------------------
	/// @name COptionMenu Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMenuItem* addEntry (CMenuItem* item, long index = -1);											///< add a new entry
	virtual CMenuItem* addEntry (COptionMenu* submenu, const char* title);									///< add a new submenu entry
	virtual CMenuItem* addEntry (const char* title, long index = -1, long itemFlags = CMenuItem::kNoFlags);	///< add a new entry
	virtual CMenuItem* addSeparator ();																		///< add a new separator entry
	virtual CMenuItem* getCurrent () const;																	///< get current entry
	virtual long getCurrentIndex (bool countSeparator = false) const;
	virtual CMenuItem* getEntry (long index) const;															///< get entry at index position
	virtual long getNbEntries () const;																		///< get number of entries
	virtual	bool setCurrent (long index, bool countSeparator = true);										///< set current entry
	virtual	bool removeEntry (long index);																	///< remove an entry
	virtual	bool removeAllEntry ();																			///< remove all entries

	virtual bool checkEntry (long index, bool state);														///< change check state of entry at index
	virtual bool checkEntryAlone (long index);																///< check entry at index and uncheck every other item
	virtual bool isCheckEntry (long index) const;															///< get check state of entry at index
	virtual void setNbItemsPerColumn (long val) { nbItemsPerColumn = val; }									///< Windows only
	virtual long getNbItemsPerColumn () const { return nbItemsPerColumn; }									///< Windows only

	long getLastResult () const { return lastResult; }														///< get last index of choosen entry
	COptionMenu* getLastItemMenu (long& idxInMenu) const;													///< get last menu and index of choosen entry

	virtual void setPrefixNumbers (long preCount);															///< set prefix numbering
	long getPrefixNumbers () const { return prefixNumbers; }												///< get prefix numbering

	COptionMenu* getSubMenu (long idx) const;																///< get a submenu

	bool popup ();																							///< pops up menu
	bool popup (CFrame* frame, const CPoint& frameLocation);												///< pops up menu at frameLocation

	CMenuItemList* getItems () const { return menuItems; }
	//@}

	// overrides
	virtual void setValue (float val, bool updateSubListeners = false);
	virtual void setMin (float val) {}
	virtual float getMin () const { return 0; }
	virtual void setMax (float val) {}
	virtual float getMax () const { return (float)(menuItems->size () - 1); }

	virtual	void draw (CDrawContext* pContext);
	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual	void takeFocus ();
	virtual	void looseFocus ();

	CLASS_METHODS(COptionMenu, CParamDisplay)
protected:
	~COptionMenu ();

	CMenuItemList* menuItems;

	bool     inPopup;
	long     currentIndex;
	long     lastButton;
	long     nbItemsPerColumn;
	long	 lastResult;
	long	 prefixNumbers;
	CBitmap* bgWhenClick;
	COptionMenu* lastMenu;
};

} // namespace

#endif
