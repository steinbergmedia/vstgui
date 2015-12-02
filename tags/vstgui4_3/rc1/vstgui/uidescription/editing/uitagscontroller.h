#ifndef __uitagscontroller__
#define __uitagscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../delegationcontroller.h"

namespace VSTGUI {
class UITagsDataSource;

//----------------------------------------------------------------------------------------------------
class UITagsController : public CBaseObject, public DelegationController
{
public:
	UITagsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UITagsController ();

protected:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UITagsDataSource* dataSource;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uitagscontroller__
