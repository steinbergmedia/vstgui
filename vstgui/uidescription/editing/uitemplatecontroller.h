// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uitemplatecontroller__
#define __uitemplatecontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../delegationcontroller.h"
#include "../../lib/cdatabrowser.h"
#include <vector>
#include <list>

namespace VSTGUI {
class UIViewListDataSource;

//----------------------------------------------------------------------------------------------------
class UITemplateController : public CBaseObject, public DelegationController, public IContextMenuController2, public IGenericStringListDataBrowserSourceSelectionChanged, public IDependency
{
public:
	UITemplateController (IController* baseController, UIDescription* description, UISelection* selection, UIUndoManager* undoManager, IActionPerformer* actionPerformer);
	~UITemplateController () override;

	const UTF8String* getSelectedTemplateName () const { return selectedTemplateName; }

	void selectTemplate (UTF8StringPtr name);
	void setTemplateView (CViewContainer* view);
	
	static void setupDataBrowser (CDataBrowser* orignalBrowser, CDataBrowser* dataBrowser);

	static IdStringPtr kMsgTemplateChanged;
	static IdStringPtr kMsgTemplateNameChanged;
protected:
	void valueChanged (CControl* pControl) override {}
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override;

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;

	void appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where) override;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	SharedPointer<UIDescription> editDescription;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	IActionPerformer* actionPerformer;
	CViewContainer* templateView;
	CDataBrowser* templateDataBrowser;
	UIViewListDataSource* mainViewDataSource;
	GenericStringListDataBrowserSource::StringVector templateNames;
	const UTF8String* selectedTemplateName;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uitemplatecontroller__
