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

#ifndef __uiundomanager__
#define __uiundomanager__

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/idependency.h"
#include <list>
#include <deque>

namespace VSTGUI {
class IAction;
class UIGroupAction;

//----------------------------------------------------------------------------------------------------
class UIUndoManager : public CBaseObject, protected std::list<IAction*>, public IDependency
{
public:
	UIUndoManager ();
	~UIUndoManager ();

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
	
	static IdStringPtr kMsgChanged;
protected:
	iterator position;
	iterator savePosition;
	typedef std::deque<UIGroupAction*> GroupActionDeque;
	GroupActionDeque groupQueue;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiundomanager__
