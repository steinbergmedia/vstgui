// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"

namespace VSTGUI {
class UIColorsDataSource;
class UIColor;

//----------------------------------------------------------------------------------------------------
class UIColorsController : public NonAtomicReferenceCounted,
						   public DelegationController,
						   public IContextMenuController2
{
public:
	UIColorsController (const SPtr<IController>& baseController,
						const SPtr<UIDescription>& description,
						WeakPointer<IActionPerformer> actionPerformer);
	~UIColorsController () override;

protected:
	SPtr<CView> createView (const UIAttributes& attributes,
							const IUIDescription& description) override;
	SPtr<CView> verifyView (const SPtr<CView>& view, const UIAttributes& attributes,
							const IUIDescription& description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl& pControl) override;
	SPtr<IController> createSubController (IdStringPtr name,
										   const IUIDescription& description) override;

	void appendContextMenuItems (COptionMenu& contextMenu, CView& view,
								 const CPoint& where) override;

	SPtr<UIDescription> editDescription;
	WeakPointer<IActionPerformer> actionPerformer;
	SPtr<UIColorsDataSource> dataSource;
	SPtr<UIColor> color;

	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag
	};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
