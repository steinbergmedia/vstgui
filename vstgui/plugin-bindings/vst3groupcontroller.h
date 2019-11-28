// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/controls/ccontrol.h"
#include "../uidescription/uidescriptionfwd.h"
#include "../uidescription/icontroller.h"
#include "base/source/fobject.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GroupController : public Steinberg::FObject, public IController
{
public:
	GroupController (Steinberg::Vst::Parameter* parameter, Steinberg::Vst::EditController* editController);
	~GroupController ();

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	
	void valueChanged (CControl* pControl) override;
	void controlBeginEdit (CControl* pControl) override;
	void controlEndEdit (CControl* pControl) override;

//-----------------------------------------------------------------------------
	OBJ_METHODS(GroupController, FObject)
protected:
	void PLUGIN_API update (Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

	Steinberg::Vst::Parameter* parameter;
	Steinberg::Vst::EditController* editController;
	
	using ControlList = std::vector<CControl*>;
	ControlList controls;
};

} // namespace
