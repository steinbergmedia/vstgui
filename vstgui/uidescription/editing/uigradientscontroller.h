// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uigradientscontroller__
#define __uigradientscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "../../lib/cdatabrowser.h"

namespace VSTGUI {

class UIGradientsDataSource;
class IActionPerformer;

//----------------------------------------------------------------------------------------------------
class UIGradientsController : public NonAtomicReferenceCounted, public DelegationController, public IGenericStringListDataBrowserSourceSelectionChanged
{
public:
	UIGradientsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIGradientsController () override;

protected:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl* pControl) override;
	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;

	void showEditDialog ();

	SharedPointer<UIDescription> editDescription;
	SharedPointer<CControl> editButton;
	IActionPerformer* actionPerformer;
	UIGradientsDataSource* dataSource;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
		kEditTag
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uigradientscontroller__
