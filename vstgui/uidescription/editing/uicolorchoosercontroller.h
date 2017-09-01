// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uicolorchoosercontroller__
#define __uicolorchoosercontroller__

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "../../lib/controls/ctextedit.h"

namespace VSTGUI {
class UIColor;

//----------------------------------------------------------------------------------------------------
class UIColorChooserController : public CBaseObject, public DelegationController
{
public:
	UIColorChooserController (IController* baseController, UIColor* color);
	~UIColorChooserController () override;
	
protected:
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;
	void valueChanged (CControl* pControl) override;
	void controlBeginEdit (CControl* pControl) override;
	void controlEndEdit (CControl* pControl) override;

	void updateColorSlider (CControl* control);
	void updateColorSliders ();

	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);

	SharedPointer<UIColor> color;
	using ControlList = std::list<SharedPointer<CControl>>;
	ControlList controls;

	enum {
		kHueTag = 0,
		kSaturationTag,
		kLightnessTag,
		kRedTag,
		kGreenTag,
		kBlueTag,
		kAlphaTag,
		kNumTags
	};
};
	
} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uicolorchoosercontroller__
