#ifndef __uitagscontroller__
#define __uitagscontroller__

#include "../uidescription.h"
#include "uiselection.h"
#include "uiundomanager.h"
#include "iactionoperation.h"

namespace VSTGUI {
class UITagsDataSource;

//----------------------------------------------------------------------------------------------------
class UITagsController : public CBaseObject, public DelegationController
{
public:
	UITagsController (IController* baseController, UIDescription* description, IActionOperator* actionOperator);
	~UITagsController ();

protected:
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name);
	void valueChanged (CControl* pControl);

	SharedPointer<UIDescription> editDescription;
	IActionOperator* actionOperator;
	UITagsDataSource* dataSource;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
	};
};

} // namespace

#endif // __uitagscontroller__
