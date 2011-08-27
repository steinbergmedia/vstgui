#ifndef __uitagscontroller__
#define __uitagscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"

namespace VSTGUI {
class UITagsDataSource;

//----------------------------------------------------------------------------------------------------
class UITagsController : public CBaseObject, public DelegationController
{
public:
	UITagsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UITagsController ();

protected:
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name);
	void valueChanged (CControl* pControl);

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UITagsDataSource* dataSource;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uitagscontroller__
