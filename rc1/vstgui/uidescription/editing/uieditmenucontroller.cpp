//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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
, editMenu (0)
, fileMenu (0)
, editLabel (0)
, fileLabel (0)
{
}

//----------------------------------------------------------------------------------------------------
UIEditMenuController::~UIEditMenuController ()
{
	if (highlightTimer)
	{
		highlightTimer = 0;
	}
}

//----------------------------------------------------------------------------------------------------
static void addEntriesToMenu (const UIEditing::MenuEntry* entries, COptionMenu* menu, CBaseObject* menuItemTarget, int32_t& index)
{
	while (entries[index].category != 0)
	{
		if (entries[index].menuFlags & UIEditing::MenuEntry::kSubMenuEnd)
		{
			break;
		}
		if (entries[index].category == UIEditing::kMenuSeparator.category)
			menu->addSeparator ();
		else if (entries[index].menuFlags & UIEditing::MenuEntry::kSubMenu)
		{
			OwningPointer<COptionMenu> subMenu = new COptionMenu ();
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
	for (std::list<const std::string*>::const_iterator it = names.begin (); it != names.end (); ++it)
	{
		if (*(*it) == name)
		{
			int32_t count = 0;
			size_t pos = name.find_last_not_of ("0123456789");
			if (pos != std::string::npos && pos != name.length () - 1)
			{
				std::string numberStr = name.substr (pos);
				count = static_cast<int32_t> (strtol (numberStr.c_str (), NULL, 10)) + 1;
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
		UTF8StringView cmdCategory (item->getCommandCategory ());
		UTF8StringView cmdName (item->getCommandName ());
		item->setChecked (false);
		if (cmdCategory == "Edit")
		{
			if (cmdName == "Undo")
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
			else if (cmdName == "Redo")
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
			else if (cmdName == "Cut")
			{
				item->setEnabled (false);
			}
			else if (cmdName == "Copy")
			{
				item->setEnabled (false);
			}
			else if (cmdName == "Paste")
			{
				item->setEnabled (false);
			}
			else if (cmdName == "Delete")
			{
				CView* view = selection->first ();
				int32_t selectionCount = selection->total ();
				bool enable = view ? (selectionCount > 1 ? true : dynamic_cast<UIEditView*> (view->getParentView ()) == 0) : false;
				item->setEnabled (enable);
				return kMessageNotified;
			}
			else if (cmdName == "Add New Template")
			{
				std::list<const std::string*> containerViewNames;
				const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
				factory->collectRegisteredViewNames (containerViewNames, "CViewContainer");
				OwningPointer<COptionMenu> submenu = new COptionMenu ();
				for (std::list<const std::string*>::const_iterator it = containerViewNames.begin (); it != containerViewNames.end (); it++)
				{
					submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "AddTemplate", (*it)->c_str ()));
				}
				item->setSubmenu (submenu);
				return kMessageNotified;
			}
			else if (cmdName == "Delete Template")
			{
				item->setSubmenu (0);
				std::list<const std::string*> templateNames;
				description->collectTemplateViewNames (templateNames);
				item->setEnabled (templateNames.empty () == false);
				if (templateNames.empty () == false)
				{
					templateNames.sort (UIEditController::std__stringCompare);
					OwningPointer<COptionMenu> submenu = new COptionMenu ();
					item->setSubmenu (submenu);
					for (std::list<const std::string*>::const_iterator it = templateNames.begin (); it != templateNames.end (); it++)
					{
						submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "RemoveTemplate", (*it)->c_str ()));
					}
				}
				return kMessageNotified;
			}
			else if (cmdName == "Duplicate Template")
			{
				item->setSubmenu (0);
				std::list<const std::string*> templateNames;
				description->collectTemplateViewNames (templateNames);
				item->setEnabled (templateNames.empty () == false);
				if (templateNames.empty () == false)
				{
					templateNames.sort (UIEditController::std__stringCompare);
					OwningPointer<COptionMenu> submenu = new COptionMenu ();
					item->setSubmenu (submenu);
					for (std::list<const std::string*>::const_iterator it = templateNames.begin (); it != templateNames.end (); it++)
					{
						submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "DuplicateTemplate", (*it)->c_str ()));
					}
				}
				return kMessageNotified;
			}
			else if (cmdName == "Embed Into")
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
				const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
				factory->collectRegisteredViewNames (containerViewNames, "CViewContainer");
				for (std::list<const std::string*>::const_iterator it = containerViewNames.begin (); it != containerViewNames.end (); it++)
				{
					submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "Embed", (*it)->c_str ()));
				}
				return kMessageNotified;
			}
			else if (cmdName == "Unembed Views")
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
			else if (cmdName == "Size To Fit")
			{
				item->setEnabled (selection->total() > 0 ? true : false);
				return kMessageNotified;
			}
			else if (cmdName == "Transform View Type")
			{
				item->setSubmenu (0);
				bool enabled = false;
				if (selection->total () == 1)
				{
					CViewContainer* container = dynamic_cast<CViewContainer*>(selection->first());
					if (container == 0 || (container && dynamic_cast<UIEditView*>(container->getParentView ()) == 0))
						enabled = true;
				}
				item->setEnabled (enabled);
				if (enabled == false)
					return kMessageNotified;
				OwningPointer<COptionMenu> submenu = new COptionMenu ();
				item->setSubmenu (submenu);
				std::list<const std::string*> containerViewNames;
				const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
				factory->collectRegisteredViewNames (containerViewNames);
				for (std::list<const std::string*>::const_iterator it = containerViewNames.begin (); it != containerViewNames.end (); it++)
				{
					submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "Transform View Type", (*it)->c_str ()));
				}
				return kMessageNotified;
			}
			else if (cmdName == "Insert Template")
			{
				item->setSubmenu (0);
				item->setEnabled (selection->total () == 1 && dynamic_cast<CViewContainer*> (selection->first ()));
				if (item->isEnabled () == false)
					return kMessageNotified;
				std::list<const std::string*> templateNames;
				description->collectTemplateViewNames (templateNames);
				item->setEnabled (templateNames.empty () == false);
				if (templateNames.empty () == false)
				{
					templateNames.sort (UIEditController::std__stringCompare);
					OwningPointer<COptionMenu> submenu = new COptionMenu ();
					item->setSubmenu (submenu);
					for (std::list<const std::string*>::const_iterator it = templateNames.begin (); it != templateNames.end (); it++)
					{
						submenu->addEntry (new CCommandMenuItem ((*it)->c_str (), this, "InsertTemplate", (*it)->c_str ()));
					}
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
		UTF8StringView cmdCategory (item->getCommandCategory ());
		UTF8StringView cmdName (item->getCommandName ());
		if (cmdCategory == "Edit")
		{
			if (cmdName == "Undo")
			{
				if (undoManager->canUndo ())
				{
					undoManager->performUndo ();
				}
				return kMessageNotified;
			}
			else if (cmdName == "Redo")
			{
				if (undoManager->canRedo ())
				{
					undoManager->performRedo ();
				}
				return kMessageNotified;
			}
			else if (cmdName == "Delete")
			{
				IAction* action = new DeleteOperation (selection);
				undoManager->pushAndPerform (action);
				return kMessageNotified;
			}
			else if (cmdName == "Unembed Views")
			{
				IAction* action = new UnembedViewOperation (selection, description->getViewFactory ());
				undoManager->pushAndPerform (action);
				return kMessageNotified;
			}
			else if (cmdName == "Size To Fit")
			{
				IAction* action = new SizeToFitOperation (selection);
				undoManager->pushAndPerform (action);
				return kMessageNotified;
			}
		}
		else if (cmdCategory == "AddTemplate")
		{
			std::list<const std::string*> tmp;
			description->collectTemplateViewNames (tmp);
			std::string templateName (item->getCommandName ());
			if (createUniqueTemplateName (tmp, templateName))
			{
				actionPerformer->performCreateNewTemplate (templateName.c_str (), item->getCommandName ());
			}
			return kMessageNotified;
		}
		else if (cmdCategory == "RemoveTemplate")
		{
			actionPerformer->performDeleteTemplate (item->getCommandName ());
			return kMessageNotified;
		}
		else if (cmdCategory == "DuplicateTemplate")
		{
			std::list<const std::string*> tmp;
			description->collectTemplateViewNames (tmp);
			std::string templateName (item->getCommandName ());
			if (createUniqueTemplateName (tmp, templateName))
			{
				actionPerformer->performDuplicateTemplate (item->getCommandName (), templateName.c_str ());
			}
			return kMessageNotified;
		}
		else if (cmdCategory == "Embed")
		{
			const IViewFactory* viewFactory = description->getViewFactory ();
			UIAttributes viewAttr;
			viewAttr.setAttribute (UIViewCreator::kAttrClass, item->getCommandName ());
			CViewContainer* newContainer = dynamic_cast<CViewContainer*> (viewFactory->createView (viewAttr, description));
			if (newContainer)
			{
				IAction* action = new EmbedViewOperation (selection, newContainer);
				undoManager->pushAndPerform (action);	
			}
			return kMessageNotified;
		}
		else if (cmdCategory == "Transform View Type")
		{
			IAction* action = new TransformViewTypeOperation (selection, item->getCommandName (), description, dynamic_cast<const UIViewFactory*> (description->getViewFactory ()));
			undoManager->pushAndPerform (action);
			return kMessageNotified;
		}
		else if (cmdCategory == "InsertTemplate")
		{
			CViewContainer* parent = dynamic_cast<CViewContainer*> (selection->first ());
			if (parent)
			{
				CView* view = description->createView (item->getCommandName (), description->getController ());
				if (view)
				{
					undoManager->pushAndPerform (new InsertViewOperation (parent, view, selection));
				}
			}
			return kMessageNotified;
		}
		CBaseObject* obj = dynamic_cast<CBaseObject*>(controller);
		return obj ? obj->notify (sender, message) : kMessageNotified;
	}
	else if (message == CVSTGUITimer::kMsgTimer)
	{
		editLabel->setTransparency (true);
		fileLabel->setTransparency (true);
		highlightTimer = 0;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
CCommandMenuItem* UIEditMenuController::findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key)
{
	for (CConstMenuItemIterator it = menu->getItems ()->begin (), end = menu->getItems ()->end (); it != end; ++it)
	{
		COptionMenu* subMenu = (*it)->getSubmenu ();
		if (subMenu)
		{
			CCommandMenuItem* result = findKeyCommandItem (subMenu, key);
			if (result)
				return result;
		}
		CCommandMenuItem* result = (*it).cast<CCommandMenuItem>();
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
				else if (result->getKeycode () && result->getKeycode ()[0] == key.character)
					return result;
			}
		}
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditMenuController::processKeyCommand (const VstKeyCode& key)
{
	COptionMenu* baseMenu = editMenu;
	CCommandMenuItem* item = baseMenu ? findKeyCommandItem (baseMenu, key) : 0;
	if (item == 0 && fileMenu)
	{
		baseMenu = fileMenu;
		item = findKeyCommandItem (baseMenu, key);
	}
	if (item && item->getTarget ())
	{
		item->getTarget ()->notify (item, CCommandMenuItem::kMsgMenuItemValidate);
		if (item->isEnabled ())
		{
			CTextLabel* label = 0;
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
				highlightTimer = new CVSTGUITimer (this, 90, true);
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
	CTextLabel* label = 0;
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
	CTextLabel* label = 0;
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
