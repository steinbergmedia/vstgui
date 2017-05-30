// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __vst3padcontroller__
#define __vst3padcontroller__

#include "../lib/controls/cxypad.h"
#include "../uidescription/delegationcontroller.h"
#include "../uidescription/uidescription.h"
#include "base/source/fobject.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class PadController : public Steinberg::FObject, public DelegationController
{
public:
	PadController (IController* baseController, Steinberg::Vst::EditController* editController, Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam);
	~PadController ();
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;

	void valueChanged (CControl* pControl) override;
	void controlBeginEdit (CControl* pControl) override;
	void controlEndEdit (CControl* pControl) override;

//-----------------------------------------------------------------------------
	OBJ_METHODS(PadController, FObject)
protected:
	void PLUGIN_API update (Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

	Steinberg::Vst::EditController* editController;
	Steinberg::Vst::Parameter* xParam;
	Steinberg::Vst::Parameter* yParam;
	CXYPad* padControl;
	SharedPointer<UIDescription> uiDescription;
};

} // namespace

#endif // __vst3padcontroller__
