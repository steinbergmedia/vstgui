// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "../delegationcontroller.h"
#include "../../lib/controls/icommandmenuitemtarget.h"

namespace VSTGUI {
class IActionPerformer;

namespace UIEditing {

//----------------------------------------------------------------------------------------------------
struct MenuEntry {
	UTF8StringPtr category {nullptr};
	UTF8StringPtr name {nullptr};
	UTF8StringPtr key {nullptr};
	int32_t modifier {0};
	int32_t virtualKey {0};
	int32_t menuFlags {0};
	
	enum {
		kSubMenu			= (1 << 0),
		kSubMenuEnd			= (1 << 1),
		kSubMenuCheckStyle	= (1 << 2),
		kMenuItemIsTitle	= (1 << 3)
	};

#if defined(_MSC_VER) && _MSC_VER < 1910 // Can be removed when dropping VS 2015 Support
	MenuEntry (UTF8StringPtr s1 = nullptr, UTF8StringPtr s2 = nullptr, UTF8StringPtr s3 = nullptr,
	           int32_t i1 = 0, int32_t i2 = 0, int32_t i3 = 0)
	: category (s1), name (s2), key (s3), modifier (i1), virtualKey (i2), menuFlags (i3)
	{
	}
#endif
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
	{ "Selection", "Select Children Of Type" , 0, 0, 0 },
	kMenuSeparator,
	{ "Selection", "Select Parent(s)" , 0, 0, 0 },
	{ "Selection", "Select All Children" , 0, 0, 0 },
	kMenuSeparator,
	{ "Selection", "Select View in Hierarchy Browser" , 0, 0, 0 },

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

} // UIEditing

//----------------------------------------------------------------------------------------------------
class UIEditMenuController : public CBaseObject, public DelegationController, public CommandMenuItemTargetAdapter
{
public:
	UIEditMenuController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description, IActionPerformer* actionPerformer);
	~UIEditMenuController () noexcept override = default;

	COptionMenu* getFileMenu () const { return fileMenu; }
	COptionMenu* getEditMenu () const { return editMenu; }

	int32_t processKeyCommand (const VstKeyCode& key);
	bool handleCommand (const UTF8StringPtr category, const UTF8StringPtr name);
	bool canHandleCommand (const UTF8StringPtr category, const UTF8StringPtr name) const;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	void valueChanged (CControl* pControl) override;

	static bool createUniqueTemplateName (std::list<const std::string*>& names, std::string& name);
protected:
	bool validateCommandMenuItem (CCommandMenuItem* item) override;
	bool onCommandMenuItemSelected (CCommandMenuItem* item) override;

	bool validateMenuItem (CCommandMenuItem& item);
	CCommandMenuItem* findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key);
	void createEditMenu (COptionMenu* menu);
	void createFileMenu (COptionMenu* menu);

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override { return this; }
	void controlBeginEdit (CControl* pControl) override;
	void controlEndEdit (CControl* pControl) override;

	void getChildrenOfType (CViewContainer* container, UTF8StringView type, std::vector<CView*>& result) const;
	
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIDescription> description;
	SharedPointer<CVSTGUITimer> highlightTimer;
	IActionPerformer* actionPerformer;

	COptionMenu* fileMenu {nullptr};
	COptionMenu* editMenu {nullptr};
	SharedPointer<CTextLabel> fileLabel;
	SharedPointer<CTextLabel> editLabel;
	
	enum {
		kMenuFileTag = 100,
		kMenuEditTag = 101
	};

};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
