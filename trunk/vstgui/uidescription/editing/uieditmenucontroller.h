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
		kSubMenuEnd	= 0xF + (1 << 1),
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
		kMenuEditTag = 101,
	};

};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditmenucontroller__
