#ifndef __uicolorscontroller__
#define __uicolorscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"

namespace VSTGUI {
class UIColorsDataSource;
class UIColor;

//----------------------------------------------------------------------------------------------------
class UIColorsController : public CBaseObject, public DelegationController
{
public:
	UIColorsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIColorsController ();

protected:
	CView* createView (const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	IController* createSubController (IdStringPtr name, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UIColorsDataSource* dataSource;
	OwningPointer<UIColor> color;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uicolorscontroller__
