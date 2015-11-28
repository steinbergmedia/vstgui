//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#include "uiundomanager.h"

#if VSTGUI_LIVE_EDITING

#include "iaction.h"
#include <string>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class UndoStackTop : public IAction
{
public:
	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD { return 0; }
	void perform () VSTGUI_OVERRIDE_VMETHOD {}
	void undo () VSTGUI_OVERRIDE_VMETHOD {}
};

//----------------------------------------------------------------------------------------------------
class UIGroupAction : public IAction, public std::list<IAction*>
{
public:
	static void doPerform (IAction* action) { action->perform (); }
	static void doUndo (IAction* action) { action->undo (); }
	static void doDelete (IAction* action) { delete action; }

	UIGroupAction (UTF8StringPtr name) : name (name) {}
	~UIGroupAction ()
	{
		std::for_each (begin (), end (), doDelete);
	}

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD { return name.c_str (); }

	void perform () VSTGUI_OVERRIDE_VMETHOD
	{
		std::for_each (begin (), end (), doPerform);
	}
	
	void undo () VSTGUI_OVERRIDE_VMETHOD
	{
		std::for_each (rbegin (), rend (), doUndo);
	}

protected:
	std::string name;
};

//----------------------------------------------------------------------------------------------------
IdStringPtr UIUndoManager::kMsgChanged = "UIUndoManagerChanged";

static void deleteUndoManagerAction (IAction* action) { delete action; }

//----------------------------------------------------------------------------------------------------
UIUndoManager::UIUndoManager ()
{
	push_back (new UndoStackTop);
	position = end ();
	savePosition = begin ();
}

//----------------------------------------------------------------------------------------------------
UIUndoManager::~UIUndoManager ()
{
	std::for_each (begin (), end (), deleteUndoManagerAction);
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::pushAndPerform (IAction* action)
{
	if (groupQueue.empty () == false)
	{
		groupQueue.back ()->push_back (action);
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
			delete (*position);
			position++;
		}
		erase (oldStack, end ());
	}
	push_back (action);
	position = end ();
	position--;
	action->perform ();
	changed (kMsgChanged);
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::performUndo ()
{
	if (position != end () && position != begin ())
	{
		(*position)->undo ();
		position--;
		changed (kMsgChanged);
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
			changed (kMsgChanged);
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
	return 0;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UIUndoManager::getRedoName ()
{
	UTF8StringPtr redoName = 0;
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
	std::for_each (begin (), end (), deleteUndoManagerAction);
	std::list<IAction*>::clear ();
	push_back (new UndoStackTop);
	position = end ();
	savePosition = begin ();
	changed (kMsgChanged);
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::startGroupAction (UTF8StringPtr name)
{
	UIGroupAction* action = new UIGroupAction (name);
	groupQueue.push_back (action);
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::endGroupAction ()
{
	UIGroupAction* action = groupQueue.back ();
	if (action)
	{
		groupQueue.pop_back ();
		if (action->empty ())
		{
			delete action;
		}
		else
		{
			pushAndPerform (action);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIUndoManager::cancelGroupAction ()
{
	UIGroupAction* action = groupQueue.back ();
	if (action)
	{
		groupQueue.pop_back ();
		delete action;
	}
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
	

} // namespace

#endif // VSTGUI_LIVE_EDITING
