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
#include "../../lib/events.h"
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
static void addEntriesToMenu (const UIEditing::MenuEntry* entries, COptionMenu* menu, ICommandMenuItemTarget* menuItemTarget, int32_t& index)
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
				subMenu->setStyle (COptionMenu::kMultipleCheckStyle|COptionMenu::kCheckStyle);
			menu->addEntry (new CMenuItem (entries[index].name, subMenu));
			index++;
			addEntriesToMenu(entries, subMenu, menuItemTarget, index);
		}
		else
		{
			CMenuItem* item = menu->addEntry (new CCommandMenuItem ({entries[index].name, menuItemTarget, entries[index].category, entries[index].name}));
			if (entries[index].key)
			{
				item->setKey (entries[index].key, entries[index].modifier);
			}
			else if (entries[index].virtualKey != VirtualKey::None)
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
			snprintf (number, 10, "%d", count);
			name += ' ';
			name += number;
			return createUniqueTemplateName (names, name);
		}
	}
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIEditMenuController::validateCommandMenuItem (CCommandMenuItem* item)
{
	return validateMenuItem (*item);
}

//----------------------------------------------------------------------------------------------------
bool UIEditMenuController::onCommandMenuItemSelected (CCommandMenuItem* item)
{
	return handleCommand (item->getCommandCategory (), item->getCommandName ());
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditMenuController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
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
			auto submenu = makeOwned<COptionMenu> ();
			const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			auto viewAndDisplayNames = factory->collectRegisteredViewAndDisplayNames ("CViewContainer");
			viewAndDisplayNames.sort ([] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
			for (auto& entry : viewAndDisplayNames)
			{
				submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc{entry.second.data (), this, "AddTemplate", entry.first->data ()}));
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
					submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc{name->data (), this, "RemoveTemplate", name->data ()}));
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
					submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc{name->data (), this, "DuplicateTemplate", name->data ()}));
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
			const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			auto viewAndDisplayNames = factory->collectRegisteredViewAndDisplayNames ("CViewContainer");
			viewAndDisplayNames.sort ([] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
			for (auto& entry : viewAndDisplayNames)
			{
				submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc{entry.second.data (), this, "Embed", entry.first->data ()}));
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
			auto numViewContainers = 0u;
			auto numNonViewContainers = 0u;
			for (auto& entry : *selection)
			{
				if (entry->asViewContainer ())
					++numViewContainers;
				else
					++numNonViewContainers;
			}
			if (numViewContainers != 0 && numNonViewContainers != 0)
			{
				item.setEnabled (false);
				return true;
			}

			auto submenu = makeOwned<COptionMenu> ();
			item.setSubmenu (submenu);
			const UIViewFactory* factory =
			    dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			auto viewAndDisplayNames = factory->collectRegisteredViewAndDisplayNames ();
			viewAndDisplayNames.sort (
			    [] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
			for (auto& entry : viewAndDisplayNames)
			{
				submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc {
				    entry.second.data (), this, "Transform View Type", entry.first->data ()}));
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
					submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc{name->data (), this, "InsertTemplate", name->data ()}));
				}
			}
			return true;
		}
	}
	else if (cmdCategory == "Selection")
	{
		if (cmdName == "Select Children Of Type")
		{
			auto submenu = makeOwned<COptionMenu> ();
			item.setSubmenu (submenu);
			const UIViewFactory* factory =
			    dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			auto viewAndDisplayNames = factory->collectRegisteredViewAndDisplayNames ();
			viewAndDisplayNames.sort (
			    [] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
			for (auto& entry : viewAndDisplayNames)
			{
				submenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc {
				    entry.second.data (), this, "Select Children Of Type", entry.first->data ()}));
			}
			return true;
		}
	}
	if (auto obj = dynamic_cast<ICommandMenuItemTarget*> (controller))
		return obj->validateCommandMenuItem (&item);
	return false;
}

