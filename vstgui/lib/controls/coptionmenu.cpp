// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "coptionmenu.h"
#include "../cbitmap.h"
#include "../cframe.h"
#include "../cstring.h"

#include "../platform/iplatformoptionmenu.h"
#include "../platform/iplatformframe.h"

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
	setBit (flags, kDisabled, !state);
}

//------------------------------------------------------------------------
void CMenuItem::setChecked (bool state)
{
	setBit (flags, kChecked, state);
}

//------------------------------------------------------------------------
void CMenuItem::setIsTitle (bool state)
{
	setBit (flags, kTitle, state);
}

//------------------------------------------------------------------------
void CMenuItem::setIsSeparator (bool state)
{
	setBit (flags, kSeparator, state);
}

//------------------------------------------------------------------------
/*! @class CCommandMenuItem

	The CCommandMenuItem supports setting a category, name and a target. The target will get a @link CBaseObject::notify notify()@endlink call before the item is
	displayed and after it was selected. @see CCommandMenuItem::kMsgMenuItemValidate and @see CCommandMenuItem::kMsgMenuItemSelected
*/
//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (Desc&& args)
: CMenuItem (args.title, args.keycode, args.keyModifiers, args.icon, args.flags)
, commandCategory (std::move (args.commandCategory))
, commandName (std::move (args.commandName))
, itemTarget (std::move (args.target))
{
}

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const Desc& args)
: CMenuItem (args.title, args.keycode, args.keyModifiers, args.icon, args.flags)
, commandCategory (args.commandCategory)
, commandName (args.commandName)
, itemTarget (args.target)
{
}

//------------------------------------------------------------------------
CCommandMenuItem::CCommandMenuItem (const CCommandMenuItem& item)
: CMenuItem (item)
, validateFunc (item.validateFunc)
, selectedFunc (item.selectedFunc)
, commandCategory (item.commandCategory)
, commandName (item.commandName)
{
	setItemTarget (item.itemTarget);
}

//------------------------------------------------------------------------
void CCommandMenuItem::setItemTarget (ICommandMenuItemTarget* target)
{
	itemTarget = target;
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

	if (itemTarget)
		itemTarget->onCommandMenuItemSelected (this);
}

//------------------------------------------------------------------------
void CCommandMenuItem::validate ()
{
	if (validateFunc)
		validateFunc (this);

	if (itemTarget)
		itemTarget->validateCommandMenuItem (this);
}

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
void COptionMenu::registerOptionMenuListener (IOptionMenuListener* listener)
{
	if (!listeners)
		listeners = std::unique_ptr<MenuListenerList> (new MenuListenerList ());
	listeners->add (listener);
}

//------------------------------------------------------------------------
void COptionMenu::unregisterOptionMenuListener (IOptionMenuListener* listener)
{
	if (listeners)
		listeners->remove (listener);
}

