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

#include "coptionmenu.h"
#include "../cbitmap.h"
#include "../cframe.h"
#include "../cstring.h"

#include "../platform/iplatformoptionmenu.h"

namespace VSTGUI {

//------------------------------------------------------------------------
// CMenuItem
//------------------------------------------------------------------------
/*! @class CMenuItem
Defines an item of a VSTGUI::COptionMenu
*/
//------------------------------------------------------------------------
/**
 * CMenuItem constructor.
 * @param inTitle title of item
 * @param inFlags CMenuItem::Flags of item
 * @param inKeycode keycode of item
 * @param inKeyModifiers keymodifiers of item
 * @param inIcon icon of item
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const UTF8String& inTitle, const UTF8String& inKeycode, int32_t inKeyModifiers, CBitmap* inIcon, int32_t inFlags)
: flags (inFlags)
{
	setTitle (inTitle);
	setKey (inKeycode, inKeyModifiers);
	setIcon (inIcon);
}

//------------------------------------------------------------------------
/**
 * CMenuItem constructor.
 * @param inTitle title of item
 * @param inSubmenu submenu of item
 * @param inIcon icon of item
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const UTF8String& inTitle, COptionMenu* inSubmenu, CBitmap* inIcon)
{
	setTitle (inTitle);
	setSubmenu (inSubmenu);
	setIcon (inIcon);
}

//------------------------------------------------------------------------
/**
 * CMenuItem constructor.
 * @param inTitle title of item
 * @param inTag tag of item
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const UTF8String& inTitle, int32_t inTag)
{
	setTitle (inTitle);
	setTag (inTag);
}

//------------------------------------------------------------------------
/**
 * CMenuItem copy constructor.
 * @param item item to copy
 */
//------------------------------------------------------------------------
CMenuItem::CMenuItem (const CMenuItem& item)
: flags (item.flags)
{
	setTitle (item.getTitle ());
	setIcon (item.getIcon ());
	if (item.getVirtualKeyCode ())
		setVirtualKey (item.getVirtualKeyCode (), item.getKeyModifiers ());
	else
		setKey (item.getKeycode (), item.getKeyModifiers ());
	setTag (item.getTag ());
	setSubmenu (item.getSubmenu ());
}

//------------------------------------------------------------------------
void CMenuItem::setTitle (const UTF8String& inTitle)
{
	title = inTitle;
}

//------------------------------------------------------------------------
void CMenuItem::setKey (const UTF8String& inKeycode, int32_t inKeyModifiers)
{
	keyCode = inKeycode;
	keyModifiers = inKeyModifiers;
	virtualKeyCode = 0;
}

//------------------------------------------------------------------------
void CMenuItem::setVirtualKey (int32_t inVirtualKeyCode, int32_t inKeyModifiers)
{
	setKey (nullptr, inKeyModifiers);
	virtualKeyCode = inVirtualKeyCode;
}

//------------------------------------------------------------------------
void CMenuItem::setSubmenu (COptionMenu* inSubmenu)
{
	submenu = inSubmenu;
}

//------------------------------------------------------------------------
void CMenuItem::setIcon (CBitmap* inIcon)
{
	icon = inIcon;
}

//------------------------------------------------------------------------
void CMenuItem::setTag (int32_t t)
{
	tag = t;
}

//------------------------------------------------------------------------
void CMenuItem::setEnabled (bool state)
{
	if (state)
		flags &= ~kDisabled;
	else
		flags |= kDisabled;
}

//------------------------------------------------------------------------
void CMenuItem::setChecked (bool state)
{
	if (state)
		flags |= kChecked;
	else
		flags &= ~kChecked;
}

//------------------------------------------------------------------------
void CMenuItem::setIsTitle (bool state)
{
	if (state)
		flags |= kTitle;
	else
		flags &= ~kTitle;
}

//------------------------------------------------------------------------
void CMenuItem::setIsSeparator (bool state)
{
	if (state)
		flags |= kSeparator;
	else
		flags &= ~kSeparator;
}

//------------------------------------------------------------------------
/*! @class CCommandMenuItem

	The CCommandMenuItem supports setting a category, name and a target. The target will get a @link CBaseObject::notify notify()@endlink call before the item is
	displayed and after it was selected. @see CCommandMenuItem::kMsgMenuItemValidate and @see CCommandMenuItem::kMsgMenuItemSelected
*/
//------------------------------------------------------------------------
IdStringPtr CCommandMenuItem::kMsgMenuItemValidate = "kMsgMenuItemValidate";
IdStringPtr CCommandMenuItem::kMsgMenuItemSelected = "kMsgMenuItemSelected";

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const UTF8String& title, const UTF8String& keycode, int32_t keyModifiers, CBitmap* icon, int32_t flags, CBaseObject* _target, const UTF8String& _commandCategory, const UTF8String& _commandName)
: CMenuItem (title, keycode, keyModifiers, icon, flags)
{
	setTarget (_target);
	setCommandCategory (_commandCategory);
	setCommandName (_commandName);
}

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const UTF8String& title, COptionMenu* submenu, CBitmap* icon, CBaseObject* _target, const UTF8String& _commandCategory, const UTF8String& _commandName)
: CMenuItem (title, submenu, icon)
{
	setTarget (_target);
	setCommandCategory (_commandCategory);
	setCommandName (_commandName);
}

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const UTF8String& title, int32_t tag, CBaseObject* _target, const UTF8String& _commandCategory, const UTF8String& _commandName)
: CMenuItem (title, tag)
{
	setTarget (_target);
	setCommandCategory (_commandCategory);
	setCommandName (_commandName);
}

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const UTF8String& title, CBaseObject* _target, const UTF8String& _commandCategory, const UTF8String& _commandName)
: CMenuItem (title, -1)
{
	setTarget (_target);
	setCommandCategory (_commandCategory);
	setCommandName (_commandName);
}

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const CCommandMenuItem& item)
: CMenuItem (item)
, validateFunc (item.validateFunc)
, selectedFunc (item.selectedFunc)
, commandCategory (item.commandCategory)
, commandName (item.commandName)
{
	setTarget (item.target);
}

