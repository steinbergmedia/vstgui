// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uigridcontroller__
#define __uigridcontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uigrid.h"
#include "../delegationcontroller.h"
#include "../../lib/controls/ctextedit.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIGridController : public UIGrid, public DelegationController
{
public:
	UIGridController (IController* baseController, UIDescription* description);
	~UIGridController () override;

protected:
	void valueChanged (CControl* pControl) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;

	SharedPointer<UIDescription> editDescription;
	SharedPointer<CTextEdit> gridControls[2];

	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);

	enum {
		kGridXTag,
		kGridYTag
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uigridcontroller__
