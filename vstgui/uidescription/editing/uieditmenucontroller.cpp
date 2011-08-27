#include "uieditmenucontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uiactions.h"
#include "uieditview.h"
#include "../uiviewfactory.h"
#include "../../lib/controls/coptionmenu.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIEditMenuController::UIEditMenuController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description)
: DelegationController (baseController)
, selection (selection)
, undoManager (undoManager)
, description (description)
, editMenu (0)
, fileMenu (0)
{
}

//----------------------------------------------------------------------------------------------------
UIEditMenuController::~UIEditMenuController ()
{
}

//----------------------------------------------------------------------------------------------------
void UIEditMenuController::createEditMenu (COptionMenu* menu)
{
	int32_t index = 0;
	while (UIEditing::editMenu[index].category != 0)
	{
		if (UIEditing::editMenu[index].category == UIEditing::menuSeparator.category)
		{
			menu->addSeparator ();
		}
		else
		{
			CMenuItem* item = menu->addEntry (new CCommandMenuItem (UIEditing::editMenu[index].name, this, UIEditing::editMenu[index].category, UIEditing::editMenu[index].name));
			if (UIEditing::editMenu[index].key)
			{
				item->setKey (UIEditing::editMenu[index].key, UIEditing::editMenu[index].modifier);
			}
		}
		index++;
	}
}