//------------------------------------------------------------------------
void CCommandMenuItem::setCommandCategory (const UTF8String& category)
{
	commandCategory = category;
}

//------------------------------------------------------------------------
bool CCommandMenuItem::isCommandCategory (const UTF8String& category) const
{
	return commandCategory == category;
}

//------------------------------------------------------------------------
void CCommandMenuItem::setCommandName (const UTF8String& name)
{
	commandName = name;
}

//------------------------------------------------------------------------
bool CCommandMenuItem::isCommandName (const UTF8String& name) const
{
	return commandName == name;
}

//------------------------------------------------------------------------
void CCommandMenuItem::setTarget (CBaseObject* _target)
{
	target = _target;
}

//------------------------------------------------------------------------
void CCommandMenuItem::setActions (SelectedCallbackFunction&& selected, ValidateCallbackFunction&& validate)
{
	selectedFunc = std::move (selected);
	validateFunc = std::move (validate);
}

//------------------------------------------------------------------------
void CCommandMenuItem::execute ()
{
	if (selectedFunc)
		selectedFunc (this);

	if (getTarget ())
		getTarget ()->notify (this, CCommandMenuItem::kMsgMenuItemSelected);
}

//------------------------------------------------------------------------
void CCommandMenuItem::validate ()
{
	if (validateFunc)
		validateFunc (this);

	if (getTarget ())
		getTarget ()->notify (this, CCommandMenuItem::kMsgMenuItemValidate);
}

//------------------------------------------------------------------------
IdStringPtr COptionMenu::kMsgBeforePopup = "kMsgBeforePopup";