//------------------------------------------------------------------------
int32_t COptionMenu::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.character == 0)
	{
		if (keyCode.virt == VKEY_RETURN)
		{
			auto self = shared (this);
			getFrame ()->doAfterEventProcessing ([self] () {
				self->doPopup ();
			});
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
	if (listeners)
		listeners->forEach ([this] (IOptionMenuListener* l) { l->onOptionMenuPrePopup (this); });
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
void COptionMenu::afterPopup ()
{
	for (auto& menuItem : *menuItems)
	{
		if (menuItem->getSubmenu ())
			menuItem->getSubmenu ()->afterPopup ();
	}
	if (listeners)
		listeners->forEach ([this] (IOptionMenuListener* l) { l->onOptionMenuPostPopup (this); });
}

//------------------------------------------------------------------------
bool COptionMenu::doPopup ()
{
	if (bgWhenClick)
		invalid ();
	auto result = popup ();
	if (bgWhenClick)
		invalid ();
	return result;
}

//------------------------------------------------------------------------
bool COptionMenu::popup (const PopupCallback& callback)
{
	if (!getFrame ())
		return false;

	beforePopup ();

	lastResult = -1;
	lastMenu = nullptr;

	if (!menuItems->empty ())
	{
		getFrame ()->onStartLocalEventLoop ();
		if (auto platformMenu = getFrame ()->getPlatformFrame ()->createPlatformOptionMenu ())
		{
			inPopup = true;
			auto self = shared (this);
			platformMenu->popup (this, [self, callback] (COptionMenu* menu, PlatformOptionMenuResult result) {
				if (result.menu != nullptr)
				{
					self->beginEdit ();
					self->lastMenu = result.menu;
					self->lastResult = result.index;
					self->lastMenu->setValue (static_cast<float> (self->lastResult));
					self->valueChanged ();
					self->invalid ();
					if (auto commandItem = dynamic_cast<CCommandMenuItem*> (
					        self->lastMenu->getEntry (self->lastResult)))
						commandItem->execute ();
					self->endEdit ();
				}
				self->afterPopup ();
				if (callback)
					callback (self);
				self->inPopup = false;
			});
		}
	}
	return true;
}

//------------------------------------------------------------------------
bool COptionMenu::popup (CFrame* frame, const CPoint& frameLocation, const PopupCallback& callback)
{
	if (frame == nullptr || menuItems->empty ())
		return false;
	if (isAttached ())
		return false;
	CView* oldFocusView = frame->getFocusView ();
	CRect size (frameLocation, CPoint (0, 0));
	setViewSize (size);
	frame->addView (this);

	auto prevFocusView = shared (oldFocusView);
	popup ([prevFocusView, callback] (COptionMenu* menu) {
		if (auto frame = menu->getFrame ())
		{
			frame->removeView (menu, false);
			frame->setFocusView (prevFocusView);
		}
		if (callback)
			callback (menu);
	});
	return true;
}

//------------------------------------------------------------------------
void COptionMenu::cleanupSeparators (bool deep)
{
	if (getItems ()->empty ())
		return;

	std::list<int32_t>indicesToRemove;
	bool lastEntryWasSeparator = true;
	for (auto i = 0; i < getNbEntries () - 1; ++i)
	{
		auto entry = getEntry (i);
		vstgui_assert (entry);
		if (entry->isSeparator ())
		{
			if (lastEntryWasSeparator)
			{
				indicesToRemove.push_front (i);
			}
			lastEntryWasSeparator = true;
		}
		else
			lastEntryWasSeparator = false;
		if (deep)
		{
			if (auto subMenu = entry->getSubmenu ())
			{
				subMenu->cleanupSeparators (deep);
			}
		}
	}
	auto lastIndex = getNbEntries () - 1;
	if (getEntry (lastIndex)->isSeparator ())
	{
		indicesToRemove.push_front (lastIndex);
	}

	for (auto index : indicesToRemove)
	{
		removeEntry (index);
	}
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
		menuItems->emplace_back (owned (item));
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
	if (index < 0 || menuItems->empty () || index >= getNbEntries ())
		return nullptr;
	
	return (*menuItems)[static_cast<size_t> (index)];
}

//-----------------------------------------------------------------------------
int32_t COptionMenu::getNbEntries () const
{
	return static_cast<int32_t> (menuItems->size ());
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
	if (index < 0 || menuItems->empty () || index >= getNbEntries ())
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
		auto self = shared (this);
		getFrame ()->doAfterEventProcessing ([self] () {
			self->doPopup ();
		});
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
	auto newIndex = static_cast<int32_t> (std::round (val));
	if (newIndex < 0 || newIndex >= getNbEntries ())
		return;
	
	currentIndex = newIndex;
	if (style & (kMultipleCheckStyle & ~kCheckStyle))
	{
		CMenuItem* item = getCurrent ();
		if (item)
			item->setChecked (!item->isChecked ());
	}
	CParamDisplay::setValue (static_cast<float> (newIndex));
	
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
	CView* receiver = getParentView () ? getParentView () : getFrame ();
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
	CParamDisplay::looseFocus ();
}

} // VSTGUI
