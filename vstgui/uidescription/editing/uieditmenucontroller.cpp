// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uieditmenucontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uiactions.h"
#include "uieditview.h"
#include "uieditcontroller.h"
#include "../uiviewfactory.h"
#include "../uiattributes.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/ctextlabel.h"
#include "../detail/uiviewcreatorattributes.h"
#include <cctype>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIEditMenuController::UIEditMenuController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, selection (selection)
, undoManager (undoManager)
, description (description)
, actionPerformer (actionPerformer)
{
}

//----------------------------------------------------------------------------------------------------
static void addEntriesToMenu (const UIEditing::MenuEntry* entries, COptionMenu* menu, CBaseObject* menuItemTarget, int32_t& index)
{
	while (entries[index].category != nullptr)
	{
		if (entries[index].menuFlags & UIEditing::MenuEntry::kSubMenuEnd)
		{
			break;
		}
		if (entries[index].category == UIEditing::kMenuSeparator.category)
			menu->addSeparator ();
		else if (entries[index].menuFlags & UIEditing::MenuEntry::kSubMenu)
		{
			auto subMenu = makeOwned<COptionMenu> ();
			if (entries[index].menuFlags & UIEditing::MenuEntry::kSubMenuCheckStyle)
				subMenu->setStyle (kMultipleCheckStyle|kCheckStyle);
			menu->addEntry (new CMenuItem (entries[index].name, subMenu));
			index++;
			addEntriesToMenu(entries, subMenu, menuItemTarget, index);
		}
		else
		{
			CMenuItem* item = menu->addEntry (new CCommandMenuItem (entries[index].name, menuItemTarget, entries[index].category, entries[index].name));
			if (entries[index].key)
			{
				item->setKey (entries[index].key, entries[index].modifier);
			}
			else if (entries[index].virtualKey)
			{
				item->setVirtualKey (entries[index].virtualKey, entries[index].modifier);
			}
			if (entries[index].menuFlags)
			{
				if (entries[index].menuFlags & UIEditing::MenuEntry::kMenuItemIsTitle)
				{
					item->setIsTitle (true);
				}
			}
		}
		index++;
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::createFileMenu (COptionMenu* menu)
{
	int32_t index = 0;
	addEntriesToMenu (UIEditing::fileMenu, menu, this, index);
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::createEditMenu (COptionMenu* menu)
{
	int32_t index = 0;
	addEntriesToMenu (UIEditing::editMenu, menu, this, index);
}

//----------------------------------------------------------------------------------------------------
bool UIEditMenuController::createUniqueTemplateName (std::list<const std::string*>& names, std::string& name)
{
	for (auto& it : names)
	{
		if (*it == name)
		{
			int32_t count = 0;
			size_t pos = name.find_last_not_of ("0123456789");
			if (pos != std::string::npos && pos != name.length () - 1)
			{
				std::string numberStr = name.substr (pos);
				count = static_cast<int32_t> (strtol (numberStr.c_str (), nullptr, 10)) + 1;
				name.erase (pos+1);
			}
			else
			{
				count = 1;
			}
			while (name.length () && std::isspace (name[name.length ()-1]))
				name.erase (name.length ()-1);
			char number[10];
			sprintf (number, "%d", count);
			name += ' ';
			name += number;
			return createUniqueTemplateName (names, name);
		}
	}
	return true;
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditMenuController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CCommandMenuItem::kMsgMenuItemValidate)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*> (sender);
		if (item)
			return validateMenuItem (*item) ? kMessageNotified : kMessageUnknown;
	}
	else if (message == CCommandMenuItem::kMsgMenuItemSelected)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*> (sender);
		if (handleCommand (item->getCommandCategory (), item->getCommandName ()))
			return kMessageNotified;
	}
	else if (message == CVSTGUITimer::kMsgTimer)
	{
		editLabel->setTransparency (true);
		fileLabel->setTransparency (true);
		highlightTimer = nullptr;
	}
	return kMessageUnknown;
}

