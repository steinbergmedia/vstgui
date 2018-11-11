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
class UIColorsController : public NonAtomicReferenceCounted, public DelegationController
{
public:
	UIColorsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIColorsController () override;

protected:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl* pControl) override;
	IController* createSubController (IdStringPtr name, const IUIDescription* description) override;

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UIColorsDataSource* dataSource;
	SharedPointer<UIColor> color;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag
	};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