//------------------------------------------------------------------------
// COptionMenu
//------------------------------------------------------------------------
/*! @class COptionMenu
Define a rectangle view where a text-value can be displayed with a given font and color.
The text-value is centered in the given rect.
A bitmap can be used as background, a second bitmap can be used when the option menu is popuped.
There are 2 styles with or without a shadowed text. When a mouse click occurs, a popup menu is displayed.
*/
//------------------------------------------------------------------------
/**
 * COptionMenu constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the background bitmap
 * @param bgWhenClick the background bitmap if the option menu is displayed
 * @param style the style of the display (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
COptionMenu::COptionMenu (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, CBitmap* bgWhenClick, const int32_t style)
: CParamDisplay (size, background, style)
, bgWhenClick (bgWhenClick)
{
	this->listener = listener;
	this->tag = tag;

	lastButton = kRButton;
	
	menuItems = new CMenuItemList;
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COptionMenu::COptionMenu ()
: CParamDisplay (CRect (0, 0, 0, 0))
{
	menuItems = new CMenuItemList;
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COptionMenu::COptionMenu (const COptionMenu& v)
: CParamDisplay (v)
, menuItems (new CMenuItemList (*v.menuItems))
, nbItemsPerColumn (v.nbItemsPerColumn)
, bgWhenClick (v.bgWhenClick)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COptionMenu::~COptionMenu () noexcept
{
	removeAllEntry ();

	delete menuItems;
}

//------------------------------------------------------------------------
int32_t COptionMenu::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.character == 0)
	{
		if (keyCode.virt == VKEY_RETURN)
		{
			CBaseObjectGuard guard (this);
			if (bgWhenClick)
				invalid ();
			popup ();
			if (bgWhenClick)
				invalid ();
			return 1;
		}
		if (!(style & (kMultipleCheckStyle & ~kCheckStyle)))
		{
			if (keyCode.virt == VKEY_UP)
			{
				int32_t value = (int32_t)getValue ()-1;
				if (value >= 0)
				{
					CMenuItem* entry = getEntry (value);
					while (entry && (entry->isSeparator () || entry->isTitle () || !entry->isEnabled () || entry->getSubmenu ()))
						entry = getEntry (--value);
					if (entry)
					{
						beginEdit ();
						setValue ((float)value);
						lastResult = (int32_t)getValue ();
						valueChanged ();
						endEdit ();
						invalid ();
					}
				}
				return 1;
			}
			if (keyCode.virt == VKEY_DOWN)
			{
				int32_t value = (int32_t)getValue ()+1;
				if (value < getNbEntries ())
				{
					CMenuItem* entry = getEntry (value);
					while (entry && (entry->isSeparator () || entry->isTitle () || !entry->isEnabled () || entry->getSubmenu ()))
						entry = getEntry (++value);
					if (entry)
					{
						beginEdit ();
						setValue ((float)value);
						lastResult = (int32_t)getValue ();
						valueChanged ();
						endEdit ();
						invalid ();
					}
				}
				return 1;
			}
		}
	}
	return CParamDisplay::onKeyDown (keyCode);
}

//------------------------------------------------------------------------
void COptionMenu::beforePopup ()
{
	changed (kMsgBeforePopup);
	for (auto& menuItem : *menuItems)
	{
		CCommandMenuItem* commandItem = menuItem.cast<CCommandMenuItem> ();
		if (commandItem)
			commandItem->validate ();
		if (menuItem->getSubmenu ())
			menuItem->getSubmenu ()->beforePopup ();
	}
}

//------------------------------------------------------------------------
bool COptionMenu::popup ()
{
	bool popupResult = false;
	if (!getFrame ())
		return popupResult;

	CBaseObjectGuard objGuard (this);

	beforePopup ();

	inPopup = true;

	beginEdit ();

	lastResult = -1;
	lastMenu = nullptr;

	getFrame ()->onStartLocalEventLoop ();

	IPlatformOptionMenu* platformMenu = getFrame ()->getPlatformFrame ()->createPlatformOptionMenu ();
	if (platformMenu)
	{
		PlatformOptionMenuResult platformPopupResult = platformMenu->popup (this);
		if (platformPopupResult.menu != nullptr)
		{
			IDependency::DeferChanges dc (this);
			lastMenu = platformPopupResult.menu;
			lastResult = platformPopupResult.index;
			lastMenu->setValue ((float)lastResult);
			valueChanged ();
			invalid ();
			popupResult = true;
			CCommandMenuItem* commandItem = dynamic_cast<CCommandMenuItem*>(lastMenu->getEntry (lastResult));
			if (commandItem)
				commandItem->execute ();
		}
		platformMenu->forget ();
	}

	endEdit ();
	inPopup = false;
	return popupResult;
}

//------------------------------------------------------------------------
bool COptionMenu::popup (CFrame* frame, const CPoint& frameLocation)
{
	if (frame == nullptr)
		return false;
	if (isAttached ())
		return false;
	CBaseObjectGuard guard (this);

	CView* oldFocusView = frame->getFocusView ();
	CBaseObjectGuard ofvg (oldFocusView);

	CRect size (frameLocation, CPoint (0, 0));
	setViewSize (size);
	frame->addView (this);
	popup ();
	frame->removeView (this, false);
	frame->setFocusView (oldFocusView);
	int32_t index;
	if (getLastItemMenu (index))
		return true;
	return false;
}

//------------------------------------------------------------------------
void COptionMenu::setPrefixNumbers (int32_t preCount)
{
	if (preCount >= 0 && preCount <= 4)
		prefixNumbers = preCount;
}

/**
 * @param item menu item to add. Takes ownership of item.
 * @param index position of insertation. -1 appends the item
 */
