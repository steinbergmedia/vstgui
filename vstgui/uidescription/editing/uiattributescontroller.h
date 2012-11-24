#ifndef __uiattributescontroller__
#define __uiattributescontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/cvstguitimer.h"

namespace VSTGUI {
class CRowColumnView;

namespace UIAttributeControllers {
class Controller;
}

//----------------------------------------------------------------------------------------------------
class UIAttributesController : public CBaseObject, public DelegationController
{
public:
	UIAttributesController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description);
	~UIAttributesController ();
	
	void performAttributeChange (const std::string& name, const std::string& value);
protected:
	CView* createViewForAttribute (const std::string& attrName);
	void rebuildAttributesView ();
	void validateAttributeViews ();

	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IController* createSubController (IdStringPtr name, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIDescription> editDescription;
	OwningPointer<CVSTGUITimer> timer;

	std::list<UIAttributeControllers::Controller*> attributeControllers;

	enum {
		kSearchFieldTag = 100,
		kViewNameTag = 101
	};

	SharedPointer<CTextEdit> searchField;
	CTextLabel* viewNameLabel;
	CRowColumnView* attributeView;

	std::string filterString;

	const std::string* currentAttributeName;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiattributescontroller__
