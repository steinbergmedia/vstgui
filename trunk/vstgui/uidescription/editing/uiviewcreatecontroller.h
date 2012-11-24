#ifndef __uiviewcreatorcontroller__
#define __uiviewcreatorcontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include <vector>
#include <string>

namespace VSTGUI {
class UIViewCreatorDataSource;

//----------------------------------------------------------------------------------------------------
class UIViewCreatorController : public CBaseObject, public DelegationController
{
public:
	UIViewCreatorController (IController* baseController, UIDescription* description);
	~UIViewCreatorController ();
protected:
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	CView* createView (const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;

	void setupDataSource (UTF8StringPtr filter = 0);
	
	UIViewCreatorDataSource* dataSource;
	SharedPointer<UIDescription> description;
	std::vector<std::string> filteredViewNames;
	std::vector<std::string> allViewNames;
	
	enum {
		kSearchFieldTag = 100,
	};

};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiviewcreatorcontroller__
