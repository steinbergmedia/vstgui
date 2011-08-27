#ifndef __uieditmenucontroller__
#define __uieditmenucontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"

namespace VSTGUI {
class COptionMenu;
class CCommandMenuItem;

namespace UIEditing {

//----------------------------------------------------------------------------------------------------
struct MenuEntry {
	UTF8StringPtr category;
	UTF8StringPtr name;
	UTF8StringPtr key;
	int32_t modifier;
};

//----------------------------------------------------------------------------------------------------
static const MenuEntry menuSeparator = { "Separator", 0, 0, 0 };

//----------------------------------------------------------------------------------------------------
static const MenuEntry editMenu[] = {
	{ "Edit", "Undo" , "z", kControl },
	{ "Edit", "Redo" , "z", kControl|kShift},
	menuSeparator,
	{ "Edit", "Cut" , "x", kControl },
	{ "Edit", "Copy" , "c", kControl },
	{ "Edit", "Paste" , "v", kControl },
	{ "Edit", "Delete" , "\b", kControl},
	menuSeparator,
	{ "Edit", "Size To Fit" , 0, 0 },
	{ "Edit", "Unembed Views" , 0, 0 },
	{ "Edit", "Embed Into" , 0, 0 },
	{ "Edit", "Transform View Type" , 0, 0 },
	menuSeparator,
	{ "Edit", "Add Template" , 0, 0},
	{ "Edit", "Remove Template" , 0, 0},
	{ "Edit", "Duplicate Template" , 0, 0},
	menuSeparator,
	{ "Edit", "Template Settings..." , 0, 0},
	{ "Edit", "Focus Drawing Settings..." , 0, 0},
	0
};

} // namespace UIEditing

//----------------------------------------------------------------------------------------------------
class UIEditMenuController : public CBaseObject, public DelegationController
{
public:
	UIEditMenuController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description);
	~UIEditMenuController ();

	COptionMenu* getFileMenu () const { return fileMenu; }
	COptionMenu* getEditMenu () const { return editMenu; }

	int32_t processKeyCommand (const VstKeyCode& key);

	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	virtual void valueChanged (CControl* pControl);
protected:
	CCommandMenuItem* findKeyCommandItem (COptionMenu* menu, const VstKeyCode& key);
	void createEditMenu (COptionMenu* menu);

	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name) { return this; }
	static bool createUniqueTemplateName (std::list<const std::string*>& names, std::string& name, int32_t count = 0);

	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIDescription> description;

	COptionMenu* fileMenu;
	COptionMenu* editMenu;
	
	enum {
		kMenuFileTag = 100,
		kMenuEditTag = 101,
	};

};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditmenucontroller__
