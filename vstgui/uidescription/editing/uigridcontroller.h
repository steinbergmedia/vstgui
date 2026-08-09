// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uigrid.h"
#include "uidialogcontroller.h"
#include "../delegationcontroller.h"
#include "../../lib/controls/ctextedit.h"
#include <vector>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIGridController : public UIGrid, public DelegationController, public IDialogController
{
public:
	UIGridController (const SPtr<IController>& baseController,
					  const SPtr<UIDescription>& description);
	~UIGridController () override;
	
protected:
	void valueChanged (CControl& pControl) override;
	SPtr<CView> verifyView (const SPtr<CView>& view, const UIAttributes& attributes,
							const IUIDescription& description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void setSize (const CPoint& p) override;

	void onDialogButton1Clicked (UIDialogController&) override;
	void onDialogButton2Clicked (UIDialogController&) override;
	void onDialogShow (UIDialogController&) override;

	void syncMenuValueAndSize ();
	void loadDefGrids ();
	void saveDefGrids ();
	UTF8String pointToDisplayString (const CPoint& p) const;
	void setupTextEdit (const SPtr<CTextEdit>& te) const;
	void setupMenu ();

	SPtr<UIDescription> editDescription;
	SPtr<COptionMenu> gridMenu;
	SPtr<CListControl> gridList;
	SPtr<CTextEdit> gridXEdit;
	SPtr<CTextEdit> gridYEdit;

	std::vector<CPoint> defGrids;

	enum {
		kGridMenuTag,
		kGridListTag,
		kGridAddTag,
		kGridRemoveTag,
		kGridXTag,
		kGridYTag
	};
	
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