//------------------------------------------------------------------------
bool UIEditMenuController::validateMenuItem (CCommandMenuItem& item)
{
	UTF8StringView cmdCategory (item.getCommandCategory ());
	UTF8StringView cmdName (item.getCommandName ());
	item.setChecked (false);
	if (cmdCategory == "Edit")
	{
		if (cmdName == "Undo")
		{
			if (undoManager->canUndo ())
			{
				std::string str ("Undo ");
				str += undoManager->getUndoName ();
				item.setTitle (str.c_str ());
				item.setEnabled (true);
			}
			else
			{
				item.setTitle ("Undo");
				item.setEnabled (false);
			}
			return true;
		}
		else if (cmdName == "Redo")
		{
			if (undoManager->canRedo ())
			{
				std::string str ("Redo ");
				str += undoManager->getRedoName ();
				item.setTitle (str.c_str ());
				item.setEnabled (true);
			}
			else
			{
				item.setTitle ("Redo");
				item.setEnabled (false);
			}
			return true;
		}
		else if (cmdName == "Cut")
		{
			item.setEnabled (false);
		}
		else if (cmdName == "Copy")
		{
			item.setEnabled (false);
		}
		else if (cmdName == "Paste")
		{
			item.setEnabled (false);
		}
		else if (cmdName == "Delete")
		{
			CView* view = selection->first ();
			int32_t selectionCount = selection->total ();
			bool enable = view ? (selectionCount > 1 ? true : dynamic_cast<UIEditView*> (view->getParentView ()) == nullptr) : false;
			item.setEnabled (enable);
			return true;
		}
		else if (cmdName == "Add New Template")
		{
			std::list<const std::string*> containerViewNames;
			const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			factory->collectRegisteredViewNames (containerViewNames, "CViewContainer");
			auto submenu = makeOwned<COptionMenu> ();
			for (auto& name : containerViewNames)
			{
				submenu->addEntry (new CCommandMenuItem (name->c_str (), this, "AddTemplate", name->c_str ()));
			}
			item.setSubmenu (submenu);
			return true;
		}
		else if (cmdName == "Delete Template")
		{
			item.setSubmenu (nullptr);
			std::list<const std::string*> templateNames;
			description->collectTemplateViewNames (templateNames);
			item.setEnabled (templateNames.empty () == false);
			if (templateNames.empty () == false)
			{
				templateNames.sort (UIEditController::std__stringCompare);
				auto submenu = makeOwned<COptionMenu> ();
				item.setSubmenu (submenu);
				for (auto& name : templateNames)
				{
					submenu->addEntry (new CCommandMenuItem (name->c_str (), this, "RemoveTemplate", name->c_str ()));
				}
			}
			return true;
		}
		else if (cmdName == "Duplicate Template")
		{
			item.setSubmenu (nullptr);
			std::list<const std::string*> templateNames;
			description->collectTemplateViewNames (templateNames);
			item.setEnabled (templateNames.empty () == false);
			if (templateNames.empty () == false)
			{
				templateNames.sort (UIEditController::std__stringCompare);
				auto submenu = makeOwned<COptionMenu> ();
				item.setSubmenu (submenu);
				for (auto& name : templateNames)
				{
					submenu->addEntry (new CCommandMenuItem (name->c_str (), this, "DuplicateTemplate", name->c_str ()));
				}
			}
			return true;
		}
		else if (cmdName == "Embed Into")
		{
			item.setSubmenu (nullptr);
			bool enable = selection->total () > 0;
			for (auto view : *selection)
			{
				if (dynamic_cast<UIEditView*>(view->getParentView()) != nullptr)
				{
					enable = false;
					break;
				}
			}
			item.setEnabled (enable);
			if (enable == false)
				return true;
			auto submenu = makeOwned<COptionMenu> ();
			item.setSubmenu (submenu);
			std::list<const std::string*> containerViewNames;
			const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			factory->collectRegisteredViewNames (containerViewNames, "CViewContainer");
			for (auto& name : containerViewNames)
			{
				submenu->addEntry (new CCommandMenuItem (name->c_str (), this, "Embed", name->c_str ()));
			}
			return true;
		}
		else if (cmdName == "Unembed Views")
		{
			bool enabled = false;
			if (selection->total () == 1)
			{
				CViewContainer* container = selection->first ()->asViewContainer ();
				if (container && container->hasChildren () && dynamic_cast<UIEditView*>(container->getParentView ()) == nullptr)
					enabled = true;
			}
			item.setEnabled (enabled);
			return true;
		}
		else if (cmdName == "Size To Fit")
		{
			item.setEnabled (selection->total() > 0 ? true : false);
			return true;
		}
		else if (cmdName == "Transform View Type")
		{
			item.setSubmenu (nullptr);
			bool enabled = false;
			if (selection->total () == 1)
			{
				CViewContainer* container = selection->first ()->asViewContainer ();
				if (container == nullptr || (container && dynamic_cast<UIEditView*>(container->getParentView ()) == nullptr))
					enabled = true;
			}
			item.setEnabled (enabled);
			if (enabled == false)
				return true;
			auto submenu = makeOwned<COptionMenu> ();
			item.setSubmenu (submenu);
			std::list<const std::string*> containerViewNames;
			const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			factory->collectRegisteredViewNames (containerViewNames);
			for (auto& name : containerViewNames)
			{
				submenu->addEntry (new CCommandMenuItem (name->c_str (), this, "Transform View Type", name->c_str ()));
			}
			return true;
		}
		else if (cmdName == "Insert Template")
		{
			item.setSubmenu (nullptr);
			item.setEnabled (selection->total () == 1 && selection->first ()->asViewContainer ());
			if (item.isEnabled () == false)
				return true;
			std::list<const std::string*> templateNames;
			description->collectTemplateViewNames (templateNames);
			item.setEnabled (templateNames.empty () == false);
			if (templateNames.empty () == false)
			{
				templateNames.sort (UIEditController::std__stringCompare);
				auto submenu = makeOwned<COptionMenu> ();
				item.setSubmenu (submenu);
				for (auto& name : templateNames)
				{
					submenu->addEntry (new CCommandMenuItem (name->c_str (), this, "InsertTemplate", name->c_str ()));
				}
			}
			return true;
		}
	}
	CBaseObject* obj = dynamic_cast<CBaseObject*>(controller);
	if (obj)
		return obj->notify (&item, CCommandMenuItem::kMsgMenuItemValidate) == kMessageNotified;
	return false;
}