//----------------------------------------------------------------------------------------------------
CCommandMenuItem* UIEditMenuController::findKeyCommandItem (COptionMenu* menu, const KeyboardEvent& event)
{
	for (auto& item : *menu->getItems ())
	{
		COptionMenu* subMenu = item->getSubmenu ();
		if (subMenu)
		{
			CCommandMenuItem* result = findKeyCommandItem (subMenu, event);
			if (result)
				return result;
		}
		CCommandMenuItem* result = item.cast<CCommandMenuItem>();
		if (result)
		{
			int32_t modifier = 0;
			if (event.modifiers.has (ModifierKey::Shift))
				modifier |= kShift;
			if (event.modifiers.has (ModifierKey::Alt))
				modifier |= kAlt;
			if (event.modifiers.has (ModifierKey::Control))
				modifier |= kControl;
			if (event.modifiers.has (ModifierKey::Super))
				modifier |= kApple;
			if (result->getKeyModifiers () == modifier)
			{
				if (event.virt != VirtualKey::None && event.virt == result->getVirtualKey ())
				{
					return result;
				}
				else if (!result->getKeycode ().empty () &&
						 static_cast<char32_t> (result->getKeycode ().getString ()[0]) ==
							 event.character)
				{
					return result;
				}
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
		undoManager->startGroupAction ("Transform View Type");
		for (auto& entry : *selection)
		{
			IAction* action = new TransformViewTypeOperation (
			    selection, entry, cmdName, description,
			    dynamic_cast<const UIViewFactory*> (description->getViewFactory ()));
			undoManager->pushAndPerform (action);
		}
		undoManager->endGroupAction ();
		return true;
	}
	else if (cmdCategory == "Select Children Of Type")
	{
		auto viewFactory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
		if (!viewFactory)
			return false;
		std::vector<CView*> newSelection;
		for (auto& entry : *selection)
		{
			if (auto viewContainer = entry->asViewContainer ())
				getChildrenOfType (viewContainer, cmdName, newSelection);
		}
		selection->clear ();
		for (auto& view : newSelection)
			selection->add (view);
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
	if (auto obj = dynamic_cast<ICommandMenuItemTarget*> (controller))
	{
		CCommandMenuItem item (CCommandMenuItem::Desc{"", 0, nullptr, category, name});
		if (obj->onCommandMenuItemSelected (&item))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool UIEditMenuController::canHandleCommand (const UTF8StringPtr category, const UTF8StringPtr name) const
{
	CCommandMenuItem item (CCommandMenuItem::Desc{"", 0, nullptr, category, name});
	if (const_cast<UIEditMenuController*> (this)->validateMenuItem (item))
	{
		return item.isEnabled ();
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::processKeyCommand (KeyboardEvent& event)
{
	COptionMenu* baseMenu = editMenu;
	CCommandMenuItem* item = baseMenu ? findKeyCommandItem (baseMenu, event) : nullptr;
	if (item == nullptr && fileMenu)
	{
		baseMenu = fileMenu;
		item = findKeyCommandItem (baseMenu, event);
	}
	if (item && item->getItemTarget ())
	{
		item->getItemTarget ()->validateCommandMenuItem (item);
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
			item->getItemTarget ()->onCommandMenuItemSelected (item);
			if (label)
			{
				highlightTimer = makeOwned<CVSTGUITimer> (this, 90u, true);
			}
			event.consumed = true;
		}
	}
}

//----------------------------------------------------------------------------------------------------
static void copyMenuItems (COptionMenu* src, COptionMenu* dst)
{
	auto srcItems = src->getItems ();
	if (!srcItems)
		return;
	for (auto& item : *srcItems)
	{
		item->remember ();
		dst->addEntry (item);
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIEditMenuController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription*)
{
	COptionMenu* menu = dynamic_cast<COptionMenu*>(view);
	if (menu)
	{
		switch (menu->getTag ())
		{
			case kMenuEditTag:
			{
				if (editMenu)
					copyMenuItems (editMenu, menu);
				else
					createEditMenu (menu);
				editMenu = menu;
				break;
			}
			case kMenuFileTag:
			{
				if (fileMenu)
					copyMenuItems (fileMenu, menu);
				else
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
			fileMenu->cleanupSeparators (true);
			label = fileLabel;
			break;
		}
		case kMenuEditTag:
		{
			editMenu->cleanupSeparators (true);
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

//------------------------------------------------------------------------
void UIEditMenuController::getChildrenOfType (CViewContainer* container, UTF8StringView type, std::vector<CView*>& result) const
{
	auto viewFactory = static_cast<const UIViewFactory*> (description->getViewFactory ());
	container->forEachChild ([&] (auto view) {
		if (type == viewFactory->getViewName (view))
			result.emplace_back (view);
		if (auto c = view->asViewContainer ())
		{
			getChildrenOfType (c, type, result);
		}
	});
}

//------------------------------------------------------------------------
} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
