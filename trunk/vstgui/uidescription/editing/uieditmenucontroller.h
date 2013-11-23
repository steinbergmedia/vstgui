//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __uieditmenucontroller__
#define __uieditmenucontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/controls/coptionmenu.h"

namespace VSTGUI {
class CCommandMenuItem;
class IActionPerformer;
class CTextLabel;

namespace UIEditing {

//----------------------------------------------------------------------------------------------------
struct MenuEntry {
	UTF8StringPtr category;
	UTF8StringPtr name;
	UTF8StringPtr key;
	int32_t modifier;
	int32_t menuFlags;
	
	enum {
		kSubMenu	= 0xF + (1 << 0),
		kSubMenuEnd	= 0xF + (1 << 1)
	};
};

//----------------------------------------------------------------------------------------------------
static const MenuEntry menuSeparator = { "Separator", 0, 0, 0, 0 };

//----------------------------------------------------------------------------------------------------
static const MenuEntry editMenu[] = {
	{ "Edit", "Undo" , "z", kControl, 0 },
	{ "Edit", "Redo" , "z", kControl|kShift, 0 },
	menuSeparator,
	{ "Edit", "Cut" , "x", kControl, 0 },
	{ "Edit", "Copy" , "c", kControl, 0 },
	{ "Edit", "Paste" , "v", kControl, 0 },
	{ "Edit", "Delete" , "\b", kControl, 0},
	menuSeparator,
	{ "Edit", "Size To Fit" , 0, 0, 0 },
	{ "Edit", "Unembed Views" , 0, 0, 0 },
	{ "Edit", "Embed Into" , 0, 0, 0 },
	{ "Edit", "Transform View Type" , 0, 0, 0 },
	{ "Edit", "Insert Template" , 0, 0, 0 },
	menuSeparator,
	{ "Edit", "Add New Template" , 0, 0, 0 },
	{ "Edit", "Delete Template" , 0, 0, 0 },
	{ "Edit", "Duplicate Template" , 0, 0, 0 },
	menuSeparator,
	{ "Edit", "Template Settings..." , 0, 0, 0 },
	{ "Edit", "Focus Drawing Settings..." , 0, 0, 0 },
	{0}
};

//----------------------------------------------------------------------------------------------------
static const MenuEntry fileMenu[] = {
	{ "File", "Save Options" , 0, 0, MenuEntry::kSubMenu },
	{ "File", "Encode Bitmaps in XML" , 0, 0, 0 },
	{ "File", "Write Windows RC File on Save" , 0, 0, 0 },
	{ 0, 0 , 0, 0, MenuEntry::kSubMenuEnd },
	menuSeparator,
	{0}
};

} // namespace UIEditing

//----------------------------------------------------------------------------------------------------
class UIEditMenuController : public CBaseObject, public DelegationController
{
public:
	UIEditMenuController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description, IActionPerformer* actionPerformer);
	~UIEditMenuController ();

	COptionMenu* getFileMenu () const { return fileMenu; }
	COptionMenu* getEditMenu () const { return editMenu; }

	int32_t processKeyCommand (const VstKeyCode& key);

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
	virtual void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
protected:
	CCommandMenuItem* findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key);
	void createEditMenu (COptionMenu* menu);
	void createFileMenu (COptionMenu* menu);

	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD { return this; }
	static bool createUniqueTemplateName (std::list<const std::string*>& names, std::string& name, int32_t count = 0);
	void controlBeginEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	void controlEndEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIDescription> description;
	OwningPointer<CVSTGUITimer> highlightTimer;
	IActionPerformer* actionPerformer;

	COptionMenu* fileMenu;
	COptionMenu* editMenu;
	SharedPointer<CTextLabel> fileLabel;
	SharedPointer<CTextLabel> editLabel;
	
	enum {
		kMenuFileTag = 100,
		kMenuEditTag = 101
	};

};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditmenucontroller__
