// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/dispatchlist.h"
#include <list>
#include <deque>

namespace VSTGUI {
class IAction;
class UIGroupAction;
class UIUndoManager;

//----------------------------------------------------------------------------------------------------
struct IUIUndoManagerListener
{
	virtual ~IUIUndoManagerListener () noexcept = default;
	virtual void onUndoManagerChange () = 0;
};

//----------------------------------------------------------------------------------------------------
class UIUndoManager : public NonAtomicReferenceCounted,
                      protected ListenerProvider<UIUndoManager, IUIUndoManagerListener>,
                      protected std::list<IAction*>
{
public:
	UIUndoManager ();
	~UIUndoManager () override;

	void pushAndPerform (IAction* action);

	UTF8StringPtr getUndoName ();
	UTF8StringPtr getRedoName ();
	
	void performUndo ();
	void performRedo ();
	bool canUndo ();
	bool canRedo ();

	void startGroupAction (UTF8StringPtr name);
	void endGroupAction ();
	void cancelGroupAction ();

	void clear ();

	void markSavePosition ();
	bool isSavePosition () const;
	
	using ListenerProvider<UIUndoManager, IUIUndoManagerListener>::registerListener;
	using ListenerProvider<UIUndoManager, IUIUndoManagerListener>::unregisterListener;
protected:
	iterator position;
	iterator savePosition;
	using GroupActionDeque = std::deque<UIGroupAction*>;
	GroupActionDeque groupQueue;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