//----------------------------------------------------------------------------------------------------
CCommandMenuItem* UIEditMenuController::findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key)
{
	for (auto& item : *menu->getItems ())
	{
		COptionMenu* subMenu = item->getSubmenu ();
		if (subMenu)
		{
			CCommandMenuItem* result = findKeyCommandItem (subMenu, key);
			if (result)
				return result;
		}
		CCommandMenuItem* result = item.cast<CCommandMenuItem>();
		if (result)
		{
			int32_t modifier = 0;
			if (key.modifier & MODIFIER_SHIFT)
				modifier |= kShift;
			if (key.modifier & MODIFIER_ALTERNATE)
				modifier |= kAlt;
			if (key.modifier & MODIFIER_CONTROL)
				modifier |= kControl;
			if (key.modifier & MODIFIER_COMMAND)
				modifier |= kApple;
			if (result->getKeyModifiers () == modifier)
			{
				if (key.virt && key.virt == result->getVirtualKeyCode ())
					return result;
				else if (!result->getKeycode ().empty () && result->getKeycode ().getString ()[0] == key.character)
					return result;
			}
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------
bool UIEditMenuController::handleCommand (const UTF8StringPtr category, const UTF8StringPtr name)
{
	UTF8StringView cmdCategory (category);
	UTF8StringView cmdName (name);
	if (cmdCategory == "Edit")
	{
		if (cmdName == "Undo")
		{
			if (undoManager->canUndo ())
			{
				undoManager->performUndo ();
			}
			return true;
		}
		else if (cmdName == "Redo")
		{
			if (undoManager->canRedo ())
			{
				undoManager->performRedo ();
			}
			return true;
		}
		else if (cmdName == "Delete")
		{
			IAction* action = new DeleteOperation (selection);
			undoManager->pushAndPerform (action);
			return true;
		}
		else if (cmdName == "Unembed Views")
		{
			IAction* action = new UnembedViewOperation (selection, description->getViewFactory ());
			undoManager->pushAndPerform (action);
			return true;
		}
		else if (cmdName == "Size To Fit")
		{
			IAction* action = new SizeToFitOperation (selection);
			undoManager->pushAndPerform (action);
			return true;
		}
	}
	if (cmdCategory == "AddTemplate")
	{
		std::list<const std::string*> tmp;
		description->collectTemplateViewNames (tmp);
		std::string templateName (cmdName);
		if (createUniqueTemplateName (tmp, templateName))
		{
			actionPerformer->performCreateNewTemplate (templateName.c_str (), cmdName);
		}
		return true;
	}
	else if (cmdCategory == "RemoveTemplate")
	{
		actionPerformer->performDeleteTemplate (cmdName);
		return kMessageNotified;
	}
	else if (cmdCategory == "DuplicateTemplate")
	{
		std::list<const std::string*> tmp;
		description->collectTemplateViewNames (tmp);
		std::string templateName (cmdName);
		if (createUniqueTemplateName (tmp, templateName))
		{
			actionPerformer->performDuplicateTemplate (cmdName, templateName.c_str ());
		}
		return true;
	}
	else if (cmdCategory == "Embed")
	{
		const IViewFactory* viewFactory = description->getViewFactory ();
		UIAttributes viewAttr;
		viewAttr.setAttribute (UIViewCreator::kAttrClass, std::string (cmdName));
		if (auto newContainer = viewFactory->createView (viewAttr, description)->asViewContainer ())
		{
			IAction* action = new EmbedViewOperation (selection, newContainer);
			undoManager->pushAndPerform (action);
		}
		return true;
	}
	else if (cmdCategory == "Transform View Type")
	{
		IAction* action = new TransformViewTypeOperation (selection, cmdName, description, dynamic_cast<const UIViewFactory*> (description->getViewFactory ()));
		undoManager->pushAndPerform (action);
		return true;
	}
	else if (cmdCategory == "InsertTemplate")
	{
		if (auto parent = selection->first ()->asViewContainer ())
		{
			CView* view = description->createView (cmdName, description->getController ());
			if (view)
			{
				undoManager->pushAndPerform (new InsertViewOperation (parent, view, selection));
			}
		}
		return true;
	}
	if (auto obj = dynamic_cast<CBaseObject*> (controller))
	{
		CCommandMenuItem item ("", 0, nullptr, category, name);
		if (obj->notify (&item, CCommandMenuItem::kMsgMenuItemSelected) == kMessageNotified)
			return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool UIEditMenuController::canHandleCommand (const UTF8StringPtr category, const UTF8StringPtr name) const
{
	CCommandMenuItem item ("", 0, nullptr, category, name);
	if (const_cast<UIEditMenuController*> (this)->validateMenuItem (item))
	{
		return item.isEnabled ();
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditMenuController::processKeyCommand (const VstKeyCode& key)
{
	COptionMenu* baseMenu = editMenu;
	CCommandMenuItem* item = baseMenu ? findKeyCommandItem (baseMenu, key) : nullptr;
	if (item == nullptr && fileMenu)
	{
		baseMenu = fileMenu;
		item = findKeyCommandItem (baseMenu, key);
	}
	if (item && item->getTarget ())
	{
		item->getTarget ()->notify (item, CCommandMenuItem::kMsgMenuItemValidate);
		if (item->isEnabled ())
		{
			CTextLabel* label = nullptr;
			if (baseMenu)
			{
				switch (baseMenu->getTag ())
				{
					case kMenuFileTag:
					{
						label = fileLabel;
						break;
					}
					case kMenuEditTag:
					{
						label = editLabel;
						break;
					}
				}
			}
			if (label)
			{
				label->setTransparency (false);
			}
			item->getTarget ()->notify (item, CCommandMenuItem::kMsgMenuItemSelected);
			if (label)
			{
				highlightTimer = makeOwned<CVSTGUITimer> (this, 90u, true);
			}
			return 1;
		}
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------
CView* UIEditMenuController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	COptionMenu* menu = dynamic_cast<COptionMenu*>(view);
	if (menu)
	{
		switch (menu->getTag ())
		{
			case kMenuEditTag:
			{
				createEditMenu (menu);
				editMenu = menu;
				break;
			}
			case kMenuFileTag:
			{
				createFileMenu (menu);
				fileMenu = menu;
				break;
			}
		}
	}
	else
	{
		CTextLabel* label = dynamic_cast<CTextLabel*>(view);
		if (label)
		{
			switch (label->getTag ())
			{
				case kMenuEditTag:
				{
					editLabel = label;
					break;
				}
				case kMenuFileTag:
				{
					fileLabel = label;
					break;
				}
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kMenuFileTag:
		{
			if (fileMenu && control->getValue () == control->getMax ())
			{
				CRect r (control->getViewSize ());
				CPoint p = r.getBottomLeft ();
				control->localToFrame (p);
				fileMenu->popup (control->getFrame (), p);
			}
			break;
		}
		case kMenuEditTag:
		{
			if (editMenu && control->getValue () == control->getMax ())
			{
				CRect r (control->getViewSize ());
				CPoint p = r.getTopLeft ();
				control->localToFrame (p);
				editMenu->popup (control->getFrame (), p);
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::controlBeginEdit (CControl* pControl)
{
	CTextLabel* label = nullptr;
	switch (pControl->getTag ())
	{
		case kMenuFileTag:
		{
			label = fileLabel;
			break;
		}
		case kMenuEditTag:
		{
			label = editLabel;
			break;
		}
	}
	if (label)
		label->setTransparency (false);
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::controlEndEdit (CControl* pControl)
{
	CTextLabel* label = nullptr;
	switch (pControl->getTag ())
	{
		case kMenuFileTag:
		{
			label = fileLabel;
			break;
		}
		case kMenuEditTag:
		{
			label = editLabel;
			break;
		}
	}
	if (label)
		label->setTransparency (true);
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
