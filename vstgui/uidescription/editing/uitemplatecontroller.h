// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../delegationcontroller.h"
#include "../uidescriptionlistener.h"
#include "../../lib/cdatabrowser.h"
#include "../../lib/dispatchlist.h"
#include "../../lib/genericstringlistdatabrowsersource.h"
#include "../../lib/iviewlistener.h"
#include <vector>
#include <list>

namespace VSTGUI {
class UIViewListDataSource;

//----------------------------------------------------------------------------------------------------
class IUITemplateControllerListener
{
public:
	virtual ~IUITemplateControllerListener () noexcept = default;
	
	virtual void onTemplateSelectionChanged () = 0;
};

//----------------------------------------------------------------------------------------------------
class UITemplateController
: public NonAtomicReferenceCounted,
  public DelegationController,
  public IContextMenuController2,
  public GenericStringListDataBrowserSourceSelectionChanged,
  public UIDescriptionListenerAdapter,
  public ViewListenerAdapter,
  public ListenerProvider<UITemplateController, IUITemplateControllerListener>
{
public:
	UITemplateController (const SPtr<IController>& baseController,
						  const SPtr<UIDescription>& description,
						  const SPtr<UISelection>& selection,
						  const SPtr<UIUndoManager>& undoManager,
						  WeakPointer<IActionPerformer> actionPerformer);
	~UITemplateController () override;

	const UTF8String* getSelectedTemplateName () const { return selectedTemplateName; }

	void selectTemplate (UTF8StringPtr name);
	void setTemplateView (const SPtr<CViewContainer>& view);
	void navigateTo (const SPtr<CView>& view);

	static void setupDataBrowser (const SPtr<CDataBrowser>& orignalBrowser,
								  const SPtr<CDataBrowser>& dataBrowser);

protected:
	void onUIDescTemplateChanged (UIDescription& desc) override;
	void valueChanged (CControl& pControl) override {}
	SPtr<CView> createView (const UIAttributes& attributes,
							const IUIDescription& description) override;
	SPtr<CView> verifyView (const SPtr<CView>& view, const UIAttributes& attributes,
							const IUIDescription& description) override;
	SPtr<IController> createSubController (UTF8StringPtr name,
										   const IUIDescription& description) override;

	void dbSelectionChanged (int32_t selectedRow,
							 GenericStringListDataBrowserSource& source) override;

	void appendContextMenuItems (COptionMenu& contextMenu, CView& view,
								 const CPoint& where) override;
	void viewWillDelete (CView& view) override;

	SPtr<UIDescription> editDescription;
	SPtr<UISelection> selection;
	SPtr<UIUndoManager> undoManager;
	WeakPointer<IActionPerformer> actionPerformer;
	SPtr<CViewContainer> templateView;
	SPtr<CDataBrowser> templateDataBrowser;
	SPtr<UIViewListDataSource> mainViewDataSource;
	GenericStringListDataBrowserSource::StringVector templateNames;
	const UTF8String* selectedTemplateName;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
