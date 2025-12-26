// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiundomanager.h"

#if VSTGUI_LIVE_EDITING

#include "iaction.h"
#include <string>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class UndoStackTop : public IAction
{
public:
	UTF8StringPtr getName () override { return nullptr; }
	void perform () override {}
	void undo () override {}
};

//----------------------------------------------------------------------------------------------------
class UIGroupAction : public IAction,
					  public std::list<SharedPointer<IAction>>
{
public:
	UIGroupAction (UTF8StringPtr name) : name (name) {}
	~UIGroupAction () override = default;

	UTF8StringPtr getName () override { return name.c_str (); }

	void perform () override
	{
		std::for_each (begin (), end (), [] (auto& action) { action->perform (); });
	}
	
	void undo () override
	{
		std::for_each (rbegin (), rend (), [] (auto& action) { action->undo (); });
	}

protected:
	std::string name;
};

//----------------------------------------------------------------------------------------------------
UIUndoManager::UIUndoManager ()
{
	emplace_back (makeOwned<UndoStackTop> ());
	position = begin ();
	savePosition = begin ();
}

//----------------------------------------------------------------------------------------------------
UIUndoManager::~UIUndoManager () = default;

//----------------------------------------------------------------------------------------------------
void UIUndoManager::pushAndPerform (const SharedPointer<IAction>& action)
{
	if (groupQueue.empty () == false)
	{
		groupQueue.back ()->emplace_back (action);
		return;
	}
	if (position != end ())
	{
		position++;
		iterator oldStack = position;
		while (position != end ())
		{
			if (position == savePosition)
				savePosition = end ();
			position++;
		}
		erase (oldStack, end ());
	}
	emplace_back (action);
	position = end ();
	position--;
	action->perform ();
	forEachListener ([] (IUIUndoManagerListener* l) { l->onUndoManagerChange (); });
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::performUndo ()
{
	if (position != end () && position != begin ())
	{
		(*position)->undo ();
		position--;
		forEachListener ([] (IUIUndoManagerListener* l) { l->onUndoManagerChange (); });
	}
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::performRedo ()
{
	if (position != end ())
	{
		position++;
		if (position != end ())
		{
			(*position)->perform ();
			forEachListener ([] (IUIUndoManagerListener* l) { l->onUndoManagerChange (); });
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UIUndoManager::canUndo ()
{
	return (position != end () && position != begin ());
}

//----------------------------------------------------------------------------------------------------
bool UIUndoManager::canRedo ()
{
	if (position == end () && position != begin ())
		return false;
	position++;
	bool result = (position != end ());
	position--;
	return result;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UIUndoManager::getUndoName ()
{
	if (position != end () && position != begin ())
		return (*position)->getName ();
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UIUndoManager::getRedoName ()
{
	UTF8StringPtr redoName = nullptr;
	if (position != end ())
	{
		position++;
		if (position != end ())
			redoName = (*position)->getName ();
		position--;
	}
	return redoName;
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::clear ()
{
	std::list<SharedPointer<IAction>>::clear ();
	emplace_back (makeOwned<UndoStackTop> ());
	position = end ();
	savePosition = begin ();
	forEachListener ([] (IUIUndoManagerListener* l) { l->onUndoManagerChange (); });
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::startGroupAction (UTF8StringPtr name)
{
	groupQueue.emplace_back (makeOwned<UIGroupAction> (name));
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::endGroupAction ()
{
	auto action = groupQueue.back ();
	if (action)
	{
		groupQueue.pop_back ();
		if (!action->empty ())
		{
			pushAndPerform (action);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::cancelGroupAction ()
{
	auto action = groupQueue.back ();
	if (action)
		groupQueue.pop_back ();
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::markSavePosition ()
{
	savePosition = position;
}

//----------------------------------------------------------------------------------------------------
bool UIUndoManager::isSavePosition () const
{
	return savePosition == position;
}
	

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