//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addEntry (CMenuItem* item, int32_t index)
{
	if (index < 0 || index > getNbEntries ())
		menuItems->push_back (owned (item));
	else
	{
		menuItems->insert (menuItems->begin () + index, owned (item));
	}
	return item;
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addEntry (COptionMenu* submenu, const UTF8String& title)
{
	CMenuItem* item = new CMenuItem (title, submenu);
	return addEntry (item);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addEntry (const UTF8String& title, int32_t index, int32_t itemFlags)
{
	if (title == "-")
		return addSeparator (index);
	CMenuItem* item = new CMenuItem (title, nullptr, 0, nullptr, itemFlags);
	return addEntry (item, index);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::addSeparator (int32_t index)
{
	CMenuItem* item = new CMenuItem ("", nullptr, 0, nullptr, CMenuItem::kSeparator);
	return addEntry (item, index);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::getCurrent () const
{
	return getEntry (currentIndex);
}

//-----------------------------------------------------------------------------
CMenuItem* COptionMenu::getEntry (int32_t index) const
{
	if (index < 0 || menuItems->empty () || index > getNbEntries ())
		return nullptr;
	
	return (*menuItems)[(size_t)index];
}

//-----------------------------------------------------------------------------
int32_t COptionMenu::getNbEntries () const
{
	return (int32_t) menuItems->size ();
}

//------------------------------------------------------------------------
COptionMenu* COptionMenu::getSubMenu (int32_t idx) const
{
	CMenuItem* item = getEntry (idx);
	if (item)
		return item->getSubmenu ();
	return nullptr;
}

//------------------------------------------------------------------------
int32_t COptionMenu::getCurrentIndex (bool countSeparator) const
{
	if (countSeparator)
		return currentIndex;
	int32_t i = 0;
	int32_t numSeparators = 0;
	for (auto& item : *menuItems)
	{
		if (item->isSeparator ())
			numSeparators++;
		if (i == currentIndex)
			break;
		i++;
	}
	return currentIndex - numSeparators;
}

//------------------------------------------------------------------------
bool COptionMenu::setCurrent (int32_t index, bool countSeparator)
{
	CMenuItem* item = nullptr;
	if (countSeparator)
	{
		item = getEntry (index);
		if (!item || item->isSeparator ())
			return false;
		currentIndex = index;
	}
	else
	{
		int32_t i = 0;
		for (auto& item : *menuItems)
		{
			if (i > index)
				break;
			if (item->isSeparator ())
				index++;
			i++;
		}
		currentIndex = index;
		item = getEntry (currentIndex);
	}
	if (item && style & (kMultipleCheckStyle & ~kCheckStyle))
		item->setChecked (!item->isChecked ());
	
	// to force the redraw
	setDirty ();

	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::removeEntry (int32_t index)
{
	if (index < 0 || menuItems->empty () || index > getNbEntries ())
		return false;
	menuItems->erase (menuItems->begin () + index);
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::removeAllEntry ()
{
	menuItems->clear ();
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::checkEntry (int32_t index, bool state)
{
	CMenuItem* item = getEntry (index);
	if (item)
	{
		item->setChecked (state);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool COptionMenu::checkEntryAlone (int32_t index)
{
	int32_t pos = 0;
	for (auto& item : *menuItems)
	{
		item->setChecked (pos == index);
		pos++;
	}
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::isCheckEntry (int32_t index) const
{
	CMenuItem* item = getEntry (index);
	if (item && item->isChecked ())
		return true;
	return false;
}

//------------------------------------------------------------------------
void COptionMenu::draw (CDrawContext *pContext)
{
	CMenuItem* item = getEntry (currentIndex);
	drawBack (pContext, inPopup ? bgWhenClick : nullptr);
	if (item)
		drawPlatformText (pContext, UTF8String (item->getTitle ()).getPlatformString ());
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult COptionMenu::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	lastButton = buttons;
	if (lastButton & (kLButton|kRButton|kApple))
	{
		if (bgWhenClick)
			invalid ();
		getFrame ()->setFocusView (this);
		popup ();
		if (bgWhenClick)
			invalid ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
COptionMenu *COptionMenu::getLastItemMenu (int32_t &idxInMenu) const
{
	idxInMenu = lastMenu ? (int32_t)lastMenu->getValue (): -1;
	return lastMenu;
}

//------------------------------------------------------------------------
void COptionMenu::setValue (float val)
{
	if ((int32_t)val < 0 || (int32_t)val >= getNbEntries ())
		return;
	
	currentIndex = (int32_t)val;
	if (style & (kMultipleCheckStyle & ~kCheckStyle))
	{
		CMenuItem* item = getCurrent ();
		if (item)
			item->setChecked (!item->isChecked ());
	}
	CParamDisplay::setValue (val);
	
	// to force the redraw
	setDirty ();
}

//------------------------------------------------------------------------
void COptionMenu::takeFocus ()
{
	CParamDisplay::takeFocus ();
}

//------------------------------------------------------------------------
void COptionMenu::looseFocus ()
{	
	CView* receiver = pParentView ? pParentView : pParentFrame;
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
	CParamDisplay::looseFocus ();
}

} // namespace
