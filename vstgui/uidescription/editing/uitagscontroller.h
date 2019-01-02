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

namespace VSTGUI {
class UITagsDataSource;

//----------------------------------------------------------------------------------------------------
class UITagsController : public NonAtomicReferenceCounted, public DelegationController
{
public:
	UITagsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UITagsController () override;

protected:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl* pControl) override;

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UITagsDataSource* dataSource;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag
	};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
