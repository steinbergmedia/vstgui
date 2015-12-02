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

#ifndef __uieditmenucontroller__
#define __uieditmenucontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "../delegationcontroller.h"

namespace VSTGUI {
class IActionPerformer;

namespace UIEditing {

//----------------------------------------------------------------------------------------------------
struct MenuEntry {
	UTF8StringPtr category;
	UTF8StringPtr name;
	UTF8StringPtr key;
	int32_t modifier;
	int32_t virtualKey;
	int32_t menuFlags;
	
	enum {
		kSubMenu			= (1 << 0),
		kSubMenuEnd			= (1 << 1),
		kSubMenuCheckStyle	= (1 << 2),
		kMenuItemIsTitle	= (1 << 3)
	};
};

//----------------------------------------------------------------------------------------------------
static const MenuEntry kMenuSeparator = { "Separator", 0, 0, 0, 0 };
static const MenuEntry kSubMenuEnd = { 0, 0 , 0, 0, 0, MenuEntry::kSubMenuEnd };

//----------------------------------------------------------------------------------------------------
static const MenuEntry editMenu[] = {
	{ "Edit", "Undo" , "z", kControl },
	{ "Edit", "Redo" , "z", kControl|kShift },
	kMenuSeparator,
	{ "Edit", "Cut" , "x", kControl },
	{ "Edit", "Copy" , "c", kControl },
	{ "Edit", "Paste" , "v", kControl },
	{ "Edit", "Delete" , 0, kControl, VKEY_BACK },
	kMenuSeparator,
	{ "Edit", "Selection" , 0, 0, 0, MenuEntry::kSubMenu},
	
	{ "Selection", "Move" , 0, 0, 0, MenuEntry::kSubMenu},
	{ "", "By Grid" , 0, 0, 0, MenuEntry::kMenuItemIsTitle},
	kMenuSeparator,
	{ "SelectionMoveByGrid", "Move Up" , 0, kControl, VKEY_UP },
	{ "SelectionMoveByGrid", "Move Down" , 0, kControl, VKEY_DOWN },
	{ "SelectionMoveByGrid", "Move Left" , 0, kControl, VKEY_LEFT },
	{ "SelectionMoveByGrid", "Move Right" , 0, kControl, VKEY_RIGHT },
	kMenuSeparator,
	{ "", "By Pixel" , 0, 0, 0, MenuEntry::kMenuItemIsTitle},
	kMenuSeparator,
	{ "SelectionMoveByPixel", "Move Up" , 0, kControl|kShift, VKEY_UP },
	{ "SelectionMoveByPixel", "Move Down" , 0, kControl|kShift, VKEY_DOWN },
	{ "SelectionMoveByPixel", "Move Left" , 0, kControl|kShift, VKEY_LEFT },
	{ "SelectionMoveByPixel", "Move Right" , 0, kControl|kShift, VKEY_RIGHT },
	kSubMenuEnd,
	
	{ "Selection", "Size" , 0, 0, 0, MenuEntry::kSubMenu},
	{ "", "By Grid" , 0, 0, 0, MenuEntry::kMenuItemIsTitle},
	kMenuSeparator,
	{ "SelectionSizeByGrid", "Increase Size Width" , 0, kControl|kAlt, VKEY_RIGHT },
	{ "SelectionSizeByGrid", "Increase Size Height" , 0, kControl|kAlt, VKEY_DOWN },
	{ "SelectionSizeByGrid", "Decrease Size Width" , 0, kControl|kAlt, VKEY_LEFT },
	{ "SelectionSizeByGrid", "Decrease Size Height" , 0, kControl|kAlt, VKEY_UP },
	kMenuSeparator,
	{ "", "By Pixel" , 0, 0, 0, MenuEntry::kMenuItemIsTitle},
	kMenuSeparator,
	{ "SelectionSizeByPixel", "Increase Size Width" , 0, kControl|kAlt|kShift, VKEY_RIGHT },
	{ "SelectionSizeByPixel", "Increase Size Height" , 0, kControl|kAlt|kShift, VKEY_DOWN },
	{ "SelectionSizeByPixel", "Decrease Size Width" , 0, kControl|kAlt|kShift, VKEY_LEFT },
	{ "SelectionSizeByPixel", "Decrease Size Height" , 0, kControl|kAlt|kShift, VKEY_UP },
	kSubMenuEnd,

	{ "Selection", "Z-Order" , 0, 0, 0, MenuEntry::kSubMenu},
	{ "SelectionZOrder", "Lower" , 0, kAlt, VKEY_UP },
	{ "SelectionZOrder", "Raise" , 0, kAlt, VKEY_DOWN },
	kSubMenuEnd,
	kMenuSeparator,
	{ "Selection", "Select All Children" , 0, 0, 0 },
	
	kSubMenuEnd,
	kMenuSeparator,
	{ "Zoom", "Zoom" , 0, 0, 0, MenuEntry::kSubMenu},
	{ "Zoom", "Zoom In" , "=", kControl },
	{ "Zoom", "Zoom Out" , "-", kControl },
	kMenuSeparator,
	{ "Zoom", "Zoom 100%" , "0", kControl|kAlt },
	kSubMenuEnd,

	kMenuSeparator,
	{ "Edit", "Size To Fit" , 0, 0, 0 },
	{ "Edit", "Unembed Views" , 0, 0, 0 },
	{ "Edit", "Embed Into" , 0, 0, 0 },
	{ "Edit", "Transform View Type" , 0, 0, 0 },
	{ "Edit", "Insert Template" , 0, 0, 0 },
	kMenuSeparator,
	{ "Edit", "Add New Template" , 0, 0, 0 },
	{ "Edit", "Delete Template" , 0, 0, 0 },
	{ "Edit", "Duplicate Template" , 0, 0, 0 },
	kMenuSeparator,
	{ "Edit", "Template Settings..." , 0, kControl, VKEY_ENTER },
	{ "Edit", "Focus Drawing Settings..." , 0, 0, 0 },
	{0}
};

//----------------------------------------------------------------------------------------------------
static const MenuEntry fileMenu[] = {
	{ "File", "Save Options" , 0, 0, 0, MenuEntry::kSubMenu|MenuEntry::kSubMenuCheckStyle },
	{ "File", "Encode Bitmaps in XML" , 0, 0, 0 },
	{ "File", "Write Windows RC File on Save" , 0, 0, 0 },
	kSubMenuEnd,
	kMenuSeparator,
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

	static bool createUniqueTemplateName (std::list<const std::string*>& names, std::string& name);
protected:
	CCommandMenuItem* findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key);
	void createEditMenu (COptionMenu* menu);
	void createFileMenu (COptionMenu* menu);

	virtual CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD { return this; }
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