//----------------------------------------------------------------------------------------------------
bool UIEditMenuController::createUniqueTemplateName (std::list<const std::string*>& names, std::string& name, int32_t count)
{
	std::string str (name);
	if (count != 0)
	{
		char number[10];
		sprintf(number, "%d", count);
		str += ' ';
		str += number;
	}
	for (std::list<const std::string*>::const_iterator it = names.begin (); it != names.end (); it++)
	{
		if (*(*it) == str)
		{
			return createUniqueTemplateName (names, name, count + 1);
		}
	}
	name = str;
	return true;
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditMenuController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CCommandMenuItem::kMsgMenuItemValidate)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*> (sender);
		if (strcmp (item->getCommandCategory (), "Edit") == 0)
		{
			if (strcmp (item->getCommandName (), "Undo") == 0)
			{
				if (undoManager->canUndo ())
				{
					std::string str ("Undo ");
					str += undoManager->getUndoName ();
					item->setTitle (str.c_str ());
					item->setEnabled (true);
				}
				else
				{
					item->setTitle ("Undo");
					item->setEnabled (false);
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Redo") == 0)
			{
				if (undoManager->canRedo ())
				{
					std::string str ("Redo ");
					str += undoManager->getRedoName ();
					item->setTitle (str.c_str ());
					item->setEnabled (true);
				}
				else
				{
					item->setTitle ("Redo");
					item->setEnabled (false);
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Cut") == 0)
			{
				// TODO: Implement
				item->setEnabled (false);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Copy") == 0)
			{
				// TODO: Implement
				item->setEnabled (false);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Paste") == 0)
			{
				// TODO: Implement
				item->setEnabled (false);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Delete") == 0)
			{
				CView* view = selection->first ();
				int32_t selectionCount = selection->total ();
				bool enable = view ? (selectionCount > 1 ? true : dynamic_cast<UIEditView*> (view->getParentView ()) == 0) : false;
				item->setEnabled (enable);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Add New Template") == 0)
			{
				std::list<const std::string*> containerViewNames;
				UIViewFactory* factory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
				factory->collectRegisteredViewNames (containerViewNames, "CViewContainer");
				OwningPointer<COptionMenu> submenu = new COptionMenu ();
				for (std::list<const std::string*>::const_iterator it = containerViewNames.begin (); it != containerViewNames.end (); it++)
				{
					submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "AddTemplate", (*it)->c_str ()));
				}
				item->setSubmenu (submenu);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Delete Template") == 0)
			{
				item->setSubmenu (0);
				std::list<const std::string*> templateNames;
				description->collectTemplateViewNames (templateNames);
				item->setEnabled (templateNames.empty () == false);
				if (templateNames.empty () == false)
				{
					OwningPointer<COptionMenu> submenu = new COptionMenu ();
					item->setSubmenu (submenu);
					for (std::list<const std::string*>::const_iterator it = templateNames.begin (); it != templateNames.end (); it++)
					{
						submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "RemoveTemplate", (*it)->c_str ()));
					}
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Duplicate Template") == 0)
			{
				item->setSubmenu (0);
				std::list<const std::string*> templateNames;
				description->collectTemplateViewNames (templateNames);
				item->setEnabled (templateNames.empty () == false);
				if (templateNames.empty () == false)
				{
					OwningPointer<COptionMenu> submenu = new COptionMenu ();
					item->setSubmenu (submenu);
					for (std::list<const std::string*>::const_iterator it = templateNames.begin (); it != templateNames.end (); it++)
					{
						submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "DuplicateTemplate", (*it)->c_str ()));
					}
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Embed Into") == 0)
			{
				item->setSubmenu (0);
				bool enable = selection->total () > 0;
				FOREACH_IN_SELECTION(selection, view)
					if (dynamic_cast<UIEditView*>(view->getParentView()) != 0)
					{
						enable = false;
						break;
					}
				FOREACH_IN_SELECTION_END
				item->setEnabled (enable);
				if (enable == false)
					return kMessageNotified;
				OwningPointer<COptionMenu> submenu = new COptionMenu ();
				item->setSubmenu (submenu);
				std::list<const std::string*> containerViewNames;
				UIViewFactory* factory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
				factory->collectRegisteredViewNames (containerViewNames, "CViewContainer");
				for (std::list<const std::string*>::const_iterator it = containerViewNames.begin (); it != containerViewNames.end (); it++)
				{
					submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "Embed", (*it)->c_str ()));
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Unembed Views") == 0)
			{
				bool enabled = false;
				if (selection->total () == 1)
				{
					CViewContainer* container = dynamic_cast<CViewContainer*>(selection->first());
					if (container && container->hasChildren () && dynamic_cast<UIEditView*>(container->getParentView ()) == 0)
						enabled = true;
				}
				item->setEnabled (enabled);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Size To Fit") == 0)
			{
				item->setEnabled (selection->total() > 0 ? true : false);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Transform View Type") == 0)
			{
				item->setSubmenu (0);
				item->setEnabled(selection->total () == 1);
				if (item->isEnabled () == false)
					return kMessageNotified;
				OwningPointer<COptionMenu> submenu = new COptionMenu ();
				item->setSubmenu (submenu);
				std::list<const std::string*> containerViewNames;
				UIViewFactory* factory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
				factory->collectRegisteredViewNames (containerViewNames);
				for (std::list<const std::string*>::const_iterator it = containerViewNames.begin (); it != containerViewNames.end (); it++)
				{
					submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "Transform View Type", (*it)->c_str ()));
				}
				return kMessageNotified;
			}
		}
		CBaseObject* obj = dynamic_cast<CBaseObject*>(controller);
		return obj ? obj->notify (sender, message) : kMessageNotified;
	}
	else if (message == CCommandMenuItem::kMsgMenuItemSelected)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*> (sender);
		if (strcmp (item->getCommandCategory (), "Edit") == 0)
		{
			if (strcmp (item->getCommandName (), "Undo") == 0)
			{
				if (undoManager->canUndo ())
				{
					undoManager->performUndo ();
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Redo") == 0)
			{
				if (undoManager->canRedo ())
				{
					undoManager->performRedo ();
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Delete") == 0)
			{
				IAction* action = new DeleteOperation (selection);
				undoManager->pushAndPerform (action);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Unembed Views") == 0)
			{
				IAction* action = new UnembedViewOperation (selection, dynamic_cast<UIViewFactory*> (description->getViewFactory ()));
				undoManager->pushAndPerform (action);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Size To Fit") == 0)
			{
				IAction* action = new SizeToFitOperation (selection);
				undoManager->pushAndPerform (action);
				return kMessageNotified;
			}
		}
		else if (strcmp (item->getCommandCategory (), "AddTemplate") == 0)
		{
			std::list<const std::string*> tmp;
			description->collectTemplateViewNames (tmp);
			std::string templateName (item->getCommandName ());
			if (createUniqueTemplateName (tmp, templateName))
			{
				UIAttributes* attr = new UIAttributes ();
				attr->setAttribute ("class", item->getCommandName ());
				attr->setAttribute ("size", "400,400");
				description->addNewTemplate (templateName.c_str (), attr);
			}
			return kMessageNotified;
		}
		else if (strcmp (item->getCommandCategory (), "RemoveTemplate") == 0)
		{
			description->removeTemplate (item->getCommandName ());
			return kMessageNotified;
		}
		else if (strcmp (item->getCommandCategory (), "DuplicateTemplate") == 0)
		{
			std::list<const std::string*> tmp;
			description->collectTemplateViewNames (tmp);
			std::string templateName (item->getCommandName ());
			if (createUniqueTemplateName (tmp, templateName))
			{
				description->changed (UIDescription::kMessageBeforeSave);
				description->duplicateTemplate (item->getCommandName (), templateName.c_str ());
			}
			return kMessageNotified;
		}
		else if (strcmp (item->getCommandCategory (), "Embed") == 0)
		{
			UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
			UIAttributes viewAttr;
			viewAttr.setAttribute ("class", item->getCommandName ());
			CViewContainer* newContainer = dynamic_cast<CViewContainer*> (viewFactory->createView (viewAttr, description));
			if (newContainer)
			{
				IAction* action = new EmbedViewOperation (selection, newContainer);
				undoManager->pushAndPerform (action);	
			}
			return kMessageNotified;
		}
		else if (strcmp (item->getCommandCategory (), "Transform View Type") == 0)
		{
			IAction* action = new TransformViewTypeOperation (selection, item->getCommandName (), description, dynamic_cast<UIViewFactory*> (description->getViewFactory ()));
			undoManager->pushAndPerform (action);
			return kMessageNotified;
		}
		CBaseObject* obj = dynamic_cast<CBaseObject*>(controller);
		return obj ? obj->notify (sender, message) : kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
CCommandMenuItem* UIEditMenuController::findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key)
{
	for (CConstMenuItemIterator it = menu->getItems ()->begin (); it != menu->getItems ()->end (); it++)
	{
		COptionMenu* subMenu = (*it)->getSubmenu ();
		if (subMenu)
		{
			CCommandMenuItem* result = findKeyCommandItem (subMenu, key);
			if (result)
				return result;
		}
		CCommandMenuItem* result = dynamic_cast<CCommandMenuItem*>(*it);
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
			if (result->getKeyModifiers () == modifier && result->getKeycode ())
			{
				if (key.virt)
				{
					if (key.virt == VKEY_BACK && result->getKeycode ()[0] == '\b')
						return result;
				}
				else if (result->getKeycode ()[0] == key.character)
					return result;
			}
		}
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditMenuController::processKeyCommand (const VstKeyCode& key)
{
	CCommandMenuItem* item = editMenu ? findKeyCommandItem (editMenu, key) : 0;
	if (item == 0 && fileMenu)
		item = findKeyCommandItem (fileMenu, key);
	if (item && item->getTarget ())
	{
		item->getTarget ()->notify (item, CCommandMenuItem::kMsgMenuItemValidate);
		if (item->isEnabled ())
		{
			item->getTarget ()->notify (item, CCommandMenuItem::kMsgMenuItemSelected);
			return 1;
		}
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------
CView* UIEditMenuController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
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
				fileMenu = menu;
				break;
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

} // namespace

#endif // VSTGUI_LIVE_EDITING
