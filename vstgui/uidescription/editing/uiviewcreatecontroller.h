// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "../iviewfactory.h"
#include "uiselection.h"
#include <string>
#include <vector>

namespace VSTGUI {
class UIViewCreatorDataSource;

//----------------------------------------------------------------------------------------------------
class UIViewCreatorController : public NonAtomicReferenceCounted, public DelegationController, public IContextMenuController
{
public:
	UIViewCreatorController (const SPtr<IController>& baseController,
							 const SPtr<UIDescription>& description);
	~UIViewCreatorController () override;

	SPtr<IController> getBaseController () const { return controller; }

protected:
	void valueChanged (CControl& pControl) override;
	SPtr<CView> createView (const UIAttributes& attributes,
							const IUIDescription& description) override;
	SPtr<CView> verifyView (const SPtr<CView>& view, const UIAttributes& attributes,
							const IUIDescription& description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void appendContextMenuItems (COptionMenu& contextMenu, const CPoint& where) override;

	void setupDataSource (UTF8StringPtr filter = nullptr);

	SPtr<UIViewCreatorDataSource> dataSource;
	SPtr<CDataBrowser> dataBrowser;
	SPtr<UIDescription> description;
	std::vector<std::string> filteredViewNames;
	std::vector<std::string> allViewNames;
	
	enum {
		kSearchFieldTag = 100
	};

};

//----------------------------------------------------------------------------------------------------
SPtr<UISelection> createSelectionFromViewName (const std::string& viewName,
											   const IViewFactory& factory,
											   const UIDescription& description,
											   const SPtr<UIAttributes>& optionalAttributes);

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
