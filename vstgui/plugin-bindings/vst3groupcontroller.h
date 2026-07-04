// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/controls/ccontrol.h"
#include "../uidescription/uidescriptionfwd.h"
#include "../uidescription/icontroller.h"
#include "pluginterfaces/base/iupdatehandler.h"
#include "pluginterfaces/base/funknownimpl.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GroupController : public IController,
						public NonAtomicReferenceCounted
{
public:
	GroupController (Steinberg::Vst::Parameter* parameter, Steinberg::Vst::EditController* editController);
	~GroupController ();

	SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
									 const UIAttributes& attributes,
									 const IUIDescription& description) override;

	void valueChanged (CControl& control) override;
	void controlBeginEdit (CControl& control) override;
	void controlEndEdit (CControl& control) override;

	//-----------------------------------------------------------------------------
protected:
	struct ParameterObserver
	: Steinberg::U::ImplementsNonDestroyable<Steinberg::U::Directly<Steinberg::IDependent>>
	{
		ParameterObserver (GroupController& controller) : controller (controller) {}

		void PLUGIN_API update (Steinberg::FUnknown* changedUnknown,
								Steinberg::int32 message) override;
		GroupController& controller;
	};

	ParameterObserver observer {*this};
	Steinberg::Vst::Parameter* parameter;
	Steinberg::Vst::EditController* editController;
	
	using ControlList = std::vector<CControl*>;
	ControlList controls;
};

} // namespace
