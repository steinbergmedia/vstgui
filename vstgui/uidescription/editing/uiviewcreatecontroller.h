#ifndef __uiviewcreatorcontroller__
#define __uiviewcreatorcontroller__

#include "../uidescription.h"
#include <list>
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
	void valueChanged (CControl* pControl);
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name);

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

#endif // __uiviewcreatorcontroller__
