#ifndef __uitemplatecontroller__
#define __uitemplatecontroller__

#include "../uidescription.h"
#include "uiselection.h"
#include "uiundomanager.h"
#include <vector>
#include <list>

namespace VSTGUI {
class UIViewListDataSource;

//----------------------------------------------------------------------------------------------------
class UITemplateController : public CBaseObject, public DelegationController, public IGenericStringListDataBrowserSourceSelectionChanged, public IDependency
{
public:
	UITemplateController (IController* baseController, UIDescription* description, UISelection* selection, UIUndoManager* undoManager);
	~UITemplateController ();

	const std::string* getSelectedTemplateName () const { return selectedTemplateName; }

	void setTemplateView (CViewContainer* view);
	
	static void setupDataBrowser (CDataBrowser* orignalBrowser, CDataBrowser* dataBrowser);

	static IdStringPtr kMsgTemplateChanged;
protected:
	virtual void valueChanged (CControl* pControl) {}
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description);
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	virtual IController* createSubController (UTF8StringPtr name, IUIDescription* description);

	virtual void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source);

	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	SharedPointer<UIDescription> editDescription;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	CViewContainer* templateView;
	CDataBrowser* templateDataBrowser;
	UIViewListDataSource* mainViewDataSource;
	std::vector<std::string> templateNames;
	std::string* selectedTemplateName;
};

} // namespace

#endif // __uitemplatecontroller__
