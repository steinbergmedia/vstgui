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
#include "../../lib/cdatabrowser.h"
#include "../../lib/genericstringlistdatabrowsersource.h"

namespace VSTGUI {
class UIBitmapsDataSource;

//----------------------------------------------------------------------------------------------------
class UIBitmapsController : public NonAtomicReferenceCounted,
                            public DelegationController,
                            public GenericStringListDataBrowserSourceSelectionChanged
{
public:
	UIBitmapsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIBitmapsController () override;

protected:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl* pControl) override;

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;
	void dbRowDoubleClick (int32_t row, GenericStringListDataBrowserSource* source) override;

	void showSettingsDialog ();

	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UIBitmapsDataSource* dataSource;
	SharedPointer<CView> bitmapView;
	SharedPointer<CTextEdit> bitmapPathEdit;
	SharedPointer<CControl> settingButton;
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
		kBitmapPathTag,
		kSettingsTag
	};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
